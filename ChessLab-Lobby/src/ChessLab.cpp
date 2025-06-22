#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/UI/UI.h"

#include "Windows/WindowsUtils.h"

#include "Manager/AppManager.h"
#include "Manager/ToolManager.h"
#include "ChessCore/pgn/PgnManager.h"

#include "Panels/ContentBrowserPanel.h"
#include "Panels/ProfilePanel.h"
#include "Panels/ToolsPanel.h"
#include "Panels/HelpPanel.h"

#include "Update/Update.h"

#include <iostream>
#include <array>

#include "GLFW/glfw3.h"

#include "implot.h"

std::string g_AppDirectory;
std::filesystem::path g_cachedDirectory = "ChessLabApp\\Resources\\cache";
Walnut::ApplicationSpecification g_spec;

bool g_AlreadyOpenedModalOpen = false;

static bool s_IamSecond = false;
static HANDLE s_hMutex;

static std::vector<std::string> s_arg;

class LobbyLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
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
			return;
		}

		Process startProcess(L"Start.exe", L"");
		
		Manager::AppManager::Init();
		Manager::ToolManager::Init();
		Chess::PgnManager::Init();

		m_ContentBrowserPanel = std::make_unique<Panels::ContentBrowserPanel>();
		m_ProfilePanel		  = std::make_unique<Panels::ProfilePanel>();
		m_ToolsPanel		  = std::make_unique<Panels::ToolsPanel>();
		m_HelpPanel			  = std::make_unique<Panels::HelpPanel>();

		m_Update			  = std::make_unique<Update>();

		m_HomeIcon			  = std::make_shared<Walnut::Image>("ChessLabApp\\Resources\\menu\\home-button.png");
		m_ProfilIcon		  = std::make_shared<Walnut::Image>("ChessLabApp\\Resources\\menu\\user.png");
		m_WebIcon			  = std::make_shared<Walnut::Image>("ChessLabApp\\Resources\\menu\\internet.png");
		m_ToolsIcon			  = std::make_shared<Walnut::Image>("ChessLabApp\\Resources\\menu\\spanner.png");
		m_HelpIcon			  = std::make_shared<Walnut::Image>("ChessLabApp\\Resources\\menu\\question-mark.png");
		m_UpdateIcon		  = std::make_shared<Walnut::Image>("ChessLabApp\\Resources\\menu\\up-arrow.png");

		using namespace std::chrono_literals;

		//std::this_thread::sleep_for(2s);

		startProcess.EndProcess();

		std::this_thread::sleep_for(200ms);

		if (s_arg.size() > 1)
		{
			std::filesystem::path pathToOpen(s_arg[1]);
			if (pathToOpen.is_relative())
				pathToOpen = std::filesystem::current_path() / s_arg[1];
			if (pathToOpen.extension().string() == ".pgn")
			{
				Manager::AppManager::Get().CreateApp(pathToOpen.string());
			}
			//else if (chess::IsFileValidFormat(commandLineArgs[1], ".cob"))
			//{
			//	//m_ChessPanel.IsOpeningBookPanelOpen() = true;
			//	m_chess.openBook.OpenCOBfile(commandLineArgs[1]);
			//}
		}

		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_TableBorderLight] = ImColor(255, 225, 135, 80);
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;

		glfwMaximizeWindow(Walnut::Application::Get().GetWindowHandle());
		glfwFocusWindow(Walnut::Application::Get().GetWindowHandle());
	}

	virtual void OnDetach() override
	{
		if (!s_IamSecond)
		{
			ReleaseMutex(s_hMutex);
			CloseHandle(s_hMutex);
		}
		else
			return;
		
		Chess::PgnManager::Shutdown();
		Manager::AppManager::Shutdown();
		Manager::ToolManager::Shutdown();
	}

	virtual void OnUIRender() override
	{
		if (s_IamSecond)
		{
			Walnut::Application::Get().Close();
			return;
		}

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

		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))
		{
			if (ImGui::IsKeyPressed(ImGuiKey_N))
			{
				New();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_O))
			{
				Open();
			}
		}

		//ImGui::ShowDemoWindow();
		//ImPlot::ShowDemoWindow();
		//ImGui::ShowMetricsWindow();

		//menu
		ImGui::Begin("Menu", 0, ImGuiWindowFlags_NoDecoration);

		ImVec2 IconSize = { ImGui::GetContentRegionAvail().x - 2*ImGui::GetStyle().ItemSpacing.x, ImGui::GetContentRegionAvail().x - 2* ImGui::GetStyle().ItemSpacing.x };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		//ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y/8);
		if (ImGui::ImageButton((ImTextureID)m_HomeIcon->GetRendererID(), IconSize))
		{
			m_MenuIntex = 0;
		}
		if (ImGui::ImageButton((ImTextureID)m_ProfilIcon->GetRendererID(), IconSize))
		{
			m_MenuIntex = 1;
		}
		if (ImGui::ImageButton((ImTextureID)m_WebIcon->GetRendererID(), IconSize))
		{
			m_MenuIntex = 2;
		}
		if (ImGui::ImageButton((ImTextureID)m_ToolsIcon->GetRendererID(), IconSize))
		{
			m_MenuIntex = 3;
		}
		if (ImGui::ImageButton((ImTextureID)m_HelpIcon->GetRendererID(), IconSize))
		{
			m_MenuIntex = 4;
		}

		ImGui::BeginDisabled(!m_Update->IsUpdateAvailable());

		ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - ImGui::GetStyle().FramePadding.y * 2.0f - IconSize.y);
		if (ImGui::ImageButton((ImTextureID)m_UpdateIcon->GetRendererID(), IconSize))
		{
			m_Update->ShowUpdateModal();
		}

		ImGui::EndDisabled();

		ImGui::PopStyleColor();

		ImGui::End();

		if (m_MenuIntex == 0)
		{
			m_ContentBrowserPanel->OnImGuiRender();
		}
		else if (m_MenuIntex == 1)
		{
			ImGui::Begin("Not Ready");
			
			ImVec2 textSize = ImGui::CalcTextSize("Coming Soon!");
			
			{
				float actualSizeX = textSize.x + ImGui::GetStyle().FramePadding.x * 2.0f;
				float availX = ImGui::GetContentRegionAvail().x;
			
				float offX = (availX - actualSizeX) * 0.5f;
				if (offX > 0.0f)
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offX);
			}
			
			{
				float actualSizeY = textSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
				float availY = ImGui::GetContentRegionAvail().y;
			
				float offY = (availY - actualSizeY) * 0.5f;
				if (offY > 0.0f)
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offY);
			}
			
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f / 255.0f, 225.0f / 255.0f, 135.0f / 255.0f, 255.0f / 255.0f));
			
			ImGui::Text("Coming Soon!");
			
			ImGui::PopStyleColor();
			
			ImGui::End();

			//m_ProfilePanel->OnImGuiRender();
		}
		else if (m_MenuIntex == 2)
		{
			ImGui::Begin("Not Ready");

			ImVec2 textSize = ImGui::CalcTextSize("Coming Soon!");

			{
				float actualSizeX = textSize.x + ImGui::GetStyle().FramePadding.x * 2.0f;
				float availX = ImGui::GetContentRegionAvail().x;

				float offX = (availX - actualSizeX) * 0.5f;
				if (offX > 0.0f)
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offX);
			}

			{
				float actualSizeY = textSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
				float availY = ImGui::GetContentRegionAvail().y;

				float offY = (availY - actualSizeY) * 0.5f;
				if (offY > 0.0f)
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offY);
			}

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f / 255.0f, 225.0f / 255.0f, 135.0f / 255.0f, 255.0f / 255.0f));

			ImGui::Text("Coming Soon!");

			ImGui::PopStyleColor();

			ImGui::End();
		}
		else if (m_MenuIntex == 3)
		{
			m_ToolsPanel->OnImGuiRender();
		}
		else if (m_MenuIntex == 4)
		{
			m_HelpPanel->OnImGuiRender();
		}

		m_Update->OnImGuiRender();

		UI_DrawAboutModal();
		AlreadyOpenedModal();
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
						
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f / 255.0f, 225.0f / 255.0f, 135.0f / 255.0f, 255.0f / 255.0f));
			ImGui::Text("v%s", m_Update->GetVersion().c_str());
			ImGui::PopStyleColor();

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
		ImGui::OpenPopup("Error-File Is Already Opened");

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		g_AlreadyOpenedModalOpen = ImGui::BeginPopupModal("Error-File Is Already Opened", 0, ImGuiWindowFlags_NoResize);

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

	void ShowAboutModal()
	{
		m_AboutModalOpen = true;
	}

	void New()
	{
		Manager::AppManager::Get().CreateApp("");
	}

	void Open()
	{
		std::string filepath = Windows::Utils::OpenFile("Chess Database (*.pgn)\0*.pgn\0");
		if (!filepath.empty())
			Manager::AppManager::Get().CreateApp(filepath);
	}

