// Copyright Mia Rolfe. All rights reserved.
#include <RenderCommon.h>

namespace ART
{

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

void Scene1(const CameraRenderConfig& render_config, AccelerationStructure acceleration_structure)
{
    ArenaAllocator arena(ONE_MEGABYTE);

    CameraViewConfig view_config
    {
        Point3(0, 0.0, 0.0),       // look_from
        Point3(0.0, 0.0, 10.0),    // look_at
        Vec3(0.0, 1.0, 0.0),       // up
        90.0,                           // vertical_fov
        0.0,                            // defocus_angle
        10.0                            // focus_distance
    };
    Camera camera(view_config, render_config);

    SceneConfig scene_config
    {
        Colour(0.7, 0.8, 1.0)      // background_colour
    };

    RayHittableList scene;

    Texture* even_texture = arena.Create<SolidColourTexture>(Colour(0.2, 0.3, 0.1));
    Texture* odd_texture = arena.Create<SolidColourTexture>(Colour(0.9, 0.9, 0.9));
    Texture* checker_texture = arena.Create<CheckerTexture>(0.32, even_texture, odd_texture);
    Material* checker_material = arena.Create<LambertianMaterial>(checker_texture);
    Material* metal_material = arena.Create<MetalMaterial>(Colour(0.7), 0.5);
    Material* dielectric_material = arena.Create<DielectricMaterial>(0.5);

    scene.Add(arena.Create<Sphere>(Point3(-7.5, 0.0, 10.0), 5.0, checker_material));
    scene.Add(arena.Create<Sphere>(Point3(0.0, 0.0, 1.0), 0.5, dielectric_material));
    scene.Add(arena.Create<Sphere>(Point3(7.5, 0.0, 10.0), 2.5, metal_material));

    RenderWithAccelerationStructure(camera, scene, scene_config, acceleration_structure);
}

void Scene2(const CameraRenderConfig& render_config, AccelerationStructure acceleration_structure)
{
    ArenaAllocator arena(ONE_MEGABYTE);

    CameraViewConfig view_config
    {
        Point3(13.0, 2.0, 3.0),    // look_from
        Point3(0.0, 0.0, 0.0),     // look_at
        Vec3(0.0, 1.0, 0.0),       // up
        20.0,                           // vertical_fov
        0.0,                            // defocus_angle
        10.0                            // focus_distance
    };
    Camera camera(view_config, render_config);

    SceneConfig scene_config
    {
        Colour(0.7, 0.8, 1.0)      // background_colour
    };

    RayHittableList scene;

    Texture* even_texture = arena.Create<SolidColourTexture>(Colour(0.2, 0.3, 0.1));
    Texture* odd_texture = arena.Create<SolidColourTexture>(Colour(0.9, 0.9, 0.9));
    Texture* checker_texture = arena.Create<CheckerTexture>(0.32, even_texture, odd_texture);
    Material* checker_material = arena.Create<LambertianMaterial>(checker_texture);

    scene.Add(arena.Create<Sphere>(Point3(0.0, -10.0, 0.0), 10.0, checker_material));
    scene.Add(arena.Create<Sphere>(Point3(0.0, 10.0, 0.0), 10.0, checker_material));

    RenderWithAccelerationStructure(camera, scene, scene_config, acceleration_structure);
}

void Scene3(const CameraRenderConfig& render_config, AccelerationStructure acceleration_structure)
{
    ArenaAllocator arena(ONE_MEGABYTE);

    // Scene 3 uses more ray bounces for glass materials
    CameraRenderConfig scene3_render_config = render_config;
    scene3_render_config.max_ray_bounces = 50;

    CameraViewConfig view_config
    {
        Point3(13.0, 2.0, 3.0),    // look_from
        Point3(0.0, 0.0, 0.0),     // look_at
        Vec3(0.0, 1.0, 0.0),       // up
        20.0,                           // vertical_fov
        0.0,                            // defocus_angle
        10.0                            // focus_distance
    };
    Camera camera(view_config, scene3_render_config);

    SceneConfig scene_config
    {
        Colour(0.7, 0.8, 1.0)      // background_colour
    };

    RayHittableList scene;

    Texture* even_texture = arena.Create<SolidColourTexture>(Colour(0.0, 0.0, 0.0));
    Texture* odd_texture = arena.Create<SolidColourTexture>(Colour(1.0, 1.0, 1.0));
    Texture* checker_texture = arena.Create<CheckerTexture>(0.5, even_texture, odd_texture);

    Texture* cyan_texture = arena.Create<SolidColourTexture>(Colour(0.0, 0.8, 0.8));
    Material* cyan_material = arena.Create<LambertianMaterial>(cyan_texture);
    Material* checker_material = arena.Create<LambertianMaterial>(checker_texture);
    Material* glass_material = arena.Create<DielectricMaterial>(1.5);
    Material* glass_inner_material = arena.Create<DielectricMaterial>(1.0 / 1.5);

    scene.Add(arena.Create<Sphere>(Point3(0.0, -1005.0, 0.0), 1000.0, cyan_material));
    scene.Add(arena.Create<Sphere>(Point3(0.0, -10.0, 0.0), 10.0, checker_material));
    scene.Add(arena.Create<Sphere>(Point3(0.0, 1.0, 0.0), 1.0, glass_material));
    scene.Add(arena.Create<Sphere>(Point3(0.0, 1.0, 0.0), 0.8, glass_inner_material));

    RenderWithAccelerationStructure(camera, scene, scene_config, acceleration_structure);
}

void Scene4(const CameraRenderConfig& render_config, AccelerationStructure acceleration_structure)
{
    ArenaAllocator arena(ONE_MEGABYTE);

    CameraViewConfig view_config
    {
        Point3(13.0, 2.0, 3.0),    // look_from
        Point3(0.0, 0.0, 0.0),     // look_at
        Vec3(0.0, 1.0, 0.0),       // up
        20.0,                           // vertical_fov
        0.0,                            // defocus_angle
        10.0                            // focus_distance
    };
    Camera camera(view_config, render_config);

    SceneConfig scene_config
    {
        Colour(0.7, 0.8, 1.0)      // background_colour
    };

    RayHittableList scene;

    Texture* even_texture = arena.Create<SolidColourTexture>(Colour(0.0, 0.0, 0.0));
    Texture* odd_texture = arena.Create<SolidColourTexture>(Colour(1.0, 1.0, 1.0));
    Texture* checker_texture = arena.Create<CheckerTexture>(0.5, even_texture, odd_texture);
    Material* checker_material = arena.Create<LambertianMaterial>(checker_texture);
    Material* glass_material = arena.Create<DielectricMaterial>(1.5);
    Material* glass_inner_material = arena.Create<DielectricMaterial>(1.0 / 1.5);

    scene.Add(arena.Create<Sphere>(Point3(0.0, -10.0, 0.0), 10.0, checker_material));
    scene.Add(arena.Create<Sphere>(Point3(0.0, 1.0, 0.0), 1.0, glass_material));
    scene.Add(arena.Create<Sphere>(Point3(0.0, 1.0, 0.0), 0.8, glass_inner_material));

    RenderWithAccelerationStructure(camera, scene, scene_config, acceleration_structure);
}

void Scene5(const CameraRenderConfig& render_config, AccelerationStructure acceleration_structure)
{
    ArenaAllocator arena(ONE_MEGABYTE);

    RayHittableList scene;

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

                Texture* texture = arena.Create<SolidColourTexture>(Colour(RandomCanonicalDouble(), RandomCanonicalDouble(), RandomCanonicalDouble()));
                Material* material = arena.Create<LambertianMaterial>(texture);
                scene.Add(arena.Create<Sphere>(sphere_position, 1.0, material));
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
                Texture* texture = arena.Create<SolidColourTexture>(Colour(RandomCanonicalDouble(), RandomCanonicalDouble(), RandomCanonicalDouble()));
                Material* material = arena.Create<LambertianMaterial>(texture);
                scene.Add(arena.Create<Sphere>(sphere_position, 1.0, material));
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
                Texture* texture = arena.Create<SolidColourTexture>(Colour(RandomCanonicalDouble(), RandomCanonicalDouble(), RandomCanonicalDouble()));
                Material* material = arena.Create<LambertianMaterial>(texture);
                scene.Add(arena.Create<Sphere>(sphere_position, 1.0, material));
            }
        }
    }

    assert(num_spheres_cluster_1 > 0);
    average_position_cluster_1 /= num_spheres_cluster_1;

    CameraViewConfig view_config
    {
        Point3(-100.0, 100.0, 100.0),  // look_from
        average_position_cluster_1,          // look_at
        Vec3(0.0, 1.0, 0.0),            // up
        18.0,                                // vertical_fov
        0.0,                                 // defocus_angle
        10.0                                 // focus_distance
    };
    Camera camera(view_config, render_config);

    SceneConfig scene_config
    {
        Colour(0.7, 0.8, 1.0)      // background_colour
    };

    RenderWithAccelerationStructure(camera, scene, scene_config, acceleration_structure);
}

