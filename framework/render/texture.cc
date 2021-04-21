// *************************************************************
// File:    texture.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "texture.h"

//--public

// gdm::gfx::Texture::Texture(gfx::EFormatType& format, int w, int h, api::CommandList& cmd, api::Device* device)
//   : device_{device}
//   , w_{w}
//   , h_{h}
// {
//   api_image_ = gfx::Resource<api::Image2D>(device_, w_, h_)
//       .AddFormatType(format)
//       .AddImageUsage(gfx::EImageUsage::COLOR_ATTACHMENT | gfx::EImageUsage::SAMPLED);

//   api_image_view_ = gfx::Resource<api::ImageView>(*device_)
//       .AddImage(*api_image_)
//       .AddFormatType(format);

//   to_read_barrier_ = gfx::Resource<api::ImageBarrier>()
//       .AddImage(*api_image_)
//       .AddOldLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
//       .AddNewLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL);

//   to_write_barrier_ = gfx::Resource<api::ImageBarrier>()
//       .AddImage(*api_image_)
//       .AddOldLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL)
//       .AddNewLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL);

//   api::ImageBarrier* init_barrier = gfx::Resource<api::ImageBarrier>()
//       .AddImage(*api_image_)
//       .AddOldLayout(gfx::EImageLayout::UNDEFINED)
//       .AddNewLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL);

//   cmd.PushBarrier(*init_barrier);

//   GMDelete(init_barrier);
// }
