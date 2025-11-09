#pragma once

#include <vulkan/vulkan.h>

#include <array>
#include <deque>
#include <functional>
#include <memory.h>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <fmt/core.h>
#include <vulkan/vk_enum_string_helper.h>

#define VK_CHECK(x)                                                            \
  do {                                                                         \
    VkResult err = x;                                                          \
    if (err) {                                                                 \
      fmt::print("Detected Vulkan error: {}", string_VkResult(err));           \
    }                                                                          \
  } while (0)
