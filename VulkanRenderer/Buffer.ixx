// Buffer.ixx

module;

#include <vulkan/vulkan_raii.hpp>

export module Buffer;

import GraphicsApi;
import GraphicsError;

export class Buffer
{
public:
	Buffer() = default;

	Buffer(Buffer &&) = default;
	Buffer & operator=(Buffer &&) = default;

	Buffer(Buffer const &) = delete;
	Buffer & operator=(Buffer const &) = delete;

	void Create(
		GraphicsApi const & graphics_api,
		vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags properties);

	vk::raii::Buffer const & Get() const { return m_buffer; }
	vk::raii::DeviceMemory const & GetMemory() const { return m_memory; }

private:
	vk::raii::Buffer m_buffer = nullptr;
	vk::raii::DeviceMemory m_memory = nullptr;
};
