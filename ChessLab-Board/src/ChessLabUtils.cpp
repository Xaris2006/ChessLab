#include "ChessLabUtils.h"

#include "ChessAPI/ChessAPI.h"
#include "AppManagerChild.h"
#include "Windows/WindowsUtils.h"

#include "Panels.h"

#include "ChessCore/external.h"

#include <fstream>

#include <Windows.h>

static Walnut::ApplicationSpecification s_spec;
static std::vector<std::string> s_arg;

std::string s_AppDirectory;
std::filesystem::path s_CacheDirectory;

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
		ChessAPI::OpenChessFile("");

		AppManagerChild::OwnChessFile("");
	}

	void Open()
	{
		std::string filepath = Windows::Utils::OpenFile(L"Any Database (*.pgn, *.cld)\0*.pgn;*.cld\0PGN Database (*.pgn)\0*.pgn\0Chess Lab Database (*.cld)\0*.cld\0\0");
		if (!filepath.empty())
		{
			bool anwser = AppManagerChild::IsChessFileAvail(filepath);

			if (anwser)
			{
				Panels::OpenLoadingPopup();

				static std::thread* openerThread = nullptr;

				if (openerThread != nullptr && openerThread->joinable())
				{
					openerThread->join();
					delete openerThread;
				}


				openerThread = new std::thread(
					[filepath]()
					{
						Panels::SetLoadingPersentage(0);
						ChessAPI::OpenChessFile(filepath, &Panels::GetLoadingPersentageRef());
						AppManagerChild::OwnChessFile(ChessAPI::GetChessFilePath());
						Panels::SetLoadingPersentage(1);
					});
			}
			else
			{
				Panels::OpenAlreadyOpenedPopup();
			}
		}
	}

	void SaveAs()
	{
		std::string filepath = Windows::Utils::SaveFile(L"PGN Database (*.pgn)\0*.pgn\0Chess Lab Database (*.cld)\0*.cld\0\0");
		if (!filepath.empty())
		{
			Panels::OpenLoadingPopup();

			std::thread(
				[filepath]()
				{
					Panels::SetLoadingPersentage(0);
					ChessAPI::OverWriteChessFile(filepath, &Panels::GetLoadingPersentageRef());
					AppManagerChild::OwnChessFile(ChessAPI::GetChessFilePath());
					Panels::SetLoadingPersentage(1);
				}
			).detach();
		}
	}

	void Save()
	{
		if (ChessAPI::GetChessFileName() == "New Game")
		{
			SaveAs();
		}
		else
		{
			Panels::OpenLoadingPopup();

			std::thread(
				[]()
				{
					Panels::SetLoadingPersentage(0);
					ChessAPI::OverWriteChessFile("", &Panels::GetLoadingPersentageRef());
					Panels::SetLoadingPersentage(1);
				}
			).detach();
		}
	}

	void LoadViewStyle()
	{
		std::ifstream lsIni("chesslab.ini");
		std::string name;
		lsIni >> name >> Panels::GetContentBrowserPanel().IsPanelOpen();
		lsIni >> name >> Panels::GetGamePropertiesPanel().IsPanelOpen();
		lsIni >> name >> Panels::GetNotePanel().IsPanelOpen();
		lsIni >> name >> Panels::GetMovePanel().IsPanelOpen();
		lsIni >> name >> Panels::GetOpeningBookPanel().IsPanelOpen();
		lsIni >> name >> Panels::GetBoardPanel().ShowPossibleMoves;
		lsIni >> name >> Panels::GetBoardPanel().ShowTags;
		lsIni >> name >> Panels::GetBoardPanel().ShowArrows;
		lsIni >> name >> Panels::GetBoardPanel().AskNewVariation;
		lsIni.close();
	}

	void SaveViewStyle()
	{
		std::ofstream lsIni("chesslab.ini");
		lsIni << "Content_Browser" << ' ' << Panels::GetContentBrowserPanel().IsPanelOpen() << '\n';
		lsIni << "Game_Properties" << ' ' << Panels::GetGamePropertiesPanel().IsPanelOpen() << '\n';
		lsIni << "Notes" << ' ' << Panels::GetNotePanel().IsPanelOpen() << '\n';
		lsIni << "Moves" << ' ' << Panels::GetMovePanel().IsPanelOpen() << '\n';
		lsIni << "Opening_Book" << ' ' << Panels::GetOpeningBookPanel().IsPanelOpen() << '\n';
		lsIni << "Possible_Moves" << ' ' << Panels::GetBoardPanel().ShowPossibleMoves << '\n';
		lsIni << "Tags" << ' ' << Panels::GetBoardPanel().ShowTags << '\n';
		lsIni << "Arrows" << ' ' << Panels::GetBoardPanel().ShowArrows << '\n';
		lsIni << "Ask_Variation" << ' ' << Panels::GetBoardPanel().AskNewVariation;
		lsIni.close();
	}

	void GetMenuBar()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New", "Ctr+N"))
			{
				ChessLab::Utils::New();
			}
			if (ImGui::MenuItem("New Window", "Ctr+Shift+N"))
			{
				AppManagerChild::OpenChessFileInOtherApp();
			}
			if (ImGui::MenuItem("Open", "Ctr+O"))
			{
				ChessLab::Utils::Open();
			}
			if (ImGui::MenuItem("Save", "Ctr+S"))
			{
				ChessLab::Utils::Save();
			}
			if (ImGui::MenuItem("Save As", "Ctr+Shift+S"))
			{
				ChessLab::Utils::SaveAs();
			}
			//if (ImGui::MenuItem("Remove Deleted and Save"))
			//{
			//	ChessAPI::DeleteGamesInFile();
			//}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "Alt+F4"))
			{
				Walnut::Application::Get().Close();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Content Browser", 0, &Panels::GetContentBrowserPanel().IsPanelOpen())) {}
			if (ImGui::MenuItem("Game Properties", 0, &Panels::GetGamePropertiesPanel().IsPanelOpen())) {}
			if (ImGui::MenuItem("Notes", 0, &Panels::GetNotePanel().IsPanelOpen())) {}
			if (ImGui::MenuItem("Moves", 0, &Panels::GetMovePanel().IsPanelOpen())) {}
			if (ImGui::MenuItem("Opening Book", 0, &Panels::GetOpeningBookPanel().IsPanelOpen())) {}
			ImGui::Separator();

			if (ImGui::MenuItem("Update View Style"))
			{
				SaveViewStyle();
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Board"))
		{
			if (ImGui::MenuItem("Copy Board", "Ctr+B"))
			{
				ImGui::SetClipboardText(("CLF" + ChessAPI::GetActiveGame().GetFen()).c_str());
			}
			if (ImGui::MenuItem("Flip Board", "Ctr+F"))
			{
				Panels::GetBoardPanel().FlipBoard();
			}
			if (ImGui::MenuItem("Editor", "Ctr+E"))
			{
				Panels::GetBoardPanel().OpenEditor();
			}
			if (ImGui::MenuItem("Go Next Move", "Right Arrow"))
			{
				ChessAPI::GetActiveGame().GoNextMove();
			}
			if (ImGui::MenuItem("Go Previous Move", "Left Arrow"))
			{
				ChessAPI::GetActiveGame().GoPreviusMove();
			}
			if (ImGui::MenuItem("Show Possible Moves", 0, &Panels::GetBoardPanel().ShowPossibleMoves)) {}
			if (ImGui::MenuItem("Show Tags", 0, &Panels::GetBoardPanel().ShowTags)) {}
			if (ImGui::MenuItem("Show Arrows", 0, &Panels::GetBoardPanel().ShowArrows)) {}
			if (ImGui::MenuItem("Ask for New Variation", 0, &Panels::GetBoardPanel().AskNewVariation)) {}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Engine"))
		{
			if (ImGui::MenuItem("Open Default", "Ctr+Up Arrow"))
			{
				Panels::GetEnginePanel().OpenEngine(Panels::GetEnginePanel().GetDefaultEngine());
			}
			if (ImGui::BeginMenu("Open"))
			{
				for (auto& Engine : Panels::GetEnginePanel().GetAvailEngines())
				{
					if (ImGui::MenuItem(Engine.c_str()))
						Panels::GetEnginePanel().OpenEngine("MyDocuments\\engines\\" + Engine + ".exe");
				}
				if (ImGui::MenuItem("from Explorer..."))
				{
					std::string filepath = Windows::Utils::OpenFile(L"Chess Engine (*.exe)\0*.exe\0");
					if (!filepath.empty())
						Panels::GetEnginePanel().OpenEngine(filepath);
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Close", "Ctr+Down Arrow"))
			{
				Panels::GetEnginePanel().CloseEngine();
				Panels::GetEnginePanel().Reset();
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

		ImGui::Spacing();

		{
			static bool s_Mode = false;

			float startingCursorX = ImGui::GetCursorPosX();

			ImVec2 size(ImGui::GetStyle().FramePadding.x * 4 + 13 + ImGui::CalcTextSize((s_Mode == true ? "Board" : "DataBase")).x, 37);

			static ImVec4 colorBoard(0.1f, 0.7f, 0.1f, 0.65f);
			static ImVec4 colorDatabase(0.3f, 0.58f, 0.97f, 0.6f);

			ImVec4 color = (s_Mode == true ? colorBoard : colorDatabase);

			ImGui::PushStyleColor(ImGuiCol_Button, color);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled | ImGuiItemFlags_ReadOnly, true);

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
			ImGui::Button("##ModeBackground", size);

			ImGui::PopItemFlag();
			ImGui::PopStyleColor(3);

			ImGui::SetCursorPosX(startingCursorX + 5);

			ImGui::GetStyle().FramePadding.y *= 0.4f;
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * ImGui::GetStyle().FramePadding.y + 5);
			ImGui::SetItemAllowOverlap();
			if (ImGui::RadioButton("##Mode", s_Mode))
			{
				s_Mode = !s_Mode;
				if (s_Mode)
					ImGui::GetIO().IniFilename = "imgui2.ini";
				else
					ImGui::GetIO().IniFilename = "imgui.ini";
				ImGui::LoadIniSettingsFromDisk(ImGui::GetIO().IniFilename);
			}
			ImGui::GetStyle().FramePadding.y *= 2.5f;

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 0.5f * ImGui::GetStyle().FramePadding.y);
			ImGui::Text((s_Mode == true ? "Board" : "DataBase"));
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
		s_spec.Name = "Chess Lab Board";
		s_spec.CustomTitlebar = true;
		s_spec.AppIconPath = "Resources\\ChessLab\\boardSmall.png";
		s_spec.IconPath = "Resources\\ChessLab\\cl.png";
		s_spec.HoveredIconPath = "Resources\\ChessLab\\clOnA.png";
		s_spec.FuncIconPressed = []()
			{
				AppManagerChild::OpenChessFileInOtherApp();
			};

		s_spec.AdditionalRightMenuIconPath.emplace_back("Resources\\Icons\\undo.png");
		s_spec.AdditionalRightMenuFuncIconPressed.emplace_back([]()
			{
				ChessAPI::OpenChessGameInFile(ChessAPI::GetActiveGameIndex() - 1);
			});
		s_spec.AdditionalRightMenuIconPath.emplace_back("Resources\\Icons\\return.png");
		s_spec.AdditionalRightMenuFuncIconPressed.emplace_back([]()
			{
				int oldActive = ChessAPI::GetActiveGameIndex();
				ChessAPI::OpenChessGameInFile(ChessAPI::GetActiveGameIndex() - 1);

				if (oldActive != ChessAPI::GetActiveGameIndex())
					ChessAPI::CloseOpenGame(oldActive);
			});
		s_spec.AdditionalRightMenuIconPath.emplace_back("Resources\\Icons\\plus.png");
		s_spec.AdditionalRightMenuFuncIconPressed.emplace_back([]()
			{
				ChessAPI::NewGameInFile();
			});
		s_spec.AdditionalRightMenuIconPath.emplace_back("Resources\\Icons\\Rreturn.png");
		s_spec.AdditionalRightMenuFuncIconPressed.emplace_back([]()
			{
				int oldActive = ChessAPI::GetActiveGameIndex();
				ChessAPI::OpenChessGameInFile(ChessAPI::GetActiveGameIndex() + 1);

				if (oldActive != ChessAPI::GetActiveGameIndex())
					ChessAPI::CloseOpenGame(oldActive);
			});
		s_spec.AdditionalRightMenuIconPath.emplace_back("Resources\\Icons\\Rundo.png");
		s_spec.AdditionalRightMenuFuncIconPressed.emplace_back([]()
			{
				ChessAPI::OpenChessGameInFile(ChessAPI::GetActiveGameIndex() + 1);
			});
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
		s_CacheDirectory = "Resources\\cache";

		Chess::GetCacheDirectory = GetCacheDirectory;

		if (!std::filesystem::exists(s_CacheDirectory))
			std::filesystem::create_directory(s_CacheDirectory);
	}

	std::filesystem::path GetCacheDirectory()
	{
		return s_CacheDirectory;
	}
}