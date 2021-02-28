// *************************************************************
// File:    spatial.cfg.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "scene/defines.h"

// todo:
/*

// https://dcgi.fel.cvut.cz/home/havran/ARTICLES/cgf2011.pdf
// https://stackoverflow.com/questions/99796/when-to-use-binary-space-partitioning-quadtree-octree
// https://web.archive.org/web/20180111010801/http://www.thomasdiewald.com/blog/?p=1488
// http://www.pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies.html#fragment-BVHBuildNodePublicMethods-0

examples/ecs
examples/scene
examples/snake - move to another repo
make single main.cc, where we call dx/vk/gl implementation

//  * wathcdog
//  * how to be with multothreading with fabircks of mhy models
//  * think of implementation load of mesh (it is not obvious now) + rework dx loading and move out
//  * pass sizes from config to loaders
//  * implement loading / unloading of models and textures outside
//  * implement AABB for AbstractMesh and AbstractModel
//  * implemen custom OBB or sphere
//  * implement triangle iteration in AbstractMesh
//  * implement octree for meshes intersection optionally
//  * draw it
//  * implement octree for scene/
*/

//--private

namespace gdm::_private
{
  static void UpdateSpatial(cfg::Models& models, CameraEul& cam, MainInput& input, DebugDraw& debug, float dt)
  {
    static ModelInstance& ball_instance = *models["model_ball"];
    // static ModelInstance& table_instance = *models["model_table"];

    // static Ball ball {ball_instance.tm_};
    // static Table table {table_instance.tm_};

    // static bool ball_inited = ResetBall(ball);

    // UpdateTable(table, input, debug, dt);
    // UpdateBall(ball, table, debug, dt);

    // debug.DrawBasis(ball.tm_, 1.f);
    // debug.DrawBasis(table.tm_, 1.f);
  }
}
