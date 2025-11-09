#pragma once
#include <vulkan/vulkan.h>

namespace qutils {
void transition_image(VkCommandBuffer cmd, VkImage image,
                      VkImageLayout currentLayout, VkImageLayout newLayout);
}
