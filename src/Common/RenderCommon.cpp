// Copyright Mia Rolfe. All rights reserved.
#include <Common/RenderCommon.h>

#include <Core/Random.h>

namespace ART
{

RenderContext::RenderContext(RenderContext&& other) noexcept
    : arena(std::move(other.arena))
    , camera(std::move(other.camera))
    , scene(std::move(other.scene))
    , scene_config(std::move(other.scene_config))
    , output_image_name(std::move(other.output_image_name))
    , acceleration_structure(other.acceleration_structure)
    , num_completed_rows(other.num_completed_rows.load())
    , total_rows(other.total_rows.load())
    , cancel_requested(other.cancel_requested.load())
    , render_complete(other.render_complete.load())
    , was_cancelled(other.was_cancelled.load())
    , construction_time_ms(other.construction_time_ms)
    , render_time_ms(other.render_time_ms)
    , memory_used_bytes(other.memory_used_bytes)
    , traversal_stats(other.traversal_stats)
{
    other.num_completed_rows.store(0);
    other.total_rows.store(0);
    other.cancel_requested.store(false);
    other.render_complete.store(false);
    other.was_cancelled.store(false);
    other.construction_time_ms = 0.0;
    other.render_time_ms = 0.0;
    other.memory_used_bytes = 0;
    other.traversal_stats = {};
}

RenderContext& RenderContext::operator=(RenderContext&& other) noexcept
{
    if (this != &other)
    {
        arena = std::move(other.arena);
        camera = std::move(other.camera);
        scene = std::move(other.scene);
        scene_config = std::move(other.scene_config);
        output_image_name = std::move(other.output_image_name);
        acceleration_structure = other.acceleration_structure;

        num_completed_rows.store(other.num_completed_rows.load());
        total_rows.store(other.total_rows.load());
        cancel_requested.store(other.cancel_requested.load());
        render_complete.store(other.render_complete.load());
        was_cancelled.store(other.was_cancelled.load());

        construction_time_ms = other.construction_time_ms;
        render_time_ms = other.render_time_ms;
        memory_used_bytes = other.memory_used_bytes;
        traversal_stats = other.traversal_stats;

        other.num_completed_rows.store(0);
        other.total_rows.store(0);
        other.cancel_requested.store(false);
        other.render_complete.store(false);
        other.was_cancelled.store(false);
        other.construction_time_ms = 0.0;
        other.render_time_ms = 0.0;
        other.memory_used_bytes = 0;
        other.traversal_stats = {};
    }
    return *this;
}

void LogRenderConfig(const CameraRenderConfig& render_config, int scene_number)
{
    std::ostringstream output_string_stream;
    output_string_stream << "Configuration: " << render_config.image_width << "x" << render_config.image_height
                         << ", " << render_config.samples_per_pixel << " samples per pixel";
    Logger::Get().LogInfo(output_string_stream.str());
}

void LogRenderStats(const RenderStats& stats)
{
    std::ostringstream output_string_stream;
    // Use fixed-point (no scientific notation)
    output_string_stream << std::fixed << std::setprecision(2);
    output_string_stream << "[Acceleration structure: " << AccelerationStructureToString(stats.m_acceleration_structure) << "] "
        << "Construction time: " << stats.m_construction_time_ms << " ms, "
        << "Render time: " << stats.m_render_time_ms << " ms, "
        << "Total time: " << stats.TotalTimeMilliseconds() << " ms, "
        << "Memory used: " << stats.m_memory_used_bytes << " B, ";

    output_string_stream << ", Avg nodes/ray: " << stats.m_traversal_stats.AvgNodesTraversedPerRay()
        << ", Avg intersection tests/ray: " << stats.m_traversal_stats.AvgIntersectionTestsPerRay();

    Logger::Get().LogInfo(output_string_stream.str());
}

RenderStats RenderWithAccelerationStructure(Camera& camera, RayHittableList& scene, const SceneConfig& scene_config, AccelerationStructure acceleration_structure)
{
    Timer timer;
    RenderStats stats;
    stats.m_acceleration_structure = acceleration_structure;

    switch (acceleration_structure)
    {
        case AccelerationStructure::NONE:
        {
            stats.m_construction_time_ms = 0.0;
            stats.m_memory_used_bytes = 0;

            timer.Start();
            camera.Render(scene, scene_config, "render_none.png", &stats.m_traversal_stats);
            timer.Stop();
            stats.m_render_time_ms = timer.ElapsedMilliseconds();
            break;
        }
        case AccelerationStructure::UNIFORM_GRID:
        {
            timer.Start();
            UniformGrid uniform_grid(scene.GetObjects());
            timer.Stop();
            stats.m_construction_time_ms = timer.ElapsedMilliseconds();
            stats.m_memory_used_bytes = uniform_grid.MemoryUsedBytes();

            timer.Start();
            camera.Render(uniform_grid, scene_config, "render_uniform_grid.png", &stats.m_traversal_stats);
            timer.Stop();
            stats.m_render_time_ms = timer.ElapsedMilliseconds();
            break;
        }
        case AccelerationStructure::HIERARCHICAL_UNIFORM_GRID:
        {
            timer.Start();
            HierarchicalUniformGrid hierarchical_uniform_grid(scene.GetObjects());
            timer.Stop();
            stats.m_construction_time_ms = timer.ElapsedMilliseconds();
            stats.m_memory_used_bytes = hierarchical_uniform_grid.MemoryUsedBytes();

            timer.Start();
            camera.Render(hierarchical_uniform_grid, scene_config, "render_hierarchical_uniform_grid.png", &stats.m_traversal_stats);
            timer.Stop();
            stats.m_render_time_ms = timer.ElapsedMilliseconds();
            break;
        }
        case AccelerationStructure::OCTREE:
        {
            timer.Start();
            OctreeNode octree(scene.GetObjects());
            timer.Stop();
            stats.m_construction_time_ms = timer.ElapsedMilliseconds();
            stats.m_memory_used_bytes = octree.MemoryUsedBytes();

            timer.Start();
            camera.Render(octree, scene_config, "render_octree.png", &stats.m_traversal_stats);
            timer.Stop();
            stats.m_render_time_ms = timer.ElapsedMilliseconds();
            break;
        }
        case AccelerationStructure::BSP_TREE:
        {
            timer.Start();
            BSPTreeNode bsp_tree(scene.GetObjects());
            timer.Stop();
            stats.m_construction_time_ms = timer.ElapsedMilliseconds();
            stats.m_memory_used_bytes = bsp_tree.MemoryUsedBytes();

            timer.Start();
            camera.Render(bsp_tree, scene_config, "render_bsp_tree.png", &stats.m_traversal_stats);
            timer.Stop();
            stats.m_render_time_ms = timer.ElapsedMilliseconds();
            break;
        }
        case AccelerationStructure::K_D_TREE:
        {
            timer.Start();
            KDTreeNode hierarchical_uniform_grid(scene.GetObjects());
            timer.Stop();
            stats.m_construction_time_ms = timer.ElapsedMilliseconds();
            stats.m_memory_used_bytes = hierarchical_uniform_grid.MemoryUsedBytes();

            timer.Start();
            camera.Render(hierarchical_uniform_grid, scene_config, "render_k_d_tree.png", &stats.m_traversal_stats);
            timer.Stop();
            stats.m_render_time_ms = timer.ElapsedMilliseconds();
            break;
        }
        case AccelerationStructure::BOUNDING_VOLUME_HIERARCHY:
        {
            timer.Start();
            BVHNode bounding_volume_hierarchy(scene.GetObjects());
            timer.Stop();
            stats.m_construction_time_ms = timer.ElapsedMilliseconds();
            stats.m_memory_used_bytes = bounding_volume_hierarchy.MemoryUsedBytes();

            timer.Start();
            camera.Render(bounding_volume_hierarchy, scene_config, "render_bounding_volume_hierarchy.png", &stats.m_traversal_stats);
            timer.Stop();
            stats.m_render_time_ms = timer.ElapsedMilliseconds();
            break;
        }
    }

    LogRenderStats(stats);
    return stats;
}

void SetupScene(RenderContext& render_context, const CameraRenderConfig& render_config, int scene_number, uint32_t colour_seed, uint32_t position_seed)
{
    SeedColourRNG(colour_seed);
    SeedPositionRNG(position_seed);

    render_context.scene_config = SceneConfig{Colour(0.7, 0.8, 1.0)};

    switch (scene_number)
    {
        case 1:
        {
            Point3 average_position_cluster_1;
            int num_spheres_cluster_1 = 0;

            // Cluster 1
            for (int i = 0; i < 10; i++)
            {
                for (int j = 0; j < 10; j++)
                {
                    for (int k = 0; k < 10; k++)
                    {
                        const Point3 sphere_position(0.0 + (i * 3.0), 0.0 + (j * 3.0), 0.0 + (k * 3.0));
                        average_position_cluster_1 += sphere_position;
                        num_spheres_cluster_1++;

                        Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                        Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                        render_context.scene.Add(render_context.arena.Create<Sphere>(sphere_position, 1.0, material));
                    }
                }
            }

            // Cluster 2
            for (int i = 0; i < 10; i++)
            {
                for (int j = 0; j < 10; j++)
                {
                    for (int k = 0; k < 10; k++)
                    {
                        const Point3 sphere_position(500.0 + (i * 3.0), 500.0 + (j * 3.0), 500.0 + (k * 3.0));
                        Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                        Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                        render_context.scene.Add(render_context.arena.Create<Sphere>(sphere_position, 1.0, material));
                    }
                }
            }

            // Cluster 3
            for (int i = 0; i < 10; i++)
            {
                for (int j = 0; j < 10; j++)
                {
                    for (int k = 0; k < 10; k++)
                    {
                        const Point3 sphere_position(1000.0 + (i * 3.0), -500.0 + (j * 3.0), 1000.0 + (k * 3.0));
                        Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                        Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                        render_context.scene.Add(render_context.arena.Create<Sphere>(sphere_position, 1.0, material));
                    }
                }
            }

            assert(num_spheres_cluster_1 > 0);
            average_position_cluster_1 /= num_spheres_cluster_1;

            CameraViewConfig view_config
            {
                Point3(-100.0, 100.0, 100.0),
                average_position_cluster_1,
                Vec3(0.0, 1.0, 0.0),
                18.0, 0.0, 10.0
            };
            render_context.camera = Camera(view_config, render_config);
            break;
        }
        case 2:
        {
            // Uniform dense field: 10,000 objects mostly uniformly distributed
            CameraViewConfig view_config
            {
                Point3(-30.0, 50.0, -30.0),
                Point3(20.0, 20.0, 20.0),
                Vec3(0.0, 1.0, 0.0),
                40.0, 0.0, 10.0
            };
            render_context.camera = Camera(view_config, render_config);

            // 21x21x21 = 9261 spheres on a regular grid with jitter
            static constexpr int SPHERE_GRID_AXIS_LENGTH = 21;
            static constexpr double SPHERE_JITTER = 0.3;
            static constexpr double SPHERE_RADIUS = 0.4;
            for (int i = 0; i < SPHERE_GRID_AXIS_LENGTH; i++)
            {
                for (int j = 0; j < SPHERE_GRID_AXIS_LENGTH; j++)
                {
                    for (int k = 0; k < SPHERE_GRID_AXIS_LENGTH; k++)
                    {
                        const double jitter_x = RandomPositionDouble(-SPHERE_JITTER, SPHERE_JITTER);
                        const double jitter_y = RandomPositionDouble(-SPHERE_JITTER, SPHERE_JITTER);
                        const double jitter_z = RandomPositionDouble(-SPHERE_JITTER, SPHERE_JITTER);
                        const Point3 position(i * 2.0 + jitter_x, j * 2.0 + jitter_y, k * 2.0 + jitter_z);

                        Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                        Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                        render_context.scene.Add(render_context.arena.Create<Sphere>(position, SPHERE_RADIUS, material));
                    }
                }
            }

            // 739 additional to hit 10k spheres
            static constexpr int NUM_RANDOMLY_DISTRIBUTED_SPHERES = 739;
            for (int i = 0; i < NUM_RANDOMLY_DISTRIBUTED_SPHERES; i++)
            {
                const Point3 position(RandomPositionDouble(0.0, 40.0), RandomPositionDouble(0.0, 40.0), RandomPositionDouble(0.0, 40.0));
                Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                render_context.scene.Add(render_context.arena.Create<Sphere>(position, SPHERE_RADIUS, material));
            }
            break;
        }
        case 3:
        {
            // Sparse clusters in void: 5 clusters of 400 spheres in a very large bounding volume
            CameraViewConfig view_config
            {
                Point3(-50.0, 300.0, -50.0),
                Point3(250.0, 250.0, 250.0),
                Vec3(0.0, 1.0, 0.0),
                60.0, 0.0, 10.0
            };
            render_context.camera = Camera(view_config, render_config);

            const Point3 cluster_centres[] =
            {
                Point3(0.0, 0.0, 0.0),
                Point3(500.0, 0.0, 0.0),
                Point3(0.0, 500.0, 0.0),
                Point3(0.0, 0.0, 500.0),
                Point3(500.0, 500.0, 500.0)
            };

            static constexpr double SPHERE_RADIUS = 0.5;
            static constexpr double SPHERE_SPACING = 1.5;
            static constexpr int CLUSTER_X_LENGTH = 8;
            static constexpr int CLUSTER_Y_LENGTH = 8;
            static constexpr int CLUSTER_Z_LENGTH = 6;
            for (const Point3& centre : cluster_centres)
            {
                for (int i = 0; i < CLUSTER_X_LENGTH; i++)
                {
                    for (int j = 0; j < CLUSTER_Y_LENGTH; j++)
                    {
                        for (int k = 0; k < CLUSTER_Z_LENGTH; k++)
                        {
                            const Point3 position = centre + Vec3(i * SPHERE_SPACING, j * SPHERE_SPACING, k * SPHERE_SPACING);
                            Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                            Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                            render_context.scene.Add(render_context.arena.Create<Sphere>(position, SPHERE_RADIUS, material));
                        }
                    }
                }
                // 16 extra random spheres per cluster to reach 400
                static constexpr int NUM_RANDOMLY_DISTRIBUTED_SPHERES_PER_CLUSTER = 16;
                for (int i = 0; i < NUM_RANDOMLY_DISTRIBUTED_SPHERES_PER_CLUSTER; i++)
                {
                    const Point3 position = centre + Vec3(RandomPositionDouble(0.0, 10.5), RandomPositionDouble(0.0, 10.5), RandomPositionDouble(0.0, 7.5));
                    Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                    Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                    render_context.scene.Add(render_context.arena.Create<Sphere>(position, SPHERE_RADIUS, material));
                }
            }
            break;
        }
        case 4:
        {
            // Extreme size Variation: huge ground + backdrop spheres with tiny spheres
            CameraViewConfig view_config
            {
                Point3(0.0, 8.0, 30.0),
                Point3(0.0, 2.0, 0.0),
                Vec3(0.0, 1.0, 0.0),
                50.0, 0.0, 10.0
            };
            render_context.camera = Camera(view_config, render_config);

            Texture* ground_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.4, 0.4, 0.4));
            Material* ground_material = render_context.arena.Create<LambertianMaterial>(ground_texture);
            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(0.0, -1000.0, 0.0), 1000.0, ground_material));

            Texture* backdrop_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.2, 0.3, 0.7));
            Material* backdrop_material = render_context.arena.Create<LambertianMaterial>(backdrop_texture);
            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(0.0, 0.0, -200.0), 100.0, backdrop_material));

            static constexpr int NUM_SMALL_GROUND_SPHERES = 2000;
            for (int i = 0; i < NUM_SMALL_GROUND_SPHERES; i++)
            {
                const double radius = RandomPositionDouble(0.1, 0.5);
                const Point3 position(RandomPositionDouble(-20.0, 20.0), radius, RandomPositionDouble(-20.0, 20.0));
                Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                render_context.scene.Add(render_context.arena.Create<Sphere>(position, radius, material));
            }

            static constexpr int NUM_MEDIUM_SPHERES = 10;
            for (int i = 0; i < NUM_MEDIUM_SPHERES; i++)
            {
                const double radius = RandomPositionDouble(2.0, 5.0);
                const Point3 position(RandomPositionDouble(-40.0, 40.0), radius, RandomPositionDouble(-40.0, 40.0));
                Material* material = render_context.arena.Create<MetalMaterial>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()), RandomPositionDouble(0.0, 0.3));
                render_context.scene.Add(render_context.arena.Create<Sphere>(position, radius, material));
            }
            break;
        }
        case 5:
        {
            // Long corridor: rings of spheres with box walls
            CameraViewConfig view_config
            {
                Point3(0.0, 5.0, -5.0),
                Point3(0.0, 5.0, 100.0),
                Vec3(0.0, 1.0, 0.0),
                50.0, 0.0, 10.0
            };
            render_context.camera = Camera(view_config, render_config);

            // 250 rings of 10 spheres each along Z axis
            static constexpr int NUM_RINGS = 250;
            static constexpr double RING_Z_SPACING = 0.8;
            static constexpr int NUM_SPHERES_PER_RING = 10;
            static constexpr double BASE_DEGREE_SPACING_PER_SPHERE = 360.0 / static_cast<double>(NUM_SPHERES_PER_RING);
            static constexpr double SPHERE_RADIUS = 0.4;
            for (int ring = 0; ring < NUM_RINGS; ring++)
            {
                const double z = ring * RING_Z_SPACING;
                for (int sphere = 0; sphere < NUM_SPHERES_PER_RING; sphere++)
                {
                    const double angle = (sphere * BASE_DEGREE_SPACING_PER_SPHERE + RandomPositionDouble(-5.0, 5.0)) * pi / 180.0;
                    const double x = 5.0 * std::cos(angle);
                    const double y = 5.0 + 5.0 * std::sin(angle);

                    Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                    Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                    render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(x, y, z), SPHERE_RADIUS, material));
                }
            }

            Texture* floor_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.5, 0.5, 0.5));
            Material* floor_material = render_context.arena.Create<LambertianMaterial>(floor_texture);
            render_context.scene.Add(render_context.arena.Create<AxisAlignedBox>(Point3(-8.0, -1.0, -1.0), Point3(8.0, 0.0, 201.0), floor_material));

            Texture* ceiling_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.6, 0.6, 0.6));
            Material* ceiling_material = render_context.arena.Create<LambertianMaterial>(ceiling_texture);
            render_context.scene.Add(render_context.arena.Create<AxisAlignedBox>(Point3(-8.0, 11.0, -1.0), Point3(8.0, 12.0, 201.0), ceiling_material));

            Material* wall_material = render_context.arena.Create<MetalMaterial>(Colour(0.7, 0.7, 0.7), 0.1);
            render_context.scene.Add(render_context.arena.Create<AxisAlignedBox>(Point3(-8.0, -1.0, -1.0), Point3(-7.0, 12.0, 201.0), wall_material));
            break;
        }
        case 6:
        {
            // Centroid co-location: concentric spheres with identical centroids
            CameraViewConfig view_config
            {
                Point3(0.0, 5.0, 40.0),
                Point3(0.0, 5.0, 0.0),
                Vec3(0.0, 1.0, 0.0),
                30.0, 0.0, 10.0
            };
            render_context.camera = Camera(view_config, render_config);

            // Concentric spheres at the same center
            static constexpr int NUM_CONCENTRIC_SPHERES = 1500;
            for (int i = 0; i < NUM_CONCENTRIC_SPHERES; i++)
            {
                const double radius = 0.1 + i * (14.9 / 1499.0);
                const bool is_glass = (i % 3 == 0);
                Material* material;
                if (is_glass)
                {
                    material = render_context.arena.Create<DielectricMaterial>(1.5);
                }
                else
                {
                    Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                    material = render_context.arena.Create<LambertianMaterial>(texture);
                }
                render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(0.0, 5.0, 0.0), radius, material));
            }

            static constexpr int NUM_NEARLY_COLOCATED_SPHERES = 500;
            for (int i = 0; i < NUM_NEARLY_COLOCATED_SPHERES; i++)
            {
                const double radius = RandomPositionDouble(0.5, 3.0);
                const Point3 position(RandomPositionDouble(-0.001, 0.001), 5.0 + RandomPositionDouble(-0.001, 0.001), RandomPositionDouble(-0.001, 0.001));
                Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                render_context.scene.Add(render_context.arena.Create<Sphere>(position, radius, material));
            }
            break;
        }
        case 7:
        {
            // Flat plane distribution: objects on a ground plane, almost 2D in distribution
            CameraViewConfig view_config
            {
                Point3(0.0, 60.0, 60.0),
                Point3(0.0, 0.0, 0.0),
                Vec3(0.0, 1.0, 0.0),
                45.0, 0.0, 10.0
            };
            render_context.camera = Camera(view_config, render_config);

            // Ground box
            Texture* ground_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.3, 0.3, 0.3));
            Material* ground_material = render_context.arena.Create<LambertianMaterial>(ground_texture);
            render_context.scene.Add(render_context.arena.Create<AxisAlignedBox>(Point3(-55.0, -0.5, -55.0), Point3(55.0, 0.0, 55.0), ground_material));

            static constexpr int NUM_GROUND_SPHERES = 3000;
            for (int i = 0; i < NUM_GROUND_SPHERES; i++)
            {
                const double radius = RandomPositionDouble(0.2, 0.8);
                const Point3 position(RandomPositionDouble(-50.0, 50.0), radius, RandomPositionDouble(-50.0, 50.0));
                Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                render_context.scene.Add(render_context.arena.Create<Sphere>(position, radius, material));
            }

            static constexpr int NUM_GROUND_BOXES = 2000;
            for (int i = 0; i < NUM_GROUND_BOXES; i++)
            {
                const double x = RandomPositionDouble(-50.0, 50.0);
                const double z = RandomPositionDouble(-50.0, 50.0);
                const double width = RandomPositionDouble(0.3, 1.0);
                const double height = RandomPositionDouble(0.1, 2.0);
                const double depth = RandomPositionDouble(0.3, 1.0);
                Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                render_context.scene.Add(render_context.arena.Create<AxisAlignedBox>(Point3(x, 0.0, z), Point3(x + width, height, z + depth), material));
            }
            break;
        }
        case 8:
        {
            // Diagonal wall: objects placed along x=z diagonal
            CameraViewConfig view_config
            {
                Point3(50.0, 50.0, -20.0),
                Point3(50.0, 25.0, 50.0),
                Vec3(0.0, 1.0, 0.0),
                50.0, 0.0, 10.0
            };
            render_context.camera = Camera(view_config, render_config);

            // Spheres in a diagonal wall
            static constexpr int NUM_DIAGONAL_STEPS = 50;
            static constexpr int NUM_VERTICAL_STEPS = 50;
            static constexpr double WALL_SPHERE_RADIUS = 0.4;
            for (int diagonal_step = 0; diagonal_step < NUM_DIAGONAL_STEPS; diagonal_step++)
            {
                const double progress_fraction = diagonal_step / (NUM_DIAGONAL_STEPS - 1.0);
                const double base_xz = progress_fraction * (2.0 * NUM_DIAGONAL_STEPS);
                for (int vertical_step = 0; vertical_step < NUM_VERTICAL_STEPS; vertical_step++)
                {
                    const double x = base_xz + RandomPositionDouble(-0.2, 0.2);
                    const double z = base_xz + RandomPositionDouble(-0.2, 0.2);
                    const double y = vertical_step * 1.0;
                    Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                    Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                    render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(x, y, z), WALL_SPHERE_RADIUS, material));
                }
            }

            static constexpr int NUM_RANDOM_SPHERES = 500;
            static constexpr double RANDOM_SPHERE_RADIUS = 0.3;
            for (int i = 0; i < NUM_RANDOM_SPHERES; i++)
            {
                const Point3 position(RandomPositionDouble(0.0, 100.0), RandomPositionDouble(0.0, 50.0), RandomPositionDouble(0.0, 100.0));
                Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                render_context.scene.Add(render_context.arena.Create<Sphere>(position, RANDOM_SPHERE_RADIUS, material));
            }
            break;
        }
        case 9:
        {
            // High object count: 12,000 objects randomly distributed
            CameraViewConfig view_config
            {
                Point3(0.0, 150.0, 250.0),
                Point3(0.0, 0.0, 0.0),
                Vec3(0.0, 1.0, 0.0),
                45.0, 0.0, 10.0
            };
            render_context.camera = Camera(view_config, render_config);

            static constexpr int NUM_SPHERES = 10000;
            for (int i = 0; i < NUM_SPHERES; i++)
            {
                const double radius = RandomPositionDouble(0.3, 1.0);
                const Point3 position(RandomPositionDouble(-100.0, 100.0), RandomPositionDouble(-100.0, 100.0), RandomPositionDouble(-100.0, 100.0));
                Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                render_context.scene.Add(render_context.arena.Create<Sphere>(position, radius, material));
            }

            static constexpr int NUM_BOXES = 2000;
            for (int i = 0; i < NUM_BOXES; i++)
            {
                const double x = RandomPositionDouble(-100.0, 100.0);
                const double y = RandomPositionDouble(-100.0, 100.0);
                const double z = RandomPositionDouble(-100.0, 100.0);
                const double width = RandomPositionDouble(0.3, 1.5);
                const double height = RandomPositionDouble(0.3, 1.5);
                const double depth = RandomPositionDouble(0.3, 1.5);
                Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                render_context.scene.Add(render_context.arena.Create<AxisAlignedBox>(Point3(x, y, z), Point3(x + width, y + height, z + depth), material));
            }
            break;
        }
        case 10:
        {
            // Overlapping box city: boxes and spheres with a lot of overlap
            CameraViewConfig view_config
            {
                Point3(0.0, 30.0, 50.0),
                Point3(0.0, 8.0, 0.0),
                Vec3(0.0, 1.0, 0.0),
                50.0, 0.0, 10.0
            };
            render_context.camera = Camera(view_config, render_config);

            Texture* ground_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.3, 0.3, 0.3));
            Material* ground_material = render_context.arena.Create<LambertianMaterial>(ground_texture);
            render_context.scene.Add(render_context.arena.Create<AxisAlignedBox>(Point3(-35.0, -1.0, -35.0), Point3(35.0, 0.0, 35.0), ground_material));

            static constexpr int NUM_TOWERS = 1000;
            for (int i = 0; i < NUM_TOWERS; i++)
            {
                const double x = RandomPositionDouble(-30.0, 30.0);
                const double z = RandomPositionDouble(-30.0, 30.0);
                const double w = RandomPositionDouble(0.5, 2.0);
                const double d = RandomPositionDouble(0.5, 2.0);
                const double h = RandomPositionDouble(3.0, 20.0);
                Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                render_context.scene.Add(render_context.arena.Create<AxisAlignedBox>(Point3(x, 0.0, z), Point3(x + w, h, z + d), material));
            }

            static constexpr int NUM_PLATFORMS = 1000;
            for (int i = 0; i < NUM_PLATFORMS; i++)
            {
                const double x = RandomPositionDouble(-30.0, 30.0);
                const double z = RandomPositionDouble(-30.0, 30.0);
                const double y = RandomPositionDouble(1.0, 15.0);
                const double w = RandomPositionDouble(2.0, 8.0);
                const double d = RandomPositionDouble(2.0, 8.0);
                const double h = RandomPositionDouble(0.2, 0.5);
                Material* material = render_context.arena.Create<MetalMaterial>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()), RandomPositionDouble(0.0, 0.5));
                render_context.scene.Add(render_context.arena.Create<AxisAlignedBox>(Point3(x, y, z), Point3(x + w, y + h, z + d), material));
            }

            static constexpr int NUM_SPHERES = 500;
            for (int i = 0; i < NUM_SPHERES; i++)
            {
                const double radius = RandomPositionDouble(0.3, 1.0);
                const Point3 position(RandomPositionDouble(-30.0, 30.0), RandomPositionDouble(0.3, 15.0), RandomPositionDouble(-30.0, 30.0));
                Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomColourDouble(), RandomColourDouble(), RandomColourDouble()));
                Material* material = render_context.arena.Create<LambertianMaterial>(texture);
                render_context.scene.Add(render_context.arena.Create<Sphere>(position, radius, material));
            }
            break;
        }
        default:
        {
            // Default to scene 1
            SetupScene(render_context, render_config, 1);
            return;
        }
    }
}

