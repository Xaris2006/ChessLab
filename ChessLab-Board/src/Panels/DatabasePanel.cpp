#include "DatabasePanel.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include "Walnut/UI/UI.h"
#include "Walnut/ImGui/ImGuiTheme.h"

#include "../ChessLabUtils.h"

extern bool g_LoadingModalOpen;
extern bool g_OpenEditor;

void DrawImage(const std::shared_ptr<Walnut::Image>& image,
	ImVec4 tintNormal, ImVec4 tintHovered, ImVec4 tintPressed,
	ImVec2 size)
{
	if (ImGui::IsItemActive())
		ImGui::Image((ImTextureID)image->GetRendererID(), size, ImVec2(0, 0), ImVec2(1, 1), tintPressed);
	else if (ImGui::IsItemHovered())
		ImGui::Image((ImTextureID)image->GetRendererID(), size, ImVec2(0, 0), ImVec2(1, 1), tintHovered);
	else
		ImGui::Image((ImTextureID)image->GetRendererID(), size, ImVec2(0, 0), ImVec2(1, 1), tintNormal);
}

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

		if (g_LoadingModalOpen)
		{
			ImGui::End();
			return;
		}

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
						std::string nameWhite = m_searchTables[i]->first.GetOptionText("White").value;
						std::string nameBlack = m_searchTables[i]->first.GetOptionText("Black").value;

						m_eco_to_search = m_searchTables[i]->first.GetOptionText("ECO").value;
						m_date_to_search = m_searchTables[i]->first.GetOptionText("Date").value;

						m_title_to_searchW = m_searchTables[i]->first.GetOptionText("WhiteTitle").value;
						m_fideId_to_searchW = m_searchTables[i]->first.GetOptionText("WhiteFideId").value;

						auto eloWhite = m_searchTables[i]->first.GetOptionRangeNumber("WhiteElo");

						if (eloWhite.valueMin == 0 && eloWhite.valueMax == 0)
						{
							m_eloMin_to_searchW = 1000;
							m_eloMax_to_searchW = 4000;
						}
						else
						{
							m_eloMin_to_searchW = eloWhite.valueMin;
							m_eloMax_to_searchW = eloWhite.valueMax;
						}


						m_title_to_searchB = m_searchTables[i]->first.GetOptionText("BlackTitle").value;
						m_fideId_to_searchB = m_searchTables[i]->first.GetOptionText("BlackFideId").value;

						auto eloBlack = m_searchTables[i]->first.GetOptionRangeNumber("BlackElo");

						if (eloBlack.valueMin == 0 && eloBlack.valueMax == 0)
						{
							m_eloMin_to_searchB = 1000;
							m_eloMax_to_searchB = 4000;
						}
						else
						{
							m_eloMin_to_searchB = eloBlack.valueMin;
							m_eloMax_to_searchB = eloBlack.valueMax;
						}

						m_event_to_search = m_searchTables[i]->first.GetOptionText("Event").value;
						m_round_to_search = m_searchTables[i]->first.GetOptionText("Round").value;
						m_site_to_search = m_searchTables[i]->first.GetOptionText("Site").value;
						m_source_to_search = m_searchTables[i]->first.GetOptionText("Source").value;
						m_result_to_search = m_searchTables[i]->first.GetOptionText("Result").value;

						if (m_result_to_search == "1-0")
						{
							m_WhiteWin_Result = true;
							m_BlackWin_Result = false;
							m_Draw_Result = false;
						}
						else if (m_result_to_search == "0-1")
						{
							m_BlackWin_Result = true;
							m_WhiteWin_Result = false;
							m_Draw_Result = false;
						}
						else if (m_result_to_search == "1/2-1/2")
						{
							m_Draw_Result = true;
							m_WhiteWin_Result = false;
							m_BlackWin_Result = false;
						}

						m_AdvancedOptions = (!nameWhite.empty() && !nameBlack.empty() && nameWhite != nameBlack)
							//|| !m_elo_to_searchW.empty()
							|| !m_title_to_searchW.empty()
							|| !m_fideId_to_searchW.empty()
							//|| !m_elo_to_searchB.empty()
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
						m_title_to_searchW.clear();
						m_fideId_to_searchW.clear();
						m_eloMax_to_searchW = 4000;
						m_eloMin_to_searchW = 1000;

						m_name_to_searchB.clear();
						m_title_to_searchB.clear();
						m_fideId_to_searchB.clear();
						m_eloMax_to_searchB = 4000;
						m_eloMin_to_searchB = 1000;

						m_result_to_search.clear();
						m_event_to_search.clear();
						m_round_to_search.clear();
						m_site_to_search.clear();
						m_source_to_search.clear();
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
					else if (ImGui::TreeNodeEx("Options", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed))
					{
						if (ImGui::TreeNodeEx("White Player", ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911131517").x);
							ImGui::InputText("Name##w", &m_name_to_searchW);

							ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

							ImGui::SetNextItemWidth(ImGui::CalcTextSize("1234567").x);
							ImGui::DragInt("##eloWMinw", &m_eloMin_to_searchW);

							ImGui::SameLine();

							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.0f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.0f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.0f));

							ImGui::Button("-");

							ImGui::PopStyleColor(3);

							ImGui::SameLine();

							ImGui::SetNextItemWidth(ImGui::CalcTextSize("1234567").x);
							ImGui::DragInt("Elo Range##w", &m_eloMax_to_searchW);

							ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

							ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345").x);
							ImGui::InputText("Title##w", &m_title_to_searchW);

							ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

							ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911").x);
							ImGui::InputText("Fide ID##w", &m_fideId_to_searchW);


							ImGui::TreePop();
						}
						if (ImGui::TreeNodeEx("Black Player", ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911131517").x);
							ImGui::InputText("Name##b", &m_name_to_searchB);

							ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

							ImGui::SetNextItemWidth(ImGui::CalcTextSize("1234567").x);
							ImGui::DragInt("##eloWMinb", &m_eloMin_to_searchB);

							ImGui::SameLine();

							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.0f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.0f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.0f));

							ImGui::Button("-");

							ImGui::PopStyleColor(3);

							ImGui::SameLine();

							ImGui::SetNextItemWidth(ImGui::CalcTextSize("1234567").x);
							ImGui::DragInt("Elo Range##b", &m_eloMax_to_searchB);

							ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

							ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345").x);
							ImGui::InputText("Title##b", &m_title_to_searchB);

							ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

							ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911").x);
							ImGui::InputText("Fide ID##b", &m_fideId_to_searchB);

							ImGui::TreePop();
						}
						if (ImGui::TreeNodeEx("Game", ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::SetNextItemWidth(ImGui::CalcTextSize(" --A00-- ").x);
							ImGui::InputText("ECO", &m_eco_to_search);

							ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

							ImGui::SetNextItemWidth(ImGui::CalcTextSize(" dd/mm/yyyy ").x);
							ImGui::InputText("Date", &m_date_to_search);

							ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

							if (ImGui::RadioButton("1-0", m_WhiteWin_Result))
							{
								if (m_WhiteWin_Result)
								{
									m_result_to_search.clear();
									m_WhiteWin_Result = false;
								}
								else
								{
									m_result_to_search = "1-0";
									m_BlackWin_Result = false;
									m_Draw_Result = false;
									m_WhiteWin_Result = true;
								}
							}
							ImGui::SameLine();
							if (ImGui::RadioButton("0-1", m_BlackWin_Result))
							{
								if (m_BlackWin_Result)
								{
									m_result_to_search.clear();
									m_BlackWin_Result = false;
								}
								else
								{
									m_result_to_search = "0-1";
									m_WhiteWin_Result = false;
									m_Draw_Result = false;
									m_BlackWin_Result = true;
								}
							}
							ImGui::SameLine();
							if (ImGui::RadioButton("½-½", m_Draw_Result))
							{
								if (m_Draw_Result)
								{
									m_result_to_search.clear();
									m_Draw_Result = false;
								}
								else
								{
									m_result_to_search = "1/2-1/2";
									m_WhiteWin_Result = false;
									m_BlackWin_Result = false;
									m_Draw_Result = true;
								}
							}

							ImGui::TreePop();
						}
						if (ImGui::TreeNodeEx("Tournament", ImGuiTreeNodeFlags_DefaultOpen))
						{
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
								.And("WhiteElo", m_eloMin_to_searchW, m_eloMax_to_searchW)
								.And("WhiteTitle", m_title_to_searchW)
								.And("WhiteFideId", m_fideId_to_searchW)
								.And("Black", m_name_to_searchB)
								.And("BlackElo", m_eloMin_to_searchB, m_eloMax_to_searchB)
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

					ImGui::SameLine(0, 20);

					ImGui::Text("%d Games Found", m_searchTables[i]->second.PossitiveIndexes.size());

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

					ImGui::BeginDisabled(chessfile.GetSearch(i)->second.Persentage > 0 && chessfile.GetSearch(i)->second.Persentage < 1);

					ImGui::BeginDisabled(std::ranges::find(m_searchTables[i]->second.PossitiveIndexes, ChessAPI::GetActiveGameIndex()) == m_searchTables[i]->second.PossitiveIndexes.end());

					const ImVec4 RMbuttonColN = ImGui::ColorConvertU32ToFloat4(Walnut::UI::Colors::ColorWithMultipliedValue(Walnut::UI::Colors::Theme::text, 1.0f));
					const ImVec4 RMbuttonColH = ImGui::ColorConvertU32ToFloat4(Walnut::UI::Colors::ColorWithMultipliedValue(Walnut::UI::Colors::Theme::text, 1.2f));
					const ImVec4 RMbuttonColP = ImGui::ColorConvertU32ToFloat4(Walnut::UI::Colors::Theme::textDarker);
					const float RMbuttonWidth = size * 0.9f;
					const float RMbuttonHeight = size * 0.9f;

					bool IsDeleted = chessfile.IsGameDeleted(ChessAPI::GetActiveGameIndex());
					ImVec2 oldCursorPos;

					if (!IsDeleted)
					{
						oldCursorPos = ImGui::GetCursorPos();
						if (ImGui::InvisibleButton("delete", ImVec2(RMbuttonWidth, RMbuttonHeight)))
							chessfile.DeleteGame(ChessAPI::GetActiveGameIndex());
						ImGui::SetCursorPos(oldCursorPos);

						DrawImage(m_IconDelete, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));
						
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Delete");
					}
					else
					{
						oldCursorPos = ImGui::GetCursorPos();
						if (ImGui::InvisibleButton("restore", ImVec2(RMbuttonWidth, RMbuttonHeight)))
							chessfile.RecoverGame(ChessAPI::GetActiveGameIndex());
						ImGui::SetCursorPos(oldCursorPos);

						DrawImage(m_IconRestore, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Recover");
					}

					ImGui::EndDisabled();

					ImGui::SameLine(0, RMbuttonWidth * 1.5);
					
					{
						oldCursorPos = ImGui::GetCursorPos();
						if (ImGui::InvisibleButton("deleteAll", ImVec2(RMbuttonWidth, RMbuttonHeight)))
						{
							for (auto index : m_searchTables[i]->second.PossitiveIndexes)
								if (!chessfile.IsGameDeleted(index))
									chessfile.DeleteGame(index);
						}
						ImGui::SetCursorPos(oldCursorPos);

						DrawImage(m_IconDeleteAll, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Delete All");
					}

					ImGui::SameLine();

					{
						oldCursorPos = ImGui::GetCursorPos();
						if (ImGui::InvisibleButton("restoreAll", ImVec2(RMbuttonWidth, RMbuttonHeight)))
						{
							for (auto index : m_searchTables[i]->second.PossitiveIndexes)
								if (chessfile.IsGameDeleted(index))
									chessfile.RecoverGame(index);
						}
						ImGui::SetCursorPos(oldCursorPos);

						DrawImage(m_IconRestoreAll, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Restore All");
					}					

					ImGui::EndDisabled();
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
				float size = ImGui::GetFrameHeight();
				const ImVec4 RMbuttonColN = ImGui::ColorConvertU32ToFloat4(Walnut::UI::Colors::ColorWithMultipliedValue(Walnut::UI::Colors::Theme::text, 1.0f));
				const ImVec4 RMbuttonColH = ImGui::ColorConvertU32ToFloat4(Walnut::UI::Colors::ColorWithMultipliedValue(Walnut::UI::Colors::Theme::text, 1.2f));
				const ImVec4 RMbuttonColP = ImGui::ColorConvertU32ToFloat4(Walnut::UI::Colors::Theme::textDarker);
				const float RMbuttonWidth = size * 0.9f;
				const float RMbuttonHeight = size * 0.9f;
				ImVec2 oldCursorPos;

				{
					oldCursorPos = ImGui::GetCursorPos();
					if (ImGui::InvisibleButton("New Game", ImVec2(RMbuttonWidth, RMbuttonHeight)))
						ChessAPI::NewGameInFile();
					ImGui::SetCursorPos(oldCursorPos);

					DrawImage(m_IconAdd, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("New Game");
				}

				ImGui::SameLine();

				{
					oldCursorPos = ImGui::GetCursorPos();
					if (ImGui::InvisibleButton("Save", ImVec2(RMbuttonWidth, RMbuttonHeight)))
					{
						ChessLab::Utils::Save();
					}
					ImGui::SetCursorPos(oldCursorPos);

					DrawImage(m_IconSave, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Save");
				}

				ImGui::SameLine();

				{
					oldCursorPos = ImGui::GetCursorPos();
					if (ImGui::InvisibleButton("Save As", ImVec2(RMbuttonWidth, RMbuttonHeight)))
					{
						ChessLab::Utils::SaveAs();
					}
					ImGui::SetCursorPos(oldCursorPos);

					DrawImage(m_IconSaveAs, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Save As");
				}

				ImGui::SameLine(0, RMbuttonWidth * 1.5);

				bool IsDeleted = chessfile.IsGameDeleted(ChessAPI::GetActiveGameIndex());

				if (IsDeleted)
				{
					oldCursorPos = ImGui::GetCursorPos();
					if (ImGui::InvisibleButton("restore", ImVec2(RMbuttonWidth, RMbuttonHeight)))
						chessfile.RecoverGame(ChessAPI::GetActiveGameIndex());
					ImGui::SetCursorPos(oldCursorPos);

					DrawImage(m_IconRestore, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Recover");
				}
				else
				{
					oldCursorPos = ImGui::GetCursorPos();
					if (ImGui::InvisibleButton("delete", ImVec2(RMbuttonWidth, RMbuttonHeight)))
						chessfile.DeleteGame(ChessAPI::GetActiveGameIndex());
					ImGui::SetCursorPos(oldCursorPos);

					DrawImage(m_IconDelete, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Delete");
				}

				ImGui::SameLine();
				
				{
					oldCursorPos = ImGui::GetCursorPos();
					if (ImGui::InvisibleButton("deleteAll", ImVec2(RMbuttonWidth, RMbuttonHeight)))
					{
						for (size_t index = 0; index < chessfile.GetSize(); index++)
							chessfile.DeleteGame(index);
					}
					ImGui::SetCursorPos(oldCursorPos);

					DrawImage(m_IconDeleteAll, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Delete All");
				}

				ImGui::SameLine();

				{
					oldCursorPos = ImGui::GetCursorPos();
					if (ImGui::InvisibleButton("restoreAll", ImVec2(RMbuttonWidth, RMbuttonHeight)))
					{
						for (size_t index = 0; index < chessfile.GetSize(); index++)
							chessfile.RecoverGame(index);
					}
					ImGui::SetCursorPos(oldCursorPos);

					DrawImage(m_IconRestoreAll, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Restore All");
				}

				ImGui::SameLine(0, RMbuttonWidth * 1.5);

				{
					oldCursorPos = ImGui::GetCursorPos();
					if (ImGui::InvisibleButton("Copy Game", ImVec2(RMbuttonWidth, RMbuttonHeight)))
					{
						ImGui::SetClipboardText(ChessAPI::GetActiveGame().GetPgnGame().GetData().c_str());
					}
					ImGui::SetCursorPos(oldCursorPos);

					DrawImage(m_IconCopyGame, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Copy Game");
				}

				ImGui::SameLine();

				{
					oldCursorPos = ImGui::GetCursorPos();
					if (ImGui::InvisibleButton("Paste Game", ImVec2(RMbuttonWidth, RMbuttonHeight)))
					{
						ChessAPI::NewGameInFile();
						ChessAPI::GetPgnGame().Parse(ImGui::GetClipboardText());
					}
					ImGui::SetCursorPos(oldCursorPos);

					DrawImage(m_IconPasteGame, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Paste Game");
				}

				ImGui::SameLine(0, RMbuttonWidth * 1.5);

				{
					oldCursorPos = ImGui::GetCursorPos();
					if (ImGui::InvisibleButton("Editor", ImVec2(RMbuttonWidth, RMbuttonHeight)))
					{
						g_OpenEditor = true;
					}
					ImGui::SetCursorPos(oldCursorPos);

					DrawImage(m_IconEditor, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Editor");
				}

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

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Add Search");

			ImGui::EndTabBar();
		}

		ImGui::End();
	}

}
