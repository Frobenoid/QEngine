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
)
FetchContent_MakeAvailable(imgui_git)

target_include_directories(vma INTERFACE vma)

add_library(imgui 
  ${imgui_git_SOURCE_DIR}/imgui.cpp
  ${imgui_git_SOURCE_DIR}/imgui.h

  ${imgui_git_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_git_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_git_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_git_SOURCE_DIR}/imgui_widgets.cpp

  ${imgui_git_SOURCE_DIR}/backends/imgui_impl_sdl2.h
  ${imgui_git_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp  
  ${imgui_git_SOURCE_DIR}/backends/imgui_impl_vulkan.h  
  ${imgui_git_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui PUBLIC 
  ${imgui_git_SOURCE_DIR}
  ${imgui_git_SOURCE_DIR}/backends
)

FetchContent_Declare(
  glm 
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG 0af55ccecd98d4e5a8d1fad7de25ba429d60e863 #refs/tags/1.0.1
)
FetchContent_MakeAvailable(glm)

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

