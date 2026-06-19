#pragma once

#include <string>

namespace Windows::Utils
{
	std::string OpenFile(const wchar_t* Filter);

	std::string SaveFile(const wchar_t* Filter);
}