message("* App ${BIN}: adding vulkan package")

include(${GDM_ROOT_DIR}/tools/cmake_includes/framework.cmake)

set(GDM_LIBVK_DIR ${GDM_ROOT_DIR}/3rdparty/vulkan_sdk/1.2.148.1)

include_directories(
  ${GDM_LIBVK_DIR}/Include
)

set(Vulkan_LIBRARY ${GDM_LIBVK_DIR}/Lib/vulkan-1.lib)
set(Vulkan_INCLUDE_DIR ${GDM_LIBVK_DIR}/Include)
find_package(Vulkan REQUIRED FATAL_ERROR)