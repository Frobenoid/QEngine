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

target_include_directories(vma INTERFACE vma)

if(UNIX)
  # SDL2
  add_library(sdl2 INTERFACE)
  set(sdl2_DIR "/usr/" CACHE PATH "Path to SDL2")
  target_include_directories(sdl2 INTERFACE ${sdl2_DIR}/include/SDL2
  ${sdl2_image_DIR}/include/SDL2)
  target_link_directories(sdl2 INTERFACE ${sdl2_DIR}/lib ${sdl2_image_DIR}/lib)
  target_link_libraries(sdl2 INTERFACE SDL2 SDL2main)
endif()
