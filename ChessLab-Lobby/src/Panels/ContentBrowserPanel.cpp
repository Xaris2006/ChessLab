#include "ContentBrowserPanel.h"

#include "Walnut/Application.h"
#include "Walnut/UI/UI.h"

#include "../Manager/AppManager.h"

#include <imgui.h>
#include "misc/cpp/imgui_stdlib.h"

#include "ChessCore/FileFormats/Pgn/PgnFile.h"
#include "ChessCore/FileFormats/Cld/CldFile.h"

//#include <atlstr.h>
#include <shlobj.h>

namespace Panels {

	static std::filesystem::path s_oldpath;
	static std::string s_inputNName;
	static ImVec4 s_textColor;

	static std::filesystem::path s_path;
	static bool s_openFilePopup = false;
	static bool s_openEmptyPopup = false;
	static bool s_openRenamePopup = false;
	static bool s_openNewFolderPopup = false;
	static bool s_openNewFilePopup = false;

	void ContentBrowserPanel::OnAttach()
	{
		m_BaseDirectory = std::filesystem::current_path() / "ChessLabApp\\MyDocuments";
		m_CurrentDirectory = m_BaseDirectory;
		m_DirectoryIcon = std::make_shared<Walnut::Image>("ChessLabApp/Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon		= std::make_shared<Walnut::Image>("ChessLabApp/Resources/Icons/ContentBrowser/FileIcon.png");
		m_FileIconPGN   = std::make_shared<Walnut::Image>("ChessLabApp/Resources/Icons/ContentBrowser/FileIconPGN.png");
		m_FileIconCLD   = std::make_shared<Walnut::Image>("ChessLabApp/Resources/Icons/ContentBrowser/FileIconCLD.png");
		m_FileIconCOB	= std::make_shared<Walnut::Image>("ChessLabApp/Resources/Icons/ContentBrowser/FileIconCOB.png");
		m_BackArrow		= std::make_shared<Walnut::Image>("ChessLabApp/Resources/Icons/ContentBrowser/previous.png");

		s_textColor = ImGui::GetStyle().Colors[ImGuiCol_Text];
	}
   
	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		static ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable;

		if (ImGui::BeginTable("table", 2, flags))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));
			ImGui::Separator();
			
			Walnut::UI::TextCentered("Browser");

			ImGui::Separator();
			ImGui::Separator();
			ImGui::PopFont();

			ImGui::BeginChild("tree", ImVec2(0, ImGui::GetContentRegionAvail().y - 8 * ImGui::GetStyle().ItemSpacing.y));

			TreeDirectory(m_BaseDirectory);

			ImGui::Separator();

			std::filesystem::path userPath;
			
			//windows only
			{
				PWSTR userFolderPath;
				HRESULT result = SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &userFolderPath);

				if (result == S_OK)
				{
					userPath = std::filesystem::path(userFolderPath);
				}
				CoTaskMemFree(static_cast<LPVOID>(userFolderPath));
			}

			TreeDirectory(userPath / "Documents");
			ImGui::Separator();
			
			TreeDirectory(userPath / "Downloads");
			ImGui::Separator();
			
			TreeDirectory(userPath / "Desktop");
			ImGui::Separator();

			ImGui::EndChild();

			ImGui::TableSetColumnIndex(1);

			ImGui::Separator();

			std::filesystem::path semiPath = m_CurrentDirectory;
			std::vector<std::string> DirectoryNames;

			int indexEnd = 0;
			while (semiPath.has_parent_path() && semiPath.has_filename())
			{
				DirectoryNames.push_back(semiPath.filename().string());
				semiPath = semiPath.parent_path();
				indexEnd++;
			}

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (ImGui::ImageButton((ImTextureID)m_BackArrow->GetRendererID(), { ImGui::CalcTextSize("C").y + ImGui::GetStyle().ItemSpacing.y, ImGui::CalcTextSize("C").y + ImGui::GetStyle().ItemSpacing.y })
				&& m_CurrentDirectory.has_filename())
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
			ImGui::PopStyleColor();

			ImGui::SameLine(0, ImGui::CalcTextSize(" <-  ").x);

			if (ImGui::Button(semiPath.root_name().string().c_str()))
			{
				m_CurrentDirectory = m_CurrentDirectory.root_path();
				goto DirectoryChange;
			}
			ImGui::SameLine();
			ImGui::Text(">");

			for (int i = indexEnd - 1; i > -1; i--)
			{
				ImGui::SameLine();
				ImGui::PushID(i);
				if (ImGui::Button(DirectoryNames[i].c_str()))
				{
					for (int j = 0; j < i; j++)
					{
						m_CurrentDirectory = m_CurrentDirectory.parent_path();
					}
					ImGui::PopID();
					goto DirectoryChange;
				}
				ImGui::PopID();
				ImGui::SameLine();
				ImGui::Text(">");
			}

			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_Text, { 0.38, 0.67, 0, 1 });
			ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));

			float oldCursorY;
			oldCursorY = ImGui::GetCursorPosY();
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

			ImGui::BeginChild("Files", ImVec2(0, ImGui::GetContentRegionAvail().y - 8 * ImGui::GetStyle().ItemSpacing.y));

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

			ImGui::NewLine();

			ImGui::Columns(columnCount, 0, false);

			for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
			{
				const auto& path = directoryEntry.path();
				std::string filenameU8String = path.filename().u8string();
				std::string filenameString = std::string(filenameU8String.begin(), filenameU8String.end());

				if (!filter.PassFilter(filenameString.c_str()))
					continue;

				auto icon = m_FileIcon;
				if (directoryEntry.is_directory())
					icon = m_DirectoryIcon;
				else if (directoryEntry.path().extension().u8string() == u8".pgn")
					icon = m_FileIconPGN;
				else if (directoryEntry.path().extension().u8string() == u8".cld")
					icon = m_FileIconCLD;
				else if (directoryEntry.path().extension().u8string() == u8".cob")
					icon = m_FileIconCOB;

				if (m_showChessFilesOnly && !directoryEntry.is_directory() && !(directoryEntry.path().extension().u8string() == u8".pgn" || directoryEntry.path().extension().u8string() == u8".cld"))
					continue;
				
				ImGui::PushID(filenameString.c_str());

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize });
				ImGui::PopStyleColor();

				if (directoryEntry.is_regular_file() && (path.extension().u8string() == u8".pgn" || path.extension().u8string() == u8".cld"))
				{
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						Manager::AppManager::Get().CreateApp(path);
					}

					if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					{
						s_openFilePopup = true;
						s_path = path;
					}
					
					if (ImGui::BeginDragDropSource())
					{
						if (!Manager::AppManager::Get().IsAppOpen(path))
						{
							std::filesystem::path relativePath(path);
							const wchar_t* itemPath = relativePath.c_str();
							ImGui::SetDragDropPayload("MERGE_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
						}
						ImGui::EndDragDropSource();
					}

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MERGE_ITEM"))
						{
							const wchar_t* wpath = (const wchar_t*)payload->Data;
							std::filesystem::path spath(wpath);

							if (spath.has_extension() && path.has_extension())
							{
								MergeFiles(path, { path, spath });
							}
						}
						ImGui::EndDragDropTarget();
					}

				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (directoryEntry.is_directory())
						m_CurrentDirectory /= path.filename();
				}

				ImGuiStyle& style = ImGui::GetStyle();

				int extensionIndex = filenameString.find_last_of('.');
				if(extensionIndex != std::string::npos)
					filenameString.erase(extensionIndex);

				float actualSize = ImGui::CalcTextSize(filenameString.c_str()).x + style.FramePadding.x * 2.0f;
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

			ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 64, 256, "%.0f");
			//ImGui::SliderFloat("Padding", &padding, 0, 32);

			ImGui::SameLine();

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize("BOX HERE Show Chess Files Only").x);

			ImGui::Checkbox("Show Chess Files Only", &m_showChessFilesOnly);

		DirectoryChange:
			ImGui::EndTable();
		}

		MergeChildPanel();

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

		if (s_openNewFolderPopup)
		{
			s_openNewFolderPopup = false;
			ImGui::OpenPopup("New Folder");
		}

		NewFolderPopup();

		if (s_openNewFilePopup)
		{
			s_openNewFilePopup = false;
			ImGui::OpenPopup("New File");
		}

		NewFilePopup();

		ImGui::End();
	}

	void ContentBrowserPanel::MergeChildPanel()
	{
		if (m_filesToBeMerged.empty())
			return;
		
		ImGui::Begin("Merge");

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.58f, 0.97f, 1.0f));
		ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));

		Walnut::UI::TextCentered("Files to be merged");

		ImGui::PopFont();
		ImGui::PopStyleColor();

		ImGui::Separator();

		static int fileToRemove = 0;

		if (ImGui::BeginTable("Files", 4, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg,
			ImVec2(0, ImGui::GetContentRegionAvail().y - 8 * 2 * ImGui::GetStyle().ItemSpacing.y)))
		{
			ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_NoHide);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);
			ImGui::TableSetupColumn("Full Path", ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);
			ImGui::TableSetupColumn("##Remove", ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);

			ImGui::TableHeadersRow();

			for (int i = 0; i < m_filesToBeMerged.size(); i++)
			{
				ImGui::TableNextRow();

				for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
				{
					if (!ImGui::TableSetColumnIndex(column) && column > 0)
						continue;

					ImGui::PushID(i * ImGui::TableGetColumnCount() + column);

					if (column == 0)
						ImGui::Text("%d", i + 1);
					else if (column == 1)
					{
						ImGui::Text(m_filesToBeMerged[i].filename().string().c_str());
					}
					else if (column == 2)
					{
						ImGui::Text(m_filesToBeMerged[i].string().c_str());
					}
					else
					{
						ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(0.7f, 0.1f, 0.1f, 0.65f)));
						ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
						ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

						ImGui::SetNextItemOpen(true);

						ImGui::Selectable("Remove");

						ImGui::PopStyleColor(2);

						if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
						{
							fileToRemove = i + 1;
						}
					}

					ImGui::PopID();
				}
			}

			ImGui::EndTable();
		}

		if (fileToRemove)
		{
			m_filesToBeMerged.erase(m_filesToBeMerged.begin() + fileToRemove - 1);
			fileToRemove = 0;
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MERGE_ITEM"))
			{
				const wchar_t* wpath = (const wchar_t*)payload->Data;
				std::filesystem::path spath(wpath);
				
				bool alreadyAdded = false;
				
				if (m_filesToBeMerged.empty())
					m_mergedName = spath.filename().string();
				else
				{
					for (auto& other : m_filesToBeMerged)
					{
						if (other == spath)
						{
							alreadyAdded = true;
							break;
						}
					}
				}
				
				if (!alreadyAdded && spath.has_extension() && spath.extension() == ".pgn")
					m_filesToBeMerged.emplace_back(spath);
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::Separator();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.58f, 0.97f, 1.0f));

		ImGui::Text(("Creation Path: " + m_BaseDirectory.string() + '\\').c_str());

		ImGui::PopStyleColor();

		ImGui::SameLine();

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);

		ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911131517192123252729313335373941").x);

		ImGui::InputText("##MergedName", &m_mergedName);

		ImGui::Separator();

		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));

			float actualSize = ImGui::CalcTextSize(" Merge ").x + ImGui::CalcTextSize(" Cancel ").x + ImGui::GetStyle().FramePadding.x * 3.0f;
			float avail = ImGui::GetContentRegionAvail().x;

			float off = (avail - actualSize) * 0.5f;
			if (off > 0.0f)
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
			if (ImGui::Button("Merge"))
			{
				MergeFiles(m_BaseDirectory / m_mergedName, m_filesToBeMerged);
				m_filesToBeMerged.clear();
			}

			ImGui::PopStyleColor(3);
		}

		ImGui::SameLine();

		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

			if (ImGui::Button("Cancel"))
				m_filesToBeMerged.clear();

			ImGui::PopStyleColor(3);
		}

		ImGui::End();
	}

	void ContentBrowserPanel::MergeFiles(const std::filesystem::path& dpath, const std::vector<std::filesystem::path>& paths)
	{
		std::ofstream dFile(dpath, std::ios::binary);
		for (auto& path : paths)
		{
			std::ifstream infile(path, std::ios::binary);
			if(infile.is_open())
				dFile << infile.rdbuf();
			infile.close();
		}
		dFile.close();
	}

	void ContentBrowserPanel::TreeDirectory(const std::filesystem::path& directory)
	{
		bool openTree = false;

		ImVec4 vcolor = { 0, 0.66, 0.95, 1 };
		if (directory == m_CurrentDirectory)
			vcolor = { 0.38, 0.67, 0, 1 };

		ImGui::PushStyleColor(ImGuiCol_Text, vcolor);
		ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));
		
		std::string directoryString = directory.filename().u8string();

		//std::string directoryU8String = directory.filename().u8string();
		//std::string directoryString = std::string(directoryU8String.begin(), directoryU8String.end());

		if (ImGui::TreeNodeEx(directoryString.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow))
			openTree = true;
		ImGui::PopFont();
		ImGui::PopStyleColor();
		

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing())
		{
			m_CurrentDirectory = directory;
		}

		if(openTree)
		{
			for (auto& directoryEntry : std::filesystem::directory_iterator(directory))
			{
				if (directoryEntry.is_directory())
				{
					TreeDirectory(directoryEntry);
				}
				else if(directoryEntry.is_regular_file())
				{
					const auto& path = directoryEntry.path();
					std::string filenameString = path.filename().u8string();
					//std::string filenameU8String = path.filename().u8string();
					//std::string filenameString = std::string(filenameU8String.begin(), filenameU8String.end());

					bool isChessFile = (path.extension().u8string() == u8".pgn" || path.extension().u8string() == u8".cld");

					ImGui::TreeNodeEx(filenameString.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						if (!isChessFile)
						{
							//printf("Could not load {0} - not a chess file", filenameString);
						}
						else
						{
							Manager::AppManager::Get().CreateApp(path);
						}
					}
					
					if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && isChessFile)
					{
						s_openFilePopup = true;
						s_path = directoryEntry.path();
					}
				}
			}
			ImGui::TreePop();
		}		
	}

	void ContentBrowserPanel::FilePopup()
	{
		if (ImGui::BeginPopup("File Popup"))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, { 0.38, 0.67, 0, 1 });
			ImGui::PushFont(Walnut::Application::Get().GetFont("Bold"));
			Walnut::UI::TextCentered(s_path.filename().u8string().c_str());
			ImGui::PopFont();
			ImGui::PopStyleColor();

			ImGui::Separator();
			ImGui::Separator();

			if (ImGui::Selectable("Open"))
			{
				//Bug when open file it does not reset the other panels
				if (s_path.extension().string() != ".pgn" && s_path.extension().string() != ".cld")
				{
					//printf("Could not load {0} - not a chess file", s_path.filename().u8string());
				}
				else
				{
					Manager::AppManager::Get().CreateApp(s_path);
				}

				ImGui::CloseCurrentPopup();
			}
			
			if (ImGui::Selectable("Rename"))
			{
				s_openRenamePopup = true;
				s_inputNName = s_path.filename().u8string();
				s_oldpath = s_path;
				ImGui::CloseCurrentPopup();
			}

			ImGui::BeginDisabled(Manager::AppManager::Get().IsAppOpen(s_path));
			
			if (s_path.extension().string() == ".pgn" && ImGui::Selectable("Merge..."))
			{
				bool alreadyAdded = false;

				if (m_filesToBeMerged.empty())
					m_mergedName = s_path.filename().string();
				else
				{
					for (auto& other : m_filesToBeMerged)
					{
						if (other == s_path)
						{
							alreadyAdded = true;
							break;
						}
					}
				}

				if (!alreadyAdded)
					m_filesToBeMerged.emplace_back(s_path);

				ImGui::CloseCurrentPopup();
			}

			ImGui::EndDisabled();

			ImGui::Separator();

			ImGui::BeginDisabled(Manager::AppManager::Get().IsAppOpen(s_path));

			if (ImGui::Selectable("Copy"))
			{
				std::error_code ec;
				std::filesystem::copy(s_path, s_path, ec);
				if (ec)
				{
					std::ofstream ef("ErrorFile.txt");
					ef << "func(std::filesystem::copy) " << ec << "path: " << s_path;
					ef.close();
				}

				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Selectable("Cut"))
			{
				std::error_code ec;
				std::filesystem::copy(s_path, s_path, ec);
				std::filesystem::remove(s_path, ec);
				if (ec)
				{
					std::ofstream ef("ErrorFile.txt");
					ef << "func(std::filesystem::remove) " << ec << "path: " << s_path;
					ef.close();
				}

				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Selectable("Delete"))
			{
				std::error_code ec;
				std::filesystem::remove(s_path, ec);
				if (ec)
				{
					std::ofstream ef("ErrorFile.txt");
					ef << "func(std::filesystem::remove) " << ec << "path: " << s_path;
					ef.close();
				}
				
				ImGui::CloseCurrentPopup();
			}
			
			if (ImGui::Selectable("Remove Deleted Games"))
			{
				Chess::PgnFile::RemoveDeletedGames(s_path);

				ImGui::CloseCurrentPopup();
			}

			ImGui::Separator();

			if (ImGui::BeginMenu("Convert To Cld"))
			{
				if (s_path.extension() == ".pgn")
				{
					if (ImGui::Selectable("CLD encoding + Table"))
					{
						Chess::PgnFile PgnFile;
						PgnFile.OpenFile(s_path);

						s_path.replace_extension(".cld");

						Chess::ConvertToCld(PgnFile, s_path, Chess::CLD, true);
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::Selectable("CORE encoding + Table"))
					{
						Chess::PgnFile PgnFile;
						PgnFile.OpenFile(s_path);

						s_path.replace_extension(".cld");

						Chess::ConvertToCld(PgnFile, s_path, Chess::CORE, true);
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::Selectable("CLD encoding"))
					{
						Chess::PgnFile PgnFile;
						PgnFile.OpenFile(s_path);

						s_path.replace_extension(".cld");

						Chess::ConvertToCld(PgnFile, s_path, Chess::CLD, false);
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::Selectable("CORE encoding"))
					{
						Chess::PgnFile PgnFile;
						PgnFile.OpenFile(s_path);

						s_path.replace_extension(".cld");

						Chess::ConvertToCld(PgnFile, s_path, Chess::CORE, false);
						ImGui::CloseCurrentPopup();
					}
				}
				else if (s_path.extension() == ".cld")
				{
					if (ImGui::Selectable("CLD encoding + Table"))
					{
						Chess::CldFile CldFile;
						CldFile.OpenFile(s_path);
						CldFile.SaveFileAs(s_path, Chess::CLD, true);
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::Selectable("CORE encoding + Table"))
					{
						Chess::CldFile CldFile;
						CldFile.OpenFile(s_path);
						CldFile.SaveFileAs(s_path, Chess::CORE, true);
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::Selectable("CLD encoding"))
					{
						Chess::CldFile CldFile;
						CldFile.OpenFile(s_path);
						CldFile.SaveFileAs(s_path, Chess::CLD, false);
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::Selectable("CORE encoding"))
					{
						Chess::CldFile CldFile;
						CldFile.OpenFile(s_path);
						CldFile.SaveFileAs(s_path, Chess::CORE, false);
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndMenu();
			}

			ImGui::BeginDisabled(s_path.extension() == ".pgn");

			if (ImGui::Selectable("Convert To Pgn"))
			{
				Chess::CldFile CldFile;
				CldFile.OpenFile(s_path);

				s_path.replace_extension(".pgn");

				Chess::ConvertToPgn(CldFile, s_path);
				
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndDisabled();
			ImGui::EndDisabled();

			ImGui::Separator();

			if (ImGui::Selectable("Open Explorer"))
			{
				std::string cmd = "explorer " + s_path.parent_path().string();
				std::system(cmd.c_str());
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void ContentBrowserPanel::EmptyPopup()
	{
		if (ImGui::BeginPopup("Empty Popup"))
		{
			if (ImGui::Selectable("New File"))
			{
				s_openNewFilePopup = true;
				s_inputNName = u8"NewFile";
				s_oldpath = m_CurrentDirectory;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("New Folder"))
			{
				s_openNewFolderPopup = true;
				s_inputNName = u8"DirectoryName";
				s_oldpath = m_CurrentDirectory;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Paste"))
			{
				s_openNewFolderPopup = true;
				s_inputNName = u8"DirectoryName";
				s_oldpath = m_CurrentDirectory;
				ImGui::CloseCurrentPopup();
			}

			ImGui::Separator();

			if (ImGui::Selectable("Open Explorer"))
			{
				std::string cmd = "explorer " + m_CurrentDirectory.string();
				std::system(cmd.c_str());
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void ContentBrowserPanel::RenamePopup()
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Rename Popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::InputText("Name", &s_inputNName);
			
			ImGui::NewLine();

			float actualSize = ImGui::CalcTextSize(" Rename ").x + ImGui::CalcTextSize(" Cancel ").x + ImGui::GetStyle().FramePadding.x * 3.0f;
			float avail = ImGui::GetContentRegionAvail().x;

			float off = (avail - actualSize) * 0.5f;
			if (off > 0.0f)
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));
			if (ImGui::Button("Rename"))
			{
				std::string fileNpath = s_oldpath.u8string().substr(0, s_oldpath.u8string().size() - s_oldpath.filename().u8string().size() - 1) + '\\' + s_inputNName;
				
				std::error_code ec;
				std::filesystem::rename(s_oldpath, fileNpath, ec);
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
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::PopStyleColor(3);
			ImGui::EndPopup();
		}
	}

	void ContentBrowserPanel::NewFolderPopup()
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("New Folder", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::InputText("Name", &s_inputNName);

			ImGui::NewLine();

			float actualSize = ImGui::CalcTextSize(" Create ").x + ImGui::CalcTextSize(" Cancel ").x + ImGui::GetStyle().FramePadding.x * 3.0f;
			float avail = ImGui::GetContentRegionAvail().x;

			float off = (avail - actualSize) * 0.5f;
			if (off > 0.0f)
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));
			if (ImGui::Button("Create"))
			{
				std::filesystem::path nPath = std::filesystem::path() / s_oldpath / s_inputNName;
				
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
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::PopStyleColor(3);
			ImGui::EndPopup();
		}
	}

	void ContentBrowserPanel::NewFilePopup()
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("New File", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::InputText("Name", &s_inputNName);

			ImGui::NewLine();

			std::string items[] = { "PGN", "CLD" };
			static int item_current_idx = 0;

			if (ImGui::BeginCombo("Format", items[item_current_idx].c_str()))
			{
				for (int n = 0; n < 2; n++)
				{
					const bool is_selected = (item_current_idx == n);
					if (ImGui::Selectable(items[n].c_str(), is_selected))
						item_current_idx = n;
				}
				ImGui::EndCombo();
			}

			static int itemEncode_current_idx = 0;
			static int itemTable_current_idx = 0;
			
			if (item_current_idx == 1)
			{
				ImGui::Separator();

				std::string itemsEncode[] = { "CLD-STANDARD", "CORE-MACHINE" };

				if (ImGui::BeginCombo("Encoding", itemsEncode[itemEncode_current_idx].c_str()))
				{
					for (int n = 0; n < 2; n++)
					{
						const bool is_selected = (itemEncode_current_idx == n);
						if (ImGui::Selectable(itemsEncode[n].c_str(), is_selected))
							itemEncode_current_idx = n;
					}
					ImGui::EndCombo();
				}
				
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f / 255.0f, 225.0f / 255.0f, 135.0f / 255.0f, 255.0f / 255.0f));
				ImGui::Text("(?)");
				ImGui::PopStyleColor();
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("CLD encoding is slower than CORE but more compatible");				

				std::string itemsTable[] = { "On", "Off" };

				if (ImGui::BeginCombo("Use Table", itemsTable[itemTable_current_idx].c_str()))
				{
					for (int n = 0; n < 2; n++)
					{
						const bool is_selected = (itemTable_current_idx == n);
						if (ImGui::Selectable(itemsTable[n].c_str(), is_selected))
							itemTable_current_idx = n;
					}
					ImGui::EndCombo();
				}

				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f / 255.0f, 225.0f / 255.0f, 135.0f / 255.0f, 255.0f / 255.0f));
				ImGui::Text("(?)");
				ImGui::PopStyleColor();
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Using Tables is slightly slower but uses less memory (Recommended for large datasets)");				
				
				ImGui::Separator();
			}

			float actualSize = ImGui::CalcTextSize(" Create ").x + ImGui::CalcTextSize(" Cancel ").x + ImGui::GetStyle().FramePadding.x * 3.0f;
			float avail = ImGui::GetContentRegionAvail().x;

			float off = (avail - actualSize) * 0.5f;
			if (off > 0.0f)
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));
			if (ImGui::Button("Create"))
			{
				std::filesystem::path nPath = std::filesystem::path() / s_oldpath / s_inputNName;

				if (item_current_idx == 0)
				{
					nPath.replace_extension(".pgn");
					std::ofstream nfile(nPath, std::ios::trunc);
					nfile.close();
				}
				else if (item_current_idx == 1)
				{
					nPath.replace_extension(".cld");
					Chess::CldFile CldFile;
					CldFile.CreateGame();
					CldFile.SaveFileAs(nPath, itemEncode_current_idx == 0 ? Chess::CLD : Chess::CORE, itemTable_current_idx == 0 ? true : false);
				}

				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::PopStyleColor(3);
			ImGui::EndPopup();
		}
	}

}
