#include "VkBootstrap.h"
#include "qinitializers.hpp"
#include <SDL.h>
#include <SDL_vulkan.h>
#include <algorithm>
#include <qengine.hpp>
#include <qtypes.hpp>
#include <vulkan/vulkan_core.h>

#include <qimages.hpp>
constexpr bool useValidationLayers = true;

QEngine::QEngine() {
  // Initialize SDL window
  SDL_Init(SDL_INIT_VIDEO);
  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

  _window = SDL_CreateWindow("QEngine", SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED, _windowExtent.width,
                             _windowExtent.height, window_flags);
  init_vulkan();
  init_swapchain();
  init_commands();
  init_sync_structures();

  _isInitialized = true;
}

QEngine::~QEngine() {
  if (_isInitialized) {
    fmt::print("Destroying engine\n");
    vkDeviceWaitIdle(_device);

    for (int i = 0; i < FRAME_OVERLAP; i++) {
      vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);

      vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
      vkDestroySemaphore(_device, _frames[i]._swapchainSemaphore, nullptr);
      vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr);
    }

    destroy_swapchain();
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyDevice(_device, nullptr);

    vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
    vkDestroyInstance(_instance, nullptr);
    SDL_DestroyWindow(_window);
  }
}

void QEngine::draw() {
  VK_CHECK(vkWaitForFences(_device, 1, &current_frame()._renderFence, true,
                           1000000000));
  VK_CHECK(vkResetFences(_device, 1, &current_frame()._renderFence));

  // request image from the swapchain
  uint32_t swapchainImageIndex;
  VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000,
                                 current_frame()._swapchainSemaphore, nullptr,
                                 &swapchainImageIndex));

  VkCommandBuffer cmd = current_frame()._mainCommandBuffer;

  VK_CHECK(vkResetCommandBuffer(cmd, 0));

  // Begin command buffer recording.
  VkCommandBufferBeginInfo cmdBeginInfo = qinit::command_buffer_begin_info(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  // Start command buffer recording.
  VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

  // make the swapchain image into writeable mode before rendering
  qutils::transition_image(cmd, _swapchainImages[swapchainImageIndex],
                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

  // make a clear-color from frame number. This will flash with a 120 frame
  // period.
  VkClearColorValue clearValue;
  float flash = std::abs(std::sin(_frameNumber / 120.f));
  clearValue = {{0.0f, 0.0f, flash, 1.0f}};

  VkImageSubresourceRange clearRange =
      qinit::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

  // clear image
  vkCmdClearColorImage(cmd, _swapchainImages[swapchainImageIndex],
                       VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

  // make the swapchain image into presentable mode
  qutils::transition_image(cmd, _swapchainImages[swapchainImageIndex],
                           VK_IMAGE_LAYOUT_GENERAL,
                           VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  // finalize the command buffer (we can no longer add commands, but it can now
  // be executed)
  VK_CHECK(vkEndCommandBuffer(cmd));

  // prepare the submission to the queue.
  // we want to wait on the _presentSemaphore, as that semaphore is signaled
  // when the swapchain is ready we will signal the _renderSemaphore, to signal
  // that rendering has finished

  VkCommandBufferSubmitInfo cmdinfo = qinit::command_buffer_submit_info(cmd);

  VkSemaphoreSubmitInfo waitInfo = qinit::semaphore_submit_info(
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
      current_frame()._swapchainSemaphore);
  VkSemaphoreSubmitInfo signalInfo = qinit::semaphore_submit_info(
      VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, current_frame()._renderSemaphore);

  VkSubmitInfo2 submit = qinit::submit_info(&cmdinfo, &signalInfo, &waitInfo);

  // submit command buffer to the queue and execute it.
  //  _renderFence will now block until the graphic commands finish execution
  VK_CHECK(
      vkQueueSubmit2(_graphicsQueue, 1, &submit, current_frame()._renderFence));
  //
  // prepare present
  //  this will put the image we just rendered to into the visible window.
  //  we want to wait on the _renderSemaphore for that,
  //  as its necessary that drawing commands have finished before the image is
  //  displayed to the user
  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = nullptr;
  presentInfo.pSwapchains = &_swapchain;
  presentInfo.swapchainCount = 1;

  presentInfo.pWaitSemaphores = &current_frame()._renderSemaphore;
  presentInfo.waitSemaphoreCount = 1;

  presentInfo.pImageIndices = &swapchainImageIndex;

  VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));

  // increase the number of frames drawn
  _frameNumber++;
}

void QEngine::run() {
  SDL_Event e;
  bool bQuit = false;

  while (!bQuit) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT)
        bQuit = true;
    }

    draw();
  }
}

