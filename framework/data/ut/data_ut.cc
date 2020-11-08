// *************************************************************
// File:    data_ut.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "3rdparty/catch/catch.hpp"

#include "data/data_factory.h"
#include "data/mesh_factory.h"
#include "data/model_factory.h"
#include "data/material_factory.h"
#include "data/texture_factory.h"
#include "data/image_factory.h"
#include "system/hash_utils.h"

#include "data/helpers.h"

using namespace gdm;

TEST_CASE("SceneManager")
{
  gdm::ImageFactory::SetPath("examples/_models_new/textures/");
  gdm::TextureFactory::SetPath("examples/_models_new/textures/");
  gdm::ModelFactory::SetPath("examples/_models_new/models/");
  gdm::MaterialFactory::SetPath("examples/_models_new/materials/");

  SECTION("Helpers")
  {
    const char* path0 = "some";
    const char* path1 = "some.ext";
    const char* path2 = "path/some.ext";
    const char* path3 = "other.path/some.extension";
    const char* path4 = "other.path/some.extension.";

    CHECK(str::GetFileExtension(path0) == "");
    CHECK(str::GetFileExtension(path1) == "ext");
    CHECK(str::GetFileExtension(path2) == "ext");
    CHECK(str::GetFileExtension(path3) == "extension");
    CHECK(str::GetFileExtension(path4) == "");
  }
 
  SECTION("Load image")
  {
    Handle img0_handle = ImageFactory::Load("cube.bmp");
    CHECK(ImageFactory::Has("boxes/cube.bmp"));
    CHECK(img0_handle == ImageFactory::GetHandle("boxes/cube.bmp"));
    CHECK(img0_handle == ImageFactory::GetHandle("cube.bmp"));
    CHECK_THROWS(ImageFactory::Get(GDM_HASH("unloaded")));
    AbstractImage* img0 = ImageFactory::Get(img0_handle);
    CHECK(img0);
    ImageFactory::Release(img0_handle);
    CHECK_THROWS(ImageFactory::Release(img0_handle));
  }

  SECTION("Reuse handle")
  {
    Handle img0_handle = ImageFactory::Load("cube.bmp");
    ImageFactory::Release(img0_handle);
    img0_handle = ImageFactory::Load("boxes/cube_test.tga");
    CHECK(img0_handle == ImageFactory::GetHandle("boxes/cube_test.tga"));
    CHECK(ImageFactory::Has(img0_handle));
    ImageFactory::Release(img0_handle);
    CHECK(!ImageFactory::Has(img0_handle));
  }

  SECTION("Load texture by path")
  {
    Handle tex_handle = TextureFactory::Load("boxes/cube.bmp");
    CHECK_THROWS(tex_handle == TextureFactory::Load("boxes/cube.bmp"));
    CHECK_THROWS(TextureFactory::Get(GDM_HASH("unloaded")));
    AbstractTexture* tex0 = TextureFactory::Get(tex_handle);
    CHECK(ImageFactory::Has("boxes/cube.bmp"));
    CHECK(tex0);
    TextureFactory::Release(tex_handle);
    CHECK_THROWS(ImageFactory::Release(tex_handle));
  }

  SECTION("Load texture by image handle")
  {
    Handle img_handle = ImageFactory::Load("boxes/cube.bmp");
    Handle tex_handle = TextureFactory::Load(img_handle);
  }

  SECTION("Load model obj")
  {
    Handle mod_handle = ModelFactory::Load("ScifiHangarScene.obj");
    CHECK(MaterialFactory::Has("DarkMetel"));
  }

  SECTION("Load model without material")
  {
    Handle mod_handle = ModelFactory::Load("wheel.ply");
    CHECK(MaterialFactory::Has(MaterialFactory::v_dummy_name));
  }

  SECTION("Load different model with same material")
  {
    Handle mod_handle1 = ModelFactory::Load("wall.ply");
    Handle mod_handle2 = ModelFactory::Load("lighthouse.ply");
    Handle img_handle3 = ImageFactory::GetHandle("wall_df.tga");
  }

  SECTION("Load models with same material but in different dir")
  {
    Handle mod_handle1 = ModelFactory::Load("cube.ply");
    Handle mod_handle2 = ModelFactory::Load("cube_copy.ply");
    CHECK(MaterialFactory::Has("box"));
    CHECK(!MaterialFactory::Has("box.mat"));
    CHECK(!MaterialFactory::Has("boxes/box.mat"));
    CHECK(MaterialFactory::Has("boxes/box"));
    CHECK(!MaterialFactory::Has("boxes_copies/box.mat"));
    CHECK(MaterialFactory::Has("boxes_copies/box"));
    CHECK(ImageFactory::Has("cube.bmp"));
  }

  SECTION("Load pure model")
  {
    // TODO
  }

  SECTION("Load model for already loaded parts")
  {
    // TODO:
  }

  SECTION("Async streaming")
  {
    // TODO:
  }
}
