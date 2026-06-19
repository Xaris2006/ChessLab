#include "DatabasePanel.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_internal.h"

#include "Walnut/UI/UI.h"
#include "Walnut/ImGui/ImGuiTheme.h"
#include "Walnut/timer.h"


#include "../ChessLabUtils.h"
#include "../Panels.h"

static Walnut::Timer s_timer;
static float s_time = 0.0f;

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
	void DatabasePanel::OnAttach()
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

		m_IconPlay = std::make_shared<Walnut::Image>("Resources/Icons/PlayButton.png");
		m_IconStop = std::make_shared<Walnut::Image>("Resources/Icons/StopButton.png");
		m_IconDelete = std::make_shared<Walnut::Image>("Resources/Icons/bin.png");
		m_IconDeleteAll = std::make_shared<Walnut::Image>("Resources/Icons/delete.png");
		m_IconRestore = std::make_shared<Walnut::Image>("Resources/Icons/restore.png");
		m_IconRestoreAll = std::make_shared<Walnut::Image>("Resources/Icons/refresh.png");
		m_IconAdd = std::make_shared<Walnut::Image>("Resources/Icons/plus.png");
		m_IconSave = std::make_shared<Walnut::Image>("Resources/Icons/save.png");
		m_IconSaveAs = std::make_shared<Walnut::Image>("Resources/Icons/save-as.png");
		m_IconCopyGame = std::make_shared<Walnut::Image>("Resources/Icons/copy.png");
		m_IconPasteGame = std::make_shared<Walnut::Image>("Resources/Icons/paste.png");
		m_IconEditor = std::make_shared<Walnut::Image>("Resources/Icons/editor.png");
		m_IconSearch = std::make_shared<Walnut::Image>("Resources/Icons/search.png");
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
		bool openGamePopup = false;
		static int s_oldSearchFocus = -1;
		bool nfile = false;

		if (m_filePath != ChessAPI::GetChessFilePath())
		{
			s_oldSearchFocus = -1;
			nfile = true;
			Reset();
			m_filePath = ChessAPI::GetChessFilePath();
		}

		Chess::ChessFile& chessfile = ChessAPI::GetChessFile();
		
		ImGui::Begin("Database");

		if (Panels::IsLoadingPopupOpen())
		{
			ImGui::End();
			return;
		}

		ImGui::TextWrapped(ChessAPI::GetChessFileName().c_str());
		
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

							ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345").x);
							ImGui::InputText("Title##w", &m_title_to_searchW);

							ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

							ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911").x);
							ImGui::InputText("Fide ID##w", &m_fideId_to_searchW);

							if (ImGui::RadioButton("Search Elo", m_elo_to_searchW))
							{
								m_elo_to_searchW = !m_elo_to_searchW;
							}

							ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

							ImGui::BeginDisabled(!m_elo_to_searchW);

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

							ImGui::EndDisabled();

							ImGui::TreePop();
						}
						if (ImGui::TreeNodeEx("Black Player", ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911131517").x);
							ImGui::InputText("Name##b", &m_name_to_searchB);

							ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

							ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345").x);
							ImGui::InputText("Title##b", &m_title_to_searchB);

							ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

							ImGui::SetNextItemWidth(ImGui::CalcTextSize("12345678911").x);
							ImGui::InputText("Fide ID##b", &m_fideId_to_searchB);

							if (ImGui::RadioButton("Search Elo", m_elo_to_searchB))
							{
								m_elo_to_searchB = !m_elo_to_searchB;
							}

							ImGui::SameLine(0, ImGui::CalcTextSize("123").x);

							ImGui::BeginDisabled(!m_elo_to_searchB);

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

							ImGui::EndDisabled();

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

							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));

							if (ImGui::Button("Paste"))
							{
								auto buffer = ImGui::GetClipboardText();

								if (buffer)
								{
									m_fen_to_search = buffer;

									if (m_fen_to_search.size() < 4 || m_fen_to_search.substr(0, 3) != "CLF")
										m_fen_to_search.clear();
									else
										m_fen_to_search = m_fen_to_search.substr(3);								
								}
							}

							ImGui::PopStyleColor(3);

							ImGui::SameLine();

							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

							if (!m_fen_to_search.empty() && ImGui::Button("Clear"))
							{
								m_fen_to_search.clear();
							}

							ImGui::PopStyleColor(3);

							ImGui::SameLine();

							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.58f, 0.97f, 1.0f));
							ImGui::PushFont(Walnut::Application::GetFont("Bold"));
							ImGui::Text(m_fen_to_search.c_str());
							ImGui::PopFont();
							ImGui::PopStyleColor();

							if (ImGui::IsItemClicked() && !m_fen_to_search.empty())
							{
								Panels::GetBoardPanel().OpenEditor(m_fen_to_search);
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
								.And("WhiteElo", m_elo_to_searchW ? m_eloMin_to_searchW : 1, m_elo_to_searchW ? m_eloMax_to_searchW : 0)
								.And("WhiteTitle", m_title_to_searchW)
								.And("WhiteFideId", m_fideId_to_searchW)
								.And("Black", m_name_to_searchB)
								.And("BlackElo", m_elo_to_searchB ? m_eloMin_to_searchB : 1, m_elo_to_searchB ? m_eloMax_to_searchB : 0)
								.And("BlackTitle", m_title_to_searchB)
								.And("BlackFideId", m_fideId_to_searchB)
								.And("Result", m_result_to_search)
								.And("Event", m_event_to_search)
								.And("Round", m_round_to_search)
								.And("Site", m_site_to_search)
								.And("Source", m_source_to_search)
								.And("ECO", m_eco_to_search)
								.And("Date", m_date_to_search)
								.And(m_fen_to_search)
								.EndOption();
						}

						chessfile.StartSearch(i);
						s_timer.Reset();
						s_time = 0.0f;
					}

					if (chessfile.GetSearch(i)->second.Persentage > 0 && chessfile.GetSearch(i)->second.Persentage < 1)
					{
						if (ImGui::ImageButton((ImTextureID)m_IconStop->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
							chessfile.StopSearch(i);

						s_time = s_timer.ElapsedMillis();
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

					ImGui::SameLine();

					ImGui::Text("(%dms)", (int)s_time);

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
						{
							for (auto index : m_pressedIndexes)
								chessfile.DeleteGame(index);
						}
						ImGui::SetCursorPos(oldCursorPos);

						DrawImage(m_IconDelete, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));
						
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Delete");
					}
					else
					{
						oldCursorPos = ImGui::GetCursorPos();
						if (ImGui::InvisibleButton("restore", ImVec2(RMbuttonWidth, RMbuttonHeight)))
						{
							for (auto index : m_pressedIndexes)
								chessfile.RecoverGame(index);
						}
						ImGui::SetCursorPos(oldCursorPos);

						DrawImage(m_IconRestore, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Restore");
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

					ImGui::SameLine(0, RMbuttonWidth * 1.5);

					ImGui::BeginDisabled(chessfile.GetSearch(i)->second.PossitiveIndexes.empty());

					static size_t goToLine = 1;
					bool goToLinePressed = false;

					auto oldValueFPy = ImGui::GetStyle().FramePadding.y;
					ImGui::GetStyle().FramePadding.y = RMbuttonHeight / 2 - ImGui::CalcTextSize("Go").y / 2;

					ImGui::SetNextItemWidth(std::max(ImGui::CalcTextSize(std::format("{}12", chessfile.GetSize()).c_str()).x, 70.0f));
					ImGui::DragScalar("##GoToLine", ImGuiDataType_U64, &goToLine, 1, 0, 0, "%d");

					ImGui::GetStyle().FramePadding.y = oldValueFPy;

					if (goToLine < 1 && !m_searchTables[i]->second.PossitiveIndexes.empty())
						goToLine = 1;
					else if (goToLine > m_searchTables[i]->second.PossitiveIndexes.size())
						goToLine = m_searchTables[i]->second.PossitiveIndexes.size();

					ImGui::SameLine();

					{
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + RMbuttonHeight * 0.3f / 2);
						oldCursorPos = ImGui::GetCursorPos();
						if (ImGui::InvisibleButton("GoToLine", ImVec2(RMbuttonWidth * 0.7f, RMbuttonHeight * 0.7f)))
						{
							m_pressedIndexes.insert(m_searchTables[i]->second.PossitiveIndexes[goToLine - 1]);
							goToLinePressed = true;
						}
						ImGui::SetCursorPos(oldCursorPos);

						DrawImage(m_IconSearch, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth * 0.7f, RMbuttonHeight * 0.7f));

						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Go to Line");

						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + RMbuttonHeight * 0.3f / 2);
					}

					ImGui::EndDisabled();
					ImGui::EndDisabled();
					ImGui::Separator();

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4());
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4());
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4());

					if (ImGui::BeginTable("searchTable", 12,
						ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInner))
					{
						ImGui::TableSetupScrollFreeze(1, 1);
						ImGui::TableSetupColumn("Line #", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
						ImGui::TableSetupColumn("Index #", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
						ImGui::TableSetupColumn("##edited", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
						ImGui::TableSetupColumn("##deleted", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize); 
						
						for (int i = 0; i < m_important_prop.size() - 2; i++)
							ImGui::TableSetupColumn(m_important_prop[i].c_str());
						ImGui::TableHeadersRow();

						ImGuiListClipper clipper;
						clipper.Begin(m_searchTables[i]->second.PossitiveIndexes.size());

						if (goToLinePressed)
							clipper.ForceDisplayRangeByIndices(goToLine - 1, goToLine);

						while (clipper.Step())
						{
							for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
							{
								ImGui::TableNextRow();

								auto indexGame = m_searchTables[i]->second.PossitiveIndexes[row];

								if (m_pressedIndexes.contains(m_searchTables[i]->second.PossitiveIndexes[row]))
									ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(40, 50, 110, 200));

								for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
								{
									if (!ImGui::TableSetColumnIndex(column) && column > 0)
										continue;

									float buttonSize = ImGui::CalcTextSize("U").y;
									ImRect BulletPosition(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + ImVec2(buttonSize, buttonSize));

									if (column == 2)
									{
										ImGui::InvisibleButton("Un", ImVec2(buttonSize, buttonSize));
										if (chessfile.IsGameEdited(indexGame))
										{
											ImGui::SameLine();

											ImGui::RenderBullet(ImGui::GetWindowDrawList(), BulletPosition.GetCenter(), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
										}
									}
									else if (column == 3)
									{
										auto oldCursorPos = ImGui::GetCursorPos();
										ImGui::InvisibleButton("Un", ImVec2(buttonSize, buttonSize));
										ImGui::SetCursorPos(oldCursorPos);

										if (chessfile.IsGameDeleted(indexGame))
										{
											ImGui::Image((ImTextureID)m_IconDelete->GetRendererID(), ImVec2(buttonSize, buttonSize));
										}
									}
									else if (column == 1)
									{
										ImGui::PushID(row);

										ImGui::Text(std::format("{}", indexGame + 1).c_str());

										ImGui::PopID();
									}
									else if (column == 0)
									{
										ImGui::PushID(row);

										ImGui::Text(std::format("{}", row + 1).c_str());

										ImGui::PopID();
									}
									else
									{
										ImGui::PushID(row * ImGui::TableGetColumnCount() + column);

										auto& game = chessfile.operator[](indexGame);

										std::string selectableName = game[m_important_prop[column - 4]];

										if (column == 10 && selectableName == "1/2-1/2")
											selectableName = "½-½";

										if (ImGui::Selectable(selectableName.c_str()))
										{
											if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))
											{
												if (m_pressedIndexes.contains(indexGame))
													m_pressedIndexes.erase(indexGame);
												else
													m_pressedIndexes.insert(indexGame);
											}
											else
											{
												m_pressedIndexes.clear();
												m_pressedIndexes.insert(indexGame);
											}
										}

										if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
											ChessAPI::OpenChessGameInFile(indexGame);

										if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && m_pressedIndexes.contains(indexGame))
											openGamePopup = true;

										if (ImGui::BeginDragDropSource())
										{
											ImGui::SetDragDropPayload("Database", &indexGame, sizeof(int));
											ImGui::EndDragDropSource();
										}
										ImGui::PopID();
									}

									if (goToLinePressed && row == goToLine - 1)
									{
										ImGui::SetScrollHereY();
										goToLinePressed = false;
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

				{
					oldCursorPos = ImGui::GetCursorPos();
					if (ImGui::InvisibleButton("delete", ImVec2(RMbuttonWidth, RMbuttonHeight)))
					{
						for (auto index : m_pressedIndexes)
							chessfile.DeleteGame(index);
					}
					ImGui::SetCursorPos(oldCursorPos);

					DrawImage(m_IconDelete, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Delete");
				}

				ImGui::SameLine();
								
				{
					oldCursorPos = ImGui::GetCursorPos();
					if (ImGui::InvisibleButton("restore", ImVec2(RMbuttonWidth, RMbuttonHeight)))
					{
						for (auto index : m_pressedIndexes)
							chessfile.RecoverGame(index);
					}
					ImGui::SetCursorPos(oldCursorPos);

					DrawImage(m_IconRestore, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Restore");
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
						auto buffer = ImGui::GetClipboardText();

						if (buffer)
						{
							std::string fenToAdd = buffer;

							if (fenToAdd.size() >= 4 && fenToAdd.substr(0, 3) == "CLG")
							{
								ChessAPI::NewGameInFile();
								ChessAPI::GetPgnGame().Parse(fenToAdd.substr(3));
							}								
						}
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
						Panels::GetBoardPanel().OpenEditor();
					}
					ImGui::SetCursorPos(oldCursorPos);

					DrawImage(m_IconEditor, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth, RMbuttonHeight));

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Editor");
				}

				ImGui::SameLine(0, RMbuttonWidth * 1.5);

				static size_t goToLine = 1;
				bool goToLinePressed = false;

				auto oldValueFPy = ImGui::GetStyle().FramePadding.y;
				ImGui::GetStyle().FramePadding.y = RMbuttonHeight / 2 - ImGui::CalcTextSize("Go").y / 2;

				ImGui::SetNextItemWidth(std::max(ImGui::CalcTextSize(std::format("{}12", chessfile.GetSize()).c_str()).x, 70.0f));
				ImGui::DragScalar("##GoToLine", ImGuiDataType_U64, &goToLine, 1, 0, 0, "%d");
								
				ImGui::GetStyle().FramePadding.y = oldValueFPy;

				if (goToLine < 1)
					goToLine = 1;
				else if (goToLine > chessfile.GetSize())
					goToLine = chessfile.GetSize();

				ImGui::SameLine();

				{
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + RMbuttonHeight * 0.3f / 2);
					oldCursorPos = ImGui::GetCursorPos();
					if (ImGui::InvisibleButton("GoToLine", ImVec2(RMbuttonWidth * 0.7f, RMbuttonHeight * 0.7f)))
					{
						m_pressedIndexes.insert(goToLine - 1);
						goToLinePressed = true;
					}
					ImGui::SetCursorPos(oldCursorPos);

					DrawImage(m_IconSearch, RMbuttonColN, RMbuttonColH, RMbuttonColP, ImVec2(RMbuttonWidth * 0.7f, RMbuttonHeight * 0.7f));

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Go to Line");

					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + RMbuttonHeight * 0.3f / 2);
				}

				ImGui::Separator();

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4());
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4());
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4());

				if (ImGui::BeginTable("MainTable", 11,
					ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInner))
				{
					ImGui::TableSetupScrollFreeze(1, 1);
					ImGui::TableSetupColumn("Line #", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
					ImGui::TableSetupColumn("##edited", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
					ImGui::TableSetupColumn("##deleted", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);

					for (int i = 0; i < m_important_prop.size() - 2; i++)
						ImGui::TableSetupColumn(m_important_prop[i].c_str());
					ImGui::TableHeadersRow();

					ImGuiListClipper clipper;
					clipper.Begin(chessfile.GetSize());

					if (goToLinePressed)
						clipper.ForceDisplayRangeByIndices(goToLine - 1, goToLine);

					while (clipper.Step())
					{
						for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
						{
							ImGui::TableNextRow();
							
							if (m_pressedIndexes.contains(row))
								ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor(40, 50, 110, 200));

							for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
							{
								if (!ImGui::TableSetColumnIndex(column) && column > 0)
									continue;
								
								float buttonSize = ImGui::CalcTextSize("U").y;
								ImRect BulletPosition(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + ImVec2(buttonSize, buttonSize));

								if (column == 1)
								{
									ImGui::InvisibleButton("Un", ImVec2(buttonSize, buttonSize));
									if (chessfile.IsGameEdited(row))
									{
										ImGui::SameLine();

										ImGui::RenderBullet(ImGui::GetWindowDrawList(), BulletPosition.GetCenter(), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]));
									}
								}
								else if (column == 2)
								{
									auto oldCursorPos = ImGui::GetCursorPos();
									ImGui::InvisibleButton("Un", ImVec2(buttonSize, buttonSize));
									ImGui::SetCursorPos(oldCursorPos);

									if (chessfile.IsGameDeleted(row))
									{
										ImGui::Image((ImTextureID)m_IconDelete->GetRendererID(), ImVec2(buttonSize, buttonSize));
									}
								}
								else if (column == 0)
								{
									ImGui::PushID(row);

									ImGui::Text(std::format("{}", row + 1).c_str());

									ImGui::PopID();
								}
								else
								{
									ImGui::PushID(row * ImGui::TableGetColumnCount() + column);

									auto& game = chessfile.operator[](row);

									std::string selectableName = game[m_important_prop[column - 3]];
									
									if (column == 9 && selectableName == "1/2-1/2")
										selectableName = "½-½";

									if (ImGui::Selectable(selectableName.c_str()))
									{
										if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))
										{
											if (m_pressedIndexes.contains(row))
												m_pressedIndexes.erase(row);
											else
												m_pressedIndexes.insert(row);
										}
										else
										{
											m_pressedIndexes.clear();
											m_pressedIndexes.insert(row);
										}
									}

									if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
										ChessAPI::OpenChessGameInFile(row);

									if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && m_pressedIndexes.contains(row))
										openGamePopup = true;

									if (ImGui::BeginDragDropSource())
									{
										ImGui::SetDragDropPayload("Database", &row, sizeof(int));
										ImGui::EndDragDropSource();
									}
									ImGui::PopID();
								}

								if (goToLinePressed && row == goToLine - 1)
								{
									ImGui::SetScrollHereY();
									goToLinePressed = false;
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

		if (openGamePopup)
			ImGui::OpenPopup("Game Popup");

		GamePopup();

		ImGui::End();
	}

	void DatabasePanel::GamePopup()
	{
		if (ImGui::BeginPopup("Game Popup"))
		{
			if (ImGui::Selectable("Open"))
			{
				for (auto index : m_pressedIndexes)
					ChessAPI::OpenChessGameInFile(index);

				ImGui::CloseCurrentPopup();
			}

			ImGui::Separator();

			if (ImGui::Selectable("Delete"))
			{
				for (auto index : m_pressedIndexes)
					ChessAPI::GetChessFile().DeleteGame(index);

				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Selectable("Restore"))
			{
				for (auto index : m_pressedIndexes)
					ChessAPI::GetChessFile().RecoverGame(index);

				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Selectable("Reset"))
			{
				for (auto index : m_pressedIndexes)
				{
					if (ChessAPI::GetChessFile().IsGameEdited(index))
					{
						auto& pgnGame = ChessAPI::GetChessFile()[index];

						pgnGame.Parse(pgnGame.GetDataRead());

						if (ChessAPI::IsGameOpen(index))
						{
							ChessAPI::OpenChessGameInFile(index);
							ChessAPI::GetActiveGame().InitPgnGame(ChessAPI::GetChessFile()[index]);
							ChessAPI::GetChessFile().RemoveFromEdited(index);
						}
					}

				}

				ImGui::CloseCurrentPopup();
			}

			ImGui::Separator();

			if (ImGui::Selectable("Copy"))
			{
				ImGui::SetClipboardText(ChessAPI::GetActiveGame().GetPgnGame().GetData().c_str());

				ImGui::CloseCurrentPopup();
			}
			
			if (ImGui::Selectable("Replace"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
}
