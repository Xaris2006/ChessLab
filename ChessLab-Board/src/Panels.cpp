#include "Panels.h"

#include "AppManagerChild.h"
#include "ChessLabUtils.h"
#include "ChessAPI/ChessAPI.h"

#include "Windows/WindowsUtils.h"
#include "GLFW/glfw3.h"

static Panels::BoardPanel s_BoardPanel;
static Panels::DatabasePanel s_DatabasePanel;
static Panels::GamePropertiesPanel s_GamePropertiesPanel;
static Panels::EnginePanel s_EnginePanel;
static Panels::MovePanel s_MovePanel;
static Panels::NotePanel s_NotePanel;
static Panels::ContentBrowserPanel s_ContentBrowserPanel;
static Panels::ReferencePanel s_ReferencePanel;

static bool s_AlreadyOpenedPopupOpen = false;
static bool s_AboutPopupOpen = false;
static bool s_LoadingPopupOpen = false;

float s_LoadingPersentage;

namespace Panels
{
	void DrawAboutPopup()
	{
		if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
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

			ImGuiStyle& style = ImGui::GetStyle();

			float actualSize = ImGui::CalcTextSize("Close").x + style.FramePadding.x * 2.0f;
			float avail = ImGui::GetContentRegionAvail().x;

			float off = (avail - actualSize) * 0.5f;
			if (off > 0.0f)
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

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

		if (ImGui::BeginPopupModal("Error! File Is Already Opened", 0))
		{
			ImGui::TextWrapped("The file that you are trying to open is already opened in a different Chess Lab Window!");

			ImGui::NewLine();

			ImGui::PushID("in2");

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize("Close").x - 18);
			if (ImGui::Button("Close"))
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
		glfwMaximizeWindow(Walnut::Application::Get().GetWindowHandle());
		glfwFocusWindow(Walnut::Application::Get().GetWindowHandle());

		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::GetStyle().WindowRounding = 10.0f;
	}

	void OnAttach()
	{
		s_BoardPanel.OnAttach();
		s_DatabasePanel.OnAttach();
		s_EnginePanel.OnAttach();
	}

	void OnDetach()
	{
		s_EnginePanel.OnDetach();
	}

	void OnImGuiRender()
	{
		s_DatabasePanel.OnImGuiRender();
		s_GamePropertiesPanel.OnImGuiRender();
		s_EnginePanel.OnImGuiRender();
		s_NotePanel.OnImGuiRender();
		s_ContentBrowserPanel.OnImGuiRender();
		s_ReferencePanel.OnImGuiRender();
		s_MovePanel.OnImGuiRender();
		s_BoardPanel.OnImGuiRender();

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
				if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))
					AppManagerChild::OpenChessFileInOtherApp();
				else
					ChessLab::Utils::New();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_O))
			{
				ChessLab::Utils::Open();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_S))
			{
				if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))
					ChessLab::Utils::SaveAs();
				else
					ChessLab::Utils::Save();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_F))
			{
				Panels::GetBoardPanel().FlipBoard();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_E))
			{
				Panels::GetBoardPanel().OpenEditor();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_B))
			{
				ImGui::SetClipboardText(("CLF" + ChessAPI::GetActiveGame().GetFen()).c_str());
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
				Panels::GetEnginePanel().OpenEngine(Panels::GetEnginePanel().GetDefaultEngine());
			}

			if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
			{
				Panels::GetEnginePanel().CloseEngine();
				Panels::GetEnginePanel().Reset();
			}
		}
	}

	BoardPanel& GetBoardPanel()
	{
		return s_BoardPanel;
	}

	DatabasePanel& GetDatabasePanel()
	{
		return s_DatabasePanel;
	}

	GamePropertiesPanel& GetGamePropertiesPanel()
	{
		return s_GamePropertiesPanel;
	}

	EnginePanel& GetEnginePanel()
	{
		return s_EnginePanel;
	}

	MovePanel& GetMovePanel()
	{
		return s_MovePanel;
	}

	NotePanel& GetNotePanel()
	{
		return s_NotePanel;
	}

	ContentBrowserPanel& GetContentBrowserPanel()
	{
		return s_ContentBrowserPanel;
	}

	ReferencePanel& GetReferencePanel()
	{
		return s_ReferencePanel;
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