void RenderScene(const CameraRenderConfig& render_config, int scene_number, AccelerationStructure acceleration_structure, uint32_t colour_seed, uint32_t position_seed)
{
    RenderContext ctx;
    SetupScene(ctx, render_config, scene_number, colour_seed, position_seed);
    RenderWithAccelerationStructure(ctx.camera, ctx.scene, ctx.scene_config, acceleration_structure);
}

RenderContext CreateAsyncRenderContext(
    const CameraRenderConfig& render_config,
    int scene_number,
    AccelerationStructure acceleration_structure,
    uint32_t colour_seed,
    uint32_t position_seed)
{
    RenderContext ctx;
    SetupScene(ctx, render_config, scene_number, colour_seed, position_seed);

    switch (acceleration_structure)
    {
    case AccelerationStructure::NONE:
        ctx.output_image_name = "render_none.png";
        break;
    case AccelerationStructure::UNIFORM_GRID:
        ctx.output_image_name = "render_uniform_grid.png";
        break;
    case AccelerationStructure::HIERARCHICAL_UNIFORM_GRID:
        ctx.output_image_name = "render_hierarchical_uniform_grid.png";
        break;
    case AccelerationStructure::OCTREE:
        ctx.output_image_name = "render_octree.png";
        break;
    case AccelerationStructure::BSP_TREE:
        ctx.output_image_name = "render_bsp_tree.png";
        break;
    case AccelerationStructure::K_D_TREE:
        ctx.output_image_name = "render_k_d_tree.png";
        break;
    case AccelerationStructure::BOUNDING_VOLUME_HIERARCHY:
        ctx.output_image_name = "render_bounding_volume_hierarchy.png";
        break;
    }
    ctx.acceleration_structure = acceleration_structure;
    ctx.total_rows.store(render_config.image_height, std::memory_order_relaxed);

    return ctx;
}

