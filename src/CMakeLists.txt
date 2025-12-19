add_executable(QEngine
main.cpp
qtypes.hpp
qengine.cpp
qengine.hpp
qinitializers.cpp
qinitializers.hpp
qimages.cpp
qimages.hpp
qvma.cpp
)

target_include_directories(QEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(QEngine vk-bootstrap fmt vma)

if(APPLE)
  message(STATUS "Finding SDL2.")
  find_package(SDL2 REQUIRED)
  find_package(SDL2_image REQUIRED)
  message(STATUS "Linking Vulkan.")
  target_link_libraries(QEngine Vulkan::Vulkan SDL2::SDL2)
elseif(UNIX)
  message(STATUS "Linking Vulkan.")
  target_link_libraries(QEngine Vulkan::Vulkan sdl2)
endif()

