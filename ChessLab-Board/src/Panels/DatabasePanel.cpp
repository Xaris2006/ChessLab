#include "DatabasePanel.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include "ChessCore/pgn/SearchWork.h"

static bool s_searched = false;

namespace Panels
{
	DatabasePanel::DatabasePanel()
	{
		m_ecoItems.reserve(500);
		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 100; j++)
			{
				char label = 'A' + i;
				if (j < 10)
					m_ecoItems.emplace_back(label + ("0" + std::to_string(j)));
				else
					m_ecoItems.emplace_back(label + std::to_string(j));
			}
		}
	}

	void DatabasePanel::Reset()
	{
		m_searchSetting.clear();
		m_name_white = true;
		m_name_black = true;
		m_name_to_search.clear();
		m_eco_to_search.clear();
		m_ecoItems.clear();
		m_date_to_search.clear();
		s_searched = false;
	}

	void DatabasePanel::OnImGuiRender()
	{
		if (m_filePath != ChessAPI::GetPgnFilePath())
		{
			Reset();
			m_filePath = ChessAPI::GetPgnFilePath();
		}

		Chess::PgnFile* pgnfile = ChessAPI::GetPgnFile();
		if (pgnfile)
		{
			ImGui::Begin("Database");
			ImGui::TextWrapped(ChessAPI::GetPgnFileName().c_str());
			
			ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911131517").x);
			ImGui::InputText("Name", &m_name_to_search);

			ImGui::SameLine();

			ImGui::SetNextItemWidth(ImGui::CalcTextSize(" --A00-- ").x);
			ImGui::InputText("ECO", &m_eco_to_search);

			//ImGuiLayer::TextInputComboBox("ECO", m_eco_to_search, m_ecoItems, 3, ImGui::CalcTextSize("1234567").x);

			ImGui::SameLine();

			ImGui::SetNextItemWidth(ImGui::CalcTextSize(" dd/mm/yyyy ").x);
			ImGui::InputText("Date", &m_date_to_search);

			ImGui::Checkbox("White", &m_name_white);

			ImGui::SameLine();

			ImGui::Checkbox("Black", &m_name_black);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));

			static Chess::SearchWork work;

			if (ImGui::Button("Search"))
			{
				work.Clear();

				s_searched = true;
				if (m_name_white || !m_name_black)
				{
					m_searchSetting.clear();

					if (m_name_to_search != "")
						(m_searchSetting)["White"] = m_name_to_search;
					if (m_eco_to_search != "")
						(m_searchSetting)["ECO"] = m_eco_to_search;
					if (m_date_to_search != "")
						(m_searchSetting)["Date"] = m_date_to_search;

					work.SearchLabel(pgnfile->GetID(), m_searchSetting);
				}
				if (m_name_black || !m_name_white)
				{
					m_searchSetting.clear();

					if (m_name_to_search != "")
						(m_searchSetting)["Black"] = m_name_to_search;
					if (m_eco_to_search != "")
						(m_searchSetting)["ECO"] = m_eco_to_search;
					if (m_date_to_search != "")
						(m_searchSetting)["Date"] = m_date_to_search;

					work.SearchLabel(pgnfile->GetID(), m_searchSetting);
				}
			}
			ImGui::PopStyleColor(3);

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

			if (ImGui::Button("Clear"))
			{
				work.Clear();
				s_searched = false;
				m_name_to_search.clear();
				m_eco_to_search.clear();
			}
			ImGui::PopStyleColor(3);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.58f, 0.97f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.58f, 0.97f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.58f, 0.97f, 0.6f));

			ImGui::Button((std::to_string((int)work.GetPercentage()) + '%').c_str(), { 55, ImGui::GetFrameHeight() });

			ImGui::PopStyleColor(3);

			ImGui::SameLine();

			auto ycursor = ImGui::GetCursorPosY();
			auto xcursor = ImGui::GetCursorPosX();

			float availx = ImGui::CalcTextSize("12345678911131517192123252729313335").x;
			ImGui::Button("##end", ImVec2(availx, 0));

			ImGui::SetCursorPosY(ycursor);
			ImGui::SetCursorPosX(xcursor);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.353f, 0.314f, 0.0118f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.353f, 0.314f, 0.0118f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.353f, 0.314f, 0.0118f, 1.0f));

			ImGui::Button("##bar", ImVec2(availx * work.GetPercentage() / 100, 0));

			ImGui::PopStyleColor(3);

			ImGui::NewLine();
			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.48f, 0.87f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.48f, 0.87f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.48f, 0.87f, 0.25f));

			if (ImGui::Button("New Game"))
			{
				ChessAPI::NewGameInFile();
			}
			ImGui::SameLine();

			bool IsDeleted = pgnfile->IsGameDeleted(ChessAPI::GetActiveGame());

			if (IsDeleted)
			{
				if (ImGui::Button("Recover Game"))
					pgnfile->RecoverGame(ChessAPI::GetActiveGame());
			}
			else
			{
				if (ImGui::Button("Delete Game"))
					pgnfile->DeleteGame(ChessAPI::GetActiveGame());
			}
			
			ImGui::PopStyleColor(3);

			ImGui::Separator();

			int all = pgnfile->GetSize();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4());
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4());
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4());

			if (ImGui::BeginTable("table_scrollx", 9, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupScrollFreeze(1, 1);
				ImGui::TableSetupColumn("Line #", ImGuiTableColumnFlags_NoHide); // Make the first column not hideable to match our use of TableSetupScrollFreeze()
				ImGui::TableSetupColumn(m_important_prop[0].c_str(), ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);
				for (int i = 1; i < m_important_prop.size() - 2; i++)
					ImGui::TableSetupColumn(m_important_prop[i].c_str());
				ImGui::TableHeadersRow();

				//TODO: Sorting
				//if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
				//{
				//	if (sorts_specs->SpecsDirty)
				//	{
				//		if (start > 0)
				//			
				//		MyItem::s_current_sort_specs = NULL;
				//		sorts_specs->SpecsDirty = false;
				//	}
				//}

				m_IsOpened.clear();//clipper.DisplayEnd - clipper.DisplayStart

				if (!s_searched)
				{
					ImGuiListClipper clipper;
					clipper.Begin(all);
					while (clipper.Step())
					{
						for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
						{
							ImGui::TableNextRow();
							
							bool IsDeleted = pgnfile->IsGameDeleted(row);

							if (row == ChessAPI::GetActiveGame())
								ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(40, 50, 110, 255));
							else if (IsDeleted)
								ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(0.7f, 0.1f, 0.1f, 0.65f));

							for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
							{
								if (!ImGui::TableSetColumnIndex(column) && column > 0)
									continue;
								if (column == 0)
								{
									ImGui::PushID(row);
									
									//m_IsOpened.emplace_back(new ) = ChessAPI::IsGameOpen(row);
									
									ImGui::GetStyle().FramePadding.y *= 0.4f;
									//ImGui::Checkbox("##isopen", (bool*)& m_IsOpened[row - clipper.DisplayStart]);
									ImGui::GetStyle().FramePadding.y *= 2.5f;

									//ImGui::SameLine();
									ImGui::Text("Line %d", row + 1);

									ImGui::PopID();
								}
								else
								{
									ImGui::PushID(row * ImGui::TableGetColumnCount() + column);

									auto& game = pgnfile->operator[](row);

									ImGui::Selectable(game[m_important_prop[column - 1]].c_str());
									
									if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
										ChessAPI::OpenChessGameInFile(row);

									if (ImGui::BeginDragDropSource())
									{
										ImGui::SetDragDropPayload("Database", &row, sizeof(int));
										ImGui::EndDragDropSource();
									}
									ImGui::PopID();
								}
							}
						}
					}
				}
				else
				{
					static std::vector<size_t> result;
					result = work.GetResult();
					all = result.size();

					ImGuiListClipper clipper;
					clipper.Begin(all);
					while (clipper.Step())
					{
						for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
						{
							ImGui::TableNextRow();

							bool IsDeleted = pgnfile->IsGameDeleted(result[row]);

							if (result[row] == ChessAPI::GetActiveGame())
								ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(40, 50, 110, 255));
							else if (IsDeleted)
								ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(0.7f, 0.1f, 0.1f, 0.65f));

							for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
							{
								if (!ImGui::TableSetColumnIndex(column) && column > 0)
									continue;
								if (column == 0)
								{
									ImGui::PushID(result[row]);

									//m_IsOpened[m_search_resualt[row] - clipper.DisplayStart] = ChessAPI::IsGameOpen(m_search_resualt[row]);

									ImGui::GetStyle().FramePadding.y *= 0.4f;
									//ImGui::Checkbox("##isopen", (bool*)&m_IsOpened[row - clipper.DisplayStart]);
									ImGui::GetStyle().FramePadding.y *= 2.5f;

									//ImGui::SameLine();
									ImGui::Text("Line %d", result[row] + 1);

									ImGui::PopID();
								}
								else
								{
									ImGui::PushID(row * ImGui::TableGetColumnCount() + column);

									auto& game = pgnfile->operator[](result[row]);

									ImGui::Selectable(game[m_important_prop[column - 1]].c_str());

									if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
										ChessAPI::OpenChessGameInFile(result[row]);
									if (ImGui::BeginDragDropSource())
									{
										ImGui::SetDragDropPayload("Database", &result[row], sizeof(int));
										ImGui::EndDragDropSource();
									}

									ImGui::PopID();
								}
							}
						}
					}
				}
				ImGui::EndTable();
			}
			ImGui::PopStyleColor(3);

			ImGui::End();
		}
		else
		{
			m_eco_to_search = "";
			m_name_to_search = "";
		}
	}

}
