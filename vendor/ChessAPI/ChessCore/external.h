#pragma once

#include <functional>
#include <filesystem>

namespace Chess
{
	inline std::function<std::filesystem::path()> GetCacheDirectory = nullptr;
}