// Copyright Mia Rolfe. All rights reserved.
#include <RayTracing/Camera.h>

#include <omp.h>
#include <stb/stb_image_write.h>

#include <Core/Common.h>
#include <Core/Logger.h>
#include <Core/Random.h>
#include <Core/Utility.h>
#include <Materials/Material.h>
#include <Maths/Colour.h>
#include <Maths/Ray.h>
#include <Maths/Vec3.h>
#include <RayTracing/RayHitResult.h>

namespace ART
{

static constexpr std::size_t num_image_components = 3; // RGB

const CameraViewConfig default_view_config =
{
    Point3(0.0),
    Point3(0.0, 0.0, 10.0),
    Vec3(0.0, 1.0, 0.0),
    45.0,
    0.0,
    10.0
};

const CameraRenderConfig default_render_config =
{
    600,
    600,
    50,
    25
};

Camera::Camera() : Camera(default_view_config, default_render_config) {}

Camera::Camera(const CameraViewConfig& view_config, const CameraRenderConfig& render_config)
{
    m_look_from = view_config.look_from;
    m_look_at = view_config.look_at;
    m_up = view_config.up;
    m_vertical_fov = view_config.vertical_fov;
    m_defocus_angle = view_config.defocus_angle;
    m_focus_distance = view_config.focus_distance;

    m_image_width = render_config.image_width;
    m_image_height = render_config.image_height;
    m_samples_per_pixel = render_config.samples_per_pixel;
    m_max_ray_bounces = render_config.max_ray_bounces;

    DeriveDependentVariables();
    ResizeImageBuffer();
}

Camera::~Camera()
{
    if (m_image_data)
    {
        delete[] m_image_data;
    }
}

Camera::Camera(Camera&& other) noexcept
    : m_image_width(other.m_image_width)
    , m_image_height(other.m_image_height)
    , m_vertical_fov(other.m_vertical_fov)
    , m_samples_per_pixel(other.m_samples_per_pixel)
    , m_max_ray_bounces(other.m_max_ray_bounces)
    , m_look_from(other.m_look_from)
    , m_look_at(other.m_look_at)
    , m_up(other.m_up)
    , m_defocus_angle(other.m_defocus_angle)
    , m_focus_distance(other.m_focus_distance)
    , m_image_data(other.m_image_data)
    , m_aspect_ratio(other.m_aspect_ratio)
    , m_pixel_sample_scale(other.m_pixel_sample_scale)
    , m_centre(other.m_centre)
    , m_pixel_0_0_location(other.m_pixel_0_0_location)
    , m_pixel_delta_u(other.m_pixel_delta_u)
    , m_pixel_delta_v(other.m_pixel_delta_v)
    , m_u(other.m_u)
    , m_v(other.m_v)
    , m_w(other.m_w)
    , m_defocus_disk_u(other.m_defocus_disk_u)
    , m_defocus_disk_v(other.m_defocus_disk_v)
{
    other.m_image_data = nullptr;
}

Camera& Camera::operator=(Camera&& other) noexcept
{
    if (this != &other)
    {
        if (m_image_data)
        {
            delete[] m_image_data;
        }

        m_image_width = other.m_image_width;
        m_image_height = other.m_image_height;
        m_vertical_fov = other.m_vertical_fov;
        m_samples_per_pixel = other.m_samples_per_pixel;
        m_max_ray_bounces = other.m_max_ray_bounces;
        m_look_from = other.m_look_from;
        m_look_at = other.m_look_at;
        m_up = other.m_up;
        m_defocus_angle = other.m_defocus_angle;
        m_focus_distance = other.m_focus_distance;
        m_image_data = other.m_image_data;
        m_aspect_ratio = other.m_aspect_ratio;
        m_pixel_sample_scale = other.m_pixel_sample_scale;
        m_centre = other.m_centre;
        m_pixel_0_0_location = other.m_pixel_0_0_location;
        m_pixel_delta_u = other.m_pixel_delta_u;
        m_pixel_delta_v = other.m_pixel_delta_v;
        m_u = other.m_u;
        m_v = other.m_v;
        m_w = other.m_w;
        m_defocus_disk_u = other.m_defocus_disk_u;
        m_defocus_disk_v = other.m_defocus_disk_v;

        other.m_image_data = nullptr;
    }
    return *this;
}

static const Interval intensity(0.0, 0.999);

void Camera::Render(const IRayHittable& scene, const SceneConfig& scene_config, const std::string& output_image_name)
{
    std::atomic<bool> no_cancel{false};

    // Uses same render process with no cancel option or progress indicators
    RenderAsync(scene, scene_config, no_cancel, nullptr, output_image_name);
}

bool Camera::RenderAsync
(
    const IRayHittable& scene,
    const SceneConfig& scene_config,
    const std::atomic<bool>& should_cancel,
    std::atomic<std::size_t>* num_completed_rows,
    const std::string& output_image_name
)
{
    assert(m_image_width > 0);
    assert(m_image_height > 0);
    assert(m_vertical_fov > 0.0);
    assert(m_samples_per_pixel >= 1);
    assert(m_max_ray_bounces >= 1);
    assert(m_image_data != nullptr);

    const Colour& background_colour = scene_config.background_colour;

    // Update progress every 16 rows
    constexpr std::size_t progress_update_interval = 16;

    #pragma omp parallel for schedule(dynamic)
    for (std::int64_t j = 0; j < static_cast<std::int64_t>(m_image_height); j++)
    {
        // Skip work in loop until return possible
        if (should_cancel.load(std::memory_order_relaxed))
        {
            continue;
        }

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
                pixel_colour += RayColour(ray, m_max_ray_bounces, scene, background_colour);
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

        // Update rows completed every (progress_update_interval) rows
        if (num_completed_rows && (static_cast<std::size_t>(j) % progress_update_interval == 0))
        {
            num_completed_rows->fetch_add(progress_update_interval, std::memory_order_relaxed);
        }
    }

    // Exit if render cancel has been requested
    if (should_cancel.load(std::memory_order_relaxed))
    {
        return false;
    }

    // Show 100% when complete
    if (num_completed_rows)
    {
        num_completed_rows->store(m_image_height, std::memory_order_relaxed);
    }

    stbi_write_png
    (
        output_image_name.c_str(),
        static_cast<int>(m_image_width),
        static_cast<int>(m_image_height),
        static_cast<int>(num_image_components),
        m_image_data,
        static_cast<int>(m_image_width * sizeof(uint8_t) * num_image_components)
    );

    return true;
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

    m_pixel_delta_u = viewport_u / static_cast<double>(m_image_width);
    m_pixel_delta_v = viewport_v / static_cast<double>(m_image_height);

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

    m_image_data = new uint8_t[m_image_width * m_image_height * num_image_components]{};
}

Colour Camera::RayColour(const Ray& ray, std::size_t depth, const IRayHittable& scene, const Colour& background_colour)
{
    if (depth <= 0)
    {
        return Colour(0.0);
    }

    RayHitResult result;
    const double min_ray_t = 0.001;
    if (!scene.Hit(ray, Interval(min_ray_t, infinity), result))
    {
        return background_colour;
    }

    Ray scattered;
    Colour attenuation;
    const Colour colour_from_emission = result.m_material->Emitted(result.m_u, result.m_v, result.m_point);
    if (!result.m_material->Scatter(ray, result, attenuation, scattered))
    {
        return colour_from_emission;
    }

    Colour colour_from_scatter = attenuation * RayColour(scattered, depth - 1, scene, background_colour);

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
