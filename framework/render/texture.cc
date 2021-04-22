// *************************************************************
// File:    texture.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "texture.h"

//--public

gdm::gfx::Texture::Texture(tag::SR, gfx::EFormatType format, Vec3u whd, api::CommandList& cmd, api::Device* device)
  : device_{device}
  , format_{format}
  , whd_{whd}
{
  api_image_ = gfx::Resource<api::Image2D>(device_, whd_.w, whd_.h)
    .AddFormatType(format_)
    .AddImageUsage(gfx::TRANSFER_DST_IMG | gfx::SAMPLED);

  api_image_view_ = gfx::Resource<api::ImageView>(*device_)
    .AddImage(*api_image_)
    .AddFormatType(format);

  to_read_barrier_ = nullptr;
  to_write_barrier_ = nullptr;
}

gdm::gfx::Texture::Texture(tag::SRRT, gfx::EFormatType format, Vec3u whd, api::CommandList& cmd, api::Device* device)
  : device_{device}
  , format_{format}
  , whd_{whd}
{
  api_image_ = gfx::Resource<api::Image2D>(device_, whd_.w, whd.h)
    .AddFormatType(format)
    .AddImageUsage(gfx::EImageUsage::COLOR_ATTACHMENT | gfx::EImageUsage::SAMPLED);

  api_image_view_ = gfx::Resource<api::ImageView>(*device_)
    .AddImage(*api_image_)
    .AddFormatType(format);

  to_read_barrier_ = gfx::Resource<api::ImageBarrier>()
    .AddImage(*api_image_)
    .AddOldLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
    .AddNewLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL);

  to_write_barrier_ = gfx::Resource<api::ImageBarrier>()
    .AddImage(*api_image_)
    .AddOldLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL)
    .AddNewLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL);

  std::unique_ptr<api::ImageBarrier> init_barrier = gfx::Resource<api::ImageBarrier>()
    .AddImage(*api_image_)
    .AddOldLayout(gfx::EImageLayout::UNDEFINED)
    .AddNewLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL);

  cmd.PushBarrier(*init_barrier);
}

gdm::gfx::Texture::Texture(tag::DRT, gfx::EFormatType format, Vec3u whd, api::CommandList& cmd, api::Device* device)
  : device_{device}
  , format_{format}
  , whd_{whd}
{
  api_image_ = gfx::Resource<api::Image2D>(device_, whd.w, whd.h)
    .AddFormatType(gfx::EFormatType::D16_UNORM)
    .AddImageUsage(gfx::EImageUsage::DEPTH_STENCIL_ATTACHMENT);

  api_image_view_ = gfx::Resource<api::ImageView>(*device_)
    .AddImage(*api_image_)
    .AddFormatType(format);

  std::unique_ptr<api::ImageBarrier> init_barrier = gfx::Resource<api::ImageBarrier>()
    .AddImage(*api_image_)
    .AddOldLayout(gfx::EImageLayout::UNDEFINED)
    .AddNewLayout(gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  cmd.PushBarrier(*init_barrier);
}
