// Buffer.ixx

module;

#include <vulkan/vulkan.h>

export module Buffer;

import GraphicsApi;

export class Buffer
{
public:
	explicit Buffer(GraphicsApi const & graphics_api);
	~Buffer();

	Buffer(Buffer && other) noexcept;
	Buffer & operator=(Buffer && other) noexcept;

	Buffer(Buffer const &) = delete;
	Buffer & operator=(Buffer const &) = delete;

	VkResult Create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	void Destroy();

	VkBuffer Get() const { return m_buffer; }
	VkDeviceMemory GetMemory() const { return m_memory; }

private:
	GraphicsApi const & m_graphics_api;

	VkBuffer m_buffer = VK_NULL_HANDLE;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
};
