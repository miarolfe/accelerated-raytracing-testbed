// Copyright Mia Rolfe. All rights reserved.
#include "Camera.h"

#include "../external/stb/stb_image_write.h"

#include "Colour.h"
#include "Common.h"
#include "Logger.h"
#include "Material.h"
#include "Random.h"
#include "Ray.h"
#include "RayHitResult.h"
#include "Utility.h"
#include "Vec3.h"

namespace ART
{

static constexpr std::size_t num_image_components = 3; // RGB

const CameraSetupParams default_camera_setup_params =
{
    600,                            // image_width
    600,                            // image_height
    Colour(0.529, 0.808, 0.922),    // background_colour
    45.0,                           // vertical_fov
    50,                             // samples_per_pixel
    25,                             // max_ray_bounces
    Point3(0.0),                    // look_from
    Point3(0.0, 0.0, 10.0),         // look_at
    Vec3(0.0, 1.0, 0.0),            // up
    0.0,                            // defocus_angle
    10.0                            // focus_distance
};

Camera::Camera() : Camera(default_camera_setup_params) {}

Camera::Camera(const CameraSetupParams& setup_params)
{
    m_image_width = setup_params.image_width;
    m_image_height = setup_params.image_height;
    m_background_colour = setup_params.background_colour;
    m_vertical_fov = setup_params.vertical_fov;
    m_samples_per_pixel = setup_params.samples_per_pixel;
    m_max_ray_bounces = setup_params.max_ray_bounces;
    m_look_from = setup_params.look_from;
    m_look_at = setup_params.look_at;
    m_up = setup_params.up;
    m_defocus_angle = setup_params.defocus_angle;
    m_focus_distance = setup_params.focus_distance;

    DeriveDependentVariables();
    ResizeImageBuffer();

    Logger::Get().LogInfo("Camera initialised at " + std::to_string(m_image_width) + "x" + std::to_string(m_image_height));
}

Camera::~Camera()
{
    if (m_image_data)
    {
        delete[] m_image_data;
    }
}

static const Interval intensity(0.0, 0.999);

void Camera::Render(const IRayHittable& scene)
{
    assert(m_image_width > 0);
    assert(m_image_height > 0);
    assert(m_vertical_fov > 0.0);
    assert(m_samples_per_pixel >= 1);
    assert(m_max_ray_bounces >= 1);
    assert(m_image_data != nullptr);

    #pragma omp parallel for schedule(static)
    for (std::int64_t j = 0; j < static_cast<std::int64_t>(m_image_height); j++)
    {
        std::size_t output_buffer_index =
            static_cast<std::size_t>(j) *
            m_image_width *
            num_image_components;

        for (std::size_t i = 0; i < m_image_width; i++)
        {
            Colour pixel_colour(0.0);

            for (std::size_t sample = 0; sample < m_samples_per_pixel; sample++)
            {
                const Ray& ray = GetRay(i, j);
                pixel_colour += RayColour(ray, m_max_ray_bounces, scene);
            }

            pixel_colour *= m_pixel_sample_scale;

            const double r_component = LinearToGamma(pixel_colour.m_x);
            const double g_component = LinearToGamma(pixel_colour.m_y);
            const double b_component = LinearToGamma(pixel_colour.m_z);

            m_image_data[output_buffer_index++] =
                static_cast<uint8_t>(256 * intensity.Clamp(r_component));
            m_image_data[output_buffer_index++] =
                static_cast<uint8_t>(256 * intensity.Clamp(g_component));
            m_image_data[output_buffer_index++] =
                static_cast<uint8_t>(256 * intensity.Clamp(b_component));
        }
    }

    const int8_t err_code = stbi_write_png
    (
        "render.png",
        m_image_width,
        m_image_height,
        num_image_components,
        m_image_data,
        m_image_width * sizeof(uint8_t) * num_image_components
    );
}


void Camera::DeriveDependentVariables()
{
    m_aspect_ratio = (static_cast<double>(m_image_width) / static_cast<double>(m_image_height));

    m_pixel_sample_scale = 1.0 / m_samples_per_pixel;

    m_centre = m_look_from;

    const double theta = DegreesToRadians(m_vertical_fov);
    const double h = std::tan(theta / 2.0);
    const double viewport_height = 2.0 * h * m_focus_distance;
    const double viewport_width = viewport_height * (static_cast<double>(m_image_width) / m_image_height);

    m_w = Normalised(m_look_from - m_look_at);
    m_u = Normalised(Cross(m_up, m_w));
    m_v = Cross(m_w, m_u);

    const Vec3 viewport_u = viewport_width * m_u;
    const Vec3 viewport_v = viewport_height * -m_v;

    m_pixel_delta_u = viewport_u / m_image_width;
    m_pixel_delta_v = viewport_v / m_image_height;

    const Point3 viewport_upper_left = m_centre - (m_focus_distance * m_w) - (viewport_u / 2.0) - (viewport_v / 2.0);
    m_pixel_0_0_location = viewport_upper_left + 0.5 * (m_pixel_delta_u + m_pixel_delta_v);

    const double defocus_radius = m_focus_distance * std::tan(DegreesToRadians(m_defocus_angle / 2.0));
    m_defocus_disk_u = m_u * defocus_radius;
    m_defocus_disk_v = m_v * defocus_radius;
}

void Camera::ResizeImageBuffer()
{
    if (m_image_data)
    {
        delete[] m_image_data;
    }

    m_image_data = new uint8_t[m_image_width * m_image_height * num_image_components];
}

Colour Camera::RayColour(const Ray& ray, std::size_t depth, const IRayHittable& scene)
{
    if (depth <= 0)
    {
        return Colour(0.0);
    }

    RayHitResult result;
    const double min_ray_t = 0.001;
    if (!scene.Hit(ray, Interval(min_ray_t, infinity), result))
    {
        return m_background_colour;
    }

    Ray scattered;
    Colour attenuation;
    const Colour colour_from_emission = result.m_material->Emitted(result.m_u, result.m_v, result.m_point);
    if (!result.m_material->Scatter(ray, result, attenuation, scattered))
    {
        return colour_from_emission;
    }

    Colour colour_from_scatter = attenuation * RayColour(scattered, depth - 1, scene);

    return colour_from_emission + colour_from_scatter;
}

Ray Camera::GetRay(std::size_t i, std::size_t j)
{
    const Vec3 offset = SampleSquare();
    const Vec3 pixel_sample = m_pixel_0_0_location + ((i + offset.m_x) * m_pixel_delta_u) + ((j + offset.m_y) * m_pixel_delta_v);

    const Point3 ray_origin = (m_defocus_angle <= 0) ? m_centre : DefocusDiskSample();
    const Vec3 ray_direction = pixel_sample - ray_origin;

    return Ray(ray_origin, ray_direction);
}

Vec3 Camera::SampleSquare() const
{
    return Vec3(RandomCanonicalDouble() - 0.5, RandomCanonicalDouble() - 0.5, 0.0);
}

Point3 Camera::DefocusDiskSample() const
{
    const Vec3 random_in_unit_disk = RandomInUnitDisk();
    return m_centre + (random_in_unit_disk[0] * m_defocus_disk_u) + (random_in_unit_disk[1] * m_defocus_disk_v);
}

} // namespace ART
