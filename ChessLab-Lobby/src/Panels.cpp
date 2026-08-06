#include "Panels.h"

#include "ChessLabUtils.h"
#include "Manager/AppManager.h"
#include "Update/Update.h"

#include "GLFW/glfw3.h"

#include "../../Walnut/Walnut/Platform/GUI/Walnut/UI/UI.h"

static Panels::ProfilePanel s_ProfilePanel;
static Panels::ToolsPanel s_ToolsPanel;
static Panels::HelpPanel s_HelpPanel;
static Panels::ContentBrowserPanel s_ContentBrowserPanel;

static int s_MenuIntex = 0;

static std::unique_ptr<Update> s_Update;

//menu Icons
static std::shared_ptr<Walnut::Image> s_HomeIcon;
static std::shared_ptr<Walnut::Image> s_ProfilIcon;
static std::shared_ptr<Walnut::Image> s_WebIcon;
static std::shared_ptr<Walnut::Image> s_ToolsIcon;
static std::shared_ptr<Walnut::Image> s_HelpIcon;
static std::shared_ptr<Walnut::Image> s_UpdateIcon;

static bool s_AlreadyOpenedPopupOpen = false;
static bool s_AboutPopupOpen = false;
static bool s_LoadingPopupOpen = false;

float s_LoadingPersentage;

