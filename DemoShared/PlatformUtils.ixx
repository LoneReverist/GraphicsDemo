// PlatformUtils.ixx

module;

#include <filesystem>
#include <string>

#if defined(_WIN32)
#include <windows.h>

#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <ApplicationServices/ApplicationServices.h>

#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>

#endif

export module PlatformUtils;

namespace PlatformUtils
{
	export std::filesystem::path GetExecutablePath()
	{
		std::filesystem::path path;

#if defined(_WIN32)
		char buffer[MAX_PATH];
		if (GetModuleFileNameA(nullptr, buffer, MAX_PATH))
		{
			path = std::filesystem::path(buffer);
		}

#elif defined(__APPLE__)
		char buffer[PATH_MAX];
		uint32_t size = sizeof(buffer);
		if (_NSGetExecutablePath(buffer, &size) == 0)
		{
			path = std::filesystem::path(buffer);
		}

#elif defined(__linux__)
		char buffer[PATH_MAX];
		ssize_t count = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
		if (count != -1)
		{
			buffer[count] = '\0';
			path = std::filesystem::path(buffer);
		}

#else
		static_assert(false, "Unsupported platform.");
#endif

		if (path.empty())
		{
			throw std::runtime_error("Failed to get executable path.");
		}
		return path;
	}

	export std::filesystem::path GetExecutableDir()
	{
		return GetExecutablePath().parent_path();
	}

	export float GetDPIScalingFactor()
	{
#if defined(_WIN32)
		HDC screen = GetDC(nullptr);
		float dpiX = GetDeviceCaps(screen, LOGPIXELSX);
		ReleaseDC(nullptr, screen);
		return dpiX / 96.0f;

#elif defined(__APPLE__)
		CGDirectDisplayID displayID = CGMainDisplayID();
		size_t pixelWidth = CGDisplayPixelsWide(displayID);
		size_t pointWidth = CGDisplayBounds(displayID).size.width;
		return static_cast<float>(pixelWidth) / static_cast<float>(pointWidth);

#elif defined(__linux__)
		Display * display = XOpenDisplay(nullptr);
		if (!display)
			return 1.0f;

		float dpi = 96.0f;
		char * resourceString = XResourceManagerString(display);
		if (resourceString)
		{
			XrmDatabase db = XrmGetStringDatabase(resourceString);
			if (db)
			{
				XrmValue value;
				char * type = nullptr;
				if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value) && value.addr)
					dpi = atof(value.addr);

				XrmDestroyDatabase(db);
			}
		}

		XCloseDisplay(display);
		return dpi / 96.0f;

#else
		static_assert(false, "Unsupported platform for DPI scaling factor.");
		return 1.0f; // Default scaling factor
#endif
	}
}
