// GraphicsApi.ixx

module;

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

export module GraphicsApi;

import <string>;
import <vector>;

export class GraphicsApi
{
public:
	GraphicsApi(
		GLFWwindow * window,
		std::string const & app_title,
		uint32_t extension_count,
		char const ** extensions);

	~GraphicsApi();

	VkDevice GetDevice() const { return m_logical_device; }
	VkFormat GetSwapChainImageFormat() const { return m_swap_chain_image_format; }
	VkExtent2D GetSwapChainExtent() const { return m_swap_chain_extent; }

private:
	VkInstance m_instance = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;

	VkPhysicalDevice m_physical_device = VK_NULL_HANDLE; // Automatically cleaned up when m_instance is destroyed
	VkDevice m_logical_device = VK_NULL_HANDLE;
	VkQueue m_graphics_queue = VK_NULL_HANDLE; // Automatically cleaned up when m_logical_device is destroyed
	VkQueue m_present_queue = VK_NULL_HANDLE; // Automatically cleaned up when m_logical_device is destroyed

	VkSwapchainKHR m_swap_chain = VK_NULL_HANDLE;
	std::vector<VkImage> m_swap_chain_images; // Automatically cleaned up when m_swap_chain is destroyed
	VkFormat m_swap_chain_image_format = VK_FORMAT_UNDEFINED;
	VkExtent2D m_swap_chain_extent{ 0, 0 };

	std::vector<VkImageView> m_swap_chain_image_views;

	std::vector<const char *> const m_device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	std::vector<char const *> const m_validation_layers = {
		"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	bool const m_enable_validation_layers = false;
#else
	bool const m_enable_validation_layers = true;
#endif
};
