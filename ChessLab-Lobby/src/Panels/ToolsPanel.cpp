#include "ToolsPanel.h"

#include <fstream>

#include "imgui.h"

#include "Walnut/Application.h"
#include "Walnut/UI/UI.h"

#include "../windowsMain.h"
#include "../Manager/ToolManager.h"


static int s_DownloadAvailIntex = -1;
static bool s_loadInProcess = false;
static int s_toolDownloading = 0;

namespace Panels
{
	void ToolsPanel::OnAttach()
	{
		m_RefreshIcon = std::make_shared<Walnut::Image>("ChessLabApp\\Resources\\Icons\\sync.png");

		FindAvailableTools();
		FindDownloadableTools();
	}

	void ToolsPanel::OnImGuiRender()
	{
		for (int i = 0; i < m_DownloadableTools.size(); i++)
		{
			if (m_DownloadableTools[i].thread
				&& (m_DownloadableTools[i].status == Web::Finished || m_DownloadableTools[i].status == Web::Error))
			{
				m_DownloadableTools[i].thread->join();
				delete m_DownloadableTools[i].thread;
				m_DownloadableTools[i].thread = nullptr;

				m_DownloadableTools[i].status = Web::Nothing;

				FindAvailableTools();

				s_toolDownloading--;
			}
		}

		if (!m_DownloadableToolIconsToLoad.empty() && !s_loadInProcess)
		{
			m_DownloadableToolIcons.clear();

			for (auto& iconPath : m_DownloadableToolIconsToLoad)
			{
				//std::u8string iconPathU8String = iconPath.u8string();
				std::string iconPathString = iconPath.u8string();//std::string(iconPathU8String.begin(), iconPathU8String.end());

				m_DownloadableToolIcons.emplace_back(std::make_shared<Walnut::Image>(iconPathString));
			}

			m_DownloadableToolIconsToLoad.clear();
		}

		m_DownloadableToolExists.clear();

		for (auto& tool : m_DownloadableTools)
		{
			bool exist = false;

			for (auto& eTool : m_AvailableTools)
			{
				if (eTool.filename().string() == tool.name)
				{
					exist = true;
					break;
				}
			}

			m_DownloadableToolExists.emplace_back(exist);
		}

		ImGui::Begin("Tools");

		ImGui::BeginTabBar("ProfileMenu");

		if (ImGui::BeginTabItem("Available"))
		{
			if (ImGui::BeginTable("table", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				ImGui::BeginChild("Files", ImVec2(0, ImGui::GetContentRegionAvail().y - 8 * ImGui::GetStyle().ItemSpacing.y));

				static float padding = 32.0f;
				static float thumbnailSize = 128.0f;
				static float cellSize;
				cellSize = thumbnailSize + padding;

				static float panelWidth;
				panelWidth = ImGui::GetContentRegionAvail().x;
				static int columnCount;
				columnCount = (int)(panelWidth / cellSize);
				if (columnCount < 1)
					columnCount = 1;

				ImGui::PushStyleColor(ImGuiCol_Text, { 0.38, 0.67, 0, 1 });
				ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));

				auto oldCursorY = ImGui::GetCursorPosY();
				ImGui::SetCursorPosY(oldCursorY + 5);

				ImGui::Text("Search");

				ImGui::PopFont();
				ImGui::PopStyleColor();

				ImGui::SameLine();

				ImGui::SetCursorPosY(oldCursorY);

				static ImGuiTextFilter filter;
				filter.Draw("##SearchFilter", ImGui::GetContentRegionAvail().x / 3);

				ImGui::Separator();
				ImGui::NewLine();

				ImGui::Columns(columnCount, 0, false);

				for (int i = 0; i < m_AvailableTools.size(); i++)
				{
					auto& path = m_AvailableTools[i];
					std::string filenameString = path.filename().string();

					if (!filter.PassFilter(filenameString.c_str()))
						continue;

					ImGui::PushID(filenameString.c_str());

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
					if (ImGui::ImageButton((ImTextureID)m_ToolIcons[i]->GetRendererID(), { thumbnailSize, thumbnailSize }))
						m_TargetedToolIndex = i;

					ImGui::PopStyleColor();

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

				ImGui::EndChild();

				ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - 6 * ImGui::GetStyle().ItemSpacing.y);

				ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 128, 256, "%.0f");


				ImGui::TableSetColumnIndex(1);

				bool unistallCurrentTool = false;

				if (m_TargetedToolIndex > -1)
				{
					float size = min(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y) * 0.8f;

					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - size) * 0.5f);
					ImGui::Image((ImTextureID)m_ToolIcons[m_TargetedToolIndex]->GetRendererID(), ImVec2(size, size));

