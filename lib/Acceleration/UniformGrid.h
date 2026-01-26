// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <Core/Common.h>
#include <Maths/Vec3.h>
#include <Maths/Vec3Int.h>
#include <RayTracing/IRayHittable.h>
#include <RayTracing/RayHitResult.h>

namespace ART
{

struct UniformGridEntry
{
public:
    std::size_t hittables_buffer_offset = 0;
    std::size_t num_hittables = 0;
};

class UniformGrid : public IRayHittable
{
public:
    UniformGrid(std::vector<IRayHittable*>& objects);

    ~UniformGrid();

    bool Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const override;

    AABB BoundingBox() const override;

protected:
    void Create(std::vector<IRayHittable*>& objects);

    void Destroy();

    bool CellHit(const UniformGridEntry& entry, const Ray& ray, Interval ray_t, RayHitResult& out_result) const;

    Vec3 DetermineCellSize(std::size_t num_objects) const;

    Vec3Int Calculate3DIndex(Vec3 position) const;

    Vec3Int Calculate3DIndex(std::size_t one_dimensional_index) const;

    std::size_t Calculate1DIndex(Vec3 position) const;

    std::size_t Calculate1DIndex(Vec3Int three_dimensional_index) const;

    AABB m_bounding_box;
    UniformGridEntry* m_grid = nullptr;
    IRayHittable** m_hittables_buffer = nullptr;
    Vec3 m_cell_size;
    std::size_t m_num_x_cells = 0;
    std::size_t m_num_y_cells = 0;
    std::size_t m_num_z_cells = 0;
    bool m_is_grid_valid = false;
};

} // namespace ART
