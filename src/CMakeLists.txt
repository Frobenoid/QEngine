add_executable(QEngine
main.cpp
qtypes.hpp
qengine.cpp
qengine.hpp
qinitializers.cpp
qinitializers.hpp
qimages.cpp
qimages.hpp
)

target_include_directories(QEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(QEngine vk-bootstrap::vk-bootstrap)
target_link_libraries(QEngine fmt::fmt)
target_link_libraries(QEngine Vulkan::Vulkan sdl2)