namespace Panels
{
	void DrawAboutPopup()
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f / 255.0f, 225.0f / 255.0f, 135.0f / 255.0f, 255.0f / 255.0f));
		bool isOpen = ImGui::BeginPopupModal("About", 0, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::PopStyleColor();

		if (isOpen)
		{
			auto image = Walnut::Application::Get().GetApplicationIcon();
			ImGui::Image((ImTextureID)image->GetRendererID(), { 48, 48 });

			ImGui::SameLine();
			Walnut::UI::ShiftCursorX(20.0f);

			ImGui::BeginGroup();
			ImGui::Text("Chess Lab is a Chess GUI");
			ImGui::Text("by C.Betsakos");
			ImGui::EndGroup();

			ImGui::NewLine();

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.53f, 0.53f, 1.0f, 1.0f));
			ImGui::Text("Version: %s", s_Update->GetVersion().c_str());
			ImGui::PopStyleColor();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

			ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize("Close").x - ImGui::GetStyle().FramePadding.x - ImGui::GetStyle().WindowPadding.x);	
			if (ImGui::SmallButton("Close"))
			{
				s_AboutPopupOpen = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::PopStyleColor(3);

			ImGui::EndPopup();
		}
		else
			s_AboutPopupOpen = false;
	}

	void DrawAlreadyOpenedPopup()
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.1f, 0.1f, 0.85f));
		bool isOpen = ImGui::BeginPopupModal("Error! File Is Already Opened", 0, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::PopStyleColor();

		if (isOpen)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f / 255.0f, 225.0f / 255.0f, 135.0f / 255.0f, 255.0f / 255.0f));
			Walnut::UI::TextCentered("The file that you are trying to open is already opened");
			Walnut::UI::TextCentered("in a different Chess Lab Window!");
			ImGui::PopStyleColor();

			ImGui::NewLine();

			ImGui::PushID("in2");

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

			ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize("Close").x - ImGui::GetStyle().FramePadding.x - ImGui::GetStyle().WindowPadding.x);
			if (ImGui::SmallButton("Close"))
			{
				s_AlreadyOpenedPopupOpen = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::PopStyleColor(3);

			ImGui::PopID();

			ImGui::EndPopup();
		}
		else
			s_AlreadyOpenedPopupOpen = false;
	}

	void DrawLoadingPopup()
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Loading Operation", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.62f, 0.24f, 1.0f));

			Walnut::UI::TextCentered("Please wait patiently...");

			ImGui::PopStyleColor();

			ImGui::NewLine();

			auto ycursor = ImGui::GetCursorPosY();
			auto xcursor = ImGui::GetCursorPosX();
			float availx = ImGui::GetContentRegionAvail().x;

			ImGui::Button("##end", ImVec2(availx, 0));

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.353f, 0.314f, 0.0118f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.353f, 0.314f, 0.0118f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.353f, 0.314f, 0.0118f, 1.0f));

			if (s_LoadingPersentage < 1)
			{
				ImGui::SetCursorPosY(ycursor);
				ImGui::SetCursorPosX(xcursor);
				if (s_LoadingPersentage > 0)
					ImGui::Button("##bar", ImVec2(availx * s_LoadingPersentage, 0));
			}
			else
			{
				s_LoadingPersentage = 0;
				s_LoadingPopupOpen = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::PopStyleColor(3);

			ImGui::SetCursorPosY(ycursor);
			ImGui::SetCursorPosX(xcursor);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.58f, 0.97f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.58f, 0.97f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.58f, 0.97f, 0.0f));

			Walnut::UI::ButtonCentered((std::to_string((int)(s_LoadingPersentage * 100)) + '%').c_str(), { 60, ImGui::GetFrameHeight() });

			ImGui::PopStyleColor(3);

			ImGui::EndPopup();
		}
		else
			s_LoadingPopupOpen = false;
	}

	void Init()
	{
		s_Update = std::make_unique<Update>();
		
		s_HomeIcon = std::make_shared<Walnut::Image>("ChessLabApp\\Resources\\menu\\home-button.png");
		s_ProfilIcon = std::make_shared<Walnut::Image>("ChessLabApp\\Resources\\menu\\user.png");
		s_WebIcon = std::make_shared<Walnut::Image>("ChessLabApp\\Resources\\menu\\internet.png");
		s_ToolsIcon = std::make_shared<Walnut::Image>("ChessLabApp\\Resources\\menu\\spanner.png");
		s_HelpIcon = std::make_shared<Walnut::Image>("ChessLabApp\\Resources\\menu\\question-mark.png");
		s_UpdateIcon = std::make_shared<Walnut::Image>("ChessLabApp\\Resources\\menu\\up-arrow.png");

		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_TableBorderLight] = ImColor(255, 225, 135, 80);
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::GetStyle().WindowRounding = 10.0f;

		glfwMaximizeWindow(Walnut::Application::Get().GetWindowHandle());
		glfwFocusWindow(Walnut::Application::Get().GetWindowHandle());
	}

	void OnAttach()
	{
		s_ContentBrowserPanel.OnAttach();
		s_ProfilePanel.OnAttach();
		s_ToolsPanel.OnAttach();
		s_HelpPanel.OnAttach();

		auto arg = ChessLab::Utils::GetArguments();

		if (arg.size() > 1)
		{
			std::filesystem::path pathToOpen = std::filesystem::u8path(arg[1]);
			if (pathToOpen.is_relative())
				pathToOpen = std::filesystem::current_path() / std::filesystem::u8path(arg[1]);
			if (pathToOpen.extension().u8string() == ".pgn")
			{
				Manager::AppManager::Get().CreateApp(pathToOpen);
			}
		}
	}

	void OnDetach()
	{

	}

	void OnImGuiRender()
	{
		//menu
		ImGui::Begin("Menu", 0, ImGuiWindowFlags_NoDecoration);

		ImVec2 IconSize = { ImGui::GetContentRegionAvail().x - 2 * ImGui::GetStyle().ItemSpacing.x, ImGui::GetContentRegionAvail().x - 2 * ImGui::GetStyle().ItemSpacing.x };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		//ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y/8);
		if (ImGui::ImageButton((ImTextureID)s_HomeIcon->GetRendererID(), IconSize))
		{
			s_MenuIntex = 0;
		}
		if (ImGui::ImageButton((ImTextureID)s_ProfilIcon->GetRendererID(), IconSize))
		{
			s_MenuIntex = 1;
		}
		if (ImGui::ImageButton((ImTextureID)s_WebIcon->GetRendererID(), IconSize))
		{
			s_MenuIntex = 2;
		}
		if (ImGui::ImageButton((ImTextureID)s_ToolsIcon->GetRendererID(), IconSize))
		{
			s_MenuIntex = 3;
		}
		if (ImGui::ImageButton((ImTextureID)s_HelpIcon->GetRendererID(), IconSize))
		{
			s_MenuIntex = 4;
		}

		ImGui::BeginDisabled(!s_Update->IsUpdateAvailable());

		ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - ImGui::GetStyle().FramePadding.y * 2.0f - IconSize.y);
		if (ImGui::ImageButton((ImTextureID)s_UpdateIcon->GetRendererID(), IconSize))
		{
			s_Update->ShowUpdateModal();
		}

		ImGui::EndDisabled();

		ImGui::PopStyleColor();

		ImGui::End();

		if (s_MenuIntex == 0)
		{
			s_ContentBrowserPanel.OnImGuiRender();
		}
		else if (s_MenuIntex == 1)
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

			//s_ProfilePanel->OnImGuiRender();
		}
		else if (s_MenuIntex == 2)
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
		else if (s_MenuIntex == 3)
		{
			s_ToolsPanel.OnImGuiRender();
		}
		else if (s_MenuIntex == 4)
		{
			s_HelpPanel.OnImGuiRender();
		}

		s_Update->OnImGuiRender();

		if (s_AlreadyOpenedPopupOpen)
			ImGui::OpenPopup("Error! File Is Already Opened");

		if (s_LoadingPopupOpen)
			ImGui::OpenPopup("Loading Operation");

		if (s_AboutPopupOpen)
			ImGui::OpenPopup("About");

		DrawAlreadyOpenedPopup();
		DrawLoadingPopup();
		DrawAboutPopup();

		//ImGui::ShowDemoWindow();
		//ImGui::ShowMetricsWindow();
		//ImGui::ShowAboutWindow();
	}

	void OnKeyEvent()
	{
		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))
		{
			if (ImGui::IsKeyPressed(ImGuiKey_N))
			{
				ChessLab::Utils::New();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_O))
			{
				ChessLab::Utils::Open();
			}
		}
	}

	ProfilePanel& GetProfilePanel()
	{
		return s_ProfilePanel;
	}

	ToolsPanel& GetToolsPanel()
	{
		return s_ToolsPanel;
	}

	HelpPanel& GetHelpPanel()
	{
		return s_HelpPanel;
	}

	ContentBrowserPanel& GetContentBrowserPanel()
	{
		return s_ContentBrowserPanel;
	}

	void OpenAlreadyOpenedPopup()
	{
		s_AlreadyOpenedPopupOpen = true;
	}

	bool IsAlreadyOpenedPopupOpen()
	{
		return s_AlreadyOpenedPopupOpen;
	}

	void OpenLoadingPopup()
	{
		s_LoadingPopupOpen = true;
	}

	bool IsLoadingPopupOpen()
	{
		return s_LoadingPopupOpen;
	}

	float GetLoadingPersentage()
	{
		return s_LoadingPersentage;
	}

	float& GetLoadingPersentageRef()
	{
		return s_LoadingPersentage;
	}

	void SetLoadingPersentage(float persentage)
	{
		s_LoadingPersentage = persentage;
	}

	void OpenAboutPopup()
	{
		s_AboutPopupOpen = true;
	}

	bool IsAboutPopupOpen()
	{
		return s_AboutPopupOpen;
	}
}