private:

	std::unique_ptr<Panels::ContentBrowserPanel> m_ContentBrowserPanel;
	std::unique_ptr<Panels::ProfilePanel>		 m_ProfilePanel;
	std::unique_ptr<Panels::ToolsPanel>			 m_ToolsPanel;
	std::unique_ptr<Panels::HelpPanel>			 m_HelpPanel;

	int m_MenuIntex = 0;

	std::unique_ptr<Update> m_Update;

	//menu Icons
	std::shared_ptr<Walnut::Image> m_HomeIcon;
	std::shared_ptr<Walnut::Image> m_ProfilIcon;
	std::shared_ptr<Walnut::Image> m_WebIcon;
	std::shared_ptr<Walnut::Image> m_ToolsIcon;
	std::shared_ptr<Walnut::Image> m_HelpIcon;
	std::shared_ptr<Walnut::Image> m_UpdateIcon;

	bool m_AboutModalOpen = false;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	g_spec.Name = "Chess Lab";
	g_spec.CustomTitlebar = true;
	g_spec.AppIconPath = "ChessLabApp\\Resources\\ChessLab\\clb.png";
	g_spec.IconPath = "ChessLabApp\\Resources\\ChessLab\\cl.png";
	g_spec.HoveredIconPath = "ChessLabApp\\Resources\\ChessLab\\clOnA.png";
	g_spec.FuncIconPressed = []()
		{
			Manager::AppManager::Get().CreateApp("");
		};

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

	Walnut::Application* app = new Walnut::Application(g_spec, 117 - 50);
	
	app->SetMinImGuiWindowSize(370.0f);
	app->SetDockNodeFlags(ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoTabBar);

	std::shared_ptr<LobbyLayer> chessLayer = std::make_shared<LobbyLayer>();
	app->PushLayer(chessLayer);
	app->SetMenubarCallback([app, chessLayer]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New", "Ctr+N"))
			{
				chessLayer->New();
			}
			if (ImGui::MenuItem("Open", "Ctr+O"))
			{
				chessLayer->Open();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "Alt+F4"))
			{
				app->Close();
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
	});

	return app;
}