#include <qtypes.hpp>

namespace qutils {
bool load_shader_module(const char *filePath, VkDevice device,
                        VkShaderModule *outShaderModule);
}
