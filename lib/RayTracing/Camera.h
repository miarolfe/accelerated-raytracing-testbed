// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <atomic>

#include <Core/Common.h>
#include <Maths/Colour.h>
#include <RayTracing/IRayHittable.h>

namespace ART
{

struct CameraViewConfig
{
public:
    Point3 look_from;
    Point3 look_at;
    Vec3 up;
    double vertical_fov;
    double defocus_angle;
    double focus_distance;
};

struct CameraRenderConfig
{
public:
    std::size_t image_width;
    std::size_t image_height;
    std::size_t samples_per_pixel;
    std::size_t max_ray_bounces;
};

struct SceneConfig
{
public:
    Colour background_colour;
};

class Camera
{
public:
    Camera();

    Camera(const CameraViewConfig& view_config, const CameraRenderConfig& render_config);

    ~Camera();

    // Non-copyable
    Camera(const Camera&) = delete;

    Camera& operator=(const Camera&) = delete;

    // Movable
    Camera(Camera&& other) noexcept;

    Camera& operator=(Camera&& other) noexcept;

    void Render(const IRayHittable& scene, const SceneConfig& scene_config, const std::string& output_image_name = "render.png");

    // Render with cancellation and progress indicator support
    // should_cancel: cancel render-in-progress
    // num_completed_rows (optional): incremented as rows complete
    bool RenderAsync
    (
        const IRayHittable& scene,
        const SceneConfig& scene_config,
        const std::atomic<bool>& should_cancel,
        std::atomic<std::size_t>* num_completed_rows = nullptr,
        const std::string& output_image_name = "render.png"
    );

    // Read-only access to image buffer for live preview
    const uint8_t* GetImageBuffer() const { return m_image_data; }

    std::size_t GetImageWidth() const { return m_image_width; }

    std::size_t GetImageHeight() const { return m_image_height; }


protected:
    void DeriveDependentVariables();

    void ResizeImageBuffer();

    Colour RayColour(const Ray& ray, std::size_t depth, const IRayHittable& scene, const Colour& background_colour);

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

    // In degrees
    double m_vertical_fov;

    // Number of rays per pixel; reduces noise
    std::size_t m_samples_per_pixel;

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
