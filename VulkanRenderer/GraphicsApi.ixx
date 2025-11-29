// GraphicsApi.ixx

module;

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

export module GraphicsApi;

struct QueueFamilyIndices
{
	std::optional<std::uint32_t> graphics_family;
	std::optional<std::uint32_t> present_family;

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
	constexpr static std::uint32_t m_max_frames_in_flight = 2;

public:
	explicit GraphicsApi(
		GLFWwindow * window, // Reminder: Do not call any glfw functions that require being on the main thread
		int width,
		int height,
		std::string const & app_title,
		std::uint32_t extension_count,
		char const ** extensions);

	~GraphicsApi();

	void RecreateSwapChain(int width, int height);
	bool SwapChainIsValid() const;

	void DrawFrame(std::function<void()> render_fn, bool & out_window_size_out_of_date);
	void WaitForLastFrame() const;

	std::uint32_t FindMemoryType(std::uint32_t type_filter, VkMemoryPropertyFlags properties) const;

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

	void DoOneTimeCommand(std::function<void(VkCommandBuffer)> const & command_fn) const;
	void CopyBuffer(VkBuffer src_buffer,VkBuffer dst_buffer,VkDeviceSize size) const;
	void CopyBufferToImage(VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height, std::uint32_t layers) const;
	void TransitionImageLayout(VkImage image, std::uint32_t layers, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) const;

	// glm expects opengl style screen coordinates, so we need to flip the Y axis
	bool ShouldFlipScreenY() const { return true; }

	VkDevice GetDevice() const { return m_logical_device; }
	VkFormat GetSwapChainImageFormat() const { return m_swap_chain_image_format; }
	VkExtent2D GetSwapChainExtent() const { return m_swap_chain_extent; }
	VkRenderPass GetRenderPass() const { return m_render_pass; }
	VkCommandBuffer GetCurCommandBuffer() const { return m_command_buffers[m_current_frame]; }
	VkFramebuffer GetCurFrameBuffer() const { return m_swap_chain_framebuffers[m_current_image_index]; }
	VkQueue GetGraphicsQueue() const { return m_graphics_queue; }
	std::uint32_t GetCurFrameIndex() const { return m_current_frame; }

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
	VkInstance m_instance = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;

	PhysicalDeviceInfo m_phys_device_info; // Automatically cleaned up when m_instance is destroyed
	VkDevice m_logical_device = VK_NULL_HANDLE;
	VkQueue m_graphics_queue = VK_NULL_HANDLE; // Automatically cleaned up when m_logical_device is destroyed
	VkQueue m_present_queue = VK_NULL_HANDLE; // Automatically cleaned up when m_logical_device is destroyed

	VkSwapchainKHR m_swap_chain = VK_NULL_HANDLE;
	VkFormat m_swap_chain_image_format = VK_FORMAT_UNDEFINED;
	VkExtent2D m_swap_chain_extent{ 0, 0 };
	std::vector<VkImage> m_swap_chain_images; // Automatically cleaned up when m_swap_chain is destroyed
	std::vector<VkImageView> m_swap_chain_image_views;
	std::vector<VkFramebuffer> m_swap_chain_framebuffers;
	std::uint32_t m_current_image_index = 0;

	VkRenderPass m_render_pass = VK_NULL_HANDLE;

	VkFormat m_depth_format = VK_FORMAT_UNDEFINED;
	VkImage m_depth_image = VK_NULL_HANDLE;
	VkDeviceMemory m_depth_image_memory = VK_NULL_HANDLE;
	VkImageView m_depth_image_view = VK_NULL_HANDLE;

	VkCommandPool m_command_pool = VK_NULL_HANDLE;
	std::array<VkCommandBuffer, m_max_frames_in_flight> m_command_buffers; // Automatically cleaned up when m_comand_pool is destroyed

	std::array<VkSemaphore, m_max_frames_in_flight> m_image_available_semaphores;
	std::array<VkSemaphore, m_max_frames_in_flight> m_render_finished_semaphores;
	std::array<VkFence, m_max_frames_in_flight> m_in_flight_fences;

	std::uint32_t m_current_frame = 0;

	std::vector<const char *> const m_device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
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
