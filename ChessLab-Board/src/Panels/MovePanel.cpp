#include "MovePanel.h"

#include "imgui.h"
#include "Walnut/UI/UI.h"
#include "Walnut/ApplicationGUI.h"

#include "../Panels.h"

#include <functional>

static bool s_IsVariationChecked = false;
static Chess::GameManager::MoveKey s_VariationKey;

static bool s_IsPopupUsed = false;
static std::function<void()> s_PopupFunction = nullptr;

static auto hoveredColor = ImVec4(0.2f, 0.48f, 0.87f, 1.0f);

namespace Panels
{
	void MovePanel::OnImGuiRender()
	{
		if (!m_viewPanel)
			return;

		ImGui::Begin("Moves", &m_viewPanel);
		
		if (Panels::IsLoadingPopupOpen())
		{
			ImGui::End();
			return;
		}

		ImGuiStyle& style = ImGui::GetStyle();

		if (ChessAPI::GetChessFileName() == "New Game" && ChessAPI::GetActiveGameIndex() >= 0)
		{
			ImGui::NewLine();

			float size = ImGui::CalcTextSize("New Game").x + style.FramePadding.x * 2.0f;
			float avail = ImGui::GetContentRegionAvail().x;

			float off = (avail - size) * 0.5;
			if (off > 0.0f)
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

			ImGui::TextWrapped("New Game");
		}
		else
		{
			ImGui::PushFont(Walnut::Application::GetFont("Bold"));

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.7, 0.7, 0.7, 1));

			float size = ImGui::CalcTextSize(ChessAPI::GetPgnGame()["White"].c_str()).y + ImGui::CalcTextSize(ChessAPI::GetPgnGame()["WhiteElo"].c_str()).y + 10;
			