// Simple test scenario with low number of objects
void Scene6(const CameraRenderConfig& render_config, AccelerationStructure acceleration_structure)
{
    ArenaAllocator arena(ONE_MEGABYTE);

    CameraViewConfig view_config
    {
        Point3(5.0, 4.0, 8.0),
        Point3(0.0, 0.5, 0.0),
        Vec3(0.0, 1.0, 0.0),
        40.0,
        0.0,
        10.0
    };
    Camera camera(view_config, render_config);

    SceneConfig scene_config
    {
        Colour(0.7, 0.8, 1.0)
    };

    RayHittableList scene;

    // Ground
    Texture* ground_texture = arena.Create<SolidColourTexture>(Colour(0.4, 0.4, 0.4));
    Material* ground_material = arena.Create<LambertianMaterial>(ground_texture);
    scene.Add(arena.Create<AxisAlignedBox>(Point3(-5.0, -0.5, -5.0), Point3(5.0, 0.0, 5.0), ground_material));

    // Red box
    Texture* red_texture = arena.Create<SolidColourTexture>(Colour(0.8, 0.2, 0.2));
    Material* red_material = arena.Create<LambertianMaterial>(red_texture);
    scene.Add(arena.Create<AxisAlignedBox>(Point3(-1.5, 0.0, -0.5), Point3(-0.5, 1.0, 0.5), red_material));

    // Green metallic box
    Material* green_metal = arena.Create<MetalMaterial>(Colour(0.3, 0.8, 0.3), 0.1);
    scene.Add(arena.Create<AxisAlignedBox>(Point3(0.5, 0.0, -0.5), Point3(1.5, 1.5, 0.5), green_metal));

    // Glass sphere
    Material* glass_material = arena.Create<DielectricMaterial>(1.5);
    scene.Add(arena.Create<Sphere>(Point3(-1.0, 1.35, 0.0), 0.35, glass_material));

    // Silver metallic sphere
    Material* silver_metal = arena.Create<MetalMaterial>(Colour(0.9, 0.9, 0.9), 0.0);
    scene.Add(arena.Create<Sphere>(Point3(0.0, 0.4, 1.5), 0.4, silver_metal));

    // Blue sphere
    Texture* blue_texture = arena.Create<SolidColourTexture>(Colour(0.2, 0.2, 0.8));
    Material* blue_material = arena.Create<LambertianMaterial>(blue_texture);
    scene.Add(arena.Create<Sphere>(Point3(-2.0, 0.3, 1.0), 0.3, blue_material));

    // Small gold metallic box
    Material* gold_metal = arena.Create<MetalMaterial>(Colour(0.8, 0.6, 0.2), 0.3);
    scene.Add(arena.Create<AxisAlignedBox>(Point3(1.8, 0.0, 1.0), Point3(2.3, 0.5, 1.5), gold_metal));

    RenderWithAccelerationStructure(camera, scene, scene_config, acceleration_structure);
}

void RenderScene(const CameraRenderConfig& render_config, int scene_number, AccelerationStructure acceleration_structure)
{
    switch (scene_number)
    {
        case 1: Scene1(render_config, acceleration_structure); break;
        case 2: Scene2(render_config, acceleration_structure); break;
        case 3: Scene3(render_config, acceleration_structure); break;
        case 4: Scene4(render_config, acceleration_structure); break;
        case 5: Scene5(render_config, acceleration_structure); break;
        case 6: Scene6(render_config, acceleration_structure); break;
    }
}

} // namespace ART
