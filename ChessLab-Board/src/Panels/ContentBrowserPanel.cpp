#include "ContentBrowserPanel.h"

#include "Walnut/Application.h"

#include "../AppManagerChild.h"

#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"
#include "../Panels.h"

#include <fstream>

namespace Panels {

	static std::filesystem::path s_oldpath;
	static std::string s_inputNName;
	static bool s_openPopup = false;

	void ContentBrowserPanel::OnAttach()
	{
		m_BaseDirectory = "MyDocuments";
		m_CurrentDirectory = m_BaseDirectory;

		m_DirectoryIcon = std::make_shared<Walnut::Image>("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = std::make_shared<Walnut::Image>("Resources/Icons/ContentBrowser/FileIcon.png");
		m_FileIconPGN = std::make_shared < Walnut::Image>("Resources/Icons/ContentBrowser/FileIconPGN.png");
		m_FileIconCOB = std::make_shared < Walnut::Image>("Resources/Icons/ContentBrowser/FileIconCOB.png");
	}
   
	void ContentBrowserPanel::OnImGuiRender()
	{
		if (!m_viewPanel)
			return;

		ImGui::Begin("Content Browser", &m_viewPanel);
		
		if (Panels::IsLoadingPopupOpen())
		{
			ImGui::End();
			return;
		}

		if (ImGui::IsWindowHovered() && !ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			ImGui::OpenPopup("Browser Popup");
		if (ImGui::BeginPopup("Browser Popup"))
		{
			if (ImGui::Selectable("New File"))
			{
				s_openPopup = true;
				s_inputNName = "NewFile.pgn";
				s_oldpath = m_CurrentDirectory;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("New Directory"))
			{
				s_openPopup = true;
				s_inputNName = "DirectoryName";
				s_oldpath = m_CurrentDirectory;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Open Explorer"))
			{
				std::string cmd = "explorer " + m_CurrentDirectory.string();
				std::system(cmd.c_str());
				ImGui::CloseCurrentPopup();
			}
			//if (ImGui::Selectable("New Directory"))
			//{
			//	s_openPopup = true;
			//	s_inputNName = "";
			//	s_oldpath = m_CurrentDirectory;
			//	ImGui::CloseCurrentPopup();
			//}

			ImGui::EndPopup();
		}

		if (s_openPopup)
		{
			s_openPopup = false;
			ImGui::OpenPopup("New File/Directory");
		}

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("New File/Directory", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::InputText("Name", &s_inputNName);

			ImGui::NewLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));
			if (ImGui::Button("Create"))
			{
				std::filesystem::path nPath = std::filesystem::path() / s_oldpath / s_inputNName;
				if (!nPath.extension().empty())
				{
					std::ofstream filepath(nPath.string());
					filepath.close();
				}
				else
				{
					std::error_code ec;
					std::filesystem::create_directory(nPath, ec);
					if (ec)
					{
						std::ofstream ef("ErrorFile.txt");
						ef << "func(std::filesystem::create_directory) " << ec << "path: " << nPath;
						ef.close();
					}
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));
			if (ImGui::Button("Cansel"))
				ImGui::CloseCurrentPopup();
			ImGui::PopStyleColor(3);
			ImGui::EndPopup();
		}

		if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory))
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			std::string filenameString = path.filename().u8string();
			//std::u8string filenameU8String = path.filename().u8string();
			//std::string filenameString = std::string(filenameU8String.begin(), filenameU8String.end());


			ImGui::PushID(filenameString.c_str());
			
			auto icon = m_FileIcon;
			if (directoryEntry.is_directory())
				icon = m_DirectoryIcon;
			else if (directoryEntry.path().extension().u8string() == u8".pgn")
				icon = m_FileIconPGN;
			else if (directoryEntry.path().extension().u8string() == u8".cob")
				icon = m_FileIconCOB;

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize });
			ImGui::PopStyleColor();

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (path.extension().u8string() == u8".pgn")
				{
					bool anwser = AppManagerChild::IsChessFileAvail(path);

					if (anwser)
					{
						ChessAPI::OpenChessFile(path.string());
						AppManagerChild::OwnChessFile(ChessAPI::GetChessFilePath());
					}
					else
					{
						Panels::OpenAlreadyOpenedPopup();
					}
				}
			}
			

			if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && directoryEntry.is_regular_file())
				ImGui::OpenPopup("File Popup");

			if (ImGui::BeginPopup("File Popup"))
			{
				if (ImGui::Selectable("Open"))
				{
					if (path.extension().u8string() == u8".pgn")
					{
						bool anwser = AppManagerChild::IsChessFileAvail(path);

						if (anwser)
						{
							ChessAPI::OpenChessFile(path.string());
							AppManagerChild::OwnChessFile(ChessAPI::GetChessFilePath());
						}
						else
						{
							Panels::OpenAlreadyOpenedPopup();
						}
					}

					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Open in new Window"))
				{
					if (path.extension().u8string() == u8".pgn")
					{
						bool anwser = AppManagerChild::IsChessFileAvail(path);

						if (anwser)
						{
							AppManagerChild::OpenChessFileInOtherApp(path);
						}
						else
						{
							Panels::OpenAlreadyOpenedPopup();
						}
					}

					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Delete"))
				{
					std::error_code ec;
					std::filesystem::remove(path, ec);
					if (ec)
					{
						std::ofstream ef("ErrorFile.txt");
						ef << "func(std::filesystem::remove) " << ec << "path: " << path;
						ef.close();
					}
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Rename"))
				{
					s_openPopup = true;
					s_inputNName = path.filename().string();
					s_oldpath = path;
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::Selectable("Open Explorer"))
				{
					std::string cmd = "explorer " + m_CurrentDirectory.string();
					std::system(cmd.c_str());
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			if (s_openPopup)
			{
				s_openPopup = false;
				ImGui::OpenPopup("Rename Popup");
			}

			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("Rename Popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::InputText("Name", &s_inputNName);

				ImGui::NewLine();

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));
				if (ImGui::Button("Rename"))
				{
					std::string fileNpath = s_oldpath.string().substr(0, s_oldpath.string().size() - s_oldpath.filename().string().size() - 1) + '\\' + s_inputNName;
					
					std::error_code ec;
					std::filesystem::rename(s_oldpath, fileNpath);
					if (ec)
					{
						std::ofstream ef("ErrorFile.txt");
						ef << "func(std::filesystem::rename) " << ec << "path: " << s_oldpath << " to: " << fileNpath;
						ef.close();
					}
					ImGui::CloseCurrentPopup();
				}
				ImGui::PopStyleColor(3);
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));
				if (ImGui::Button("Cansel"))
					ImGui::CloseCurrentPopup();
				ImGui::PopStyleColor(3);
				ImGui::EndPopup();
			}

			if (ImGui::BeginDragDropSource())
			{
				std::filesystem::path relativePath(path);
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
					m_CurrentDirectory /= path.filename();

			}

			int extensionIndex = filenameString.find_last_of('.');
			if (extensionIndex != std::string::npos)
				filenameString.erase(extensionIndex);

			float actualSize = ImGui::CalcTextSize(filenameString.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f;
			float avail = ImGui::GetContentRegionAvail().x;

			float off = (avail - actualSize) * 0.5f;
			if (off > 0.0f)
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

			ImGui::TextWrapped(filenameString.c_str());


			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);

		// TODO: status bar
		ImGui::End();
		
	}

	bool& ContentBrowserPanel::IsPanelOpen()
	{
		return m_viewPanel;
	}
}
