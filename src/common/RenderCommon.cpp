// Copyright Mia Rolfe. All rights reserved.
#include <RenderCommon.h>

#include <Random.h>

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
{
    other.num_completed_rows.store(0);
    other.total_rows.store(0);
    other.cancel_requested.store(false);
    other.render_complete.store(false);
    other.was_cancelled.store(false);
    other.construction_time_ms = 0.0;
    other.render_time_ms = 0.0;
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

        other.num_completed_rows.store(0);
        other.total_rows.store(0);
        other.cancel_requested.store(false);
        other.render_complete.store(false);
        other.was_cancelled.store(false);
        other.construction_time_ms = 0.0;
        other.render_time_ms = 0.0;
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
        << "Total time: " << stats.TotalTimeMilliseconds() << " ms";
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

            timer.Start();
            camera.Render(scene, scene_config, "render_none.png");
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

            timer.Start();
            camera.Render(uniform_grid, scene_config, "render_uniform_grid.png");
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

            timer.Start();
            camera.Render(hierarchical_uniform_grid, scene_config, "render_hierarchical_uniform_grid.png");
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

            timer.Start();
            camera.Render(octree, scene_config, "render_octree.png");
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

            timer.Start();
            camera.Render(bsp_tree, scene_config, "render_bsp_tree.png");
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

            timer.Start();
            camera.Render(hierarchical_uniform_grid, scene_config, "render_k_d_tree.png");
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

            timer.Start();
            camera.Render(bounding_volume_hierarchy, scene_config, "render_bounding_volume_hierarchy.png");
            timer.Stop();
            stats.m_render_time_ms = timer.ElapsedMilliseconds();
            break;
        }
    }

    LogRenderStats(stats);
    return stats;
}

