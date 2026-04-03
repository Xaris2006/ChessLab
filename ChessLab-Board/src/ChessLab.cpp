#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/UI/UI.h"

#include "Windows/WindowsUtils.h"

#include "AppManagerChild.h"

#include "ChessAPI/ChessAPI.h"
#include "ChessCore/FileFormats/FileManager.h"
#include "ChessCore/FileFormats/ChessFileManager.h"
#include "ChessCore/FileFormats/cld/CldFile.h"

#include "ImGuiBoard.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/DatabasePanel.h"
#include "Panels/GamePropertiesPanel.h"
#include "Panels/NotePanel.h" //will be included in Move panel
#include "Panels/MovePanel.h"
#include "Panels/OpeningBookPanel.h"
#include "Panels/EnginePanel.h"

#include <iostream>
#include <array>
#include <fstream>

#include "GLFW/glfw3.h"

std::string g_AppDirectory;
std::filesystem::path g_cachedDirectory = "Resources\\cache";
Walnut::ApplicationSpecification g_spec;
bool g_AlreadyOpenedModalOpen = false;

static std::vector<std::string> s_arg;
static bool s_Mode = false;

class ChessLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
	{
		glfwMaximizeWindow(Walnut::Application::Get().GetWindowHandle());
		glfwFocusWindow(Walnut::Application::Get().GetWindowHandle());

		AppManagerChild::Init();
		Chess::FileManager::Init();
		Chess::ChessFileManager::Init();
		ChessAPI::Init();
		
		m_ChessBoard.OnAttach();

		if (s_arg.size() > 1)
		{
			if (Chess::IsFileValidFormat(s_arg[1], ".pgn"))
			{
				ChessAPI::OpenChessFile(s_arg[1]);
				AppManagerChild::OwnChessFile(ChessAPI::GetPgnFilePath());
			}
		}

		std::ifstream lsIni("chesslab.ini");
		std::string name;
		lsIni >> name >> ContentBrowserPanelViewStatus();
		lsIni >> name >> GamePropertiesPanelViewStatus();
		lsIni >> name >> NotePanelViewStatus();
		lsIni >> name >> MovePanelViewStatus();
		lsIni >> name >> OpeningBookPanelViewStatus();
		lsIni >> name >> ShowPossibleMoves();
		lsIni >> name >> ShowTags();
		lsIni >> name >> ShowArrows();
		lsIni >> name >> AskNewVariation();
		lsIni.close();

		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;

		if (false)
		{
			Chess::CldFile file;
			file.OpenFile("C:\\Users\\xarbe\\Documents\\appa\\appa.cld");

			while (file.GetSearchesCount() != 0)
				file.RemoveSearch(0);

			auto searchData = file.AddSearch();
			searchData->first.StartOption().And("WhiteElo", "1", 0).EndOption();
			searchData->first.StartOption().And("WhiteElo", "?", 0).EndOption();
			searchData->first.StartOption().And("BlackElo", "1", 0).EndOption();
			searchData->first.StartOption().And("BlackElo", "?", 0).EndOption();
			searchData->first.StartOption().And("WhiteTitle", "BOT").EndOption();
			searchData->first.StartOption().And("BlackTitle", "BOT").EndOption();
			file.StartSearch(0);

			using namespace std::chrono_literals;

			while (searchData->second.Persentage < 1)
				std::this_thread::sleep_for(1s);

			for (auto& re : searchData->second.PossitiveIndexes)
			{
				file.DeleteGame(re);
			}

			file.SaveFile("C:\\Users\\xarbe\\Documents\\appa\\appa.cld");

			file.Clear();

		}

		if(false)
		{
			Chess::CldFile::RemoveDeletedGames("C:\\Users\\xarbe\\Documents\\appa\\appa.cld");
		}
	}

	virtual void OnDetach() override
	{
		Chess::ChessFileManager::Shutdown();
		Chess::FileManager::Shutdown();
		AppManagerChild::ShutDown();
	}

	virtual void OnUIRender() override
	{	
		AppManagerChild::OnUpdate();

		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))
		{
			if (ImGui::IsKeyPressed(ImGuiKey_N))
			{
				if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))
					AppManagerChild::OpenChessFileInOtherApp();
				else
					New();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_O))
			{
				Open();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_S))
			{
				if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))
					SaveAs();
				else
					Save();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_F))
			{
				FlipBoard();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_E))
			{
				OpenEditor();
			}
			
			if (ImGui::IsKeyPressed(ImGuiKey_B))
			{
				ImGui::SetClipboardText(ChessAPI::GetActiveGame().GetFen().c_str());
			}

			if (ImGui::IsKeyPressed(ImGuiKey_R))
			{
				//s_Mode = !s_Mode;
				//if (s_Mode)
				//	ImGui::GetIO().IniFilename = "imgui2.ini";
				//else
				//	ImGui::GetIO().IniFilename = "imgui.ini";
				//ImGui::LoadIniSettingsFromDisk(ImGui::GetIO().IniFilename);
			}

			if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
			{
				OpenChessEngine();
			}
			
			if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
			{
				CloseChessEngine();
			}
		}

		m_ChessBoard.OnUIRender();
		m_ContentBrowserPanel.OnImGuiRender();
		m_DatabasePanel.OnImGuiRender();
		m_NotePanel.OnImGuiRender();
		m_OpeningBookPanel.OnImGuiRender();
		m_GamePropertiesPanel.OnImGuiRender();
		m_MovePanel.OnImGuiRender();
		m_ChessEnginePanel.OnImGuiRender();
		
		UI_DrawAboutModal();
		AlreadyOpenedModal();

		//ImGui::ShowDemoWindow();
		//ImGui::ShowMetricsWindow();
		//ImGui::ShowAboutWindow();
	}

	void UI_DrawAboutModal()
	{
		if (!m_AboutModalOpen)
			return;

		ImGui::OpenPopup("About");
		m_AboutModalOpen = ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		if (m_AboutModalOpen)
		{
			auto image = Walnut::Application::Get().GetApplicationIcon();
			ImGui::Image((ImTextureID)image->GetRendererID(), { 48, 48 });

			ImGui::SameLine();
			Walnut::UI::ShiftCursorX(20.0f);

			ImGui::BeginGroup();
			ImGui::Text("Chess Lab is a Chess GUI");
			ImGui::Text("by C.Betsakos");
			ImGui::EndGroup();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

			if (Walnut::UI::ButtonCentered("Close"))
			{
				m_AboutModalOpen = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::PopStyleColor(3);

			ImGui::EndPopup();
		}
	}

	void AlreadyOpenedModal()
	{
		if (!g_AlreadyOpenedModalOpen)
			return;
		ImGui::OpenPopup("Error! File Is Already Opened");

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		
		g_AlreadyOpenedModalOpen = ImGui::BeginPopupModal("Error! File Is Already Opened", 0);
		
		if (g_AlreadyOpenedModalOpen)
		{
			ImGui::TextWrapped("The file that you are trying to open is already opened in a different Chess Lab Window!");

			ImGui::NewLine();

			ImGui::PushID("in2");

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize("Close").x - 18);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));
			if (ImGui::Button("Close"))
			{
				g_AlreadyOpenedModalOpen = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor(3);

			ImGui::PopID();

			ImGui::EndPopup();
		}
	}

	bool& ContentBrowserPanelViewStatus()
	{
		return m_ContentBrowserPanel.IsPanelOpen();
	}
	
	bool& GamePropertiesPanelViewStatus()
	{
		return m_GamePropertiesPanel.IsPanelOpen();
	}

	bool& NotePanelViewStatus()
	{
		return m_NotePanel.IsPanelOpen();
	}

	bool& MovePanelViewStatus()
	{
		return m_MovePanel.IsPanelOpen();
	}
	
	bool& OpeningBookPanelViewStatus()
	{
		return m_OpeningBookPanel.IsPanelOpen();
	}

	void OpenChessEngine(const std::string& path)
	{
		m_ChessEnginePanel.OpenEngine(path);
	}

	void OpenChessEngine()
	{
		OpenChessEngine(m_ChessEnginePanel.GetDefaultEngine());
	}

	void CloseChessEngine()
	{
		m_ChessEnginePanel.CloseEngine();
		m_ChessEnginePanel.Reset();
	}

	std::vector<std::string>& GetAvailableChessEngines()
	{
		return m_ChessEnginePanel.GetAvailEngines();
	}

	void OpenEditor()
	{
		m_ChessBoard.OpenEditor();
	}

	void FlipBoard()
	{
		m_ChessBoard.FlipBoard();
	}

	bool& ShowPossibleMoves()
	{
		return m_ChessBoard.ShowPossibleMoves;
	}

	bool& ShowTags()
	{
		return m_ChessBoard.ShowTags;
	}

	bool& ShowArrows()
	{
		return m_ChessBoard.ShowArrows;
	}

	bool& AskNewVariation()
	{
		return m_ChessBoard.AskNewVariation;
	}

	void ShowAboutModal()
	{
		m_AboutModalOpen = true;
	}

	void New()
	{
		ChessAPI::OpenChessFile("");

		AppManagerChild::OwnChessFile("");
	}

	void Open()
	{
		std::string filepath = Windows::Utils::OpenFile(L"PGN Database (*.pgn)\0*.pgn\0Chess Lab Database (*.cld)\0*.cld\0\0");
		if (!filepath.empty())
		{
			bool anwser = AppManagerChild::IsChessFileAvail(filepath);

			if (anwser)
			{
				ChessAPI::OpenChessFile(filepath);
				AppManagerChild::OwnChessFile(ChessAPI::GetPgnFilePath());
			}
			else
			{
				g_AlreadyOpenedModalOpen = true;
			}
		}
	}

	void SaveAs()
	{
		std::string filepath = Windows::Utils::SaveFile(L"PGN Database (*.pgn)\0*.pgn\0Chess Lab Database (*.cld)\0*.cld\0\0");
		if (!filepath.empty())
		{
			ChessAPI::OverWriteChessFile(filepath);
			AppManagerChild::OwnChessFile(ChessAPI::GetPgnFilePath());
		}
	}

	void Save()
	{
		if (ChessAPI::GetPgnFileName() == "New Game")
		{
			SaveAs();
		}
		else
			ChessAPI::OverWriteChessFile("");
	}


