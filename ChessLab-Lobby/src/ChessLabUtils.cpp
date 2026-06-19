#include "ChessLabUtils.h"

#include "Walnut/Application.h"

#include "Walnut/Image.h"

#include "Windows/WindowsUtils.h"

#include "Manager/AppManager.h"

#include "ChessCore/external.h"

#include "Panels.h"

#include <fstream>

static Walnut::ApplicationSpecification s_spec;
static std::vector<std::string> s_arg;

std::string s_AppDirectory;
std::filesystem::path s_CacheDirectory;

static bool s_IamSecond = false;
static HANDLE s_hMutex;

static std::string WCharToString(const wchar_t* w)
{
	if (!w) return {};

	int size = WideCharToMultiByte(
		CP_UTF8, 0,
		w, -1,
		nullptr, 0,
		nullptr, nullptr
	);

	std::string str(size, 0);

	WideCharToMultiByte(
		CP_UTF8, 0,
		w, -1,
		&str[0], size,
		nullptr, nullptr
	);

	// Remove Windows null terminator
	if (!str.empty() && str.back() == '\0')
		str.pop_back();

	return str;
}

namespace ChessLab::Utils
{
	void New()
	{
		Manager::AppManager::Get().CreateApp("");
	}

	void Open()
	{
		std::string filepath = Windows::Utils::OpenFile(L"Any Database (*.pgn, *.cld)\0*.pgn;*.cld\0PGN Database (*.pgn)\0*.pgn\0Chess Lab Database (*.cld)\0*.cld\0\0");
		if (!filepath.empty())
			Manager::AppManager::Get().CreateApp(filepath);
	}

	void CreateSingularity()
	{
		s_hMutex = CreateMutex(NULL, TRUE, L"MyUniqueAppMutex");

		if (GetLastError() == ERROR_ALREADY_EXISTS)
			s_IamSecond = true;

		if (s_IamSecond)
		{
			if (s_arg.size() > 1)
			{
				std::ofstream outfile("secondAppRequest.txt");
				outfile << s_arg[1];
				outfile.close();
			}
		}
	}

	void DestroySingularity()
	{
		if (!s_IamSecond)
		{
			ReleaseMutex(s_hMutex);
			CloseHandle(s_hMutex);
		}
	}

	bool IsSecondInstance()
	{
		return s_IamSecond;
	}

	void ReadRequestFile()
	{
		{
			std::ifstream infile("secondAppRequest.txt");
			std::string path;
			infile >> path;
			if (!path.empty())
			{
				std::filesystem::path pathToOpen(path);

				while (!pathToOpen.has_extension())
				{
					std::string addpath;
					infile >> addpath;
					path = path + ' ' + addpath;
					pathToOpen = std::filesystem::path(path);
				}

				if (pathToOpen.is_relative())
					pathToOpen = std::filesystem::current_path() / path;

				Manager::AppManager::Get().CreateApp(pathToOpen.string());
			}
			infile.close();
		}

		{
			std::ofstream outfile("secondAppRequest.txt");
			outfile << "";
			outfile.close();
		}
	}

	void GetMenuBar()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New", "Ctr+N"))
			{
				New();
			}
			if (ImGui::MenuItem("Open", "Ctr+O"))
			{
				Open();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "Alt+F4"))
			{
				Walnut::Application().Get().Close();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About"))
			{
				Panels::OpenAboutPopup();
			}
			ImGui::EndMenu();
		}
	}

	void InitializeArguments(int argc, wchar_t** wargv)
	{
		s_arg.clear();

		if (argc < 1)
			return;

		s_arg.emplace_back(WCharToString(wargv[0]));
		for (int i = 1; i < argc; i++)
		{
			if (std::filesystem::path(s_arg[s_arg.size() - 1]).has_extension())
				s_arg.emplace_back(WCharToString(wargv[i]));
			else
			{
				s_arg[s_arg.size() - 1] += ' ';
				s_arg[s_arg.size() - 1] += WCharToString(wargv[i]);
			}
		}
	}

	std::vector<std::string> GetArguments()
	{
		return s_arg;
	}

	void InitializeAppSpecification()
	{
		s_spec.Name = "Chess Lab";
		s_spec.CustomTitlebar = true;
		s_spec.AppIconPath = "ChessLabApp\\Resources\\ChessLab\\clb.png";
		s_spec.IconPath = "ChessLabApp\\Resources\\ChessLab\\cl.png";
		s_spec.HoveredIconPath = "ChessLabApp\\Resources\\ChessLab\\clOnA.png";
		s_spec.FuncIconPressed = []()
			{
				Manager::AppManager::Get().CreateApp("");
			};
	}

	Walnut::ApplicationSpecification& GetAppSpecification()
	{
		return s_spec;
	}

	void InitializeAppDirectory()
	{
		if (s_arg.empty())
		{
			s_AppDirectory = std::filesystem::current_path().string();
			return;
		}

		s_AppDirectory = std::filesystem::path(s_arg[0]).parent_path().string();

#if defined(WL_DIST)
		std::filesystem::current_path(s_AppDirectory);
#endif
	}

	std::filesystem::path GetAppDirectory()
	{
		return s_AppDirectory;
	}

	void InitializeCacheDirectory()
	{
		s_CacheDirectory = "ChessLabApp\\Resources\\cache";

		Chess::GetCacheDirectory = GetCacheDirectory;

		if (!std::filesystem::exists(s_CacheDirectory))
			std::filesystem::create_directory(s_CacheDirectory);
	}

	std::filesystem::path GetCacheDirectory()
	{
		return s_CacheDirectory;
	}
}