					std::string filenameString = m_AvailableTools[m_TargetedToolIndex].filename().string();

					{
						float actualSize = ImGui::CalcTextSize(filenameString.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f;
						float avail = ImGui::GetContentRegionAvail().x;

						float off = (avail - actualSize) * 0.5f;
						if (off > 0.0f)
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

						ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));

						ImGui::TextWrapped(filenameString.c_str());

						ImGui::PopFont();
					}

					ImGui::Separator();

					ImGui::NewLine();

					if (!Manager::ToolManager::Get().IsToolRunning(filenameString))
					{

						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));

							float actualSize = ImGui::CalcTextSize(" Open ").x + ImGui::CalcTextSize(" Unistall ").x + ImGui::GetStyle().FramePadding.x * 3.0f;
							float avail = ImGui::GetContentRegionAvail().x;

							float off = (avail - actualSize) * 0.5f;
							if (off > 0.0f)
								ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);


							if (ImGui::Button("Open"))
							{
								Manager::ToolManager::Get().RunTool(filenameString);
							}

							ImGui::PopStyleColor(3);
						}

						ImGui::SameLine();

						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

							if (ImGui::Button("Unistall"))
								unistallCurrentTool = true;

							ImGui::PopStyleColor(3);
						}
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

						if (Walnut::UI::ButtonCentered("Close"))
							Manager::ToolManager::Get().ShutdownTool(filenameString);

						ImGui::PopStyleColor(3);
					}

					ImGui::NewLine();

					ImGui::Separator();

					ImGui::BeginChild("Description", ImVec2(0, ImGui::GetContentRegionAvail().y / 2), true);

					Walnut::UI::TextCentered("Description");

					ImGui::NewLine();

					ImGui::TextWrapped(m_ToolLabelNameToValue[filenameString + "Description"].c_str());

					ImGui::EndChild();

					ImGui::BeginChild("Devaloper Details", ImVec2(0, ImGui::GetContentRegionAvail().y - 10), true);

					Walnut::UI::TextCentered("Devaloper Details");

					ImGui::NewLine();

					ImGui::TextWrapped(m_ToolLabelNameToValue[filenameString + "Devaloper_Details"].c_str());

					ImGui::EndChild();

				}
				else
				{
					ImVec2 textSize = ImGui::CalcTextSize("Select a Tool to interact with it!");

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

					ImGui::Text("Select a Tool to interact with it!");

					ImGui::PopStyleColor();
				}

				if (unistallCurrentTool)
				{
					std::string filenameString = m_AvailableTools[m_TargetedToolIndex].filename().string();

					std::error_code ec;
					std::filesystem::remove_all(m_AvailableTools[m_TargetedToolIndex], ec);
					if (ec)
					{
						std::ofstream ef("ErrorFile.txt");
						ef << "func(std::filesystem::remove_all) " << ec << "path: " << m_AvailableTools[m_TargetedToolIndex];
						ef.close();
					}

					m_AvailableTools.erase(m_AvailableTools.begin() + m_TargetedToolIndex);

					m_ToolLabelNameToValue.erase(filenameString + "Icon");
					m_ToolLabelNameToValue.erase(filenameString + "Description");
					m_ToolLabelNameToValue.erase(filenameString + "Devaloper_Details");

					m_ToolIcons.erase(m_ToolIcons.begin() + m_TargetedToolIndex);

					m_TargetedToolIndex = -1;
					unistallCurrentTool = false;
				}
			}

			ImGui::EndTable();

			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("Download"))
		{
			if (ImGui::BeginTable("table", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable))
			{
				ImGui::TableNextRow();

				//--Engines--

				ImGui::TableSetColumnIndex(0);

				ImGui::BeginChild("Engines", ImVec2(0, ImGui::GetContentRegionAvail().y - 8 * ImGui::GetStyle().ItemSpacing.y));

				static float padding = 32.0f;
				static float thumbnailSize = 128.0f;
				static float cellSize;
				cellSize = thumbnailSize + padding;

				static float panelWidth;
				panelWidth = ImGui::GetContentRegionAvail().x;
				static int columnCount;
				columnCount = (int)(panelWidth / cellSize);
				if (columnCount < 1)
					columnCount = 1;

				ImGui::PushStyleColor(ImGuiCol_Text, { 0.38, 0.67, 0, 1 });
				ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));

				auto oldCursorY = ImGui::GetCursorPosY();
				ImGui::SetCursorPosY(oldCursorY + 5);

				ImGui::Text("Search");

				ImGui::PopFont();
				ImGui::PopStyleColor();

				ImGui::SameLine();

				ImGui::SetCursorPosY(oldCursorY);

				static ImGuiTextFilter filter;
				filter.Draw("##SearchFilter", ImGui::GetContentRegionAvail().x / 3);

				ImGui::Separator();

				if (s_loadInProcess)
				{
					ImVec2 textSize = ImGui::CalcTextSize("Loading, Please wait patiently!");

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

					ImGui::Text("Loading, Please wait patiently!");

					ImGui::PopStyleColor();
				}
				else
				{
					ImGui::NewLine();

					ImGui::Columns(columnCount, 0, false);

					for (int i = 0; i < m_DownloadableTools.size(); i++)
					{
						std::string& filenameString = m_DownloadableTools[i].name;

						if (!filter.PassFilter(filenameString.c_str()))
							continue;

						ImGui::PushID(filenameString.c_str());

						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

						if (ImGui::ImageButton((ImTextureID)m_DownloadableToolIcons[i]->GetRendererID(), {thumbnailSize, thumbnailSize}))
							s_DownloadAvailIntex = i;

						ImGui::PopStyleColor();

						float actualSize = ImGui::CalcTextSize(filenameString.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f;
						float avail = ImGui::GetContentRegionAvail().x;

						float off = (avail - actualSize) * 0.5f;
						if (off > 0.0f)
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

						ImGui::TextWrapped(filenameString.c_str());

						ImGui::NextColumn();

						ImGui::PopID();
					}
				}

				ImGui::Columns(1);

				ImGui::EndChild();

				ImGui::Separator();

				ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - 6 * ImGui::GetStyle().ItemSpacing.y);

				ImGui::BeginDisabled(s_loadInProcess || s_toolDownloading);

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				if (ImGui::ImageButton((ImTextureID)m_RefreshIcon->GetRendererID(), { ImGui::CalcTextSize("C").y + ImGui::GetStyle().ItemSpacing.y / 3, ImGui::CalcTextSize("C").y + ImGui::GetStyle().ItemSpacing.y / 3 }))
					FindDownloadableTools();
				ImGui::SameLine(0, ImGui::CalcTextSize(" <-  ").x);
				ImGui::PopStyleColor();

				ImGui::EndDisabled();

				ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 128, 256, "%.0f");

				ImGui::TableSetColumnIndex(1);

				if (s_DownloadAvailIntex > -1)
				{
					float size = min(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y) * 0.8f;

					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - size) * 0.5f);
					ImGui::Image((ImTextureID)m_DownloadableToolIcons[s_DownloadAvailIntex]->GetRendererID(), ImVec2(size, size));

					std::string filenameString = m_DownloadableTools[s_DownloadAvailIntex].name;

					{
						float actualSize = ImGui::CalcTextSize(filenameString.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f;
						float avail = ImGui::GetContentRegionAvail().x;

						float off = (avail - actualSize) * 0.5f;
						if (off > 0.0f)
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

						ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));

						ImGui::TextWrapped(filenameString.c_str());

						ImGui::PopFont();
					}

					ImGui::Separator();

					ImGui::NewLine();

					if (m_DownloadableToolExists[s_DownloadAvailIntex])
					{
						ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0.66, 0.95, 0.65 });
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 0.66, 0.95, 0.65 });
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 0.66, 0.95, 0.65 });

						if (Walnut::UI::ButtonCentered("Already Downloaded"))
						{

						}

						ImGui::PopStyleColor(3);
					}
					else
					{
						if (m_DownloadableTools[s_DownloadAvailIntex].status == Web::Nothing)
						{
							ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0.66, 0.95, 0.65 });
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 0.66, 0.95, 0.45 });
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 0.66, 0.95, 0.25 });

							if (Walnut::UI::ButtonCentered("Download") && !m_DownloadableTools[s_DownloadAvailIntex].thread)
							{
								s_toolDownloading++;

								m_DownloadableTools[s_DownloadAvailIntex].thread = new std::thread(
									[&](int index)
									{
										std::string filename = Web::DownLoadFileFromGoogleDrive(m_DownloadableTools[index].id, m_DownloadableTools[index].at, m_DownloadableTools[index].status);

										if (m_DownloadableTools[index].status == Web::Finished)
										{
											std::string command = "powershell -command \"Expand-Archive -Path '";
											command += filename;
											command += "' -DestinationPath '";
											command += (std::filesystem::current_path() / "ChessLabApp\\MyDocuments\\Tools").string();
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
												return -1;
											}

											// Wait until the process completes
											WaitForSingleObject(pi.hProcess, INFINITE);

											// Close process and thread handles
											CloseHandle(pi.hProcess);
											CloseHandle(pi.hThread);

											std::cout << "Command executed successfully." << std::endl;

											return 0;

											std::error_code ec;
											std::filesystem::remove_all(filename, ec);
											if (ec)
											{
												std::ofstream ef("ErrorFile.txt");
												ef << "func(std::filesystem::remove_all) " << ec << "path: " << filename;
												ef.close();
											}
										}
									}
								, s_DownloadAvailIntex);
							}

							ImGui::PopStyleColor(3);
						}
						else
						{
							ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0.66, 0.95, 0.65 });
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 0.66, 0.95, 0.65 });
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 0.66, 0.95, 0.65 });

							if (Walnut::UI::ButtonCentered("Downloading"))
							{

							}

							ImGui::PopStyleColor(3);
						}
					}

					ImGui::NewLine();
					ImGui::Separator();

					ImGui::BeginChild("Description", ImVec2(0, ImGui::GetContentRegionAvail().y / 2), true);

					Walnut::UI::TextCentered("Description");

					ImGui::NewLine();

					ImGui::TextWrapped(m_DownloadableToolLabelNameToValue[filenameString + "Description"].c_str());

					ImGui::EndChild();

					ImGui::BeginChild("Devaloper Details", ImVec2(0, ImGui::GetContentRegionAvail().y - 10), true);

					Walnut::UI::TextCentered("Devaloper Details");

					ImGui::NewLine();

					ImGui::TextWrapped(m_DownloadableToolLabelNameToValue[filenameString + "Devaloper_Details"].c_str());

					ImGui::EndChild();
				}
				else
				{
					ImVec2 textSize = ImGui::CalcTextSize("Select a Tool to interact with it!");

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

					ImGui::Text("Select a Tool to interact with it!");

					ImGui::PopStyleColor();
				}

				ImGui::EndTable();
			}

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();

		ImGui::End();
	}

	//do not use it inside other threads! (opengl problems)
	void ToolsPanel::FindAvailableTools()
	{
		m_AvailableTools.clear();
		m_ToolLabelNameToValue.clear();
		m_ToolIcons.clear();
		m_TargetedToolIndex = -1;

		for (auto& directoryEntry : std::filesystem::directory_iterator(std::filesystem::current_path() / "ChessLabApp\\MyDocuments\\Tools\\"))
		{
			if (directoryEntry.is_directory())
			{
				m_AvailableTools.emplace_back(directoryEntry.path());
			}
		}

		for (auto& path : m_AvailableTools)
		{
			std::string filenameString = path.filename().string();

			std::ifstream toolDetails(path / "tool.details", std::ios::binary);
			toolDetails.seekg(0, std::ios_base::end);
			std::streampos maxSize = toolDetails.tellg();
			toolDetails.seekg(0, std::ios_base::beg);

			if (maxSize <= 0)
				continue;

			char* data = new char[maxSize];
			toolDetails.read(data, maxSize);
			toolDetails.close();

			bool labelName = true;
			std::string name = "", value = "";

			for (size_t i = 0; i < maxSize; i++)
			{
				if (data[i] == '\n' || data[i] == '\r')
					continue;
				if (labelName && data[i] == ' ')
					continue;

				if (data[i] == '[')
				{
					labelName = false;
					continue;
				}

				if (data[i] == ']')
				{
					labelName = true;

					m_ToolLabelNameToValue[filenameString + name] = value;

					name.clear();
					value.clear();

					continue;
				}

				if (labelName)
					name += data[i];
				else
					value += data[i];

			}

			delete[] data;
			
			auto iconPath = (path / m_ToolLabelNameToValue[filenameString + "Icon"]);
			//std::string iconPathU8String = iconPath.u8string();
			std::string iconPathString = iconPath.u8string();//std::string(iconPathU8String.begin(), iconPathU8String.end());

			m_ToolIcons.emplace_back(std::make_shared<Walnut::Image>(iconPathString));
			Manager::ToolManager::Get().AddTool(path / (filenameString + ".exe"), filenameString);
		}
	}

	void  ToolsPanel::FindDownloadableTools()
	{
		static std::thread* loadThread = nullptr;

		if (loadThread)
		{
			if (s_loadInProcess)
				return;

			loadThread->join();
			delete loadThread;
			loadThread = nullptr;
		}

		s_loadInProcess = true;

		loadThread = new std::thread(
			[&]()
			{
				s_DownloadAvailIntex = -1;

				std::this_thread::sleep_for(std::chrono::milliseconds(60));

				m_DownloadableTools.clear();
				m_DownloadableToolLabelNameToValue.clear();

#ifndef WL_DIST
				if (m_DownloadableToolIconsToLoad.size())
					__debugbreak();
#endif // !Dist

				static Web::DownLoadStatus status;
				std::string filename = Web::DownLoadFileFromGoogleDrive("1vQ3b6RYMkc89iqf3cwcDyaST9KYEIx6O", "AO7h07c8OtoRrFgJGB3ePNLiA9f4:1727098162643", status);
				
				std::ifstream inDownFile(filename);
				while (inDownFile.good())
				{
					std::string name, id, at, iid, iat, did, dat;
					inDownFile >> name >> id >> at >> iid >> iat >> did >> dat;
					m_DownloadableTools.emplace_back(DownloadableTool{ name, id, at, iid, iat, did, dat, Web::Nothing, nullptr });
				}
				inDownFile.close();

				std::filesystem::path IconPath = "ChessLabApp\\Resources\\DownLoadableToolIcons";

				std::error_code ec;
				std::filesystem::remove_all(IconPath, ec);
				if (ec)
				{
					std::ofstream ef("ErrorFile.txt");
					ef << "func(std::filesystem::remove_all) " << ec.message() << "path: " << IconPath;
					ef.close();
				}

				std::filesystem::create_directory(IconPath, ec);
				if (ec)
				{
					std::ofstream ef("ErrorFile.txt");
					ef << "func(std::filesystem::create_directory) " << ec.message() << "path: " << IconPath;
					ef.close();
				}
				
				for (auto& tool : m_DownloadableTools)
				{
					std::string filename = Web::DownLoadFileFromGoogleDrive(tool.iconId, tool.iconAt, status);

					std::filesystem::create_directory(IconPath / tool.name, ec);
					if (ec)
					{
						std::ofstream ef("ErrorFile.txt");
						ef << "func(std::filesystem::create_directory) " << ec.message() << "path: " << IconPath / tool.name;
						ef.close();
					}

					std::filesystem::copy_file(filename, IconPath / tool.name / filename, std::filesystem::copy_options::overwrite_existing, ec);
					if (ec)
					{
						std::ofstream ef("ErrorFile.txt");
						ef << "func(std::filesystem::copy) " << ec.message() << "path: " << filename << " to: " << IconPath / tool.name / filename;
						ef.close();
					}

					std::filesystem::remove_all(filename, ec);
					if (ec)
					{
						std::ofstream ef("ErrorFile.txt");
						ef << "func(std::filesystem::remove_all) " << ec.message() << "path: " << filename;
						ef.close();
					}

					m_DownloadableToolIconsToLoad.emplace_back(IconPath / tool.name / filename);

					std::string detailsfilename = Web::DownLoadFileFromGoogleDrive(tool.detailsId, tool.detailsAt, status);

					std::ifstream toolDetails(detailsfilename, std::ios::binary);
					toolDetails.seekg(0, std::ios_base::end);
					std::streampos maxSize = toolDetails.tellg();
					toolDetails.seekg(0, std::ios_base::beg);

					if (maxSize <= 0)
						continue;

					char* data = new char[maxSize];
					toolDetails.read(data, maxSize);
					toolDetails.close();

					bool labelName = true;
					std::string name = "", value = "";

					for (size_t i = 0; i < maxSize; i++)
					{
						if (data[i] == '\n' || data[i] == '\r')
							continue;
						if (labelName && data[i] == ' ')
							continue;

						if (data[i] == '[')
						{
							labelName = false;
							continue;
						}

						if (data[i] == ']')
						{
							labelName = true;

							m_DownloadableToolLabelNameToValue[tool.name + name] = value;

							name.clear();
							value.clear();

							continue;
						}

						if (labelName)
							name += data[i];
						else
							value += data[i];

					}

					delete[] data;

				}

				s_loadInProcess = false;
			}
		);
	}


}