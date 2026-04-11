// GraphicsApi.ixx

module;

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

export module GraphicsApi;

struct SwapChainSupportDetails
{
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> present_modes;
};

constexpr std::uint32_t InvalidQueueIndex = ~0;

struct PhysicalDeviceInfo
{
	vk::raii::PhysicalDevice device = nullptr;

	std::uint32_t queue_index = ~0;
	SwapChainSupportDetails sws_details;

	vk::PhysicalDeviceMemoryProperties mem_properties;
	vk::PhysicalDeviceProperties properties;
};

export class GraphicsApi
{
public:
	constexpr static std::uint32_t m_max_frames_in_flight = 2;

public:
	explicit GraphicsApi(
		GLFWwindow * window, // Reminder: Do not call any glfw functions that require being on the main thread
		int width_pixels,
		int height_pixels,
		std::string const & app_title,
		std::uint32_t extension_count,
		char const ** extensions);

	void RecreateSwapChain(int width_pixels, int height_pixels);
	bool SwapChainIsValid() const;

	void DrawFrame(std::function<void()> render_fn, bool & out_window_size_out_of_date);
	void WaitForLastFrame() const;

	std::uint32_t FindMemoryType(std::uint32_t type_filter, VkMemoryPropertyFlags properties) const;
	std::uint32_t FindMemoryType(std::uint32_t type_filter, vk::MemoryPropertyFlags properties) const;

	VkResult Create2dImage(
		std::uint32_t width,
		std::uint32_t height,
		std::uint32_t layers,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkImageCreateFlags flags,
		VkMemoryPropertyFlags properties,
		VkImage & out_image,
		VkDeviceMemory & out_image_memory) const;

	VkResult CreateImageMemory(
		VkImage image,
		VkMemoryPropertyFlags properties,
		VkDeviceMemory & out_image_memory) const;

	VkResult CreateImageView(
		VkImage image,
		VkImageViewType view_type,
		VkFormat format,
		VkImageAspectFlags aspect_flags,
		std::uint32_t layers,
		VkImageView & out_image_view) const;

	void DoOneTimeCommand(std::function<void(vk::raii::CommandBuffer const &)> command_fn) const;
	void CopyBuffer(VkBuffer src_buffer,VkBuffer dst_buffer,VkDeviceSize size) const;
	void CopyBufferToImage(VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height, std::uint32_t layers) const;
	void TransitionImageLayout(VkImage image, std::uint32_t layers, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) const;

	// glm expects opengl style screen coordinates, so we need to flip the Y axis
	bool ShouldFlipScreenY() const { return true; }

	vk::raii::Device const & GetDevice() const { return m_logical_device; }
	vk::Extent2D GetSwapChainExtent() const { return m_swap_chain_extent; }
	vk::Format GetSwapChainImageFormat() const { return m_swap_chain_image_format; }
	vk::Image const & GetCurSwapChainImage() const { return m_swap_chain_images[m_current_image_index]; }
	vk::raii::ImageView const & GetCurSwapChainImageView() const { return m_swap_chain_image_views[m_current_image_index]; }
	vk::Format GetDepthImageFormat() const { return m_depth_image_format; }
	vk::raii::Image const & GetDepthImage() const { return m_depth_image; }
	vk::raii::ImageView const & GetDepthImageView() const { return m_depth_image_view; }
	vk::raii::CommandBuffer const & GetCurCommandBuffer() const { return m_command_buffers[m_current_frame]; }
	std::uint32_t GetCurFrameIndex() const { return m_current_frame; }

	PhysicalDeviceInfo const & GetPhysicalDeviceInfo() const { return m_phys_device_info; }

private:
	void destroy_swap_chain();

	void create_depth_resources();

private:
	vk::raii::Context m_context;
	vk::raii::Instance m_instance = nullptr;

	vk::raii::SurfaceKHR m_surface = nullptr;

	PhysicalDeviceInfo m_phys_device_info;
	vk::raii::Device m_logical_device = nullptr;
	vk::raii::Queue m_queue = nullptr;

	vk::raii::SwapchainKHR m_swap_chain = nullptr;
	vk::Format m_swap_chain_image_format = vk::Format::eUndefined;
	vk::Extent2D m_swap_chain_extent{ 0, 0 };
	std::vector<vk::Image> m_swap_chain_images;
	std::vector<vk::raii::ImageView> m_swap_chain_image_views;
	std::uint32_t m_current_image_index = 0;

	vk::Format m_depth_image_format = vk::Format::eUndefined;
	vk::raii::Image m_depth_image = nullptr;
	vk::raii::DeviceMemory m_depth_image_memory = nullptr;
	vk::raii::ImageView m_depth_image_view = nullptr;

	vk::raii::CommandPool m_command_pool = nullptr;
	vk::raii::CommandBuffers m_command_buffers = nullptr;

	// VkSemaphore is used for synchronizing commands on the gpu
	std::vector<vk::raii::Semaphore> m_present_complete_semaphores;
	std::vector<vk::raii::Semaphore> m_render_finished_semaphores;
	// VkFence is used for synchronizing the cpu with the gpu
	std::vector<vk::raii::Fence> m_draw_fences;

	std::uint32_t m_current_frame = 0;

	std::vector<const char *> const m_device_extensions = {
		vk::KHRSwapchainExtensionName
	};

	std::vector<char const *> m_validation_layers = {
		"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	bool m_enable_validation_layers = false;
#else
	bool m_enable_validation_layers = true;
#endif
};
