// *************************************************************
// File:    texture.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// api wrappers - with chaining
// high level - several special constructors

#ifndef GDM_GFX_TEXTURE_H
#define GDM_GFX_TEXTURE_H

#include "render/api.h"
#include "render/defines.h"
#include "render/renderer.h"

namespace gdm::gfx {

  struct Texture
  {
    [[deprecated("Tmp")]] Texture(api::Image* image, api::ImageView* image_view)
    {
      implemented_already_ = true;
      api_image_ = std::unique_ptr<api::Image>(image);
      api_image_view_ = std::unique_ptr<api::ImageView>(image_view);
    }
    [[deprecated("Tmp")]] ~Texture()
    {
      if (implemented_already_)
      {
        api_image_.release();
        api_image_view_.release();
      }
    }

    // Texture() =default;
    // Texture(gfx::EFormatType& format, int w, int h, api::CommandList& cmd, api::Device* device);
    Texture(const Texture&) =delete;
    Texture& operator=(const Texture&) =delete;

    auto GetImageImpl() const -> const api::Image& { return *api_image_.get(); }
    auto GetImageImpl() -> api::Image& { return *api_image_.get(); }
    auto GetImageViewImpl() const -> const api::ImageView& { return *api_image_view_.get(); }
    auto GetImageViewImpl() -> api::ImageView& { return *api_image_view_.get(); }

    operator api::Image&() { return *api_image_.get(); }
    operator api::Image&() const { return *api_image_.get(); }
    operator api::ImageView&() { return *api_image_view_.get(); }
    operator api::ImageView&() const { return *api_image_view_.get(); }

    void ToRead();
    void ToWrite();

  private:
    [[deprecated]] bool implemented_already_ = false;
    api::Device* device_;
    // ImageHandle image_;
    gfx::EFormatType format_;
    int w_;
    int h_;
    std::unique_ptr<api::Image> api_image_;
    std::unique_ptr<api::ImageView> api_image_view_;
    std::unique_ptr<api::ImageBarrier> to_read_barrier_;
    std::unique_ptr<api::ImageBarrier> to_write_barrier_;
  
  }; // struct Texture

  auto CreateRenderTarget() -> std::unique_ptr<Texture>;
  auto CreateTexture() -> std::unique_ptr<Texture>;
  
  inline auto GetMipLevelCount(int w, int h) -> int { return floor(log2(float(min(w, h)))) + 1; }

} // namespace gdm::gfx

#include "texture.inl"

#endif // GDM_GFX_TEXTURE_H