bool ExecuteAsyncRender(RenderContext& context)
{
    Timer timer;

    auto do_render = [&](const IRayHittable& hittable) -> bool
    {
        timer.Start();
        bool completed = context.camera.RenderAsync(
            hittable,
            context.scene_config,
            context.cancel_requested,
            &context.num_completed_rows,
            context.output_image_name,
            &context.traversal_stats
        );
        timer.Stop();
        context.render_time_ms = timer.ElapsedMilliseconds();
        return completed;
    };

    bool completed = false;

    switch (context.acceleration_structure)
    {
        case AccelerationStructure::NONE:
        {
            context.construction_time_ms = 0.0;
            context.memory_used_bytes = 0;
            completed = do_render(context.scene);
            break;
        }
        case AccelerationStructure::UNIFORM_GRID:
        {
            timer.Start();
            UniformGrid accel(context.scene.GetObjects());
            timer.Stop();
            context.construction_time_ms = timer.ElapsedMilliseconds();
            context.memory_used_bytes = accel.MemoryUsedBytes();
            completed = do_render(accel);
            break;
        }
        case AccelerationStructure::HIERARCHICAL_UNIFORM_GRID:
        {
            timer.Start();
            HierarchicalUniformGrid accel(context.scene.GetObjects());
            timer.Stop();
            context.construction_time_ms = timer.ElapsedMilliseconds();
            context.memory_used_bytes = accel.MemoryUsedBytes();
            completed = do_render(accel);
            break;
        }
        case AccelerationStructure::OCTREE:
        {
            timer.Start();
            OctreeNode accel(context.scene.GetObjects());
            timer.Stop();
            context.construction_time_ms = timer.ElapsedMilliseconds();
            context.memory_used_bytes = accel.MemoryUsedBytes();
            completed = do_render(accel);
            break;
        }
        case AccelerationStructure::BSP_TREE:
        {
            timer.Start();
            BSPTreeNode accel(context.scene.GetObjects());
            timer.Stop();
            context.construction_time_ms = timer.ElapsedMilliseconds();
            context.memory_used_bytes = accel.MemoryUsedBytes();
            completed = do_render(accel);
            break;
        }
        case AccelerationStructure::K_D_TREE:
        {
            timer.Start();
            KDTreeNode accel(context.scene.GetObjects());
            timer.Stop();
            context.construction_time_ms = timer.ElapsedMilliseconds();
            context.memory_used_bytes = accel.MemoryUsedBytes();
            completed = do_render(accel);
            break;
        }
        case AccelerationStructure::BOUNDING_VOLUME_HIERARCHY:
        {
            timer.Start();
            BVHNode accel(context.scene.GetObjects());
            timer.Stop();
            context.construction_time_ms = timer.ElapsedMilliseconds();
            context.memory_used_bytes = accel.MemoryUsedBytes();
            completed = do_render(accel);
            break;
        }
    }

    context.was_cancelled.store(!completed, std::memory_order_relaxed);
    context.render_complete.store(true, std::memory_order_release);

    // Log stats if render completed successfully
    if (completed)
    {
        RenderStats stats;
        stats.m_acceleration_structure = context.acceleration_structure;
        stats.m_construction_time_ms = context.construction_time_ms;
        stats.m_render_time_ms = context.render_time_ms;
        stats.m_memory_used_bytes = context.memory_used_bytes;
        stats.m_traversal_stats = context.traversal_stats;
        LogRenderStats(stats);
    }

    return completed;
}

} // namespace ART
