#pragma once

#include <xstring>

namespace Engine
{
	class StringHash
	{
	public:
		static unsigned int Hash(const std::string& stringToHash) { return (unsigned int)hash.operator()(stringToHash); }

	private:
		StringHash() = delete;
		~StringHash() = delete;

		static std::hash<std::string> hash;
	};
}