#include "DatabasePanel.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

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
		m_name_white = true;
		m_name_black = true;
		m_name_to_search.clear();
		m_eco_to_search.clear();
		m_ecoItems.clear();
		m_date_to_search.clear();
	}

	void DatabasePanel::OnImGuiRender()
	{
		static int s_oldSearchFocus = -1;
		bool nfile = false;

		if (m_filePath != ChessAPI::GetPgnFilePath())
		{
			s_oldSearchFocus = -1;
			nfile = true;
			Reset();
			m_filePath = ChessAPI::GetPgnFilePath();
		}

		Chess::ChessFile& chessfile = ChessAPI::GetPgnFile();
		
		ImGui::Begin("Database");
		ImGui::TextWrapped(ChessAPI::GetPgnFileName().c_str());
		
		m_searchTables.resize(chessfile.GetSearchesCount());

		if (ImGui::BeginTabBar("Tables", (!nfile ? ImGuiTabBarFlags_AutoSelectNewTabs : ImGuiTabBarFlags_None) | ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyResizeDown))
		{
			for (int i = 0; i < chessfile.GetSearchesCount();)
			{
				bool open = true;
				std::string* name = chessfile.GetSearchName(i);

				if (ImGui::BeginTabItem((*name + "###" + std::to_string(i)).c_str(), &open))
				{
					m_searchTables[i] = chessfile.GetSearch(i);

					if (s_oldSearchFocus != i)
					{
						std::string nameWhite = m_searchTables[i]->first.GetOptionValue("White");
						std::string nameBlack = m_searchTables[i]->first.GetOptionValue("Black");

						m_eco_to_search = m_searchTables[i]->first.GetOptionValue("ECO");
						m_date_to_search = m_searchTables[i]->first.GetOptionValue("Date");

						m_elo_to_searchW = m_searchTables[i]->first.GetOptionValue("WhiteElo");
						m_title_to_searchW = m_searchTables[i]->first.GetOptionValue("WhiteTitle");
						m_fideId_to_searchW = m_searchTables[i]->first.GetOptionValue("WhiteFideId");

						m_elo_to_searchB = m_searchTables[i]->first.GetOptionValue("BlackElo");
						m_title_to_searchB = m_searchTables[i]->first.GetOptionValue("BlackTitle");
						m_fideId_to_searchB = m_searchTables[i]->first.GetOptionValue("BlackFideId");

						m_event_to_search = m_searchTables[i]->first.GetOptionValue("Event");
						m_round_to_search = m_searchTables[i]->first.GetOptionValue("Round");
						m_site_to_search = m_searchTables[i]->first.GetOptionValue("Site");
						m_source_to_search = m_searchTables[i]->first.GetOptionValue("Source");
						m_result_to_search = m_searchTables[i]->first.GetOptionValue("Result");

						m_AdvancedOptions = (!nameWhite.empty() && !nameBlack.empty() && nameWhite != nameBlack)
							|| !m_elo_to_searchW.empty()
							|| !m_title_to_searchW.empty()
							|| !m_fideId_to_searchW.empty()
							|| !m_elo_to_searchB.empty()
							|| !m_title_to_searchB.empty()
							|| !m_fideId_to_searchB.empty()
							|| !m_event_to_search.empty()
							|| !m_round_to_search.empty()
							|| !m_site_to_search.empty()
							|| !m_source_to_search.empty()
							|| !m_result_to_search.empty();

						if (m_AdvancedOptions)
						{
							m_name_to_searchW = nameWhite;
							m_name_to_searchB = nameBlack;
						}
						else
						{
							m_name_to_search = !nameWhite.empty() ? nameWhite : nameBlack;

							m_name_white = !nameWhite.empty();
							m_name_black = !nameBlack.empty();
						}
					}

					s_oldSearchFocus = i;

					ImGui::BeginDisabled(chessfile.GetSearch(i)->second.Persentage > 0 && chessfile.GetSearch(i)->second.Persentage < 1);

					ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911131517").x);
					ImGui::InputText("Table Name", name);

					ImGui::NewLine();

					if (m_AdvancedOptions)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
					}


					if (ImGui::Button("Default") && m_AdvancedOptions)
					{
						m_AdvancedOptions = false;

						m_name_to_searchW.clear();
						m_elo_to_searchW.clear();
						m_title_to_searchW.clear();
						m_fideId_to_searchW.clear();

						m_name_to_searchB.clear();
						m_elo_to_searchB.clear();
						m_title_to_searchB.clear();
						m_fideId_to_searchB.clear();

						m_event_to_search.clear();
						m_round_to_search.clear();
						m_site_to_search.clear();
						m_source_to_search.clear();
						m_result_to_search.clear();
					}

					ImGui::PopStyleColor(3);

					ImGui::SameLine();

					if (!m_AdvancedOptions)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
					}

					if (ImGui::Button("Advanced") && !m_AdvancedOptions)
					{
						m_AdvancedOptions = true;

						if (!m_name_to_search.empty())
						{
							if (m_name_white || !m_name_black)
								m_name_to_searchW = m_name_to_search;
							if (m_name_black || !m_name_white)
								m_name_to_searchB = m_name_to_search;
						}
					}

					ImGui::PopStyleColor(3);

					ImGui::Separator();

					if (!m_AdvancedOptions)
					{
						ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911131517").x);
						ImGui::InputText("Name", &m_name_to_search);

						//ImGuiLayer::TextInputComboBox("ECO", m_eco_to_search, m_ecoItems, 3, ImGui::CalcTextSize("1234567").x);

						ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

						ImGui::SetNextItemWidth(ImGui::CalcTextSize(" --A00-- ").x);
						ImGui::InputText("ECO", &m_eco_to_search);

						ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

						ImGui::SetNextItemWidth(ImGui::CalcTextSize(" dd/mm/yyyy ").x);
						ImGui::InputText("Date", &m_date_to_search);

						ImGui::Checkbox("White", &m_name_white);

						ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

						ImGui::Checkbox("Black", &m_name_black);
					}
					else if (ImGui::TreeNodeEx("Options", ImGuiTreeNodeFlags_DefaultOpen))
					{
						//white

						ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911131517").x);
						ImGui::InputText("White's Name", &m_name_to_searchW);

						ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

						ImGui::SetNextItemWidth(ImGui::CalcTextSize("1234567").x);
						ImGui::InputText("White's Elo", &m_elo_to_searchW);

						ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

						ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345").x);
						ImGui::InputText("White's title", &m_title_to_searchW);

						ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

						ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911").x);
						ImGui::InputText("White's Fide ID", &m_fideId_to_searchW);

						//eco, date, result

						ImGui::SetNextItemWidth(ImGui::CalcTextSize(" --A00-- ").x);
						ImGui::InputText("ECO", &m_eco_to_search);

						ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

						ImGui::SetNextItemWidth(ImGui::CalcTextSize(" dd/mm/yyyy ").x);
						ImGui::InputText("Date", &m_date_to_search);

						ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

						ImGui::SetNextItemWidth(ImGui::CalcTextSize("--1/2-1/2--").x);
						ImGui::InputText("Result", &m_result_to_search);

						//black

						ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911131517").x);
						ImGui::InputText("Black's Name", &m_name_to_searchB);

						ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

						ImGui::SetNextItemWidth(ImGui::CalcTextSize("1234567").x);
						ImGui::InputText("Black's Elo", &m_elo_to_searchB);

						ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

						ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345").x);
						ImGui::InputText("Black's title", &m_title_to_searchB);

						ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

						ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911").x);
						ImGui::InputText("Black's Fide ID", &m_fideId_to_searchB);

						//other

						ImGui::SetNextItemWidth(ImGui::CalcTextSize(" --Event Name-- ").x);
						ImGui::InputText("Event", &m_event_to_search);

						ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

						ImGui::SetNextItemWidth(ImGui::CalcTextSize("123456").x);
						ImGui::InputText("Round", &m_round_to_search);

						ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

						ImGui::SetNextItemWidth(ImGui::CalcTextSize(" --Site Name-- ").x);
						ImGui::InputText("Site", &m_site_to_search);

						ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

						ImGui::SetNextItemWidth(ImGui::CalcTextSize(" --Source Name-- ").x);
						ImGui::InputText("Source", &m_source_to_search);

						ImGui::TreePop();
					}

					ImGui::EndDisabled();

					ImGui::NewLine();

					float size = ImGui::GetFrameHeight();

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
					if ((chessfile.GetSearch(i)->second.Persentage == 0 || chessfile.GetSearch(i)->second.Persentage == 1) && ImGui::ImageButton((ImTextureID)m_IconPlay->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
					{
						m_searchTables[i]->first.Clear();

						if (!m_AdvancedOptions)
						{
							if (m_name_white || !m_name_black)
							{
								m_searchTables[i]->first.StartOption()
									.And("White", m_name_to_search)
									.And("ECO", m_eco_to_search)
									.And("Date", m_date_to_search)
									.EndOption();
							}
							if (m_name_black || !m_name_white)
							{
								m_searchTables[i]->first.StartOption()
									.And("Black", m_name_to_search)
									.And("ECO", m_eco_to_search)
									.And("Date", m_date_to_search)
									.EndOption();
							}
						}
						else
						{
							m_searchTables[i]->first.StartOption()
								.And("White", m_name_to_searchW)
								.And("WhiteElo", m_elo_to_searchW)
								.And("WhiteTitle", m_title_to_searchW)
								.And("WhiteFideId", m_fideId_to_searchW)
								.And("Black", m_name_to_searchB)
								.And("BlackElo", m_elo_to_searchB)
								.And("BlackTitle", m_title_to_searchB)
								.And("BlackFideId", m_fideId_to_searchB)
								.And("Result", m_result_to_search)
								.And("Event", m_event_to_search)
								.And("Round", m_round_to_search)
								.And("Site", m_site_to_search)
								.And("Source", m_source_to_search)
								.And("ECO", m_eco_to_search)
								.And("Date", m_date_to_search)
								.EndOption();
						}

						chessfile.StartSearch(i);
					}

					if (chessfile.GetSearch(i)->second.Persentage > 0 && chessfile.GetSearch(i)->second.Persentage < 1 && ImGui::ImageButton((ImTextureID)m_IconStop->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
					{
						chessfile.StopSearch(i);
					}

					ImGui::PopStyleColor();

					ImGui::SameLine();

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

					if (ImGui::Button("Clear"))
					{
						chessfile.StopSearch(i);
						m_searchTables[i]->first.Clear();

						m_name_to_search.clear();

						m_eco_to_search.clear();
						m_date_to_search.clear();

						if (m_AdvancedOptions)
						{
							m_name_to_searchW.clear();
							m_elo_to_searchW.clear();
							m_title_to_searchW.clear();
							m_fideId_to_searchW.clear();

							m_name_to_searchB.clear();
							m_elo_to_searchB.clear();
							m_title_to_searchB.clear();
							m_fideId_to_searchB.clear();

							m_result_to_search.clear();
							m_event_to_search.clear();
							m_round_to_search.clear();
							m_site_to_search.clear();
							m_source_to_search.clear();
						}
					}

					ImGui::PopStyleColor(3);

					ImGui::SameLine();
					//ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x / 4);

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.58f, 0.97f, 0.6f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.58f, 0.97f, 0.6f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.58f, 0.97f, 0.6f));

					ImGui::Button((std::to_string((int)(m_searchTables[i]->second.Persentage * 100)) + '%').c_str(), { 60, ImGui::GetFrameHeight() });

					ImGui::PopStyleColor(3);

					ImGui::SameLine();

					auto ycursor = ImGui::GetCursorPosY();
					auto xcursor = ImGui::GetCursorPosX();

					float availx = ImGui::GetContentRegionAvail().x / 2;
					ImGui::Button("##end", ImVec2(availx, 0));

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.353f, 0.314f, 0.0118f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.353f, 0.314f, 0.0118f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.353f, 0.314f, 0.0118f, 1.0f));

					if (m_searchTables[i]->second.Persentage > 0)
					{
						ImGui::SetCursorPosY(ycursor);
						ImGui::SetCursorPosX(xcursor);

						ImGui::Button("##bar", ImVec2(availx * m_searchTables[i]->second.Persentage, 0));
					}

					ImGui::PopStyleColor(3);

					ImGui::Separator();
					ImGui::NewLine();
					ImGui::Separator();

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4());
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4());
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4());

					if (ImGui::BeginTable("table_scrollx", 9,
						ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg))
					{
						ImGui::TableSetupScrollFreeze(1, 1);
						ImGui::TableSetupColumn("Line #", ImGuiTableColumnFlags_NoHide); // Make the first column not hideable to match our use of TableSetupScrollFreeze()
						ImGui::TableSetupColumn(m_important_prop[0].c_str(), ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);
						for (int i = 1; i < m_important_prop.size() - 2; i++)
							ImGui::TableSetupColumn(m_important_prop[i].c_str());
						ImGui::TableHeadersRow();

						m_IsOpened.clear();

						ImGuiListClipper clipper;
						clipper.Begin(m_searchTables[i]->second.PossitiveIndexes.size());
						while (clipper.Step())
						{
							for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
							{
								ImGui::TableNextRow();

								bool IsDeleted = chessfile.IsGameDeleted(m_searchTables[i]->second.PossitiveIndexes[row]);

								if (m_searchTables[i]->second.PossitiveIndexes[row] == ChessAPI::GetActiveGameIndex())
									ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(40, 50, 110, 255));
								else if (IsDeleted)
									ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(0.7f, 0.1f, 0.1f, 0.65f));

								for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
								{
									if (!ImGui::TableSetColumnIndex(column) && column > 0)
										continue;
									if (column == 0)
									{
										ImGui::PushID(m_searchTables[i]->second.PossitiveIndexes[row]);

										//m_IsOpened[m_search_resualt[row] - clipper.DisplayStart] = ChessAPI::IsGameOpen(m_search_resualt[row]);

										ImGui::GetStyle().FramePadding.y *= 0.4f;
										//ImGui::Checkbox("##isopen", (bool*)&m_IsOpened[row - clipper.DisplayStart]);
										ImGui::GetStyle().FramePadding.y *= 2.5f;

										//ImGui::SameLine();
										ImGui::Text("Line %d", m_searchTables[i]->second.PossitiveIndexes[row] + 1);

										ImGui::PopID();
									}
									else
									{
										ImGui::PushID(row * ImGui::TableGetColumnCount() + column);

										auto& game = chessfile.operator[](m_searchTables[i]->second.PossitiveIndexes[row]);

										ImGui::Selectable(game[m_important_prop[column - 1]].c_str());

										if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
											ChessAPI::OpenChessGameInFile(m_searchTables[i]->second.PossitiveIndexes[row]);
										if (ImGui::BeginDragDropSource())
										{
											ImGui::SetDragDropPayload("Database", &m_searchTables[i]->second.PossitiveIndexes[row], sizeof(int));
											ImGui::EndDragDropSource();
										}

										ImGui::PopID();
									}
								}
							}
						}
						ImGui::EndTable();
					}

					ImGui::PopStyleColor(3);

					ImGui::EndTabItem();
				}

				if (!open)
				{
					chessfile.RemoveSearch(i);
					s_oldSearchFocus = -1;
				}
				else
					i++;
			}

			if (ImGui::BeginTabItem("Main", 0, ImGuiTabItemFlags_Leading | (nfile ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None)))
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.48f, 0.87f, 0.65f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.48f, 0.87f, 0.45f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.48f, 0.87f, 0.25f));

				if (ImGui::Button("New Game"))
				{
					ChessAPI::NewGameInFile();
				}
				ImGui::SameLine();

				bool IsDeleted = chessfile.IsGameDeleted(ChessAPI::GetActiveGameIndex());

				if (IsDeleted)
				{
					if (ImGui::Button("Recover Game"))
						chessfile.RecoverGame(ChessAPI::GetActiveGameIndex());
				}
				else
				{
					if (ImGui::Button("Delete Game"))
						chessfile.DeleteGame(ChessAPI::GetActiveGameIndex());
				}

				ImGui::PopStyleColor(3);

				ImGui::Separator();

				int all = chessfile.GetSize();

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4());
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4());
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4());

				if (ImGui::BeginTable("table_scrollx", 9,
					ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg))
				{
					ImGui::TableSetupScrollFreeze(1, 1);
					ImGui::TableSetupColumn("Line #", ImGuiTableColumnFlags_NoHide); // Make the first column not hideable to match our use of TableSetupScrollFreeze()
					ImGui::TableSetupColumn(m_important_prop[0].c_str(), ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);
					for (int i = 1; i < m_important_prop.size() - 2; i++)
						ImGui::TableSetupColumn(m_important_prop[i].c_str());
					ImGui::TableHeadersRow();

					m_IsOpened.clear();

					ImGuiListClipper clipper;
					clipper.Begin(all);
					while (clipper.Step())
					{
						for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
						{
							ImGui::TableNextRow();

							bool IsDeleted = chessfile.IsGameDeleted(row);

							if (row == ChessAPI::GetActiveGameIndex())
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

									auto& game = chessfile.operator[](row);

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
					ImGui::EndTable();
				}

				ImGui::PopStyleColor(3);

				ImGui::EndTabItem();
			}

			if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip))
				chessfile.AddSearch();

			ImGui::EndTabBar();
		}

		ImGui::End();
	}

}
