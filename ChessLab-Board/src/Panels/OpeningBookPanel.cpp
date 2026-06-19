#include "OpeningBookPanel.h"

#include "imgui.h"

#include "../ChessAPI/ChessAPI.h"
#include "../Windows/WindowsUtils.h"

#include <filesystem>

namespace Panels
{
	void OpeningBookPanel::OnDetach()
	{
		m_EndPlayThreadJob = true;
		if (m_PlayThread)
			m_PlayThread->join();

		delete m_PlayThread;
		delete m_OpeningBook;
	}

	void OpeningBookPanel::OnImGuiRender()
	{
		if (!m_viewPanel)
		{
			if (m_cobPath.empty())
				CloseCOBfile();
			return;
		}

		ImGui::Begin("Opening Book", &m_viewPanel);
		
		if (!m_cobPath.empty())
		{
			ImGui::TextWrapped(m_cobFilename.c_str());
			if (ImGui::Button("Close"))
			{
				CloseCOBfile();
			}
		}
		else
		{
			ImVec2 textSize = ImGui::CalcTextSize("Load Chess Opening Book (.cob)");

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

			if (ImGui::Button("Load Chess Opening Book (.cob)"))
			{
				CloseCOBfile();
				std::string path = Windows::Utils::OpenFile(L"Chess Opening Book (*.cob)\0*.cob\0");
				if (!path.empty())
					OpenCOBfile(path);
			}

			ImGui::PopStyleColor();

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::wstring wstr(path);
					std::string str(wstr.begin(), wstr.end());
					if (std::filesystem::path(str).extension() == ".cob")
						OpenCOBfile(str);
				}
				ImGui::EndDragDropTarget();
			}
		}

		if (!ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && !ImGui::IsKeyPressed(ImGuiKey_RightArrow))
		{
			ImVec4 white = ImVec4(0.1, 0.79, 0.31, 1);
			ImVec4 draw = ImVec4(0.3, 0.58, 0.97, 1);
			ImVec4 black = ImVec4(0.79, 0.1, 0.1, 1);
			std::string move;
			auto posID = ChessAPI::GetActiveGame().GetFormatedFEN();
			const auto& moveptr = GetOpeningBookMoves(posID);

			if (&moveptr && moveptr.size() && posID.size())
			{
				ImGui::BeginChild("Played Moves", ImVec2(0, 0), true);
				for (auto& moveob : moveptr)
				{
					ImGui::NewLine();

					float cursorStartPosY = ImGui::GetCursorPosY();
					ImGui::SetCursorPosY(cursorStartPosY + ImGui::GetTextLineHeight());
					ImGui::CalcItemWidth();
					if (ImGui::Button(moveob.strmove.c_str(), ImVec2(70, 0)))
						ChessAPI::GetActiveGame().MakeMove(moveob.strmove);
					
					ImGui::SameLine();
					ImGui::SetCursorPosY(cursorStartPosY);
					//ImGui::Text("%d white won -- %d draw -- %d black won", i->second.whitewins, i->second.played - i->second.whitewins - i->second.blackwins, i->second.blackwins);

					float availx = ImGui::GetContentRegionAvail().x;
					int persentage[3] = { 100 * moveob.whitewins / moveob.played,100 * (moveob.played - moveob.blackwins - moveob.whitewins) / moveob.played,100 * moveob.blackwins / moveob.played };

					while (persentage[0] + persentage[1] + persentage[2] < 100)
					{
						persentage[1]++;
					}

					ImGui::PushStyleColor(ImGuiCol_Button, white);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, white);
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, white);

					if (persentage[0] >= 8) { move = std::to_string(persentage[0]) + "%"; }
					else
						move = "##1";
					ImGui::Button(move.c_str(), ImVec2(availx * persentage[0] / 100, 0));

					ImGui::PopStyleColor(3);

					ImGui::PushStyleColor(ImGuiCol_Button, draw);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, draw);
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, draw);

					if (persentage[1] >= 8) { move = std::to_string(persentage[1]) + "%"; }
					else
						move = "##2";
					if (persentage[1])
					{
						ImGui::SetCursorPosY(cursorStartPosY);
						ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - availx - 5 + availx * persentage[0] / 100 + 5);
						ImGui::Button(move.c_str(), ImVec2(availx * persentage[1] / 100, 0));
					}

					ImGui::PopStyleColor(3);

					ImGui::PushStyleColor(ImGuiCol_Button, black);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, black);
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, black);

					if (persentage[2] >= 8) { move = std::to_string(persentage[2]) + "%"; }
					else
						move = "##3";
					if (persentage[2])
					{
						ImGui::SetCursorPosY(cursorStartPosY);
						ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - availx - 5 + availx * persentage[0] / 100 + availx * persentage[1] / 100 + 5);
						ImGui::Button(move.c_str(), ImVec2(availx * persentage[2] / 100, 0));
					}

					ImGui::PopStyleColor(3);

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3, 0.4, 0.5, 1));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3, 0.4, 0.5, 1));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3, 0.4, 0.5, 1));

					ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - availx);
					ImGui::Button(("Played " + std::to_string(moveob.played) + " Times").c_str(), ImVec2(availx, 0));

					ImGui::PopStyleColor(3);
				}
				ImGui::EndChild();
			}
		}
		
		ImGui::End();
	}

	void OpeningBookPanel::OpenCOBfile(const std::string& filepath)
	{
		if (filepath == m_cobPath) { return; }

		delete m_OpeningBook;

		m_cobPath = filepath;

		auto nameWithExtension = std::filesystem::path(filepath).filename();
		m_cobFilename = nameWithExtension.string().substr(0, nameWithExtension.string().find_last_of('.'));

		m_EndPlayThreadJob = false;
		m_PlayThread = new std::thread(
			[this, filepath]()
			{
				m_OpeningBook = new Chess::OpeningBook(filepath);

				while (true)
				{
					if (m_EndPlayThreadJob)
						return;

					using namespace std::chrono_literals;
					std::this_thread::sleep_for(0.2s);

					if (m_CurPosition.empty())
						continue;
					m_Moves = m_OpeningBook->GetMovesByPosition(m_CurPosition, Chess::OpeningBook::MOSTPLAYED);
				}
			}
		);
	}

	bool OpeningBookPanel::CloseCOBfile()
	{
		if (!m_EndPlayThreadJob)
		{
			m_EndPlayThreadJob = true;
			m_PlayThread->join();
			delete m_PlayThread;
			m_PlayThread = nullptr;

			delete m_OpeningBook;
			m_OpeningBook = nullptr;
			m_cobFilename = "";
			m_cobPath = "";

			m_CurPosition.clear();
			m_Moves.clear();

			return true;
		}
		return false;
	}

	std::vector<Chess::OpeningBook::MoveOB>& OpeningBookPanel::GetOpeningBookMoves(const Chess::OpeningBook::PositionID& posID)
	{
		if (!m_OpeningBook) { return m_Moves; }
		m_CurPosition = posID;
		return m_Moves;
	}

	bool& OpeningBookPanel::IsPanelOpen()
	{
		return m_viewPanel;
	}
}