private:

	ImGuiBoard m_ChessBoard;
	Panels::ContentBrowserPanel m_ContentBrowserPanel;
	Panels::DatabasePanel m_DatabasePanel;
	Panels::GamePropertiesPanel m_GamePropertiesPanel;
	Panels::NotePanel m_NotePanel;
	Panels::MovePanel m_MovePanel;
	Panels::OpeningBookPanel m_OpeningBookPanel;
	Panels::EnginePanel m_ChessEnginePanel;

	bool m_AboutModalOpen = false;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	g_spec.Name = "Chess Lab Board";
	g_spec.CustomTitlebar = true;
	g_spec.AppIconPath = "Resources\\ChessLab\\boardSmall.png";
	g_spec.IconPath = "Resources\\ChessLab\\cl.png";
	g_spec.HoveredIconPath = "Resources\\ChessLab\\clOnA.png";
	g_spec.FuncIconPressed = []()
		{
			AppManagerChild::OpenChessFileInOtherApp();
		};

	g_spec.AdditionalRightMenuIconPath.emplace_back("Resources\\Icons\\undo.png");
	g_spec.AdditionalRightMenuFuncIconPressed.emplace_back([]()
		{
			ChessAPI::OpenChessGameInFile(ChessAPI::GetActiveGameIndex() - 1);
		});
	g_spec.AdditionalRightMenuIconPath.emplace_back("Resources\\Icons\\return.png");
	g_spec.AdditionalRightMenuFuncIconPressed.emplace_back([]()
		{
			int oldActive = ChessAPI::GetActiveGameIndex();
			ChessAPI::OpenChessGameInFile(ChessAPI::GetActiveGameIndex() - 1);

			if (oldActive != ChessAPI::GetActiveGameIndex())
				ChessAPI::CloseOpenGame(oldActive);
		});
	g_spec.AdditionalRightMenuIconPath.emplace_back("Resources\\Icons\\plus.png");
	g_spec.AdditionalRightMenuFuncIconPressed.emplace_back([]()
		{
			ChessAPI::NewGameInFile();
		});
	g_spec.AdditionalRightMenuIconPath.emplace_back("Resources\\Icons\\Rreturn.png");
	g_spec.AdditionalRightMenuFuncIconPressed.emplace_back([]()
		{
			int oldActive = ChessAPI::GetActiveGameIndex();
			ChessAPI::OpenChessGameInFile(ChessAPI::GetActiveGameIndex() + 1);

			if (oldActive != ChessAPI::GetActiveGameIndex())
				ChessAPI::CloseOpenGame(oldActive);
		});
	g_spec.AdditionalRightMenuIconPath.emplace_back("Resources\\Icons\\Rundo.png");
	g_spec.AdditionalRightMenuFuncIconPressed.emplace_back([]()
		{
			ChessAPI::OpenChessGameInFile(ChessAPI::GetActiveGameIndex() + 1);
		});

	//fix arg
	s_arg.emplace_back(argv[0]);
	for (int i = 1; i < argc; i++)
	{
		if (std::filesystem::path(s_arg[s_arg.size() - 1]).has_extension())
			s_arg.emplace_back(argv[i]);
		else
		{
			s_arg[s_arg.size() - 1] += ' ';
			s_arg[s_arg.size() - 1] += argv[i];
		}
	}

	g_AppDirectory = std::filesystem::path(s_arg[0]).parent_path().string();

