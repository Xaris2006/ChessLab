#pragma once

#include "Walnut/Application.h"

namespace ChessLab::Utils
{
	void New();
	void Open();

	void CreateSingularity();
	void DestroySingularity();
	bool IsSecondInstance();
	void ReadRequestFile();

	void InitializeArguments(int argc, char** argv);
	std::vector<std::string> GetArguments();

	void InitializeAppSpecification();
	Walnut::ApplicationSpecification& GetAppSpecification();

	void InitializeAppDirectory();
	std::filesystem::path GetAppDirectory();

	void InitializeCacheDirectory();
	std::filesystem::path GetCacheDirectory();

	void GetMenuBar();
}