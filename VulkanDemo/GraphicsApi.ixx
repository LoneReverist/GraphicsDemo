// GraphicsApi.ixx

module;

#include <vulkan/vulkan.h>

export module GraphicsApi;

import <string>;
import <vector>;

class GraphicsApi
{
public:
	GraphicsApi(std::string const & app_title, uint32_t extension_count, char const ** extensions);
	~GraphicsApi();

	VkInstance GetInstance() { return m_instance; }

private:
	VkInstance m_instance;

	std::vector<char const *> const m_validation_layers = {
		"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	bool const m_enable_validation_layers = false;
#else
	bool const m_enable_validation_layers = true;
#endif
};