			if (ImGui::BeginChild("White", ImVec2(0, size)))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(21, 21, 21, 255));
				if(ChessAPI::GetPgnGame()["White"] == "?")
					Walnut::UI::TextCentered("Unknown");
				else
					Walnut::UI::TextCentered(ChessAPI::GetPgnGame()["White"].c_str());
				
				if (ChessAPI::GetPgnGame()["WhiteElo"] == "?")
					Walnut::UI::TextCentered("");
				else
					Walnut::UI::TextCentered(ChessAPI::GetPgnGame()["WhiteElo"].c_str());

				ImGui::PopStyleColor();
			}

			ImGui::EndChild();

			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.4, 0.4, 0.4, 1));

			size = ImGui::CalcTextSize(ChessAPI::GetPgnGame()["Black"].c_str()).y + ImGui::CalcTextSize(ChessAPI::GetPgnGame()["BlackElo"].c_str()).y + 10;
			
			if (ImGui::BeginChild("Black", ImVec2(0, size)))
			{
				if (ChessAPI::GetPgnGame()["Black"] == "?")
					Walnut::UI::TextCentered("Unknown");
				else
					Walnut::UI::TextCentered(ChessAPI::GetPgnGame()["Black"].c_str());

				if (ChessAPI::GetPgnGame()["BlackElo"] == "?")
					Walnut::UI::TextCentered("");
				else
					Walnut::UI::TextCentered(ChessAPI::GetPgnGame()["BlackElo"].c_str());
			}
			
			ImGui::EndChild();
			
			ImGui::PopStyleColor();
			ImGui::PopFont();

			ImVec4 bcolor = { 0, 0.66, 0.95, 1 };
			ImVec4 gcolor = { 0.38, 0.67, 0, 1 };

			ImGui::PushStyleColor(ImGuiCol_Text, bcolor);
			ImGui::Text("ECO:");
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, gcolor);
			ImGui::PushFont(Walnut::Application::GetFont("Bold"));
			ImGui::Text(ChessAPI::GetPgnGame()["ECO"].c_str());
			ImGui::PopFont();
			ImGui::PopStyleColor();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Text, bcolor);
			ImGui::Text("Result:");
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, gcolor);
			std::string result = ChessAPI::GetPgnGame()["Result"];
			if (result == "1/2-1/2")
				result = "½-½";
			ImGui::PushFont(Walnut::Application::GetFont("Bold"));
			ImGui::Text(result.c_str());
			ImGui::PopFont();
			ImGui::PopStyleColor();
		}
		ImGui::NewLine();

		m_moves = ChessAPI::GetPgnGame().GetMovePathbyRef();

		std::vector<int> pathmove;
		pathmove.push_back(-1);

		if (!ImGui::IsAnyItemHovered())
			m_HoveredMove.clear();

		if (ImGui::BeginTabBar("movebar"))
		{
			if (ImGui::BeginTabItem("Raw"))
			{
				ImGui::BeginChild("##DrawMoves", ImVec2(0, ImGui::GetContentRegionAvail().y - 8 * ImGui::GetStyle().ItemSpacing.y));

				if (!ChessAPI::GetActiveGame().GetNote({ -1 }).note.empty())
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.62f, 0.24f, 1.0f));
					ImGui::TextWrapped(ChessAPI::GetActiveGame().GetNote({ -1 }).note.c_str());
					ImGui::PopStyleColor();

					ImGui::NewLine();
				}

				WriteMove(m_moves, pathmove, {}, 0);
				
				ImGui::EndChild();

				ImGui::EndTabItem();
			}

			bool isTraining = false;

			if (ImGui::BeginTabItem("Training"))
			{
				isTraining = true;

				auto curMovePath = ChessAPI::GetActiveGame().GetLastMoveKey();
				Chess::PgnGame::ChessMovesPath* ptrpgnMovePath = &m_moves;
				for (int i = 1; i < curMovePath.size(); i += 2)
				{
					ptrpgnMovePath = &ptrpgnMovePath->children[curMovePath[i]];
				}
				if (curMovePath[curMovePath.size() - 1] >= 0)
					ImGui::Text(ptrpgnMovePath->move[curMovePath[curMovePath.size() - 1]].c_str());

				if (!ChessAPI::GetActiveGame().GetNote(curMovePath).note.empty())
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.62f, 0.24f, 1.0f));
					ImGui::TextWrapped(ChessAPI::GetActiveGame().GetNote(curMovePath).note.c_str());
					ImGui::PopStyleColor();
				}

				ImGui::EndTabItem();
			}

			{
				ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y
					- ImGui::GetStyle().FramePadding.y * 2
					//- ImGui::GetStyle().ItemSpacing.y * 2
					- ImGui::GetStyle().WindowPadding.y
					- ImGui::CalcTextSize("U").y);
			
				ImGui::Separator();
					
				auto curNote = ChessAPI::GetActiveGame().GetNote(ChessAPI::GetActiveGame().GetLastMoveKey());

				if (curNote.cmds.contains("eval") && !isTraining)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.48f, 0.87f, 0.2f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.48f, 0.87f, 0.2f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.48f, 0.87f, 0.2f));

					ImGui::Button(curNote.cmds["eval"].c_str());
					
					ImGui::PopStyleColor(3);

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Current Evaluation");
					
					ImGui::SameLine();
				}

				if (curNote.cmds.contains("clk"))
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.48f, 0.87f, 0.2f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.48f, 0.87f, 0.2f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.48f, 0.87f, 0.2f));

					ImGui::Button(curNote.cmds["clk"].c_str());

					ImGui::PopStyleColor(3);

					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Current Clock");

					ImGui::SameLine();
				}

				std::string childID;
				{
					auto currentPathMove = ChessAPI::GetActiveGame().GetLastMoveKey();

					if (currentPathMove.size() > 1)
					{
						int j = 1;

						if (currentPathMove[1] < 24)
						{
							childID += (char)('A' + currentPathMove[1]);
							childID += '.';
							j = 3;
						}

						for (; j < currentPathMove.size(); j += 2)
						{
							childID += std::to_string(currentPathMove[j] + 1);
							childID += '.';
						}

						childID.pop_back();
					}
				}
				if (childID.empty())
					childID = "Main Line";
				{
					ImGui::PushID(childID.c_str());

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.48f, 0.87f, 0.65f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.48f, 0.87f, 0.65f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.48f, 0.87f, 0.65f));

					ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(childID.c_str()).x - style.FramePadding.x * 2.0f);
					ImGui::Button(childID.c_str());

					ImGui::PopStyleColor(3);

					ImGui::PopID();
				}
			}

			ImGui::EndTabBar();
		}

		//for (auto& i : m_HoveredMove)
		//	std::cout << i << ' ';
		//std::cout << '\n';

		if (s_IsPopupUsed)
		{
			s_IsPopupUsed = false;
			s_PopupFunction();
		}

		ImGui::End();
		
	}

	bool& MovePanel::IsPanelOpen()
	{
		return m_viewPanel;
	}

	void MovePanel::WriteMove(const Chess::PgnGame::ChessMovesPath& par, Chess::GameManager::MoveKey& pathmove, Chess::GameManager::MoveKey prev_pathmove, float extrain)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		size_t index = 0;

		Chess::GameManager::MoveKey prevMoveKey = prev_pathmove;

		for (size_t i = 0; i < par.move.size(); i++)
		{
			if (par.move[i] == "child")
			{
				prevMoveKey[prevMoveKey.size() - 1] -= 1;

				pathmove[pathmove.size() - 1] += 1;
				pathmove.push_back(index);
				pathmove.push_back(-1);

				Chess::GameManager::MoveKey curVariationKey = pathmove;

				ImGui::NewLine();
				ImGui::SameLine(ImGui::GetCursorPosX() + extrain + 30);
				
				bool didICloseIt = false;

				if (s_IsVariationChecked)
				{
					didICloseIt = true;
					s_IsVariationChecked = false;

					ImGui::PopStyleColor();
				}

				std::string childID;
				{
					int j = 1;

					if (pathmove[1] < 24)
					{
						childID += (char)('A' + pathmove[1]);
						childID += '.';
						j = 3;
					}

					for (; j < pathmove.size(); j += 2)
					{
						childID += std::to_string(pathmove[j] + 1);
						childID += '.';
					}

					childID.pop_back();
				}

				{
					ImGui::PushID(childID.c_str());

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.48f, 0.87f, 0.65f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.48f, 0.87f, 0.45f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.48f, 0.87f, 0.45f));

					ImGui::Button(childID.c_str());

					ImGui::PopStyleColor(3);

					ImGui::PopID();
				}

				bool isHovered = ImGui::IsItemHovered() || s_VariationKey == curVariationKey;

				if (isHovered)
				{
					s_IsVariationChecked = true;
					s_VariationKey.clear();

					ImGui::PushStyleColor(ImGuiCol_Text, hoveredColor);
				}

				ImGui::SameLine();

				ImGui::Text("[ ");

				WriteMove(par.children[index], pathmove, prevMoveKey, extrain + 30);

				ImGui::SameLine();

				if (ImGui::GetCursorPosX() < (extrain + 30))
					ImGui::SetCursorPosX(extrain + 30);

				ImGui::Text("] ");

				if (isHovered)
				{
					s_IsVariationChecked = false;

					ImGui::PopStyleColor();
				}

				ImGui::SameLine();

				{
					ImGui::PushID(('s' + childID).c_str());
				
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.48f, 0.87f, 0.65f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.48f, 0.87f, 0.45f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.48f, 0.87f, 0.45f));
				
					ImGui::SmallButton(childID.c_str());
				
					if (ImGui::IsItemHovered())
						s_VariationKey = curVariationKey;
					
					ImGui::PopStyleColor(3);
				
					ImGui::PopID();
				}

				if (didICloseIt)
				{
					s_IsVariationChecked = true;

					ImGui::PushStyleColor(ImGuiCol_Text, hoveredColor);
				}

				ImGui::NewLine();
				//ImGui::SetCursorPosX(ImGui::GetCursorPosX() + extrain);

				pathmove.resize(pathmove.size() - 2);
				prevMoveKey[prevMoveKey.size() - 1] += 1;

				index += 1;
			}
			else
			{
				float toAddSize = 0;

				pathmove[pathmove.size() - 1] += 1;
				
				if (i == par.move.size() - 1)
					toAddSize += (ImGui::CalcTextSize("] ").x + style.FramePadding.x * 2.0f);

				toAddSize += (ImGui::CalcTextSize(ChessAPI::GetActiveGame().GetNote(pathmove).note.c_str()).x + ImGui::CalcTextSize(par.move[i].c_str()).x + style.FramePadding.x * 4.0f);

				ImGui::SameLine();
				
				if (ImGui::GetCursorPosX() < extrain)
					ImGui::SetCursorPosX(extrain);

				if (ImGui::GetContentRegionAvail().x - toAddSize < 30)
				{
					ImGui::NewLine();
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + extrain);
				}

				float red = 0, green = 0, blue = 0, alfa = 0;
				
				if (Panels::GetBoardPanel().IsMoveChooseOpen() && prevMoveKey == ChessAPI::GetActiveGame().GetLastMoveKey())
				{
					red = 0.6; green = 0.5; blue = 0.8; alfa = 0.5;
				}

				if (pathmove == ChessAPI::GetActiveGame().GetLastMoveKey())
				{ 
					red = 0.5; green = 0.8; blue = 0.6; alfa = 0.7; 
				}

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(red, green, blue, alfa));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(red, green, blue, alfa));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(red, green, blue, 0));

				auto hovered = ImGui::GetStyleColorVec4(ImGuiCol_Text);
				if (pathmove == m_HoveredMove)
					hovered = hoveredColor;

				ImGui::PushStyleColor(ImGuiCol_Text, hovered);

				auto io = ImGui::GetIO();
				ImFont* font = io.FontDefault;
				if (!par.parent)
					font = Walnut::Application::GetFont("Bold");
				else if (i == 0 || (i + 1 < par.move.size() && par.move[i + 1] == "child"))
					font = Walnut::Application::GetFont("Italic");

				ImGui::PushFont(font);

				std::string id = "";

				for (auto& i : pathmove)
				{
					id += std::to_string(i);
					id += '.';
				}

				ImGui::PushID(id.c_str());

				if (ImGui::Button(par.move[i].c_str()))
					ChessAPI::GetActiveGame().GoToPositionByKey(pathmove);
				
				static Chess::GameManager::MoveKey oldMoveKey;
				if (pathmove != oldMoveKey && pathmove == ChessAPI::GetActiveGame().GetLastMoveKey())
				{
					oldMoveKey = pathmove;
					ImGui::SetScrollHereY();
				}

				prevMoveKey = pathmove;

				if (ImGui::IsItemHovered())
					m_HoveredMove = pathmove;

				ImGui::PopID();
				ImGui::PopFont();
				ImGui::PopStyleColor(4);

				std::string PopupID = "MovePopup";
				for (auto& i : pathmove)
					PopupID += (char)(i + 1);

				if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					ImGui::OpenPopup(PopupID.c_str());

				if (ImGui::BeginPopup(PopupID.c_str()))
				{
					if (ImGui::Selectable("Delete Move"))
					{
						s_IsPopupUsed = true;
						s_PopupFunction = [pathmove]()
							{
								ChessAPI::GetActiveGame().DeleteMove(pathmove);
							};
					}

					if (ImGui::Selectable("Delete Variation"))
					{
						s_IsPopupUsed = true;
						s_PopupFunction = [pathmove]()
							{
								auto pathcopy = pathmove;
								pathcopy.back() = 0;

								ChessAPI::GetActiveGame().DeleteMove(pathcopy);
							};
					}

					ImGui::BeginDisabled(pathmove.size() == 1);

					if (ImGui::Selectable("Promote Variation"))
					{
						s_IsPopupUsed = true;
						s_PopupFunction = [pathmove]()
							{
								ChessAPI::GetActiveGame().EditVariation(pathmove, Chess::GameManager::SWAP);
							};
					}

					ImGui::EndDisabled();

					ImGui::EndPopup();
				}

				if (!ChessAPI::GetActiveGame().GetNote(pathmove).note.empty())
				{
					ImGui::SameLine();

					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.62f, 0.24f, 1.0f));
					ImGui::TextWrapped(ChessAPI::GetActiveGame().GetNote(pathmove).note.c_str());
					//ImGui::SameLine();
					//ImGui::TextWrapped("%f", totalsize);
					ImGui::PopStyleColor();
				}
			}
		}
	}
}
