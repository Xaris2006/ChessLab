#include "Engine_Manager.h"

#include "misc/cpp/imgui_stdlib.h"
#include "Walnut/UI/UI.h"

#define NOMINMAX
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3.h"
#include <GLFW/glfw3native.h>

#include "../../Windows/WindowsUtils.h"

#include <fstream>
#include <iostream>

static std::filesystem::path s_PathToEngines = "engines\\";
static std::vector<std::string> s_arg;

static int s_DownloadAvailIntex = -1;
static bool s_loadInProcess = false;
static int s_engineDownloading = 0;

static std::filesystem::path s_oldpath;
static std::string s_inputNName;

static std::filesystem::path s_path;
static bool s_openFilePopup = false;
static bool s_openRenamePopup = false;
static bool s_openEmptyPopup = false;
static bool s_openErrorODPopup = false;

extern std::string g_AppDirectory;
extern Walnut::ApplicationSpecification g_spec;

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


namespace Tools::EngineManager
{
	void Layer::OnAttach()
	{
		GetEngineSettings();
		FindAvailEngines();
		FindAvailOnWebEngines();

		m_EngineIcon = std::make_shared<Walnut::Image>("EngineIcon.png");
		m_RefreshIcon = std::make_shared<Walnut::Image>("sync.png");

		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_TableBorderLight] = ImColor(255, 225, 135, 80);

