// Copyright Mia Rolfe. All rights reserved.
#include "Camera.h"
#include "Utility.h"
#include "Vec3.h"
#include <cstdint>

namespace ART
{

const CameraSetupParams default_camera_setup_params =
{
    1280,                           // image_width
    720,                            // image_height
    3,                              // num_image_components
    Colour(0.529, 0.808, 0.922),    // background_colour
    60.0,                           // vertical_fov
    500,                            // pixel_sample_scale
    50,                             // max_ray_bounces
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
    m_num_image_components = setup_params.num_image_components;
    m_background_colour = setup_params.background_colour;
    m_vertical_fov = setup_params.vertical_fov;
    m_pixel_sample_scale = setup_params.pixel_sample_scale;
    m_max_ray_bounces = setup_params.max_ray_bounces;
    m_look_from = setup_params.look_from;
    m_look_at = setup_params.look_at;
    m_up = setup_params.up;
    m_defocus_angle = setup_params.defocus_angle;
    m_focus_distance = setup_params.focus_distance;
}

void Camera::DeriveDependentVariables()
{
    m_aspect_ratio = (static_cast<double>(m_image_width) / static_cast<double>(m_image_height));

    m_pixel_sample_scale = 1.0 / m_samples_per_pixel;

    ResizeImageBuffer();

    m_centre = m_look_from;

    const double theta = DegreesToRadians(m_vertical_fov);
    const double h = std::tan(theta / 2.0);
    const double viewport_height = 2.0 * h * m_focus_distance;
    // Review this line
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

    m_image_data = new uint8_t[m_image_width * m_image_height * m_num_image_components];
}

} // namespace ART
