find_package(Vulkan REQUIRED)

add_library(vma INTERFACE)
include(FetchContent)

# FMT
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        e69e5f977d458f2650bb346dadf2ad30c5320281) # 10.2.1

FetchContent_MakeAvailable(fmt)

#VK Bootstrap
FetchContent_Declare(
    fetch_vk_bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
    GIT_TAG        main #suggest using a tag so the library doesn't update whenever new commits are pushed to a branch
)
FetchContent_MakeAvailable(fetch_vk_bootstrap)

# imgui
FetchContent_Declare(
  imgui_git
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.92.5
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)
FetchContent_MakeAvailable(imgui_git)

target_include_directories(vma INTERFACE vma)

add_library(imgui STATIC
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h"

  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl2.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl2.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp"
)

target_include_directories(imgui
  PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/imgui
  ${CMAKE_CURRENT_LIST_DIR}/imgui/backends

)


if(APPLE)
  find_package(SDL2 REQUIRED)
  target_link_libraries(imgui PUBLIC Vulkan::Vulkan SDL2::SDL2)
elseif(UNIX)
  add_library(sdl2 INTERFACE)
  set(sdl2_DIR "/usr/" CACHE PATH "Path to SDL2")
  target_include_directories(sdl2 INTERFACE ${sdl2_DIR}/include/SDL2
  ${sdl2_image_DIR}/include/SDL2)
  target_link_directories(sdl2 INTERFACE ${sdl2_DIR}/lib ${sdl2_image_DIR}/lib)
  target_link_libraries(sdl2 INTERFACE SDL2 SDL2main)
  target_link_libraries(imgui PUBLIC Vulkan::Vulkan sdl2)
endif()

