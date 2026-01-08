// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include "Common.h"
#include "IRayHittable.h"
#include "RayHitResult.h"
#include "UniformGrid.h"
#include "Vec3.h"
#include "Vec3Int.h"

namespace ART
{

struct HierarchicalUniformGridEntry
{
public:
    UniformGrid* subgrid = nullptr;
};

class HierarchicalUniformGrid : public IRayHittable
{
public:
    HierarchicalUniformGrid(const std::vector<std::shared_ptr<IRayHittable>>& objects);

    ~HierarchicalUniformGrid();

    bool Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const override;

    AABB BoundingBox() const override;

protected:
    void Create(const std::vector<std::shared_ptr<IRayHittable>>& objects);

    void Destroy();

    bool CellHit(const HierarchicalUniformGridEntry& entry, const Ray& ray, Interval ray_t, RayHitResult& out_result) const;

    Vec3 DetermineCellSize(std::size_t num_objects) const;

    Vec3Int Calculate3DIndex(Vec3 position) const;

    Vec3Int Calculate3DIndex(std::size_t one_dimensional_index) const;

    std::size_t Calculate1DIndex(Vec3 position) const;

    std::size_t Calculate1DIndex(Vec3Int three_dimensional_index) const;

    AABB m_bounding_box;
    HierarchicalUniformGridEntry* m_grid = nullptr;
    Vec3 m_cell_size;
    std::size_t m_num_x_cells = 0;
    std::size_t m_num_y_cells = 0;
    std::size_t m_num_z_cells = 0;
    bool m_is_grid_valid = false;
};

} // namespace ART