void SetupScene(RenderContext& render_context, const CameraRenderConfig& render_config, int scene_number)
{
    render_context.scene_config = SceneConfig{Colour(0.7, 0.8, 1.0)};

    switch (scene_number)
    {
        case 1:
        {
            CameraViewConfig view_config
            {
                Point3(0, 0.0, 0.0),
                Point3(0.0, 0.0, 10.0),
                Vec3(0.0, 1.0, 0.0),
                90.0, 0.0, 10.0
            };
            render_context.camera = Camera(view_config, render_config);

            Texture* even_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.2, 0.3, 0.1));
            Texture* odd_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.9, 0.9, 0.9));
            Texture* checker_texture = render_context.arena.Create<CheckerTexture>(0.32, even_texture, odd_texture);
            Material* checker_material = render_context.arena.Create<LambertianMaterial>(checker_texture);
            Material* metal_material = render_context.arena.Create<MetalMaterial>(Colour(0.7), 0.5);
            Material* dielectric_material = render_context.arena.Create<DielectricMaterial>(0.5);

            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(-7.5, 0.0, 10.0), 5.0, checker_material));
            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(0.0, 0.0, 1.0), 0.5, dielectric_material));
            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(7.5, 0.0, 10.0), 2.5, metal_material));
            break;
        }
        case 2:
        {
            CameraViewConfig view_config
            {
                Point3(13.0, 2.0, 3.0),
                Point3(0.0, 0.0, 0.0),
                Vec3(0.0, 1.0, 0.0),
                20.0, 0.0, 10.0
            };
            render_context.camera = Camera(view_config, render_config);

            Texture* even_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.2, 0.3, 0.1));
            Texture* odd_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.9, 0.9, 0.9));
            Texture* checker_texture = render_context.arena.Create<CheckerTexture>(0.32, even_texture, odd_texture);
            Material* checker_material = render_context.arena.Create<LambertianMaterial>(checker_texture);

            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(0.0, -10.0, 0.0), 10.0, checker_material));
            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(0.0, 10.0, 0.0), 10.0, checker_material));
            break;
        }
        case 3:
        {
            CameraRenderConfig scene3_config = render_config;
            scene3_config.max_ray_bounces = 50;

            CameraViewConfig view_config
            {
                Point3(13.0, 2.0, 3.0),
                Point3(0.0, 0.0, 0.0),
                Vec3(0.0, 1.0, 0.0),
                20.0, 0.0, 10.0
            };
            render_context.camera = Camera(view_config, scene3_config);

            Texture* even_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.0, 0.0, 0.0));
            Texture* odd_texture = render_context.arena.Create<SolidColourTexture>(Colour(1.0, 1.0, 1.0));
            Texture* checker_texture = render_context.arena.Create<CheckerTexture>(0.5, even_texture, odd_texture);
            Texture* cyan_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.0, 0.8, 0.8));

            Material* cyan_material = render_context.arena.Create<LambertianMaterial>(cyan_texture);
            Material* checker_material = render_context.arena.Create<LambertianMaterial>(checker_texture);
            Material* glass_material = render_context.arena.Create<DielectricMaterial>(1.5);
            Material* glass_inner_material = render_context.arena.Create<DielectricMaterial>(1.0 / 1.5);

            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(0.0, -1005.0, 0.0), 1000.0, cyan_material));
            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(0.0, -10.0, 0.0), 10.0, checker_material));
            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(0.0, 1.0, 0.0), 1.0, glass_material));
            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(0.0, 1.0, 0.0), 0.8, glass_inner_material));
            break;
        }
        case 4:
        {
            CameraViewConfig view_config
            {
                Point3(13.0, 2.0, 3.0),
                Point3(0.0, 0.0, 0.0),
                Vec3(0.0, 1.0, 0.0),
                20.0, 0.0, 10.0
            };
            render_context.camera = Camera(view_config, render_config);

            Texture* even_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.0, 0.0, 0.0));
            Texture* odd_texture = render_context.arena.Create<SolidColourTexture>(Colour(1.0, 1.0, 1.0));
            Texture* checker_texture = render_context.arena.Create<CheckerTexture>(0.5, even_texture, odd_texture);

            Material* checker_material = render_context.arena.Create<LambertianMaterial>(checker_texture);
            Material* glass_material = render_context.arena.Create<DielectricMaterial>(1.5);
            Material* glass_inner_material = render_context.arena.Create<DielectricMaterial>(1.0 / 1.5);

            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(0.0, -10.0, 0.0), 10.0, checker_material));
            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(0.0, 1.0, 0.0), 1.0, glass_material));
            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(0.0, 1.0, 0.0), 0.8, glass_inner_material));
            break;
        }
        case 5:
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

                        Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomCanonicalDouble(), RandomCanonicalDouble(), RandomCanonicalDouble()));
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
                        Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomCanonicalDouble(), RandomCanonicalDouble(), RandomCanonicalDouble()));
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
                        Texture* texture = render_context.arena.Create<SolidColourTexture>(Colour(RandomCanonicalDouble(), RandomCanonicalDouble(), RandomCanonicalDouble()));
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
        case 6:
        default:
        {
            CameraViewConfig view_config
            {
                Point3(5.0, 4.0, 8.0),
                Point3(0.0, 0.5, 0.0),
                Vec3(0.0, 1.0, 0.0),
                40.0, 0.0, 10.0
            };
            render_context.camera = Camera(view_config, render_config);

            // Ground
            Texture* ground_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.4, 0.4, 0.4));
            Material* ground_material = render_context.arena.Create<LambertianMaterial>(ground_texture);
            render_context.scene.Add(render_context.arena.Create<AxisAlignedBox>(Point3(-5.0, -0.5, -5.0), Point3(5.0, 0.0, 5.0), ground_material));

            // Red box
            Texture* red_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.8, 0.2, 0.2));
            Material* red_material = render_context.arena.Create<LambertianMaterial>(red_texture);
            render_context.scene.Add(render_context.arena.Create<AxisAlignedBox>(Point3(-1.5, 0.0, -0.5), Point3(-0.5, 1.0, 0.5), red_material));

            // Green metallic box
            Material* green_metal = render_context.arena.Create<MetalMaterial>(Colour(0.3, 0.8, 0.3), 0.1);
            render_context.scene.Add(render_context.arena.Create<AxisAlignedBox>(Point3(0.5, 0.0, -0.5), Point3(1.5, 1.5, 0.5), green_metal));

            // Glass sphere
            Material* glass_material = render_context.arena.Create<DielectricMaterial>(1.5);
            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(-1.0, 1.35, 0.0), 0.35, glass_material));

            // Silver metallic sphere
            Material* silver_metal = render_context.arena.Create<MetalMaterial>(Colour(0.9, 0.9, 0.9), 0.0);
            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(0.0, 0.4, 1.5), 0.4, silver_metal));

            // Blue sphere
            Texture* blue_texture = render_context.arena.Create<SolidColourTexture>(Colour(0.2, 0.2, 0.8));
            Material* blue_material = render_context.arena.Create<LambertianMaterial>(blue_texture);
            render_context.scene.Add(render_context.arena.Create<Sphere>(Point3(-2.0, 0.3, 1.0), 0.3, blue_material));

            // Small gold metallic box
            Material* gold_metal = render_context.arena.Create<MetalMaterial>(Colour(0.8, 0.6, 0.2), 0.3);
            render_context.scene.Add(render_context.arena.Create<AxisAlignedBox>(Point3(1.8, 0.0, 1.0), Point3(2.3, 0.5, 1.5), gold_metal));
            break;
        }
    }
}

