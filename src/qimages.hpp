#pragma once
#include <vulkan/vulkan.h>

namespace qutils {
void copy_image_to_image(VkCommandBuffer cmd, VkImage source,
                         VkImage destination, VkExtent2D srcSize,
                         VkExtent2D dstSize);

void transition_image(VkCommandBuffer cmd, VkImage image,
                      VkImageLayout currentLayout, VkImageLayout newLayout);
} // namespace qutils