		glfwMaximizeWindow(Walnut::Application::Get().GetWindowHandle());
		glfwFocusWindow(Walnut::Application::Get().GetWindowHandle());
	}

	void  Layer::OnDetach()
	{

	}

	void Layer::OnUIRender()
	{
		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))
		{
			if (ImGui::IsKeyPressed(ImGuiKey_A))
			{
				AddEngine();
			}
		}

		if (ImGui::IsKeyPressed(ImGuiKey_F4) && (ImGui::IsKeyDown(ImGuiKey_LeftAlt) || ImGui::IsKeyDown(ImGuiKey_RightAlt)))
		{
			Walnut::Application::Get().Close();
		}

		ImGui::Begin("Main");

		ImGui::BeginTabBar("ProfileMenu");

		if (ImGui::BeginTabItem("Available"))
		{
			if (ImGui::BeginTable("table", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable))
			{
				ImGui::TableNextRow();

				//--Engines--

				ImGui::TableSetColumnIndex(0);

				ImGui::BeginChild("Engines", ImVec2(0, ImGui::GetContentRegionAvail().y - 8 * ImGui::GetStyle().ItemSpacing.y));

				if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
					s_openEmptyPopup = true;

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

				for (int i = 0; i < m_AvailableEngines.size(); i++)
				{
					const auto& path = s_PathToEngines / m_AvailableEngines[i];
					std::string filenameString = path.filename().u8string();

					if (!filter.PassFilter(filenameString.c_str()))
						continue;

					ImGui::PushID(filenameString.c_str());

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
					ImGui::ImageButton((ImTextureID)m_EngineIcon->GetRendererID(), { thumbnailSize, thumbnailSize });
					ImGui::PopStyleColor();


					if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					{
						s_openFilePopup = true;
						s_path = path;
						m_TargetedEngineIndex = i;
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

				ImGui::EndChild();

				ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - 6 * ImGui::GetStyle().ItemSpacing.y);

				ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 128, 256, "%.0f");

				//--Setting--

				ImGui::TableSetColumnIndex(1);

				ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));

				Walnut::UI::TextCentered("Settings");

				ImGui::PopFont();

				ImGui::Separator();

				ImGui::NewLine();

				if (ImGui::InputInt("Thread Count", &m_threadCount, 1, 1))
				{
					if (m_threadCount < 1) { m_threadCount = 1; }
					if (m_threadCount > std::thread::hardware_concurrency()) { m_threadCount = std::thread::hardware_concurrency(); }
				}
				if (ImGui::InputInt("Hash", &m_hashMb, 1, 1))
				{
					if (m_hashMb < 64) { m_hashMb = 64; }
					if (m_hashMb > 1024) { m_hashMb = 1024; }
				}

				ImGui::InputInt("Lines", &m_lines, 1, 1);
				if (m_lines < 1) { m_lines = 1; }
				if (m_lines > 5) { m_lines = 5; }

				ImGui::InputInt("Skill Level", &m_SkillLevel, 1, 1);
				if (m_SkillLevel < 0) { m_SkillLevel = 0; }
				if (m_SkillLevel > 20) { m_SkillLevel = 20; }

				ImGui::Separator();

				ImGui::InputText("Syzygy Path", &m_SyzygyPath);
				ImGui::Checkbox("50 Move Rule", &m_Syzygy50MoveRule);

				ImGui::Separator();

				ImGui::Checkbox("Limit Strength", &m_LimitStrength);

				ImGui::BeginDisabled(!m_LimitStrength);


				ImGui::InputInt("ELO", &m_Elo, 10, 100);
				if (m_Elo < 1320) { m_Elo = 1320; }
				if (m_Elo > 3190) { m_Elo = 3190; }

				ImGui::EndDisabled();

				ImGui::Separator();

				ImGui::PushStyleColor(ImGuiCol_Text, { 0, 0.66, 0.95, 1 });

				ImGui::Text("Default Engine: ");

				ImGui::PopStyleColor();

				ImGui::SameLine();

				ImGui::PushStyleColor(ImGuiCol_Text, { 0.38, 0.67, 0, 1 });

				ImGui::Text(m_DefaultEngine.c_str());

				ImGui::PopStyleColor();

				ImGui::Separator();

				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));

					float actualSize = ImGui::CalcTextSize(" Save ").x + ImGui::CalcTextSize(" Reset ").x + ImGui::GetStyle().FramePadding.x * 3.0f;
					float avail = ImGui::GetContentRegionAvail().x;

					float off = (avail - actualSize) * 0.5f;
					if (off > 0.0f)
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);


					if (ImGui::Button("Save"))
					{
						SetEngineSettings();
					}

					ImGui::PopStyleColor(3);
				}

				ImGui::SameLine();

				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

					if (ImGui::Button("Reset"))
					{
						GetEngineSettings();
					}

					ImGui::PopStyleColor(3);
				}

				ImGui::EndTable();
			}

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
					
					for (int i = 0; i < m_DownloadableEngines.size(); i++)
					{
						std::string& filenameString = m_DownloadableEngines[i].name;

						if (!filter.PassFilter(filenameString.c_str()))
							continue;

						ImGui::PushID(filenameString.c_str());

						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

						if (ImGui::ImageButton((ImTextureID)m_EngineIcon->GetRendererID(), { thumbnailSize, thumbnailSize }))
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

				ImGui::BeginDisabled(s_loadInProcess || s_engineDownloading);

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				if (ImGui::ImageButton((ImTextureID)m_RefreshIcon->GetRendererID(), { ImGui::CalcTextSize("C").y + ImGui::GetStyle().ItemSpacing.y / 3, ImGui::CalcTextSize("C").y + ImGui::GetStyle().ItemSpacing.y / 3 }))
					FindAvailOnWebEngines();
				ImGui::SameLine(0, ImGui::CalcTextSize(" <-  ").x);
				ImGui::PopStyleColor();

				ImGui::EndDisabled();

				ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 128, 256, "%.0f");

				//--Details

				ImGui::TableSetColumnIndex(1);

				if (s_DownloadAvailIntex > -1)
				{
					float size = std::min(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y) * 0.8f;

					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - size) * 0.5f);
					ImGui::Image((ImTextureID)m_EngineIcon->GetRendererID(), ImVec2(size, size));

					std::string filenameString = m_DownloadableEngines[s_DownloadAvailIntex].name;

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

					if (m_DownloadableEngines[s_DownloadAvailIntex].thread
						&& (m_DownloadableEngines[s_DownloadAvailIntex].status == Web::Finished || m_DownloadableEngines[s_DownloadAvailIntex].status == Web::Error))
					{
						m_DownloadableEngines[s_DownloadAvailIntex].thread->join();
						delete m_DownloadableEngines[s_DownloadAvailIntex].thread;
						m_DownloadableEngines[s_DownloadAvailIntex].thread = nullptr;

						m_DownloadableEngines[s_DownloadAvailIntex].status = Web::Nothing;
					}
					
					if (m_DownloadableEngines[s_DownloadAvailIntex].status == Web::Nothing)
					{
						{
							ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0.66, 0.95, 0.65 });
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 0.66, 0.95, 0.45 });
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 0.66, 0.95, 0.25 });

							if (Walnut::UI::ButtonCentered("Download") && !m_DownloadableEngines[s_DownloadAvailIntex].thread)
							{
								s_engineDownloading++;

								m_DownloadableEngines[s_DownloadAvailIntex].thread = new std::thread(
									[&]()
									{
										std::string filename = Web::DownLoadFileFromDropBox(m_DownloadableEngines[s_DownloadAvailIntex].url,m_DownloadableEngines[s_DownloadAvailIntex].status);

										if (m_DownloadableEngines[s_DownloadAvailIntex].status == Web::Finished)
										{
											std::error_code ec;
											std::filesystem::copy(std::filesystem::u8path(filename), s_PathToEngines / std::filesystem::u8path(filename).filename(), std::filesystem::copy_options::overwrite_existing, ec);
											
											if (!ec)
												std::filesystem::remove_all(std::filesystem::u8path(filename));
											
											FindAvailEngines();
										}

										s_engineDownloading--;
									}
								);
							}

							ImGui::PopStyleColor(3);
						}
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
				else
				{
					ImVec2 textSize = ImGui::CalcTextSize("Select an Engine to interact with it!");

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

					ImGui::Text("Select an Engine to interact with it!");

					ImGui::PopStyleColor();
				}
				
				ImGui::EndTable();
			}

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();

		ImGui::End();

		UI_DrawAboutModal();

		if (s_openFilePopup)
		{
			s_openFilePopup = false;

			ImGui::OpenPopup("File Popup");
		}

		FilePopup();

		if (s_openRenamePopup)
		{
			s_openRenamePopup = false;
			ImGui::OpenPopup("Rename Popup");
		}

		RenamePopup();

		if (s_openEmptyPopup)
		{
			s_openEmptyPopup = false;
			ImGui::OpenPopup("Empty Popup");
		}

		EmptyPopup();

		if (s_openErrorODPopup)
		{
			s_openErrorODPopup = false;
			ImGui::OpenPopup("Error on Deleting");
		}

		ErrorOnDeletingPopup();
	}

	void Layer::ShowAboutModal()
	{
		m_AboutModalOpen = true;
	}

	void Layer::FindAvailEngines()
	{
		m_AvailableEngines.clear();

		bool DefaultFinded = false;

		for (auto& directoryEntry : std::filesystem::directory_iterator(s_PathToEngines))
		{
			if (directoryEntry.path().extension().u8string() == ".exe")
			{
				if (directoryEntry.path().filename().u8string() == m_DefaultEngine)
					DefaultFinded = true;
				m_AvailableEngines.emplace_back(directoryEntry.path().filename());
			}
		}

		if (!DefaultFinded)
		{
			m_DefaultEngine = "";
			SetEngineSettings();
		}

		if (m_AvailableEngines.empty())
			return;

		std::ofstream outFile(s_PathToEngines / "Avail.ce");
		outFile << m_AvailableEngines[0].u8string();

		for (int i = 1; i < m_AvailableEngines.size(); i++)
			outFile << '\n' << m_AvailableEngines[i].u8string();

		outFile.close();
	}

	void Layer::FindAvailOnWebEngines()
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

				m_DownloadableEngines.clear();

				std::string file_id = "1P-SrPs7pG-H00sspTkTDsaN41Gs2mbY-";
				std::string file_at = "AN8xHoo1TeyyQ2xkkEYR5_qCvJvu:1749797388983";
				
				std::string file_path = "AvailToDownload_2.txt";

				static Web::DownLoadStatus status;
				Web::DownLoadFileFromGoogleDrive(file_id, file_at, status);

				std::ifstream inDownFile(std::filesystem::u8path(file_path));
				while (inDownFile.good())
				{
					//std::string name, id, at;
					//inDownFile >> name >> id >> at;
					
					std::string name, url;
					inDownFile >> name >> url;

					m_DownloadableEngines.emplace_back(DownloadableEngine{ name, url, Web::Nothing, nullptr });
				}
				inDownFile.close();

				s_loadInProcess = false;
			}
		);
	}

	void Layer::GetEngineSettings()
	{
		std::ifstream inFile(s_PathToEngines / "settings.ce");

		inFile >> m_threadCount;
		inFile >> m_hashMb;
		inFile >> m_lines;
		inFile >> m_SkillLevel;
		inFile >> m_SyzygyPath;
		inFile >> m_Syzygy50MoveRule;
		inFile >> m_LimitStrength;
		inFile >> m_Elo;
		inFile >> m_DefaultEngine;

		if (m_SyzygyPath == "False")
			m_SyzygyPath = "";

		if (m_DefaultEngine == "False")
			m_DefaultEngine = "";

		inFile.close();
	}

	void Layer::SetEngineSettings() const
	{
		std::ofstream outFile(s_PathToEngines / "settings.ce");

		outFile << m_threadCount		   << '\n';
		outFile << m_hashMb				   << '\n';
		outFile << m_lines				   << '\n';
		outFile << m_SkillLevel			   << '\n';
		
		if(m_SyzygyPath.empty())
			outFile << "False"			   << '\n';
		else
			outFile << m_SyzygyPath		   << '\n';
		
		outFile << (int)m_Syzygy50MoveRule << '\n';
		outFile << (int)m_LimitStrength	   << '\n';
		outFile << m_Elo				   << '\n';
		
		if (m_DefaultEngine.empty())
			outFile << "False"			   << '\n';
		else
			outFile << m_DefaultEngine     << '\n';
		

		outFile.close();
	}

	void Layer::AddEngine()
	{
		std::string filepath = Windows::Utils::OpenFile(L"Chess Engine (*.exe)\0*.exe\0");
		
		if (!filepath.empty())
		{
			std::error_code ec;
			std::filesystem::copy(std::filesystem::u8path(filepath), s_PathToEngines / std::filesystem::u8path(filepath).filename(), std::filesystem::copy_options::overwrite_existing, ec);
			
			FindAvailEngines();
		}
	}

	void Layer::UI_DrawAboutModal()
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
			ImGui::Text("Engine Manager is a Chess Engine Manager Tool");
			ImGui::Text("by C.Betsakos");
			ImGui::EndGroup();

			if (Walnut::UI::ButtonCentered("Close"))
			{
				m_AboutModalOpen = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void Layer::FilePopup()
	{
		if (ImGui::BeginPopup("File Popup"))
		{
			if (ImGui::Selectable("Rename"))
			{
				s_openRenamePopup = true;
				s_inputNName = s_path.filename().u8string();
				s_oldpath = s_path;
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Selectable("Set Default"))
			{
				m_DefaultEngine = m_AvailableEngines[m_TargetedEngineIndex].u8string();
				ImGui::CloseCurrentPopup();
			}

			ImGui::BeginDisabled(m_AvailableEngines.size() == 1 || m_AvailableEngines[m_TargetedEngineIndex].u8string() == m_DefaultEngine);

			if (ImGui::Selectable("Delete"))
			{
				std::error_code ec;
				std::filesystem::remove_all(s_PathToEngines / m_AvailableEngines[m_TargetedEngineIndex], ec);
				
				if (ec)
					s_openErrorODPopup = true;

				FindAvailEngines();

				ImGui::CloseCurrentPopup();
			}

			ImGui::EndDisabled();

			if (ImGui::Selectable("Open Explorer"))
			{
				std::string cmd = "explorer " + s_PathToEngines.u8string();
				std::system(cmd.c_str());
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void Layer::RenamePopup()
	{
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
				std::filesystem::path fileNpath = s_oldpath.parent_path() / s_inputNName;
				std::error_code ec;
				std::filesystem::rename(s_oldpath, fileNpath, ec); 
				
				if (!ec)
				{
					if (m_AvailableEngines[m_TargetedEngineIndex].u8string() == m_DefaultEngine)
						m_DefaultEngine = s_inputNName;
				}

				FindAvailEngines();
				SetEngineSettings();

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
	}

	void Layer::EmptyPopup()
	{
		if (ImGui::BeginPopup("Empty Popup"))
		{
			if (ImGui::Selectable("Add Engine"))
			{
				ImGui::CloseCurrentPopup();
				AddEngine();
			}

			if (ImGui::Selectable("Open Explorer"))
			{
				std::string cmd = "explorer " + s_PathToEngines.u8string();
				std::system(cmd.c_str());
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void Layer::ErrorOnDeletingPopup()
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Error on Deleting", 0, ImGuiWindowFlags_NoResize))
		{
			ImGui::TextWrapped("The engine that you are trying to delete is being used by a different application!");

			ImGui::NewLine();

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize("Close").x - 18);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
			if (ImGui::Button("Close"))
				ImGui::CloseCurrentPopup();

			ImGui::PopStyleColor();

			ImGui::EndPopup();
		}
	}

	Walnut::Application* CreateApplication(int argc, wchar_t** wargv)
	{
		g_spec.Name = "Engine Manager";
		g_spec.CustomTitlebar = true;
		g_spec.AppIconPath = "car-engine.png";
		g_spec.IconPath = "car-engine.png";
		g_spec.HoveredIconPath = "car-engine.png";
		g_spec.FuncIconPressed = []()
			{

			};

		//fix arg
		if (argc > 0)
		{

			s_arg.emplace_back(WCharToString(wargv[0]));
			for (int i = 1; i < argc; i++)
			{
				if (std::filesystem::u8path(s_arg[s_arg.size() - 1]).has_extension())
					s_arg.emplace_back(WCharToString(wargv[i]));
				else
				{
					s_arg[s_arg.size() - 1] += ' ';
					s_arg[s_arg.size() - 1] += WCharToString(wargv[i]);
				}
			}

			g_AppDirectory = std::filesystem::u8path(s_arg[0]).parent_path().u8string();

#if defined(WL_DIST)
			std::filesystem::current_path(g_AppDirectory);

			s_PathToEngines = "..\\..\\engines\\";
#endif
		}
	

		Walnut::Application* app = new Walnut::Application(g_spec, 117 - 15);

		//app->SetMinImGuiWindowSize(370.0f);
		app->SetDockNodeFlags(ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoTabBar);

		std::shared_ptr<Layer> EMLayer = std::make_shared<Layer>();
		app->PushLayer(EMLayer);
		app->SetMenubarCallback([app, EMLayer]()
			{
				if (ImGui::BeginMenu("Menu"))
				{
					if (ImGui::MenuItem("Add Engine", "Ctr+A"))
					{
						EMLayer->AddEngine();
					}

					if (ImGui::MenuItem("Exit", "Alt+F4"))
					{
						Walnut::Application::Get().Close();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Help"))
				{
					if (ImGui::MenuItem("About"))
					{
						EMLayer->ShowAboutModal();
					}
					ImGui::EndMenu();
				}
			});

		return app;
	}
}