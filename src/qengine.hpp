#pragma once

#include <cstdint>
#include <qtypes.hpp>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

struct DeletionQueue {
  std::deque<std::function<void()>> deletors;

  void push_function(std::function<void()> &&function) {
    deletors.push_back(function);
  }

  void flush() {
    // reverse iterate the deletion queue to execute all the functions
    for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
      (*it)(); // call functors
    }

    deletors.clear();
  }
};
struct FrameData {
  VkCommandPool _commandPool;
  VkCommandBuffer _mainCommandBuffer;
  VkSemaphore _swapchainSemaphore, _renderSemaphore;
  VkFence _renderFence;
  DeletionQueue _deletionQueue;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class QEngine {
public:
  int _frameNumber{0};
  bool _isInitialized;

  // Initialize everything.
  QEngine();
  void init();

  // Shuts down the engine.
  ~QEngine();
  void cleanup();

  // Draw loop.
  void draw();
  // Runs main loop.
  void run();

  FrameData &current_frame() { return _frames[_frameNumber % FRAME_OVERLAP]; }
  void draw_background(VkCommandBuffer cmd);

private:
  // Window
  struct SDL_Window *_window{nullptr};
  VkExtent2D _windowExtent{1700, 900};

  VkInstance _instance;
  VkDebugUtilsMessengerEXT _debug_messenger;
  VkPhysicalDevice _chosenGPU;
  VkDevice _device;
  VkSurfaceKHR _surface;

  // Swapchain
  VkSwapchainKHR _swapchain;
  VkFormat _swapchainImageFormat;
  std::vector<VkImage> _swapchainImages;
  std::vector<VkImageView> _swapchainImageViews;
  VkExtent2D _swapchainExtent;

  // Frames
  FrameData _frames[FRAME_OVERLAP];
  VkQueue _graphicsQueue;
  uint32_t _graphicsQueueFamily;

  // Memory
  DeletionQueue _mainDeletionQueue;
  VmaAllocator _allocator;

  // Draw resources
  AllocatedImage _drawImage;
  VkExtent2D _drawExtent;

  void init_vulkan();
  void init_swapchain();
  void init_commands();
  void init_sync_structures();
  void create_swapchain(uint32_t width, uint32_t height);
  void destroy_swapchain();
};
