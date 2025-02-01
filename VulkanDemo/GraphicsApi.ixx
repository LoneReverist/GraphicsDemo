// GraphicsApi.ixx

module;

#include <vulkan/vulkan.h>

export module GraphicsApi;

import <string>;
import <vector>;

export class GraphicsApi
{
public:
	GraphicsApi(std::string const & app_title, uint32_t extension_count, char const ** extensions);
	~GraphicsApi();

	VkInstance GetInstance() { return m_instance; }

private:
	VkInstance m_instance = VK_NULL_HANDLE;
	VkPhysicalDevice m_physical_device = VK_NULL_HANDLE; // implicitly destroyed when m_instance is destroyed
	VkDevice m_logical_device = VK_NULL_HANDLE;
	VkQueue m_graphics_queue = VK_NULL_HANDLE; // implicitly cleaned up when m_logical_device is destroyed

	std::vector<char const *> const m_validation_layers = {
		"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	bool const m_enable_validation_layers = false;
#else
	bool const m_enable_validation_layers = true;
#endif
};
