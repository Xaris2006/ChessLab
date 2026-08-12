#include "ReferencePanel.h"

#include "ImGui.h"

#include "Walnut/timer.h"

#include "../Windows/WindowsUtils.h"

#include "../ChessAPI/ChessAPI.h"

#include "../Panels.h"

static Walnut::Timer s_timer;
static float s_time = 0.0f;

static Walnut::Timer s_timerChecker;
static Walnut::Timer s_timerLoadGames;

namespace Panels {

	void ReferencePanel::OnAttach()
	{
		m_IconPlay = std::make_shared<Walnut::Image>("Resources/Icons/PlayButton.png");
		m_IconStop = std::make_shared<Walnut::Image>("Resources/Icons/StopButton.png");
	}

	void ReferencePanel::OnImGuiRender()
	{
		if (!m_viewPanel)
			return;

		ImGui::Begin("Reference", &m_viewPanel);

		if (m_ClrFile)
		{
			if (!m_Running)
			{
				if (!m_clrFen.empty())
				{
					m_clrFen.clear();
					m_ClrFile->StopSearch(0);

					s_timer.Reset();
					s_time = 0.0f;

					for (auto& [game, index] : m_TopGames)
						game->RemoveReference();

					m_TopGames.clear();
					m_ResultsOrdered.clear();
				}				
			}
			else if (ChessAPI::GetActiveGame().GetFen() != m_clrFen)
			{
				m_ClrFile->StopSearch(0);

				m_clrFen = ChessAPI::GetActiveGame().GetFen();
				
				m_ClrFile->GetSearch(0)->first.StartOption().And(m_clrFen).EndOption();

				m_ClrFile->StartSearch(0);
				s_timer.Reset();
				s_time = 0.0f;

				for (auto& [game, index] : m_TopGames)
					game->RemoveReference();

				m_TopGames.clear();
			}

			auto search = m_ClrFile->GetSearch(0);

			if (search->second.Persentage > 0 && search->second.Persentage < 1)
			{
				s_time = s_timer.ElapsedMillis();
			}
			
			float startPos = ImGui::GetCursorPosX();

			Walnut::UI::TextCentered(Chess::FileManager::Get().GetFilePath(m_ClrFile->GetID()).filename().string().c_str());

			ImGui::SameLine();

			ImGui::SetCursorPosX(startPos);

			ImGui::Text("%d%%", (int)(search->second.Persentage * 100));

			ImGui::SameLine();

			ImGui::Text("(%dms)", (int)s_time);
			
			ImGui::SameLine();

			float size = ImGui::GetFontSize();
			ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - size - ImGui::GetStyle().ItemSpacing.x - ImGui::CalcTextSize("Unload").x - ImGui::GetStyle().FramePadding.x - ImGui::GetStyle().WindowPadding.x);

			if (!m_Running && ImGui::ImageButton((ImTextureID)m_IconPlay->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
			{
				m_Running = true;

				ImGui::End();
				return;
				
			}
			else if (m_Running && ImGui::ImageButton((ImTextureID)m_IconStop->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f)))
			{
				m_Running = false;

				ImGui::End();
				return;
			}

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(m_Running ? "Pause" : "Start");

			ImGui::SameLine();

			ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize("Unload").x - ImGui::GetStyle().FramePadding.x - ImGui::GetStyle().WindowPadding.x);

			static bool s_unloadConfirm = false;

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

			if (ImGui::SmallButton("Unload"))
				s_unloadConfirm = true;

			ImGui::PopStyleColor(3);

			ImGui::Separator();

			if (s_timerChecker.ElapsedMillis() > 100 && m_Running)
			{
				s_timerChecker.Reset();
				m_ResultsOrdered.clear();
				auto moveRes = m_ClrFile->GetMoveResults();

				for (auto& [move, res] : (*moveRes))
				{
					if (move == std::pair<uint8_t, uint8_t>{0, 0})
						continue;

					std::string strMove;

					if (m_ClrFile->GetEncoding() == Chess::CLD)
						strMove = ChessAPI::GetActiveGame().GetBoard().ConvertCLDMoveToPGNMove(move);
					else
					{
						Chess::Board::Move moveToPlay;
						Chess::Piece piecePromote = Chess::NONE;

						moveToPlay.index = move.first;
						moveToPlay.move = (move.second >> 2) - moveToPlay.index;
						piecePromote = Chess::Piece((move.second & 0b00000011ui8) + 1ui8);

						strMove = ChessAPI::GetActiveGame().GetBoard().ConvertCoreMoveToPGNMove(moveToPlay, piecePromote);
					}

					size_t mw = std::get<0>(res);
					size_t mb = std::get<1>(res);
					size_t md = std::get<2>(res);
					size_t mElo = std::get<3>(res);
					uint16_t mLastPlayed = std::get<5>(res);
					size_t mOverall = mw + mb + md;

					if (strMove.empty() || mOverall == 0)
						continue;

					bool inserted = false;

					for (int i = 0; i < m_ResultsOrdered.size(); i++)
					{
						if (std::get<4>(m_ResultsOrdered[i]) < mOverall)
						{
							m_ResultsOrdered.insert(m_ResultsOrdered.begin() + i, { strMove, mw, mb, md, mOverall, mElo, mLastPlayed });
							inserted = true;
							break;
						}
					}

					if (!inserted)
						m_ResultsOrdered.emplace_back(strMove, mw, mb, md, mOverall, mElo, mLastPlayed);
				}

				if (search->second.Persentage < 1.0f)
				{
					s_timerLoadGames.Reset();
				}

				if (s_timerLoadGames.ElapsedMillis() > 500 && m_TopGames.empty())
				{
					auto copyIndexes = m_ClrFile->GetSearchTopGames();

					for (auto& gameIndex : copyIndexes)
					{
						m_TopGames.emplace_back(&(*m_ClrFile)[gameIndex], gameIndex).first->AddReference();

						if (m_TopGames.size() >= 20)
							break;
					}
				}				
			}

			float backup_padding_y = ImGui::GetStyle().FramePadding.y;
			ImGui::GetStyle().FramePadding.y = 0.0f;

			if (ImGui::BeginTable("MoveTable", 5,
				ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg,
				ImVec2(0, ImGui::GetContentRegionAvail().y / 2)))
			{
				ImGui::TableSetupScrollFreeze(1, 1);
				ImGui::TableSetupScrollFreeze(2, 2);
				ImGui::TableSetupColumn("##moves", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
				ImGui::TableSetupColumn("##persentage", ImGuiTableColumnFlags_NoResize);
				ImGui::TableSetupColumn("Games", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
				ImGui::TableSetupColumn("Avg Elo", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
				ImGui::TableSetupColumn("Last Played", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);

				ImGui::TableHeadersRow();

				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);

				ImGui::TableSetColumnIndex(1);
				
				float barSize = ImGui::GetContentRegionAvail().x;

				float oldRounding = ImGui::GetStyle().FrameRounding;
				ImGui::GetStyle().FrameRounding = 0;

				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1.0f));

				size_t w = m_ClrFile->GetWhiteWins();
				size_t b = m_ClrFile->GetBlackWins();
				size_t d = m_ClrFile->GetDraws();
				size_t elo = m_ClrFile->GetAverageElo();
				size_t overall = w + b + d;

				if (int(100 * (double)w / overall) != 0)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8, 0.8, 0.8, 0.8));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8, 0.8, 0.8, 0.8));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8, 0.8, 0.8, 0.8));

					float wSize = (barSize * (float)w / overall);
					ImGui::Button(std::format("{}", int(100 * (double)w / overall)).c_str(), ImVec2(wSize, 0.0f));

					ImGui::PopStyleColor(3);

					ImGui::SameLine(0, 0);
				}
				if (int(100 * (double)d / overall) != 0)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5, 0.5, 0.5, 0.8));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5, 0.5, 0.5, 0.8));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5, 0.5, 0.5, 0.8));

					float dSize = (barSize * (float)d / overall);
					ImGui::Button(std::format("{}", int(100 * (double)d / overall)).c_str(), ImVec2(dSize, 0.0f));

					ImGui::PopStyleColor(3);

					ImGui::SameLine(0, 0);
				}
				if (int(100 * (double)b / overall) != 0)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(95 / 255.0f, 60 / 255.0f, 0, 1));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(95 / 255.0f, 60 / 255.0f, 0, 1));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(95 / 255.0f, 60 / 255.0f, 0, 1));

					float bSize = (barSize * (float)b / overall);
					ImGui::Button(std::format("{}", int(100 * (double)b / overall)).c_str(), ImVec2(bSize, 0.0f));

					ImGui::PopStyleColor(3);
				}

				ImGui::PopStyleColor();

				ImGui::GetStyle().FrameRounding = oldRounding;

				ImGui::TableSetColumnIndex(2);
				ImGui::Text(std::format("{}", search->second.PossitiveIndexes.size()).c_str());

				ImGui::TableSetColumnIndex(3);
				Walnut::UI::TextCentered(std::format("{}", elo).c_str());

				for (auto& [move, mw, mb, md, mOverall, mElo, mLastPlayed] : m_ResultsOrdered)
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);

					if (ImGui::Button(std::format("{}", move).c_str()))
					{
						ChessAPI::GetActiveGame().MakeMove(move);
						break;
					}

					if (ImGui::IsItemHovered())
					{
						Chess::Board::Move coreMove;
						Chess::Piece piecePromote = Chess::NONE;
						ChessAPI::GetActiveGame().GetBoard().ConvertPGNMoveToCoreMove(coreMove, piecePromote, move);

						GetBoardPanel().ShowArrowAt(coreMove.index % 8, coreMove.index / 8, (coreMove.index + coreMove.move) % 8, (coreMove.index + coreMove.move) / 8);
					}

					ImGui::TableSetColumnIndex(1);

					float barSize = ImGui::GetContentRegionAvail().x;

					float oldRounding = ImGui::GetStyle().FrameRounding;
					ImGui::GetStyle().FrameRounding = 0;

					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1.0f));

					if (int(100 * (double)mw / mOverall) != 0)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8, 0.8, 0.8, 0.8));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8, 0.8, 0.8, 0.8));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8, 0.8, 0.8, 0.8));

						float wSize = (barSize * (float)mw / mOverall);
						ImGui::Button(std::format("{}", int(100 * (double)mw / mOverall)).c_str(), ImVec2(wSize, 0.0f));

						ImGui::PopStyleColor(3);

						ImGui::SameLine(0, 0);
					}
					if (int(100 * (double)md / mOverall) != 0)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5, 0.5, 0.5, 0.8));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5, 0.5, 0.5, 0.8));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5, 0.5, 0.5, 0.8));

						float dSize = (barSize * (float)md / mOverall);
						ImGui::Button(std::format("{}", int(100 * (double)md / mOverall)).c_str(), ImVec2(dSize, 0.0f));

						ImGui::PopStyleColor(3);

						ImGui::SameLine(0, 0);
					}
					if (int(100 * (double)mb / mOverall) != 0)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(95 / 255.0f, 60 / 255.0f, 0, 1));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(95 / 255.0f, 60 / 255.0f, 0, 1));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(95 / 255.0f, 60 / 255.0f, 0, 1));

						float bSize = (barSize * (float)mb / mOverall);
						ImGui::Button(std::format("{}", int(100 * (double)mb / mOverall)).c_str(), ImVec2(bSize, 0.0f));

						ImGui::PopStyleColor(3);
					}

					ImGui::PopStyleColor();

					ImGui::GetStyle().FrameRounding = oldRounding;

					ImGui::TableSetColumnIndex(2);
					ImGui::Text(std::format("{}", mOverall).c_str());
					
					ImGui::TableSetColumnIndex(3);
					Walnut::UI::TextCentered(std::format("{}", mElo).c_str());

					if (mLastPlayed != 0)
					{
						ImGui::TableSetColumnIndex(4);
						Walnut::UI::TextCentered(std::format("{}", mLastPlayed).c_str());
					}
				}

				ImGui::EndTable();
			}	

			ImGui::GetStyle().FramePadding.y = backup_padding_y;

			if (ImGui::BeginTable("GamesTable", 6,
				ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInner))
			{
				ImGui::TableSetupScrollFreeze(1, 1);

				for (int i = 0; i < 2; i++)
					ImGui::TableSetupColumn(m_important_prop[i].c_str());
				
				for (int i = 2; i < m_important_prop.size(); i++)
					ImGui::TableSetupColumn(m_important_prop[i].c_str(), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
				
				ImGui::TableHeadersRow();

				for (int row = 0; row < m_TopGames.size(); row++)
				{
					ImGui::TableNextRow();

					for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
					{
						if (!ImGui::TableSetColumnIndex(column) && column > 0)
							continue;
						
						ImGui::PushID(row * ImGui::TableGetColumnCount() + column);

						auto& [game, index] = m_TopGames[row];

						std::string selectableName = (*game)[m_important_prop[column]];

						if (column == 5 && selectableName == "1/2-1/2")
							selectableName = "½-½";

						ImGui::Selectable(selectableName.c_str());

						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
							ChessAPI::OpenChessGameInFile(-index - 1);

						ImGui::PopID();
					}
				}
				
				ImGui::EndTable();
			}

			if (s_unloadConfirm)
			{
				s_unloadConfirm = false;
				m_ClrFile.reset();
				m_clrFen.clear();
			}
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.1f, 0.1f, 0.85f));
			Walnut::UI::TextCentered("No Reference loaded.");
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f / 255.0f, 225.0f / 255.0f, 135.0f / 255.0f, 255.0f / 255.0f));
			Walnut::UI::TextCentered("Load a Reference file (*.clr) to view reference information for the current position.");
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.8f, 0.85f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.8f, 0.65f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.8f, 0.35f));
			if (Walnut::UI::ButtonCentered("Load"))
			{
				std::string filepath = Windows::Utils::OpenFile(L"Chess Lab Reference (*.clr)\0*.clr\0\0");

				if (filepath.size())
				{
					ChessAPI::UnShareClrFile();
					m_ClrFile = std::make_shared<Chess::ClrFile>();
					m_ClrFile->OpenFile(std::filesystem::u8path(filepath));
					ChessAPI::ShareClrFile(m_ClrFile);
				}
			}
			ImGui::PopStyleColor(3);
		}

		ImGui::End();
	}
	
	bool& ReferencePanel::IsPanelOpen()
	{
		return m_viewPanel;
	}
}