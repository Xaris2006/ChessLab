#pragma once

#include "Walnut/Application.h"

namespace ChessLab::Utils
{
	void New();
	void Open();
	void SaveAs();
	void Save();

	void LoadViewStyle();
	void SaveViewStyle();

	void InitializeArguments(int argc, wchar_t** wargv);
	std::vector<std::string> GetArguments();

	void InitializeAppSpecification();
	Walnut::ApplicationSpecification& GetAppSpecification();

	void InitializeAppDirectory();
	std::filesystem::path GetAppDirectory();

	void InitializeCacheDirectory();
	std::filesystem::path GetCacheDirectory();

	void GetMenuBar();
}