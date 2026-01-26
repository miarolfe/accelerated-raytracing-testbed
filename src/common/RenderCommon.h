// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <atomic>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <ArenaAllocator.h>
#include <AxisAlignedBox.h>
#include <BoundingVolumeHierarchy.h>
#include <BSPTree.h>
#include <Camera.h>
#include <Colour.h>
#include <HierarchicalUniformGrid.h>
#include <KDTree.h>
#include <Logger.h>
#include <Material.h>
#include <Octree.h>
#include <RayHittableList.h>
#include <Sphere.h>
#include <Texture.h>
#include <Timer.h>
#include <UniformGrid.h>
#include <Utility.h>
#include <Vec3.h>

namespace ART
{

constexpr std::size_t MIN_RENDER_WIDTH = 1;
constexpr std::size_t MAX_RENDER_WIDTH = 7680;
constexpr std::size_t MIN_RENDER_HEIGHT = 1;
constexpr std::size_t MAX_RENDER_HEIGHT = 4320;
constexpr std::size_t MIN_SAMPLES_PER_PIXEL = 1;
constexpr std::size_t MAX_SAMPLES_PER_PIXEL = 10000;

// Holds all scene data needed for async rendering
struct RenderContext
{
public:
    // Default constructor
    RenderContext() = default;

    // Non-copyable (atomics are not copyable)
    RenderContext(const RenderContext&) = delete;
    RenderContext& operator=(const RenderContext&) = delete;

    // Custom move constructor (atomics need explicit handling)
    RenderContext(RenderContext&& other) noexcept;

    // Custom move assignment (atomics need explicit handling)
    RenderContext& operator=(RenderContext&& other) noexcept;

    ArenaAllocator arena{ONE_MEGABYTE * 4};
    Camera camera;
    RayHittableList scene;
    SceneConfig scene_config;
    std::string output_image_name;
    AccelerationStructure acceleration_structure = AccelerationStructure::NONE;

    // Progress tracking (updated by render thread, read by UI thread)
    std::atomic<std::size_t> num_completed_rows{0};
    std::atomic<std::size_t> total_rows{0};

    // Control flag (set by UI thread to cancel)
    std::atomic<bool> cancel_requested{false};

    // Status flags
    std::atomic<bool> render_complete{false};
    std::atomic<bool> was_cancelled{false};

    // Timing (set by render thread)
    double construction_time_ms{0.0};
    double render_time_ms{0.0};
};

void LogRenderConfig(const CameraRenderConfig& render_config, int scene_number);

void LogRenderStats(const RenderStats& stats);

RenderStats RenderWithAccelerationStructure
(
    Camera& camera,
    RayHittableList& scene,
    const SceneConfig& scene_config,
    AccelerationStructure acceleration_structure
);

void SetupScene
(
    RenderContext& render_context,
    const CameraRenderConfig& render_config,
    int scene_number
);

void RenderScene
(
    const CameraRenderConfig& render_config,
    int scene_number,
    AccelerationStructure acceleration_structure
);

// Set up a scene for async rendering
RenderContext CreateAsyncRenderContext(
    const CameraRenderConfig& render_config,
    int scene_number,
    AccelerationStructure acceleration_structure
);

// Execute the render (call from background thread)
// Returns true if completed, false if cancelled
bool ExecuteAsyncRender(RenderContext& context);

} // namespace ART
