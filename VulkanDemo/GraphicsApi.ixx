// GraphicsApi.ixx

module;

#include <vulkan/vulkan.h>

export module GraphicsApi;

import <string>;

class GraphicsApi
{
public:
	GraphicsApi(std::string const & app_title, uint32_t extension_count, char const ** extensions);
	~GraphicsApi();

	VkInstance GetInstance() { return m_instance; }

private:
	VkInstance m_instance;
};
