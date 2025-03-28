// GraphicsApi.ixx

module;

#include <array>
#include <functional>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/ext/matrix_float4x4.hpp>

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
	VkPhysicalDevice device = VK_NULL_HANDLE;

	QueueFamilyIndices qfis;
	SwapChainSupportDetails sws_details;

	VkPhysicalDeviceMemoryProperties mem_properties;
	VkPhysicalDeviceProperties properties;
};

export class GraphicsApi
{
public:
	constexpr static uint32_t m_max_frames_in_flight = 2;

public:
	GraphicsApi(
		GLFWwindow * window, // Reminder: Do not call any glfw functions that require being on the main thread
		int width,
		int height,
		std::string const & app_title,
		uint32_t extension_count,
		char const ** extensions);

	~GraphicsApi();

	void RecreateSwapChain(int width, int height);
	bool SwapChainIsValid() const;

	void DrawFrame(std::function<void()> render_fn, bool & out_window_size_out_of_date);
	void WaitForLastFrame() const;

	VkResult CreateBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer & out_buffer,
		VkDeviceMemory & out_buffer_memory) const;

	VkResult CreateBufferMemory(
		VkBuffer buffer,
		VkMemoryPropertyFlags properties,
		VkDeviceMemory & out_buffer_memory) const;

	VkResult Create2dImage(
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage & out_image,
		VkDeviceMemory & out_image_memory) const;

	VkResult CreateImageMemory(
		VkImage image,
		VkMemoryPropertyFlags properties,
		VkDeviceMemory & out_image_memory) const;

	VkResult CreateImageView(
		VkImage image,
		VkFormat format,
		VkImageAspectFlags aspect_flags,
		VkImageView & out_image_view) const;

	void DoOneTimeCommand(std::function<void(VkCommandBuffer)> const & command_fn) const;
	void CopyBuffer(VkBuffer src_buffer,VkBuffer dst_buffer,VkDeviceSize size) const;
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) const;

	VkDevice GetDevice() const { return m_logical_device; }
	VkFormat GetSwapChainImageFormat() const { return m_swap_chain_image_format; }
	VkExtent2D GetSwapChainExtent() const { return m_swap_chain_extent; }
	VkRenderPass GetRenderPass() const { return m_render_pass; }
	VkCommandBuffer GetCurCommandBuffer() const { return m_command_buffers[m_current_frame]; }
	VkFramebuffer GetCurFrameBuffer() const { return m_swap_chain_framebuffers[m_current_image_index]; }
	VkQueue GetGraphicsQueue() const { return m_graphics_queue; }
	uint32_t GetCurFrameIndex() const { return m_current_frame; }

	PhysicalDeviceInfo const & GetPhysicalDeviceInfo() const { return m_phys_device_info; }

private:
	void destroy_swap_chain();

	VkResult create_swap_chain_framebuffers();
	void destroy_swap_chain_framebuffers();

	VkResult create_depth_resources(
		VkImage & out_image,
		VkDeviceMemory & out_image_memory,
		VkImageView & out_image_view) const;

private:
	VkInstance m_instance{ VK_NULL_HANDLE };
	VkSurfaceKHR m_surface{ VK_NULL_HANDLE };

	PhysicalDeviceInfo m_phys_device_info; // Automatically cleaned up when m_instance is destroyed
	VkDevice m_logical_device{ VK_NULL_HANDLE };
	VkQueue m_graphics_queue{ VK_NULL_HANDLE }; // Automatically cleaned up when m_logical_device is destroyed
	VkQueue m_present_queue{ VK_NULL_HANDLE }; // Automatically cleaned up when m_logical_device is destroyed

	VkSwapchainKHR m_swap_chain{ VK_NULL_HANDLE };
	VkFormat m_swap_chain_image_format{ VK_FORMAT_UNDEFINED };
	VkExtent2D m_swap_chain_extent{ 0, 0 };
	std::vector<VkImage> m_swap_chain_images; // Automatically cleaned up when m_swap_chain is destroyed
	std::vector<VkImageView> m_swap_chain_image_views;
	std::vector<VkFramebuffer> m_swap_chain_framebuffers;
	uint32_t m_current_image_index = 0;

	VkRenderPass m_render_pass{ VK_NULL_HANDLE };

	VkFormat m_depth_format{ VK_FORMAT_UNDEFINED };
	VkImage m_depth_image{ VK_NULL_HANDLE };
	VkDeviceMemory m_depth_image_memory{ VK_NULL_HANDLE };
	VkImageView m_depth_image_view{ VK_NULL_HANDLE };

	VkCommandPool m_command_pool{ VK_NULL_HANDLE };
	std::array<VkCommandBuffer, m_max_frames_in_flight> m_command_buffers; // Automatically cleaned up when m_comand_pool is destroyed

	std::array<VkSemaphore, m_max_frames_in_flight> m_image_available_semaphores;
	std::array<VkSemaphore, m_max_frames_in_flight> m_render_finished_semaphores;
	std::array<VkFence, m_max_frames_in_flight> m_in_flight_fences;

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
