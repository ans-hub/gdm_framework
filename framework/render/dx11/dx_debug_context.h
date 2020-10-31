// *************************************************************
// File:    debug_context.h
// Author:  Novoselov Anton @ 2018-2019
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GML_DEBUG_CONTEXT_H
#define AH_GML_DEBUG_CONTEXT_H

#include <d3d11.h>
#include <vector>

#include <math/vector3.h>
#include <math/vector4.h>
#include <math/matrix.h>

#include <render/colors.h>

namespace gdm {

struct DebugContext
{
  enum Type
  {
    LINE

  }; // enum Type

  DebugContext(ID3D11Device* device);
  ~DebugContext();

  void AddLine(const Vec3f& v0, const Vec3f& v1, const Vec4f& color = Vec4f(0,0,0,1));
  auto GetVertices() const -> const std::vector<float>& { return vertices_; }
  auto GetVertexBuffer() -> ID3D11Buffer* { return vertex_buffer_; } 
  auto GetVerticesCount() const -> unsigned int { return vertices_count_; }
  void Clear(unsigned int types = 0);

private:
  std::vector<float> vertices_;
  unsigned int vertices_count_;
  ID3D11Buffer* vertex_buffer_;

  const unsigned int k_max_vertices_;

  void CreateVertexBuffer(ID3D11Device* device); 

}; // struct DebugContext

namespace render_dbg
{
  void AddLinesFromTm(DebugContext& ctx, const Mat4f& tm, float length);
}

} // namespace gdm

#endif // AH_GML_DEBUG_CONTEXT_H