void RenderScene(const CameraRenderConfig& render_config, int scene_number, AccelerationStructure acceleration_structure)
{
    RenderContext ctx;
    SetupScene(ctx, render_config, scene_number);
    RenderWithAccelerationStructure(ctx.camera, ctx.scene, ctx.scene_config, acceleration_structure);
}

RenderContext CreateAsyncRenderContext(
    const CameraRenderConfig& render_config,
    int scene_number,
    AccelerationStructure acceleration_structure)
{
    RenderContext ctx;
    SetupScene(ctx, render_config, scene_number);

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
            context.output_image_name
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
            completed = do_render(context.scene);
            break;
        }
        case AccelerationStructure::UNIFORM_GRID:
        {
            timer.Start();
            UniformGrid accel(context.scene.GetObjects());
            timer.Stop();
            context.construction_time_ms = timer.ElapsedMilliseconds();
            completed = do_render(accel);
            break;
        }
        case AccelerationStructure::HIERARCHICAL_UNIFORM_GRID:
        {
            timer.Start();
            HierarchicalUniformGrid accel(context.scene.GetObjects());
            timer.Stop();
            context.construction_time_ms = timer.ElapsedMilliseconds();
            completed = do_render(accel);
            break;
        }
        case AccelerationStructure::OCTREE:
        {
            timer.Start();
            OctreeNode accel(context.scene.GetObjects());
            timer.Stop();
            context.construction_time_ms = timer.ElapsedMilliseconds();
            completed = do_render(accel);
            break;
        }
        case AccelerationStructure::BSP_TREE:
        {
            timer.Start();
            BSPTreeNode accel(context.scene.GetObjects());
            timer.Stop();
            context.construction_time_ms = timer.ElapsedMilliseconds();
            completed = do_render(accel);
            break;
        }
        case AccelerationStructure::K_D_TREE:
        {
            timer.Start();
            KDTreeNode accel(context.scene.GetObjects());
            timer.Stop();
            context.construction_time_ms = timer.ElapsedMilliseconds();
            completed = do_render(accel);
            break;
        }
        case AccelerationStructure::BOUNDING_VOLUME_HIERARCHY:
        {
            timer.Start();
            BVHNode accel(context.scene.GetObjects());
            timer.Stop();
            context.construction_time_ms = timer.ElapsedMilliseconds();
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
        LogRenderStats(stats);
    }

    return completed;
}

} // namespace ART
