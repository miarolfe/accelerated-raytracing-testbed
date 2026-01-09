// Copyright Mia Rolfe. All rights reserved.
#include "HierarchicalUniformGrid.h"

#include "IRayHittable.h"
#include "RayHitResult.h"
#include "UniformGrid.h"

namespace ART
{

HierarchicalUniformGrid::HierarchicalUniformGrid(std::vector<IRayHittable*>& objects)
    : m_is_grid_valid(false), m_grid(nullptr), m_num_x_cells(0), m_num_y_cells(0), m_num_z_cells(0)
{
    for (std::size_t object_index = 0; object_index < objects.size(); object_index++)
    {
        m_bounding_box = AABB(m_bounding_box, objects[object_index]->BoundingBox());
    }

    Create(objects);
}

HierarchicalUniformGrid::~HierarchicalUniformGrid()
{
    Destroy();
}

bool HierarchicalUniformGrid::Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const
{
    if (!m_grid || !m_is_grid_valid)
    {
        return false;
    }

    if (!m_bounding_box.Hit(ray, ray_t))
    {
        return false;
    }

    // Starting point inside the bounding box
    const Vec3 entry_point = ray.At(ray_t.m_min);
    Vec3Int current_cell = Calculate3DIndex(entry_point);

    // Clamp to grid
    current_cell.m_x = std::clamp(current_cell.m_x, 0, static_cast<int>(m_num_x_cells) - 1);
    current_cell.m_y = std::clamp(current_cell.m_y, 0, static_cast<int>(m_num_y_cells) - 1);
    current_cell.m_z = std::clamp(current_cell.m_z, 0, static_cast<int>(m_num_z_cells) - 1);

    // Ray direction signs
    const int step_x = (ray.m_direction.m_x >= 0) ? 1 : -1;
    const int step_y = (ray.m_direction.m_y >= 0) ? 1 : -1;
    const int step_z = (ray.m_direction.m_z >= 0) ? 1 : -1;

    // Distance along ray to next cell boundary
    Vec3 t_max;

    if (step_x > 0)
    {
        t_max.m_x = (m_bounding_box.m_x.m_min + (current_cell.m_x + 1) * m_cell_size.m_x - ray.m_origin.m_x) / ray.m_direction.m_x;
    }
    else
    {
        t_max.m_x = (m_bounding_box.m_x.m_min + current_cell.m_x * m_cell_size.m_x - ray.m_origin.m_x) / ray.m_direction.m_x;
    }

    if (step_y > 0)
    {
        t_max.m_y = (m_bounding_box.m_y.m_min + (current_cell.m_y + 1) * m_cell_size.m_y - ray.m_origin.m_y) / ray.m_direction.m_y;
    }
    else
    {
        t_max.m_y = (m_bounding_box.m_y.m_min + current_cell.m_y * m_cell_size.m_y - ray.m_origin.m_y) / ray.m_direction.m_y;
    }

    if (step_z > 0)
    {
        t_max.m_z = (m_bounding_box.m_z.m_min + (current_cell.m_z + 1) * m_cell_size.m_z - ray.m_origin.m_z) / ray.m_direction.m_z;
    }
    else
    {
        t_max.m_z = (m_bounding_box.m_z.m_min + current_cell.m_z * m_cell_size.m_z - ray.m_origin.m_z) / ray.m_direction.m_z;
    }

    // Distance along ray to cross one full cell
    const Vec3 t_delta
    (
        std::abs(m_cell_size.m_x / ray.m_direction.m_x),
        std::abs(m_cell_size.m_y / ray.m_direction.m_y),
        std::abs(m_cell_size.m_z / ray.m_direction.m_z)
    );

    bool hit_anything = false;
    double closest_t = ray_t.m_max;
    RayHitResult temp_result;

    // 3DDDA (Amanatides & Woo)
    while
    (
        current_cell.m_x >= 0 && current_cell.m_x < static_cast<int>(m_num_x_cells) &&
        current_cell.m_y >= 0 && current_cell.m_y < static_cast<int>(m_num_y_cells) &&
        current_cell.m_z >= 0 && current_cell.m_z < static_cast<int>(m_num_z_cells)
    )
    {
        const std::size_t cell_index = Calculate1DIndex(current_cell);
        if (CellHit(m_grid[cell_index], ray, Interval(ray_t.m_min, closest_t), temp_result))
        {
            hit_anything = true;
            closest_t = temp_result.m_t;
            out_result = temp_result;
        }

        // Step to next cell
        if (t_max.m_x < t_max.m_y)
        {
            if (t_max.m_x < t_max.m_z)
            {
                current_cell.m_x += step_x;
                t_max.m_x += t_delta.m_x;
            }
            else
            {
                current_cell.m_z += step_z;
                t_max.m_z += t_delta.m_z;
            }
        }
        else
        {
            if (t_max.m_y < t_max.m_z)
            {
                current_cell.m_y += step_y;
                t_max.m_y += t_delta.m_y;
            }
            else
            {
                current_cell.m_z += step_z;
                t_max.m_z += t_delta.m_z;
            }
        }

        // Stop if closest hit is before next cell boundary
        if (hit_anything && closest_t < std::min(t_max.m_x, std::min(t_max.m_y, t_max.m_z)))
        {
            break;
        }
    }

    return hit_anything;
}

AABB HierarchicalUniformGrid::BoundingBox() const
{
    return m_bounding_box;
}

void HierarchicalUniformGrid::Create(std::vector<IRayHittable*>& objects)
{
    m_cell_size = DetermineCellSize(objects.size());

    m_num_x_cells = std::max(static_cast<std::size_t>(1), static_cast<std::size_t>(std::round(m_bounding_box.m_x.Size() / m_cell_size.m_x)));
    m_num_y_cells = std::max(static_cast<std::size_t>(1), static_cast<std::size_t>(std::round(m_bounding_box.m_y.Size() / m_cell_size.m_y)));
    m_num_z_cells = std::max(static_cast<std::size_t>(1), static_cast<std::size_t>(std::round(m_bounding_box.m_z.Size() / m_cell_size.m_z)));

    const std::size_t num_cells = m_num_x_cells * m_num_y_cells * m_num_z_cells;
    m_grid = new HierarchicalUniformGridEntry[num_cells];

    // Count objects per cell first to allocate exact sizes
    std::size_t* objects_per_cell_count = new std::size_t[num_cells]();

    for (std::size_t object_index = 0; object_index < objects.size(); object_index++)
    {
        const AABB bounding_box = objects[object_index]->BoundingBox();

        const Vec3 min_bound = Vec3(bounding_box.m_x.m_min, bounding_box.m_y.m_min, bounding_box.m_z.m_min);
        const Vec3 max_bound = Vec3(bounding_box.m_x.m_max, bounding_box.m_y.m_max, bounding_box.m_z.m_max);

        Vec3Int min_bound_grid_index = Calculate3DIndex(min_bound);
        Vec3Int max_bound_grid_index = Calculate3DIndex(max_bound);

        // Clamp indices to valid grid bounds
        min_bound_grid_index.m_x = std::clamp(min_bound_grid_index.m_x, 0, static_cast<int>(m_num_x_cells) - 1);
        min_bound_grid_index.m_y = std::clamp(min_bound_grid_index.m_y, 0, static_cast<int>(m_num_y_cells) - 1);
        min_bound_grid_index.m_z = std::clamp(min_bound_grid_index.m_z, 0, static_cast<int>(m_num_z_cells) - 1);
        max_bound_grid_index.m_x = std::clamp(max_bound_grid_index.m_x, 0, static_cast<int>(m_num_x_cells) - 1);
        max_bound_grid_index.m_y = std::clamp(max_bound_grid_index.m_y, 0, static_cast<int>(m_num_y_cells) - 1);
        max_bound_grid_index.m_z = std::clamp(max_bound_grid_index.m_z, 0, static_cast<int>(m_num_z_cells) - 1);

        for (int i = min_bound_grid_index.m_x; i <= max_bound_grid_index.m_x; i++)
        {
            for (int j = min_bound_grid_index.m_y; j <= max_bound_grid_index.m_y; j++)
            {
                for (int k = min_bound_grid_index.m_z; k <= max_bound_grid_index.m_z; k++)
                {
                    const std::size_t one_dimensional_index = Calculate1DIndex(Vec3Int(i, j, k));
                    objects_per_cell_count[one_dimensional_index]++;
                }
            }
        }
    }

    // Allocate arrays for each cell
    IRayHittable*** objects_for_each_cell = new IRayHittable**[num_cells];
    std::size_t* cell_fill_indices = new std::size_t[num_cells]();

    for (std::size_t i = 0; i < num_cells; i++)
    {
        if (objects_per_cell_count[i] > 0)
        {
            objects_for_each_cell[i] = new IRayHittable*[objects_per_cell_count[i]];
        }
        else
        {
            objects_for_each_cell[i] = nullptr;
        }
    }

    // Distribute objects to cells
    for (std::size_t object_index = 0; object_index < objects.size(); object_index++)
    {
        const AABB bounding_box = objects[object_index]->BoundingBox();

        const Vec3 min_bound = Vec3(bounding_box.m_x.m_min, bounding_box.m_y.m_min, bounding_box.m_z.m_min);
        const Vec3 max_bound = Vec3(bounding_box.m_x.m_max, bounding_box.m_y.m_max, bounding_box.m_z.m_max);

        Vec3Int min_bound_grid_index = Calculate3DIndex(min_bound);
        Vec3Int max_bound_grid_index = Calculate3DIndex(max_bound);

        // Clamp indices to valid grid bounds
        min_bound_grid_index.m_x = std::clamp(min_bound_grid_index.m_x, 0, static_cast<int>(m_num_x_cells) - 1);
        min_bound_grid_index.m_y = std::clamp(min_bound_grid_index.m_y, 0, static_cast<int>(m_num_y_cells) - 1);
        min_bound_grid_index.m_z = std::clamp(min_bound_grid_index.m_z, 0, static_cast<int>(m_num_z_cells) - 1);
        max_bound_grid_index.m_x = std::clamp(max_bound_grid_index.m_x, 0, static_cast<int>(m_num_x_cells) - 1);
        max_bound_grid_index.m_y = std::clamp(max_bound_grid_index.m_y, 0, static_cast<int>(m_num_y_cells) - 1);
        max_bound_grid_index.m_z = std::clamp(max_bound_grid_index.m_z, 0, static_cast<int>(m_num_z_cells) - 1);

        for (int i = min_bound_grid_index.m_x; i <= max_bound_grid_index.m_x; i++)
        {
            for (int j = min_bound_grid_index.m_y; j <= max_bound_grid_index.m_y; j++)
            {
                for (int k = min_bound_grid_index.m_z; k <= max_bound_grid_index.m_z; k++)
                {
                    const std::size_t one_dimensional_index = Calculate1DIndex(Vec3Int(i, j, k));
                    objects_for_each_cell[one_dimensional_index][cell_fill_indices[one_dimensional_index]++] = objects[object_index];
                }
            }
        }
    }

    // Create subgrids for occupied cells
    for (std::size_t i = 0; i < num_cells; i++)
    {
        if (objects_per_cell_count[i] > 0)
        {
            std::vector<IRayHittable*> objects_vec(objects_for_each_cell[i], objects_for_each_cell[i] + objects_per_cell_count[i]);
            m_grid[i].subgrid = new UniformGrid(objects_vec);
            delete[] objects_for_each_cell[i];
        }
    }

    // Clean up temporary arrays
    delete[] objects_for_each_cell;
    delete[] objects_per_cell_count;
    delete[] cell_fill_indices;

    m_is_grid_valid = true;
}

void HierarchicalUniformGrid::Destroy()
{
    if (m_grid && m_is_grid_valid)
    {
        const std::size_t total_cells = m_num_x_cells * m_num_y_cells * m_num_z_cells;

        // Destroy subgrids
        for (std::size_t i = 0; i < total_cells; i++)
        {
            if (m_grid[i].subgrid != nullptr)
            {
                delete m_grid[i].subgrid;
                m_grid[i].subgrid = nullptr;
            }
        }

        delete[] m_grid;
        m_grid = nullptr;
    }

    m_bounding_box = AABB();
    m_cell_size = Vec3(0.0);
    m_num_x_cells = 0;
    m_num_y_cells = 0;
    m_num_z_cells = 0;
    m_is_grid_valid = false;
}

bool HierarchicalUniformGrid::CellHit(const HierarchicalUniformGridEntry& entry, const Ray& ray, Interval ray_t, RayHitResult& out_result) const
{
    if (entry.subgrid == nullptr)
    {
        return false;
    }

    return entry.subgrid->Hit(ray, ray_t, out_result);
}

Vec3 HierarchicalUniformGrid::DetermineCellSize(std::size_t num_objects) const
{
    const double sixth_root_n = std::pow(static_cast<double>(num_objects), 1.0 / 6.0);
    const double cell_size = 3.0 * std::max(m_bounding_box.m_x.Size(), std::max(m_bounding_box.m_y.Size(), m_bounding_box.m_z.Size())) / std::max(1.0, sixth_root_n);

    return Vec3(cell_size);
}

Vec3Int HierarchicalUniformGrid::Calculate3DIndex(Vec3 position) const
{
    const double cell_size_x = m_bounding_box.m_x.Size() / m_num_x_cells;
    const double cell_size_y = m_bounding_box.m_y.Size() / m_num_y_cells;
    const double cell_size_z = m_bounding_box.m_z.Size() / m_num_z_cells;

    int x = static_cast<int>((position.m_x - m_bounding_box.m_x.m_min) / cell_size_x);
    int y = static_cast<int>((position.m_y - m_bounding_box.m_y.m_min) / cell_size_y);
    int z = static_cast<int>((position.m_z - m_bounding_box.m_z.m_min) / cell_size_z);

    // Edge case where position is exactly on max boundary
    if (x >= static_cast<int>(m_num_x_cells)) x = m_num_x_cells - 1;
    if (y >= static_cast<int>(m_num_y_cells)) y = m_num_y_cells - 1;
    if (z >= static_cast<int>(m_num_z_cells)) z = m_num_z_cells - 1;

    return Vec3Int(x, y, z);
}

Vec3Int HierarchicalUniformGrid::Calculate3DIndex(std::size_t one_dimensional_index) const
{
    const int x = one_dimensional_index / (m_num_y_cells * m_num_z_cells);
    const int y = (one_dimensional_index % (m_num_y_cells * m_num_z_cells)) / m_num_z_cells;
    const int z = one_dimensional_index % m_num_z_cells;

    return Vec3Int(x, y, z);
}

std::size_t HierarchicalUniformGrid::Calculate1DIndex(Vec3 position) const
{
    return Calculate1DIndex(Calculate3DIndex(position));
}

std::size_t HierarchicalUniformGrid::Calculate1DIndex(Vec3Int three_dimensional_index) const
{
    // i = x * (Y_size * Z_size) + y * Z_size + z;
    return three_dimensional_index.m_x * (m_num_y_cells * m_num_z_cells) + three_dimensional_index.m_y * m_num_z_cells + three_dimensional_index.m_z;
}

} // namespace ART
