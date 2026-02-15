// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <atomic>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <Acceleration/BoundingVolumeHierarchy.h>
#include <Acceleration/BSPTree.h>
#include <Acceleration/HierarchicalUniformGrid.h>
#include <Acceleration/KDTree.h>
#include <Acceleration/Octree.h>
#include <Acceleration/UniformGrid.h>
#include <Core/ArenaAllocator.h>
#include <Core/Logger.h>
#include <Core/Timer.h>
#include <Core/Utility.h>
#include <Geometry/AxisAlignedBox.h>
#include <Geometry/Sphere.h>
#include <Materials/Material.h>
#include <Materials/Texture.h>
#include <Maths/Colour.h>
#include <Maths/Vec3.h>
#include <RayTracing/Camera.h>
#include <RayTracing/RayHittableList.h>

namespace ART
{

constexpr std::size_t MIN_RENDER_WIDTH = 1;
constexpr std::size_t MAX_RENDER_WIDTH = 7680;
constexpr std::size_t MIN_RENDER_HEIGHT = 1;
constexpr std::size_t MAX_RENDER_HEIGHT = 4320;
constexpr std::size_t MIN_SAMPLES_PER_PIXEL = 1;
constexpr std::size_t MAX_SAMPLES_PER_PIXEL = 10000;
constexpr uint32_t DEFAULT_POSITION_SEED = 22052003;
constexpr uint32_t DEFAULT_COLOUR_SEED = 13012025;

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

    // Memory usage by render thread
    std::size_t memory_used_bytes{0};

    // Traversal efficiency metrics
    TraversalStats traversal_stats;
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
    int scene_number,
    uint32_t colour_seed = DEFAULT_COLOUR_SEED,
    uint32_t position_seed = DEFAULT_POSITION_SEED
);

void RenderScene
(
    const CameraRenderConfig& render_config,
    int scene_number,
    AccelerationStructure acceleration_structure,
    uint32_t colour_seed = DEFAULT_COLOUR_SEED,
    uint32_t position_seed = DEFAULT_POSITION_SEED
);

// Set up a scene for async rendering
RenderContext CreateAsyncRenderContext(
    const CameraRenderConfig& render_config,
    int scene_number,
    AccelerationStructure acceleration_structure,
    uint32_t colour_seed = DEFAULT_COLOUR_SEED,
    uint32_t position_seed = DEFAULT_POSITION_SEED
);

// Execute the render (call from background thread)
// Returns true if completed, false if cancelled
bool ExecuteAsyncRender(RenderContext& context);

} // namespace ART
