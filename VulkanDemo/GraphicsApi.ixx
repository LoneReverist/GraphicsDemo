// GraphicsApi.ixx

module;

#include <functional>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

export module GraphicsApi;

import <optional>;
import <string>;
import <vector>;

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphics_family;
	std::optional<uint32_t> present_family;

	bool IsComplete() const { return graphics_family.has_value() && present_family.has_value(); }
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};

struct PhysicalDeviceInfo
{
	VkPhysicalDevice device;
	QueueFamilyIndices qfis;
	SwapChainSupportDetails sws_details;
};

export class GraphicsApi
{
public:
	GraphicsApi(
		GLFWwindow * window,
		std::string const & app_title,
		uint32_t extension_count,
		char const ** extensions);

	~GraphicsApi();

	void DestroySwapChain();
	void RecreateSwapChain();

	void DrawFrame(std::function<void()> render_fn);
	void WaitForLastFrame() const;

	VkDevice GetDevice() const { return m_logical_device; }
	VkFormat GetSwapChainImageFormat() const { return m_swap_chain_image_format; }
	VkExtent2D GetSwapChainExtent() const { return m_swap_chain_extent; }
	VkRenderPass GetRenderPass() const { return m_render_pass; }
	VkCommandBuffer GetCurCommandBuffer() const { return m_command_buffers[m_current_frame]; }
	VkFramebuffer GetCurFrameBuffer() const { return m_swap_chain_framebuffers[m_current_image_index]; }

private:
	GLFWwindow * m_window = nullptr;
	VkInstance m_instance = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;

	PhysicalDeviceInfo m_phys_device_info; // Automatically cleaned up when m_instance is destroyed
	VkDevice m_logical_device = VK_NULL_HANDLE;
	VkQueue m_graphics_queue = VK_NULL_HANDLE; // Automatically cleaned up when m_logical_device is destroyed
	VkQueue m_present_queue = VK_NULL_HANDLE; // Automatically cleaned up when m_logical_device is destroyed

	VkSwapchainKHR m_swap_chain = VK_NULL_HANDLE;
	std::vector<VkImage> m_swap_chain_images; // Automatically cleaned up when m_swap_chain is destroyed
	VkFormat m_swap_chain_image_format = VK_FORMAT_UNDEFINED;
	VkExtent2D m_swap_chain_extent{ 0, 0 };

	VkRenderPass m_render_pass = VK_NULL_HANDLE;
	std::vector<VkImageView> m_swap_chain_image_views;
	std::vector<VkFramebuffer> m_swap_chain_framebuffers;
	uint32_t m_current_image_index = 0;

	VkCommandPool m_command_pool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> m_command_buffers; // Automatically cleaned up when m_comand_pool is destroyed

	std::vector<VkSemaphore> m_image_available_semaphores;
	std::vector<VkSemaphore> m_render_finished_semaphores;
	std::vector<VkFence> m_in_flight_fences;

	constexpr static uint32_t m_max_frames_in_flight = 2;
	uint32_t m_current_frame = 0;

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