void QEngine::init_vulkan() {
  vkb::InstanceBuilder builder;

  //
  // Instance
  //
  auto inst_ret = builder.set_app_name("QEngine")
                      .request_validation_layers(useValidationLayers)
                      .use_default_debug_messenger()
                      .require_api_version(1, 3, 0)
                      .build();

  vkb::Instance vkb_inst = inst_ret.value();

  _instance = vkb_inst.instance;
  _debug_messenger = vkb_inst.debug_messenger;

  //
  // Device
  //
  SDL_Vulkan_CreateSurface(_window, _instance, &_surface);

  VkPhysicalDeviceVulkan13Features feat13{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  feat13.dynamicRendering = true;
  feat13.synchronization2 = true;

  VkPhysicalDeviceVulkan12Features feat12{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
  feat12.bufferDeviceAddress = true;
  feat12.descriptorIndexing = true;

  vkb::PhysicalDeviceSelector selector{vkb_inst};
  vkb::PhysicalDevice physicalDevice = selector.set_minimum_version(1, 3)
                                           .set_required_features_13(feat13)
                                           .set_required_features_12(feat12)
                                           .set_surface(_surface)
                                           .select()
                                           .value();

  vkb::DeviceBuilder deviceBuilder{physicalDevice};
  vkb::Device vkbDevice = deviceBuilder.build().value();

  _device = vkbDevice.device;
  _chosenGPU = physicalDevice.physical_device;

  //
  // Graphics queue
  //
  _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
  _graphicsQueueFamily =
      vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void QEngine::create_swapchain(uint32_t width, uint32_t height) {
  vkb::SwapchainBuilder swapchainBuilder{_chosenGPU, _device, _surface};
  _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
  vkb::Swapchain vkbSwapchain =
      swapchainBuilder
          .set_desired_format(VkSurfaceFormatKHR{
              .format = _swapchainImageFormat,
              .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR})
          .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
          .set_desired_extent(width, height)
          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
          .build()
          .value();

  _swapchainExtent = vkbSwapchain.extent;
  // Store swapchain and its related images.
  _swapchain = vkbSwapchain.swapchain;
  _swapchainImages = vkbSwapchain.get_images().value();
  _swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void QEngine::init_swapchain() {
  create_swapchain(_windowExtent.width, _windowExtent.height);
}

void QEngine::init_commands() {
  VkCommandPoolCreateInfo commandPoolinfo = qinit::command_pool_create_info(
      _graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    VK_CHECK(vkCreateCommandPool(_device, &commandPoolinfo, nullptr,
                                 &_frames[i]._commandPool));

    VkCommandBufferAllocateInfo cmdAllocInfo =
        qinit::command_buffer_allocate_info(_frames[i]._commandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo,
                                      &_frames[i]._mainCommandBuffer););
  }
}

void QEngine::init_sync_structures() {
  VkFenceCreateInfo fenceCreateInfo =
      qinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
  VkSemaphoreCreateInfo semaphoreCreateInfo = qinit::semaphore_create_info();

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr,
                           &_frames[i]._renderFence));
    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr,
                               &_frames[i]._swapchainSemaphore));
    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr,
                               &_frames[i]._renderSemaphore));
  }
}

void QEngine::destroy_swapchain() {
  vkDestroySwapchainKHR(_device, _swapchain, nullptr);

  std::ranges::for_each(_swapchainImageViews, [this](auto imageView) {
    vkDestroyImageView(_device, imageView, nullptr);
  });
}
