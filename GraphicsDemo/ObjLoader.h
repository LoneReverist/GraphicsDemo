// ObjLoader.h

#pragma once

#include "RenderObject.h"

namespace ObjLoader
{
	bool LoadObjFile(std::filesystem::path filepath, RenderObject & render_object);
}
