#include "Update.h"

#include <fstream>
#include "Web.h"

#include "imgui.h"
#include "Walnut/ApplicationGUI.h"
#include "Walnut/ImGui/ImGuiTheme.h"
#include "Walnut/UI/UI.h"

#include <Windows.h>
#include "../windowsMain.h"

Update::Update()
{
	{
		std::ifstream infile("version.txt");
		infile >> m_Version;
		infile.close();

		m_LatestVersion = m_Version;
	}

	{
		Web::DownLoadStatus status = Web::Nothing;
		Web::DownLoadFileFromGoogleDrive("11FT3jadKrJbzUEwpd5H2qUZF7bggyNUM", "ALoNOgkrtxj6M_1Z5JK-Qdpb7cg4:1747596794374", status);

		std::ifstream infile("LatestVersion.txt");
		infile >> m_LatestVersion;
		infile.close();

		if (m_LatestVersion.empty())
			m_LatestVersion = '.';

		if (m_LatestVersion != m_Version && m_LatestVersion[0] != '.')
			m_UpdateAvailable = true;
	}
}

void Update::OnImGuiRender()
{
	UI_DrawUpdateModal();
	UI_DrawStartUpdatingModal();
}

void Update::ShowUpdateModal()
{
	m_UpdateModalOpen = true;
}

void Update::ShowStartUpdatingModal()
{
	m_StartUpdatingModalOpen = true;
}

std::string Update::GetVersion() const
{
	return m_Version;
}

std::string Update::GetLatestVersion() const
{
	return m_LatestVersion;
}

bool Update::IsUpdateAvailable() const
{
	return m_UpdateAvailable;
}

void Update::StartOperationForUpdate()
{
	if (m_Updater || !m_UpdateAvailable)
	{
		return;
	}

	m_Updater = new std::thread(
		[this]()
		{
			Web::DownLoadStatus status = Web::Nothing;
			auto name = Web::DownLoadFileFromGoogleDrive("1brkDjWEsoV5Y_UNrksbnzT_SIFLqZmir", "AN8xHooOZ0AaBH50QGYFw873WScq:1750179988527", status);

			std::filesystem::remove_all("toUpdate");

			if (status == Web::Finished)
			{
				std::string command = "powershell -command \"Expand-Archive -Path '";
				command += name;
				command += "' -DestinationPath '";
				command += (std::filesystem::current_path() / "toUpdate").string();
				command += "'\"";

				STARTUPINFO si = { sizeof(STARTUPINFO) };
				PROCESS_INFORMATION pi;
				si.dwFlags = STARTF_USESHOWWINDOW;
				si.wShowWindow = SW_HIDE;  // This hides the window

				// Create the process
				if (!CreateProcess(
					NULL,           // cmd executable
					(wchar_t*)std::wstring(command.begin(), command.end()).c_str(), // Command line (including arguments)
					NULL,                        // Process handle not inheritable
					NULL,                        // Thread handle not inheritable
					FALSE,                       // Set handle inheritance to FALSE
					CREATE_NO_WINDOW,            // Do not create a console window
					NULL,                        // Use parent's environment block
					NULL,                        // Use parent's starting directory 
					&si,                         // Pointer to STARTUPINFO structure
					&pi)                         // Pointer to PROCESS_INFORMATION structure
					)
				{
					std::cerr << "CreateProcess failed (" << GetLastError() << ").\n";
					m_UpdateFailed = false;

					m_ThreadEnded = true;
					return;
				}

				// Wait until the process completes
				WaitForSingleObject(pi.hProcess, INFINITE);

				// Close process and thread handles
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);

				std::cout << "Command executed successfully." << std::endl;


				//std::error_code ec;
				//std::filesystem::remove_all(name, ec);
				//if (ec)
				//{
				//	std::ofstream ef("ErrorFile.txt");
				//	ef << "func(std::filesystem::remove_all) " << ec << "path: " << name;
				//	ef.close();
				//}

				std::filesystem::copy_file(std::filesystem::current_path() / "Update.exe", std::filesystem::current_path() / "toUpdate" / "Update.exe", std::filesystem::copy_options::overwrite_existing);

				Process startProcess(L"toUpdate\\Update.exe", L"");
			}
			else
				m_UpdateFailed = true;

			m_ThreadEnded = true;
		});

}

void Update::UI_DrawUpdateModal()
{
	if (!m_UpdateModalOpen)
		return;

	ImGui::OpenPopup("Update");

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f / 255.0f, 225.0f / 255.0f, 135.0f / 255.0f, 255.0f / 255.0f));
	m_UpdateModalOpen = ImGui::BeginPopupModal("Update", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::PopStyleColor();
	
	if (m_UpdateModalOpen)
	{
		auto image = Walnut::Application::Get().GetApplicationIcon();
		ImGui::Image((ImTextureID)image->GetRendererID(), { 48, 48 });
		ImGui::SameLine();
		Walnut::UI::ShiftCursorX(20.0f);
		ImGui::BeginGroup();
		
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.53f, 0.53f, 1.0f, 1.0f));
		ImGui::Text("Version: %s", m_Version.c_str());
		ImGui::PopStyleColor();
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.53f, 1.0f, 0.53f, 1.0f));
		ImGui::Text("Latest Version: %s", m_LatestVersion.c_str());
		ImGui::PopStyleColor();
		
		ImGui::Separator();

		ImGui::PushFont(Walnut::Application::GetFont("Bold"));

		ImGui::Text("If you choose to update then make sure to save any opened files!");
		
		ImGui::PopFont();

		ImGui::EndGroup();

		ImGui::NewLine();

		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));

			float actualSize = ImGui::CalcTextSize(" Open ").x + ImGui::CalcTextSize(" Unistall ").x + ImGui::GetStyle().FramePadding.x * 3.0f;
			float avail = ImGui::GetContentRegionAvail().x;

			float off = (avail - actualSize) * 0.5f;
			if (off > 0.0f)
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);


			if (ImGui::SmallButton("Update"))
			{
				m_UpdateModalOpen = false;
				ImGui::CloseCurrentPopup();

				ShowStartUpdatingModal();
			}

			ImGui::PopStyleColor(3);
		}

		ImGui::SameLine();

		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

			if (ImGui::SmallButton("Cancel"))
			{
				m_UpdateModalOpen = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::PopStyleColor(3);
		}
		
		ImGui::EndPopup();
	}
}

void Update::UI_DrawStartUpdatingModal()
{
	if (!m_StartUpdatingModalOpen)
		return;
	ImGui::OpenPopup("Updating");
	m_StartUpdatingModalOpen = ImGui::BeginPopupModal("Updating", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	if (m_StartUpdatingModalOpen)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f / 255.0f, 225.0f / 255.0f, 135.0f / 255.0f, 255.0f / 255.0f));

		ImGui::Text("Updating Chess Lab to the latest version...");

		ImGui::PopStyleColor();

		StartOperationForUpdate();
		
		if (m_ThreadEnded)
		{
			if (m_Updater->joinable())
				m_Updater->join();

			delete m_Updater;
			m_Updater = nullptr;
			m_ThreadEnded = false;

			if (m_UpdateFailed)
			{
				m_UpdateFailed = false;
				
				ImGui::Text("Update Failed!");
				ImGui::Text("Please try again later!");
				
				ImGui::NewLine();

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

				if (Walnut::UI::ButtonCentered("Close"))
				{
					m_StartUpdatingModalOpen = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::PopStyleColor(3);
			}
			else
				Walnut::Application::Get().Close();
		}

		ImGui::EndPopup();
	}
}
