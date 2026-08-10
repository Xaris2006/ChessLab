#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Tools/Engine_Manager/Engine_Manager.h"

std::string g_AppDirectory;
std::filesystem::path g_cachedDirectory = "Resources\\cache";
Walnut::ApplicationSpecification g_spec;

Walnut::Application* Walnut::CreateApplication(int argc, wchar_t** wargv)
{
	return Tools::EngineManager::CreateApplication(argc, wargv);
}