// Copyright Mia Rolfe. All rights reserved.
#include "IRayHittable.h"
#include "RayHitResult.h"
#include <memory>
#include <vector>

namespace ART
{

class RayHittableList : public IRayHittable
{
public:
    RayHittableList();

    RayHittableList(std::shared_ptr<IRayHittable> hittable);

    void Clear();

    void Add(std::shared_ptr<IRayHittable> hittable);

    bool Hit(const Ray& ray, Interval ray_t, RayHitResult& out_result) const override;

    std::vector<std::shared_ptr<IRayHittable>>& GetObjects();

    AABB BoundingBox() const override;

protected:
    std::vector<std::shared_ptr<IRayHittable>> m_objects;
    AABB m_bounding_box;
};

} // namespace ART
