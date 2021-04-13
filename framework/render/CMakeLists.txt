cmake_minimum_required (VERSION 3.10)

project("gdm/framework/render")

set(BIN render)

message("* Lib: ${BIN} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

include_directories(
  "."
  ${GDM_FRAMEWORK_DIR}/render/dx11/
  ${GDM_FRAMEWORK_DIR}/render/ogl/
  ${GDM_FRAMEWORK_DIR}/render/vk/
  ${INCLUDE_DIRS}
)

set(CMAKE_CXX_STANDARD 20)

if ("${PROFILING_ENABLED}")
  set(PROFILING_DEFINITIONS
    -DGDM_LABELS_ENABLED=1)
endif()

if ("${DX_API}")
  message("  ** using api: DX")
  set(SRC_FILES
      dx11/dx_renderer.cc
      dx11/dx_material.cc
      dx11/dx_mesh.cc
      dx11/dx_object.cc
      dx11/dx_texture.cc
      dx11/dx_debug_context.cc
      dx11/dx_vertex_shader.cc
      dx11/dx_pixel_shader.cc
      dx11/dx_shader_loader.cc
      dx11/dx_utils.cc)
elseif ("${VK_API}")
  message("  ** using api: VK")
  set(SRC_FILES
      vk/vk_renderer.cc
      vk/vk_device.cc
      vk/vk_pipeline.cc
      vk/vk_fence.cc
      vk/vk_semaphore.cc
      vk/vk_barrier.cc
      vk/vk_image.cc
      vk/vk_image_view.cc
      vk/vk_render_pass.cc
      vk/vk_framebuffer.cc
      vk/vk_host_allocator.cc
      vk/vk_sampler.cc
      vk/vk_device_allocator.cc
      vk/vk_descriptor_set.cc
      vk/vk_command_list.cc
      vk/vk_buffer.cc
      vk/vk_blend_state.cc
      vk/vk_pointers.cc
      vk/vk_physical_device.cc)
else()
  message("  ** using api: OpenGL")
  set(SRC_FILES
      ogl/debug_draw.cc
      ogl/gl_mesh.cc
      ogl/gl_pointers.cc
      ogl/gl_shader.cc
      ogl/gl_texture.cc
      ogl/gl_utils.cc)
endif()

set(SRC_FILES
  ${SRC_FILES})

if ("${VK_API}")
  add_definitions(-DVK_PROTOTYPES)
  add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
endif()

add_library(${BIN} STATIC ${SRC_FILES})

message("* App ${BIN}: copy dlls")