#if defined(WL_DIST)
	std::filesystem::current_path(g_AppDirectory);
#endif

	Walnut::Application* app = new Walnut::Application(g_spec, 237 + 60);
	std::shared_ptr<ChessLayer> chessLayer = std::make_shared<ChessLayer>();
	app->PushLayer(chessLayer);
	app->SetMenubarCallback([app, chessLayer]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New", "Ctr+N"))
			{
				chessLayer->New();
			}
			if (ImGui::MenuItem("New Window", "Ctr+Shift+N"))
			{
				AppManagerChild::OpenChessFileInOtherApp();
			}
			if (ImGui::MenuItem("Open", "Ctr+O"))
			{
				chessLayer->Open();
			}
			if (ImGui::MenuItem("Save", "Ctr+S"))
			{
				chessLayer->Save();
			}
			if (ImGui::MenuItem("Save As", "Ctr+Shift+S"))
			{
				chessLayer->SaveAs();
			}
			//if (ImGui::MenuItem("Remove Deleted and Save"))
			//{
			//	ChessAPI::DeleteGamesInFile();
			//}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "Alt+F4"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Content Browser", 0, &chessLayer->ContentBrowserPanelViewStatus())){}
			if (ImGui::MenuItem("Game Properties", 0, &chessLayer->GamePropertiesPanelViewStatus())){}
			if (ImGui::MenuItem("Notes",		   0, &chessLayer->NotePanelViewStatus())){}
			if (ImGui::MenuItem("Moves",		   0, &chessLayer->MovePanelViewStatus())){}
			if (ImGui::MenuItem("Opening Book",    0, &chessLayer->OpeningBookPanelViewStatus())){}

			ImGui::Separator();

			if (ImGui::MenuItem("Update View Style"))
			{
				std::ofstream lsIni("chesslab.ini");
				lsIni << "Content_Browser" << ' ' << chessLayer->ContentBrowserPanelViewStatus() << '\n';
				lsIni << "Game_Properties" << ' ' << chessLayer->GamePropertiesPanelViewStatus() << '\n';
				lsIni << "Notes"		   << ' ' << chessLayer->NotePanelViewStatus()			 << '\n';
				lsIni << "Moves"		   << ' ' << chessLayer->MovePanelViewStatus()			 << '\n';
				lsIni << "Opening_Book"	   << ' ' << chessLayer->OpeningBookPanelViewStatus()	 << '\n';
				lsIni << "Possible_Moves"  << ' ' << chessLayer->ShowPossibleMoves()			 << '\n';
				lsIni << "Tags"			   << ' ' << chessLayer->ShowTags()						 << '\n';
				lsIni << "Arrows"		   << ' ' << chessLayer->ShowArrows()					 << '\n';
				lsIni << "Ask_Variation"   << ' ' << chessLayer->AskNewVariation();
				lsIni.close();
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Board"))
		{
			if (ImGui::MenuItem("Copy Board", "Ctr+B"))
			{
				ImGui::SetClipboardText(ChessAPI::GetActiveGame().GetFen().c_str());
			}
			if (ImGui::MenuItem("Flip Board", "Ctr+F"))
			{
				chessLayer->FlipBoard();
			}
			if (ImGui::MenuItem("Editor", "Ctr+E"))
			{
				chessLayer->OpenEditor();
			}
			if (ImGui::MenuItem("Go Next Move", "Right Arrow"))
			{
				ChessAPI::GetActiveGame().GoNextMove();
			}
			if (ImGui::MenuItem("Go Previous Move", "Left Arrow"))
			{
				ChessAPI::GetActiveGame().GoPreviusMove();
			}
			if (ImGui::MenuItem("Show Possible Moves", 0, &chessLayer->ShowPossibleMoves())) {}
			if (ImGui::MenuItem("Show Tags", 0, &chessLayer->ShowTags())) {}
			if (ImGui::MenuItem("Show Arrows", 0, &chessLayer->ShowArrows())) {}
			if (ImGui::MenuItem("Ask for New Variation", 0, &chessLayer->AskNewVariation())) {}
			
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Engine"))
		{
			if (ImGui::MenuItem("Open Default", "Ctr+Up Arrow"))
			{
				chessLayer->OpenChessEngine();
			}
			if (ImGui::BeginMenu("Open"))
			{
				for (auto& Engine : chessLayer->GetAvailableChessEngines())
				{
					if (ImGui::MenuItem(Engine.c_str()))
						chessLayer->OpenChessEngine("MyDocuments\\engines\\" + Engine + ".exe");
				}
				if (ImGui::MenuItem("from Explorer..."))
				{
					std::string filepath = Windows::Utils::OpenFile(L"Chess Engine (*.exe)\0*.exe\0");
					if (!filepath.empty())
						chessLayer->OpenChessEngine(filepath);
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Close", "Ctr+Down Arrow"))
			{
				chessLayer->CloseChessEngine();
			}
			ImGui::EndMenu();
		}


		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About"))
			{
				chessLayer->ShowAboutModal();
			}
			ImGui::EndMenu();
		}

		ImGui::Spacing();
		
		{
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
	});
	return app;
}