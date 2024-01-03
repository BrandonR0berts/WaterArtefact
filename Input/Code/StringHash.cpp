#include "StringHash.h"

namespace Engine
{
	std::hash<std::string> StringHash::hash = std::hash<std::string>();
}