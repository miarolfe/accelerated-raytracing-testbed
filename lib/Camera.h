// Copyright Mia Rolfe. All rights reserved.
#pragma once
#include "Colour.h"
#include "Common.h"
#include "IRayHittable.h"

namespace ART
{

struct CameraSetupParams
{
public:
    std::size_t image_width;
    std::size_t image_height;
    Colour background_colour;
    double vertical_fov;
    double samples_per_pixel;
    std::size_t max_ray_bounces;
    Point3 look_from;
    Point3 look_at;
    Vec3 up;
    double defocus_angle;
    double focus_distance;
};

class Camera
{
public:
    Camera();

    Camera(const CameraSetupParams& setup_params);

    ~Camera();

    void Render(const IRayHittable& scene);


protected:
    void DeriveDependentVariables();

    void ResizeImageBuffer();

    Colour RayColour(const Ray& ray, std::size_t depth, const IRayHittable& scene);

    Ray GetRay(std::size_t i, std::size_t j);

    Vec3 SampleSquare() const;

    Point3 DefocusDiskSample() const;

    ///
    /// Non-derived member variables
    ///

    // Width of the output image in pixels
    std::size_t m_image_width;

    // Height of the output image in pixels
    std::size_t m_image_height;

    Colour m_background_colour;

    // In degrees
    double m_vertical_fov;

    // Number of rays per pixel; reduces noise
    double m_samples_per_pixel;

    // Max number of recursions for each ray bouncing
    std::size_t m_max_ray_bounces;

    // The point where the camera is looking from, i.e. its position
    Point3 m_look_from;

    // The point the camera is looking at, i.e. where your eyes would be focused
    Point3 m_look_at;

    // What direction is up
    Point3 m_up;

    // Variation angle (in degrees) of rays through each pixel
    double m_defocus_angle;

    // Distance from m_look_from to plane of perfect focus
    double m_focus_distance;

    ///
    /// Derived member variables
    ///

    // The output image buffer
    uint8_t* m_image_data = nullptr;

    // Derived from (m_image_width / m_image_height)
    double m_aspect_ratio;

    // Inverse of m_samples_per_pixel
    double m_pixel_sample_scale;

    Point3 m_centre;

    Point3 m_pixel_0_0_location;

    Vec3 m_pixel_delta_u;

    Vec3 m_pixel_delta_v;

    // Camera frame basis vectors
    Vec3 m_u;
    Vec3 m_v;
    Vec3 m_w;

    Vec3 m_defocus_disk_u;
    Vec3 m_defocus_disk_v;
};

} // namespace ART
