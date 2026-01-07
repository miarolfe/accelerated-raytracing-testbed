// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include "Common.h"
#include "IRayHittable.h"
#include "RayHitResult.h"
#include "Vec3.h"
#include "Vec3Int.h"

namespace ART
{

struct UniformGridEntry
{
    std::set<std::shared_ptr<IRayHittable>> hittables;
};

class UniformGrid : public IRayHittable
{
public:
    UniformGrid(const std::vector<std::shared_ptr<IRayHittable>>& objects);

    ~UniformGrid();

    bool Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const override;

    std::vector<std::shared_ptr<IRayHittable>>& GetObjects();

    AABB BoundingBox() const override;

    void Create();

    void Destroy();

protected:
    bool CellHit(const UniformGridEntry& entry, const Ray& ray, Interval ray_t, RayHitResult& out_result) const;

    Vec3 DetermineCellSize() const;

    Vec3Int Calculate3DIndex(Vec3 position) const;

    Vec3Int Calculate3DIndex(std::size_t one_dimensional_index) const;

    std::size_t Calculate1DIndex(Vec3 position) const;

    std::size_t Calculate1DIndex(Vec3Int three_dimensional_index) const;

    AABB m_bounding_box;
    std::vector<std::shared_ptr<IRayHittable>> m_objects;
    UniformGridEntry* m_grid;
    Vec3 m_cell_size;
    std::size_t m_num_x_cells;
    std::size_t m_num_y_cells;
    std::size_t m_num_z_cells;
    bool m_is_grid_valid = false;
};

} // namespace ART
