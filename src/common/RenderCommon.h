// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>

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

void LogRenderConfig(const CameraRenderConfig& render_config, int scene_number);

void LogRenderStats(const RenderStats& stats);

RenderStats RenderWithAccelerationStructure(Camera& camera, RayHittableList& scene, const SceneConfig& scene_config, AccelerationStructure acceleration_structure);

void RenderScene(const CameraRenderConfig& render_config, int scene_number, AccelerationStructure acceleration_structure);

} // namespace ART
