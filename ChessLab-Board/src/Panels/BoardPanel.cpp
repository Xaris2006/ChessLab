#include "BoardPanel.h"

#include "../ChessAPI/ChessAPI.h"
#include "ChessCore/GameManager.h"
#include "ChessCore/FileFormats/pgn/PgnFile.h"

#include "../AppManagerChild.h"

#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#include "../../Walnut/Source/Walnut/Application.h"

#include "../Panels.h"

static std::string s_fen;
static bool cross = true;
static std::array<std::array<int, 8>, 8> s_tags;

struct ArrowsData
{
	int type = 0;
	ImVec2 start = ImVec2(-1, -1);
	ImVec2 end = ImVec2(-1, -1);
};

static std::vector<ArrowsData> s_arrows;
static ImVec2 s_startPressedPos = ImVec2(-1, -1);


static void RenderRotatedImage(ImTextureID texture, ImVec2 pos, ImVec2 size, float cosValue, float sinValue, ImU32 color = IM_COL32_WHITE)
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
	float cos_a = cosValue;
	float sin_a = sinValue;

	ImVec2 vertices[4];
	vertices[0] = ImVec2(-0.5f, -0.5f); // Top-left
	vertices[1] = ImVec2(0.5f, -0.5f); // Top-right
	vertices[2] = ImVec2(0.5f, 0.5f); // Bottom-right
	vertices[3] = ImVec2(-0.5f, 0.5f); // Bottom-left

	for (int i = 0; i < 4; i++) {
		float x = vertices[i].x * size.x;
		float y = vertices[i].y * size.y;
		vertices[i].x = center.x + x * cos_a - y * sin_a;
		vertices[i].y = center.y + x * sin_a + y * cos_a;
	}

	ImVec2 uv0 = ImVec2(0.0f, 0.0f);
	ImVec2 uv1 = ImVec2(1.0f, 0.0f);
	ImVec2 uv2 = ImVec2(1.0f, 1.0f);
	ImVec2 uv3 = ImVec2(0.0f, 1.0f);
	
	draw_list->AddImageQuad(texture, vertices[0], vertices[1], vertices[2], vertices[3], uv0, uv1, uv2, uv3, color);
}

static void DrawRedDotAt(const ImVec2& SetPosition, float radius = 5.0f)
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddCircleFilled(ImVec2(SetPosition.x + ImGui::GetWindowPos().x, SetPosition.y + ImGui::GetWindowPos().y), radius, IM_COL32(255, 0, 0, 255));
}

namespace Panels
{

void BoardPanel::OnAttach()
{
	m_board[0] = std::make_shared<Walnut::Image>("Resources\\Board\\board.png");
	m_board[1] = std::make_shared<Walnut::Image>("Resources\\Board\\boardRev.png");
	m_bar = std::make_shared<Walnut::Image>("Resources\\ChessBar.png");

	m_WhiteBox = std::make_shared<Walnut::Image>("Resources\\Board\\WhiteBox.png");
	m_BlackBox = std::make_shared<Walnut::Image>("Resources\\Board\\BlackBox.png");

	m_pieces[0] = std::make_shared<Walnut::Image>("Resources\\piecies\\white_pawn.png");
	m_pieces[1] = std::make_shared<Walnut::Image>("Resources\\piecies\\white_knight.png");
	m_pieces[2] = std::make_shared<Walnut::Image>("Resources\\piecies\\white_bishop.png");
	m_pieces[3] = std::make_shared<Walnut::Image>("Resources\\piecies\\white_rook.png");
	m_pieces[4] = std::make_shared<Walnut::Image>("Resources\\piecies\\white_queen.png");
	m_pieces[5] = std::make_shared<Walnut::Image>("Resources\\piecies\\white_king.png");
	m_pieces[6] = std::make_shared<Walnut::Image>("Resources\\piecies\\black_pawn.png");
	m_pieces[7] = std::make_shared<Walnut::Image>("Resources\\piecies\\black_knight.png");
	m_pieces[8] = std::make_shared<Walnut::Image>("Resources\\piecies\\black_bishop.png");
	m_pieces[9] = std::make_shared<Walnut::Image>("Resources\\piecies\\black_rook.png");
	m_pieces[10] = std::make_shared<Walnut::Image>("Resources\\piecies\\black_queen.png");
	m_pieces[11] = std::make_shared<Walnut::Image>("Resources\\piecies\\black_king.png");

	m_circleFromStart = std::make_shared<Walnut::Image>("Resources\\Board\\f.png");
	m_circleToEnd = std::make_shared<Walnut::Image>("Resources\\Board\\f2New.png");
	
	m_RedTag = std::make_shared<Walnut::Image>("Resources\\Board\\RedTagB.png");
	m_GreenTag = std::make_shared<Walnut::Image>("Resources\\Board\\GreenTagB.png");
	m_BlueTag = std::make_shared<Walnut::Image>("Resources\\Board\\BlueTagB.png");

	m_RedArrow = std::make_shared<Walnut::Image>("Resources\\Board\\RedArrow.png");
	m_GreenArrow = std::make_shared<Walnut::Image>("Resources\\Board\\GreenArrow.png");
	m_BlueArrow = std::make_shared<Walnut::Image>("Resources\\Board\\BlueArrow.png");

	m_RedLine = std::make_shared<Walnut::Image>("Resources\\Board\\RedLine.png");

	m_RedX = std::make_shared<Walnut::Image>("Resources\\RedX.png");

	UpdateBoardValues();

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			s_tags[i][j] = 0;
		}
	}

	m_PromoteMove.move = 0;
}

void BoardPanel::OnImGuiRender()
{
	auto MousePos = FindMousePos();

	ImGui::Begin("Game", 0, ImGuiWindowFlags_NoScrollbar);

	if (IsLoadingPopupOpen())
	{
		ImGui::End();
		return;
	}

	int tabRemove = -1;
	auto& opened = ChessAPI::GetOpenGameIndexes();

	if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_TabListPopupButton))
	{
		cross = true;

		for (int n = 0; n < opened.size(); n++)
		{
			bool activeTab = false;

			if (ChessAPI::GetActiveGameIndex() == opened[n])
			{
				activeTab = true;
			}			
			
			ImGui::PushID(opened[n]);

			bool* crossAddress = &cross;
			if (opened.size() == 1)
				crossAddress = nullptr;

			auto flagUnsaved = ImGuiTabItemFlags_None;
			auto flagActive = ImGuiTabItemFlags_None;

			if (ChessAPI::GetChessFile().IsGameEdited(opened[n]) && opened[n] >= 0)
				flagUnsaved = ImGuiTabItemFlags_UnsavedDocument;

			if (activeTab)
				flagActive = ImGuiTabItemFlags_SetSelected;

			std::string tabName;

			if (opened[n] >= 0)
				tabName = std::to_string(opened[n] + 1) + ": ";
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Tab, ImColor(135, 225, 255, 50).Value);
				ImGui::PushStyleColor(ImGuiCol_TabHovered, ImColor(135, 225, 255, 80).Value);
				ImGui::PushStyleColor(ImGuiCol_TabActive, ImColor(135, 225, 255, 110).Value);
				ImGui::PushStyleColor(ImGuiCol_TabUnfocused, ImColor(135, 225, 255, 50).Value);
				ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, ImColor(135, 225, 255, 80).Value);
			}
			
			tabName += ChessAPI::GetPgnGameByIndex(opened[n])["White"] + " - " + ChessAPI::GetPgnGameByIndex(opened[n])["Black"];
			
			//rewrite
			if (ImGui::BeginTabItem(tabName.c_str(),
				crossAddress, flagActive | flagUnsaved))
			{
				activeTab = true;
				ImGui::EndTabItem();
			}

			if (opened[n] < 0)
			{
				ImGui::PopStyleColor(5);
			}
			
			ImGui::PopID();

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				ChessAPI::OpenChessGameInFile(opened[n]);
			}

			if (!cross && tabRemove == -1)
			{
				tabRemove = n;
			}
		}
		
		ImGui::EndTabBar();
	}

	m_size = ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - 4 * ImGui::GetStyle().ItemSpacing.y;

	m_startCursor.y = ImGui::GetCursorPosY();
	
	m_startCursor.x = ImGui::GetWindowContentRegionWidth() / 2 - m_size / 2;
	
	RenderPlayerColorBox();
	RenderBoard();

	if (Panels::GetEnginePanel().IsBarOpen())
		RenderBar();

	if(ShowTags)
		RenderTags();
	
	RenderPieces();

	if (m_CapturedPieceIndex > 0 && ShowPossibleMoves)
		RenderCirclesAtPossibleMoves();

	if(ShowArrows)
		RenderArrows();

	MousePos = FindMousePos();

	ImVec2 bsize = { m_size / 10, m_size / 10 };

	//Piece Moving and Playing
	if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && !ImGui::IsKeyDown(ImGuiKey_RightCtrl))
		{
			if (MousePos.x > -1 && MousePos.y > -1
				&& MousePos.x < 8 && MousePos.y < 8)
			{
				m_CapturedPieceIndex = m_block[MousePos.x][MousePos.y];
				m_oldNumX = MousePos.x;
				m_oldNumY = MousePos.y;
				m_block[MousePos.x][MousePos.y] = 0;
			}
		}
		else if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_CapturedPieceIndex)
		{
			ImGui::SetCursorPos(ImVec2(ImGui::GetMousePos().x - ImGui::GetWindowPos().x - bsize.x / 2, ImGui::GetMousePos().y - ImGui::GetWindowPos().y - bsize.y / 2));
			ImGui::Image((ImTextureID)m_pieces[m_CapturedPieceIndex - 1]->GetRendererID(), bsize);
		}
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && m_CapturedPieceIndex)
		{
			if (MousePos.x > -1 && MousePos.y > -1
				&& MousePos.x < 8 && MousePos.y < 8)
			{
				m_block[MousePos.x][MousePos.y] = m_CapturedPieceIndex;
				m_CapturedPieceIndex = 0;

				//checking for new variation
				Chess::PgnGame::ChessMovesPath PgnMoves;
				Chess::PgnGame::ChessMovesPath* PtrPgnMoves;
				int childAmount = 0;
				auto moveKey = ChessAPI::GetActiveGame().GetLastMoveKey();

				if(AskNewVariation)
				{
					PgnMoves = ChessAPI::GetPgnGame().GetMovePathbyRef();
					PtrPgnMoves = &PgnMoves;

					for (int i = 1; i < moveKey.size(); i += 2)
						PtrPgnMoves = &PtrPgnMoves->children[moveKey[i]];

					childAmount = PtrPgnMoves->children.size();
				}

				auto moveToPlay = Chess::Board::Move(m_oldNumX + 8 * m_oldNumY, MousePos.x - m_oldNumX + 8 * (MousePos.y - m_oldNumY));

				auto moveStatus = ChessAPI::GetActiveGame().MakeMove(moveToPlay);

				if (moveStatus == Chess::Board::PROMOTION)
					m_PromoteMove = moveToPlay;

				if (AskNewVariation)
				{
					PgnMoves = ChessAPI::GetPgnGame().GetMovePathbyRef();
					PtrPgnMoves = &PgnMoves;

					for (int i = 1; i < moveKey.size(); i += 2)
						PtrPgnMoves = &PtrPgnMoves->children[moveKey[i]];

					if (childAmount != PtrPgnMoves->children.size())
					{
						ImGui::OpenPopup("New_Variant");
						ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
					}
				}

				UpdateBoardValues();
			}
			else
			{
				m_block[m_oldNumX][m_oldNumY] = m_CapturedPieceIndex;
				m_CapturedPieceIndex = 0;
			}
		}
		else
			UpdateBoardValues();
	}

	//check if there are multiple next moves
	if (m_NextMove)
	{
		auto movePath = ChessAPI::GetActiveGame().GetLastMoveKey();

		Chess::PgnGame::ChessMovesPath curMovesRef;
		curMovesRef = ChessAPI::GetPgnGame().GetMovePathbyRef();
		Chess::PgnGame::ChessMovesPath* curMoves = &curMovesRef;

		for (int i = 1; i < movePath.size(); i++)
			if (i % 2 == 1)
				curMoves = &curMoves->children[movePath[i]];

		int index = -1;
		for (int i = movePath[movePath.size() - 1] + 1; i < curMoves->move.size(); i++)
			if (curMoves->move[i] != "child")
			{
				index = i;
				break;
			}

		bool openPopup = false;
		if (index + 1)
		{
			for (int i = index + 1; i < curMoves->move.size(); i++)
			{
				if (curMoves->move[i] != "child")
				{
					if (i - index > 1)
						openPopup = true;
					break;
				}
				if (i + 1 == curMoves->move.size())
					openPopup = true;

			}
		}

		if (openPopup)
		{
			ImGui::OpenPopup("Move_Choose");
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

			m_PossibleNextMoves.clear();

			int amountOfPreviousChildren = 0;
			for (int i = 0; i < index; i++)
				if (curMoves->move[i] == "child")
					amountOfPreviousChildren += 1;

			int indexChild = 0;
			for (int i = index + 1; i < curMoves->move.size(); i++)
			{
				if (curMoves->move[i] == "child")
				{
					auto curPath = movePath;
					curPath[curPath.size() - 1] = i;
					curPath.push_back(amountOfPreviousChildren + indexChild);
					curPath.push_back(0);

					m_PossibleNextMoves[curMoves->children[amountOfPreviousChildren + indexChild].move[0]] = curPath;

					indexChild += 1;
				}
				else
					break;
			}

			m_MainMove = curMoves->move[index];
		}
		else
			ChessAPI::GetActiveGame().GoNextMove();

		m_NextMove = false;
	}

	//check if a pawn is ready to be promoted
	if (m_PromoteMove.move != 0 && !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
		ImGui::OpenPopup("New_Piece");

	m_Center = ImGui::GetWindowPos();
	m_Center.x += ImGui::GetContentRegionMax().x;
	m_Center.y += ImGui::GetContentRegionMax().y * 0.25;

	if (m_OpenEditor)
	{
		m_OpenEditor = false;
		OpenEditor();
	}

	if (m_ToOpenEditor)
	{
		m_ToOpenEditor = false;
		ImGui::OpenPopup("Editor");
	}

	auto mif = (std::vector<int>)ChessAPI::GetActiveGame().GetLastMoveKey();
	auto& cmds = ChessAPI::GetActiveGame().GetNote(mif).cmds;

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			s_tags[i][j] = 0;
		}
	}

	s_arrows.clear();

	if (&cmds && cmds.contains("csl"))
	{
		static const std::string hor = "abcdefgh";
		static const std::string ver = "12345678";
		static const std::string type = " RGY";

		std::string clsCmd = cmds["csl"];

		for (int i = 0; i < clsCmd.size(); i++)
		{
			if (clsCmd[i] == ',')
				continue;

			char t = clsCmd[i];
			char h = clsCmd[++i];
			char v = clsCmd[++i];

			if (!m_reverse)
				s_tags[7 - ver.find(v)][hor.find(h)] = type.find(t);
			else
				s_tags[ver.find(v)][7 - hor.find(h)] = type.find(t);
		}
	}

	if (&cmds && cmds.contains("cal"))
	{
		static const std::string hor = "abcdefgh";
		static const std::string ver = "12345678";
		static const std::string type = " RGY";

		std::string calCmd = cmds["cal"];

		for (int i = 0; i < calCmd.size(); i++)
		{
			if (calCmd[i] == ',')
				continue;

			char t = calCmd[i];
			char hs = calCmd[++i];
			char vs = calCmd[++i];
			char he = calCmd[++i];
			char ve = calCmd[++i];

			if (!m_reverse)
			{
				ArrowsData adata;
				adata.type = type.find(t);
				adata.end = ImVec2(7 - ver.find(ve), hor.find(he));
				adata.start = ImVec2(7 - ver.find(vs), hor.find(hs));
				s_arrows.emplace_back(adata);
			}
			else
			{
				ArrowsData adata;
				adata.type = type.find(t);
				adata.end = ImVec2(ver.find(ve), 7 - hor.find(he));
				adata.start = ImVec2(ver.find(vs), 7 - hor.find(hs));
				s_arrows.emplace_back(adata);
			}
		}

	}

	if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
	{
		if (!ImGui::IsAnyMouseDown())
		{
			if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
			{
				m_NextMove = true;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
			{
				ChessAPI::GetActiveGame().GoPreviusMove();
			}
		}

		{
			static const std::string hor = "abcdefgh";
			static const std::string ver = "12345678";

			bool redKey = ImGui::IsKeyDown(ImGuiKey_R);
			bool greenKey = ImGui::IsKeyDown(ImGuiKey_G);
			bool blueKey = ImGui::IsKeyDown(ImGuiKey_B);
			bool magicKey = redKey + greenKey + blueKey;

			bool rMouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Right) || (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)));
			bool rMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Right) || (ImGui::IsMouseDown(ImGuiMouseButton_Left) && (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)));
			bool rMouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Right) || (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)));

			bool doArrow = false;
			bool doTag = false;

			static bool start = false;

			if (rMouseClicked && magicKey
				&& MousePos.x > -1 && MousePos.y > -1
				&& MousePos.x < 8 && MousePos.y < 8)
			{
				start = true;
				s_startPressedPos = MousePos;
			}

			if (rMouseReleased && start)
			{
				if (MousePos.x > -1 && MousePos.y > -1
					&& MousePos.x < 8 && MousePos.y < 8
					&& magicKey)
				{
					if (MousePos.x == s_startPressedPos.x
						&& MousePos.y == s_startPressedPos.y)
					{
						doTag = true;
					}
					else
						doArrow = true;
				}
				else
				{
					start = false;
					s_startPressedPos = ImVec2(-1, -1);
				}
			}

			if (doTag)
			{
				auto& CslCmd = cmds["csl"];

				if (redKey)
				{
					std::string vh = "";

					if (!m_reverse)
					{
						vh += hor[(int)MousePos.x];
						vh += ver[(int)MousePos.y];
					}
					else
					{
						vh += hor[(int)MousePos.x];
						vh += ver[(int)MousePos.y];
					}

					int indexVH = CslCmd.find(vh);
					if (indexVH != std::string::npos)
					{
						if (CslCmd[indexVH - 1] == 'R')
						{
							bool first = false;
							if (indexVH == 1)
								first = true;

							if (!first)
								CslCmd.erase(indexVH - 2, 4);
							else
							{
								if (CslCmd.size() == 1 + 2)
									CslCmd.erase(indexVH - 1, 3);
								else
									CslCmd.erase(indexVH - 1, 4);
							}
						}
						else
							CslCmd[indexVH - 1] = 'R';
					}
					else
					{
						if (CslCmd.empty())
							CslCmd = ('R' + vh);
						else
							CslCmd += (",R" + vh);
					}
				}
				if (greenKey)
				{
					std::string vh = "";

					if (!m_reverse)
					{
						vh += hor[(int)MousePos.x];
						vh += ver[(int)MousePos.y];
					}
					else
					{
						vh += hor[(int)MousePos.x];
						vh += ver[(int)MousePos.y];
					}

					int indexVH = CslCmd.find(vh);
					if (indexVH != std::string::npos)
					{
						if (CslCmd[indexVH - 1] == 'G')
						{
							bool first = false;
							if (indexVH == 1)
								first = true;

							if (!first)
								CslCmd.erase(indexVH - 2, 4);
							else
							{
								if (CslCmd.size() == 1 + 2)
									CslCmd.erase(indexVH - 1, 3);
								else
									CslCmd.erase(indexVH - 1, 4);
							}
						}
						else
							CslCmd[indexVH - 1] = 'G';
					}
					else
					{
						if (CslCmd.empty())
							CslCmd = ('G' + vh);
						else
							CslCmd += (",G" + vh);
					}
				}
				if (blueKey)
				{
					std::string vh = "";

					if (!m_reverse)
					{
						vh += hor[(int)MousePos.x];
						vh += ver[(int)MousePos.y];
					}
					else
					{
						vh += hor[(int)MousePos.x];
						vh += ver[(int)MousePos.y];
					}

					int indexVH = CslCmd.find(vh);
					if (indexVH != std::string::npos)
					{
						if (CslCmd[indexVH - 1] == 'Y')
						{
							bool first = false;
							if (indexVH == 1)
								first = true;

							if (!first)
								CslCmd.erase(indexVH - 2, 4);
							else
							{
								if (CslCmd.size() == 1 + 2)
									CslCmd.erase(indexVH - 1, 3);
								else
									CslCmd.erase(indexVH - 1, 4);
							}
						}
						else
							CslCmd[indexVH - 1] = 'Y';
					}
					else
					{
						if (CslCmd.empty())
							CslCmd = ('Y' + vh);
						else
							CslCmd += (",Y" + vh);
					}
				}
			}

			if (doArrow)
			{
				auto& CalCmd = cmds["cal"];

				if (redKey)
				{
					std::string vh = "";

					if (!m_reverse)
					{
						vh += hor[(int)s_startPressedPos.x];
						vh += ver[(int)s_startPressedPos.y];
						vh += hor[(int)MousePos.x];
						vh += ver[(int)MousePos.y];
					}
					else
					{
						vh += hor[(int)s_startPressedPos.x];
						vh += ver[(int)s_startPressedPos.y];
						vh += hor[(int)MousePos.x];
						vh += ver[(int)MousePos.y];
					}

					int indexVH = CalCmd.find(vh);
					if (indexVH != std::string::npos)
					{
						if (CalCmd[indexVH - 1] == 'R')
						{
							bool first = false;
							if (indexVH == 1)
								first = true;

							if (!first)
								CalCmd.erase(indexVH - 2, 6);
							else
							{
								if (CalCmd.size() == 1 + 2 + 2)
									CalCmd.erase(indexVH - 1, 1 + 2 + 2);
								else
									CalCmd.erase(indexVH - 1, 1 + 2 + 2 + 1);
							}
						}
						else
							CalCmd[indexVH - 1] = 'R';
					}
					else
					{
						if (CalCmd.empty())
							CalCmd = ('R' + vh);
						else
							CalCmd += (",R" + vh);
					}
				}
				else if (greenKey)
				{
					std::string vh = "";

					if (!m_reverse)
					{
						vh += hor[(int)s_startPressedPos.x];
						vh += ver[(int)s_startPressedPos.y];
						vh += hor[(int)MousePos.x];
						vh += ver[(int)MousePos.y];
					}
					else
					{
						vh += hor[(int)s_startPressedPos.x];
						vh += ver[(int)s_startPressedPos.y];
						vh += hor[(int)MousePos.x];
						vh += ver[(int)MousePos.y];
					}

					int indexVH = CalCmd.find(vh);
					if (indexVH != std::string::npos)
					{
						if (CalCmd[indexVH - 1] == 'G')
						{
							bool first = false;
							if (indexVH == 1)
								first = true;

							if (!first)
								CalCmd.erase(indexVH - 2, 6);
							else
							{
								if (CalCmd.size() == 1 + 2 + 2)
									CalCmd.erase(indexVH - 1, 1 + 2 + 2);
								else
									CalCmd.erase(indexVH - 1, 1 + 2 + 2 + 1);
							}
						}
						else
							CalCmd[indexVH - 1] = 'G';
					}
					else
					{
						if (CalCmd.empty())
							CalCmd = ('G' + vh);
						else
							CalCmd += (",G" + vh);
					}
				}
				else if (blueKey)
				{
					std::string vh = "";

					if (!m_reverse)
					{
						vh += hor[(int)s_startPressedPos.x];
						vh += ver[(int)s_startPressedPos.y];
						vh += hor[(int)MousePos.x];
						vh += ver[(int)MousePos.y];
					}
					else
					{
						vh += hor[(int)s_startPressedPos.x];
						vh += ver[(int)s_startPressedPos.y];
						vh += hor[(int)MousePos.x];
						vh += ver[(int)MousePos.y];
					}

					int indexVH = CalCmd.find(vh);
					if (indexVH != std::string::npos)
					{
						if (CalCmd[indexVH - 1] == 'Y')
						{
							bool first = false;
							if (indexVH == 1)
								first = true;

							if (!first)
								CalCmd.erase(indexVH - 2, 6);
							else
							{
								if (CalCmd.size() == 1 + 2 + 2)
									CalCmd.erase(indexVH - 1, 1 + 2 + 2);
								else
									CalCmd.erase(indexVH - 1, 1 + 2 + 2 + 1);
							}
						}
						else
							CalCmd[indexVH - 1] = 'Y';
					}
					else
					{
						if (CalCmd.empty())
							CalCmd = ('Y' + vh);
						else
							CalCmd += (",Y" + vh);
					}
				}
			}
		}
	}

	if (tabRemove != -1)
	{
		if (ChessAPI::GetActiveGameIndex() == opened[tabRemove])
			ChessAPI::OpenChessGameInFile(opened[(0 == tabRemove ? 1 : 0)]);

		ChessAPI::CloseOpenGame(opened[tabRemove]);
		tabRemove = -1;
	}

	m_IsMoveChooseOpen = ImGui::IsPopupOpen("Move_Choose");

	NextMovePopup();
	NewVariantPopup();
	NewPiecePopup();
	EditorPopup();

	ImGui::End();
}

void BoardPanel::RenderPlayerColorBox()
{
	ImVec2 bsize = { m_size / 10, m_size / 10 };
	float blockSize = m_size / 9;

	float sizef = 1.43f;
	float xposition = blockSize;
	float yposition = blockSize;
	xposition = 8.2f * blockSize;
	yposition = 8.2f * blockSize;

	ImGui::SetCursorPos(ImVec2(xposition - bsize.x / 2 + m_startCursor.x, yposition - bsize.y / 2 + m_startCursor.y));

	if (ChessAPI::GetActiveGame().GetPlayerToPlay())
		ImGui::Image((ImTextureID)m_WhiteBox->GetRendererID(), { bsize.x * sizef, bsize.y * sizef });
	else
		ImGui::Image((ImTextureID)m_BlackBox->GetRendererID(), { bsize.x * sizef, bsize.y * sizef });
}

void BoardPanel::RenderBoard()
{
	auto& board = m_board[m_reverse];

	ImGui::SetCursorPos(m_startCursor);
	ImGui::Image((ImTextureID)board->GetRendererID(), { m_size, m_size });
}

void BoardPanel::RenderPieces()
{
	ImVec2 bsize = { m_size / 10.0f, m_size / 10.0f };

	float BoxSize = 100.5f / 900.0f * m_size;
	float startBoxPos = 48.0f / 900.0f * m_size + 0.5f * BoxSize;
	
	float xposition = startBoxPos;
	float yposition = startBoxPos;

	if (!m_reverse)
	{
		for (int j = 7; j > -1; --j)
		{
			for (int i = 0; i < 8; ++i)
			{
				if (m_block[i][j])
				{
					ImGui::SetCursorPos(ImVec2(xposition - bsize.x / 2.0f + m_startCursor.x, yposition - bsize.y / 2.0f + m_startCursor.y));
					ImGui::Image((ImTextureID)m_pieces[m_block[i][j] - 1]->GetRendererID(), bsize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, Panels::GetEnginePanel().ShowEngineBoard() ? 0.5f : 1.0f));
				}

				if (i == m_oldNumX && j == m_oldNumY && m_CapturedPieceIndex)
				{
					ImGui::SetCursorPos(
						ImVec2(xposition - bsize.x / 2.0f + m_startCursor.x + bsize.x / 2.5f / 2.0f + bsize.x / 2.0f - bsize.x / 2.5f
							, yposition - bsize.y / 2.0f + m_startCursor.y + bsize.y / 2.5f / 2.0f + bsize.y / 2.0f - bsize.y / 2.5f));
					ImGui::Image((ImTextureID)m_circleFromStart->GetRendererID(), { bsize.x / 2.5f, bsize.y / 2.5f });
				}

				xposition += BoxSize;
			}
			xposition = startBoxPos;
			yposition += BoxSize;
		}
	}
	else
	{
		for (int j = 0; j < 8; ++j)
		{
			for (int i = 7; i > -1; --i)
			{
				if (m_block[i][j])
				{
					ImGui::SetCursorPos(ImVec2(xposition - bsize.x / 2 + m_startCursor.x, yposition - bsize.y / 2 + m_startCursor.y));
					ImGui::Image((ImTextureID)m_pieces[m_block[i][j] - 1]->GetRendererID(), bsize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, Panels::GetEnginePanel().ShowEngineBoard() ? 0.5f : 1.0f));
				}

				if (i == m_oldNumX && j == m_oldNumY && m_CapturedPieceIndex)
				{
					ImGui::SetCursorPos(
						ImVec2(xposition - bsize.x / 2 + m_startCursor.x + bsize.x / 2.5f / 2.0f + bsize.x / 2.0f - bsize.x / 2.5f
							, yposition - bsize.y / 2 + m_startCursor.y + bsize.y / 2.5f / 2.0f + bsize.y / 2.0f - bsize.y / 2.5f));
					ImGui::Image((ImTextureID)m_circleFromStart->GetRendererID(), { bsize.x / 2.5f, bsize.y / 2.5f });
				}

				xposition += BoxSize;
			}
			xposition = startBoxPos;
			yposition += BoxSize;
		}
	}
}

void BoardPanel::RenderTags()
{
	ImVec2 bsize = { m_size / 10, m_size / 10 };

	float BoxSize = 100.5f / 900.0f * m_size;
	float startBoxPos = 48.0f / 900.0f * m_size + 0.5f * BoxSize;

	float xposition = startBoxPos;
	float yposition = startBoxPos;
	
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (s_tags[i][j])
			{
				auto tag = m_RedTag;
				if (s_tags[i][j] == 2)
					tag = m_GreenTag;
				else if (s_tags[i][j] == 3)
					tag = m_BlueTag;
				ImGui::SetCursorPos(ImVec2(xposition - bsize.x / 2 + m_startCursor.x, yposition - bsize.y / 2 + m_startCursor.y));
				ImGui::Image((ImTextureID)tag->GetRendererID(), bsize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.9, 0.9, 0.9, 0.6));
			}

			xposition += BoxSize;
		}
		xposition = startBoxPos;
		yposition += BoxSize;
	}
}

void BoardPanel::RenderArrows()
{
	ImVec2 bsize = { m_size / 10, m_size / 10 };
	float BoxSize = 100.5f / 900.0f * m_size;

	for(int i = 0; i < s_arrows.size(); i++)
	{
		auto& arrowD = s_arrows[i];

		auto arrow = m_RedArrow;
		auto colorLine = IM_COL32(161, 7, 13, 170);
		if (arrowD.type == 2)
		{
			arrow = m_GreenArrow;
			colorLine = IM_COL32(30, 172, 8, 170);
		}
		else if (arrowD.type == 3 || arrowD.type == 4)
		{
			arrow = m_BlueArrow;
			colorLine = IM_COL32(0, 172, 234, 170);
		}

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		
		ImVec2 startPosCenter = ImVec2(arrowD.start.y * BoxSize + BoxSize + m_startCursor.x, arrowD.start.x * BoxSize + BoxSize + m_startCursor.y);
		ImVec2 endPosCenter = ImVec2(arrowD.end.y * BoxSize + BoxSize + m_startCursor.x, arrowD.end.x * BoxSize + BoxSize + m_startCursor.y);

		float xCor = endPosCenter.x - startPosCenter.x;
		float yCor = endPosCenter.y - startPosCenter.y;

		ImGui::SetCursorPos(ImVec2(arrowD.end.y * BoxSize + BoxSize - bsize.x / 2 + m_startCursor.x, arrowD.end.x * BoxSize + BoxSize - bsize.y / 2 + m_startCursor.y));
		ImVec2 arrowPos = window->DC.CursorPos;

		RenderRotatedImage((ImTextureID)arrow->GetRendererID(), arrowPos, bsize, -yCor / std::sqrt(std::pow(xCor, 2) + std::pow(yCor, 2)), xCor / std::sqrt(std::pow(xCor, 2) + std::pow(yCor, 2)), IM_COL32(255, 255, 255, 210));
		
		ImGui::SetCursorPos(startPosCenter);
		ImVec2 startPosWindow = window->DC.CursorPos;
		//startPosWindow.x -= (bsize.x / 10);

		ImGui::SetCursorPos(endPosCenter);
		ImVec2 endPosWindow = window->DC.CursorPos;

		float len = std::sqrt(std::pow(endPosWindow.x - startPosWindow.x, 2) + std::pow(endPosWindow.y - startPosWindow.y, 2));
		endPosWindow.y -= (BoxSize / 3 * (endPosWindow.y - startPosWindow.y) / len);
		endPosWindow.x -= (BoxSize / 3 * (endPosWindow.x - startPosWindow.x) / len);
		startPosWindow.y += (BoxSize / 6 * (endPosWindow.y - startPosWindow.y) / len);
		startPosWindow.x += (BoxSize / 6 * (endPosWindow.x - startPosWindow.x) / len);

		draw_list->AddLine(startPosWindow, endPosWindow, colorLine, bsize.x/7);
	}
}

void BoardPanel::RenderCirclesAtPossibleMoves()
{
	ImVec2 bsize = { m_size / 10, m_size / 10 };
	float BoxSize = 100.5f / 900.0f * m_size;
	float startBoxPos = 48.0f / 900.0f * m_size + 0.5f * BoxSize;

	float xposition = startBoxPos;
	float yposition = startBoxPos;

	std::vector<Chess::Board::Move> possibleMoves;
	ChessAPI::GetActiveGame().GetAvailableMoves(possibleMoves);

	for (int i = 0; i < possibleMoves.size(); i++)
	{
		if (m_oldNumX + m_oldNumY * 8 != possibleMoves[i].index)
		{
			possibleMoves.erase(possibleMoves.begin() + i);
			i--;
		}
	}

	for (auto& move : possibleMoves)
	{
		if (!m_reverse)
		{
			xposition = ((move.move + move.index) % 8) * BoxSize + startBoxPos;
			yposition = (7 - (move.move + move.index) / 8) * BoxSize + startBoxPos;
		}
		else
		{
			xposition = (7 - (move.move + move.index) % 8) * BoxSize + startBoxPos;
			yposition = ((move.move + move.index) / 8) * BoxSize+ startBoxPos;
		}

		ImGui::SetCursorPos(ImVec2(xposition - bsize.x / 2 + m_startCursor.x, yposition - bsize.y / 2 + m_startCursor.y));
		ImGui::Image((ImTextureID)m_circleToEnd->GetRendererID(), bsize);
	}
}

void BoardPanel::RenderBar()
{
	float barSize = m_size / 1.3f;
	float boxSize = barSize / 10.0f;

	float zero = barSize / 2.0f - barSize * 0.076f;

	float plusOne = zero / 5.0f;

	static float barValue = zero;

	barValue += ((zero + std::max(-5.0f, std::min(Panels::GetEnginePanel().GetBarValue(), 5.0f)) * plusOne - barValue) / ImGui::GetIO().Framerate);

	ImVec2 bottomRight = ImVec2(ImGui::GetWindowPos().x + m_startCursor.x - m_size / (14.0f * 2.0f),
		ImGui::GetWindowPos().y + m_startCursor.y + barSize + barSize * 0.076f);

	ImVec2 topLeft = ImVec2(bottomRight.x - boxSize, bottomRight.y - barValue);

	//white bar
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImU32 color = IM_COL32(185, 185, 185, 255);
		draw_list->AddRectFilled(topLeft, bottomRight, color);
	}

	ImGui::SetCursorPos(
		ImVec2(m_startCursor.x - barSize / 2.0f - m_size / 14.0f,
			m_startCursor.y + (m_size - barSize) / 2.0f));

	ImGui::Image((ImTextureID)m_bar->GetRendererID(), { barSize, barSize });
}

ImVec2 BoardPanel::FindMousePos()
{
	float blockSize = m_size / 9;

	static int numY = 0;
	static int numX = 0;

	numX = (ImGui::GetMousePos().x - m_startCursor.x - ImGui::GetWindowPos().x - blockSize / 2) / blockSize;
	if (ImGui::GetMousePos().x - m_startCursor.x - ImGui::GetWindowPos().x - blockSize / 2 < 0)
		numX = -1;
	numY = (ImGui::GetMousePos().y - m_startCursor.y - ImGui::GetWindowPos().y - blockSize / 2) / blockSize;
	if (ImGui::GetMousePos().y - m_startCursor.y - ImGui::GetWindowPos().y - blockSize / 2 < 0)
		numY = -1;

	if (!m_reverse)
		return ImVec2(numX, 7 - numY);

	return ImVec2(7 - numX, numY);
}

void BoardPanel::UpdateBoardValues()
{
	if (Panels::GetEnginePanel().ShowEngineBoard())
	{
		m_block = Panels::GetEnginePanel().GetEngineBlocks();
		return;
	}

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
			m_block[i][j] = ChessAPI::GetBlockID(i + 8 * j);
	}
}

void BoardPanel::NextMovePopup()
{
	ImGui::SetNextWindowPos(m_Center, ImGuiCond_Appearing, ImVec2(1, 1));
	if (ImGui::BeginPopupModal("Move_Choose", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::PushFont(Walnut::Application::GetFont("Bold"));
		if (ImGui::Selectable(m_MainMove.c_str()) ||
			(ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_RightArrow)))
		{
			ChessAPI::GetActiveGame().GoNextMove();
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopFont();

		for (auto it = m_PossibleNextMoves.begin(); it != m_PossibleNextMoves.end(); it++)
		{
			if (ImGui::Selectable(it->first.c_str()) ||
				(ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_RightArrow)))
			{
				ChessAPI::GetActiveGame().GoToPositionByKey(it->second);
				ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
				ImGui::CloseCurrentPopup();
			}
			
			ImGui::SameLine();

			{
				std::string childID;
				if (it->second.size() == 3)
					childID = (char)('A' + it->second[it->second.size() - 2]);
				else
					childID = std::to_string(it->second[it->second.size() - 2] + 1);

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.48f, 0.87f, 0.65f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.48f, 0.87f, 0.45f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.48f, 0.87f, 0.45f));

				ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);

				ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - ImGui::GetStyle().FramePadding.x * 2.0f - ImGui::CalcTextSize(childID.c_str()).x);
				ImGui::SmallButton(childID.c_str());

				ImGui::PopItemFlag();

				ImGui::PopStyleColor(3);
			}

		}

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));

		if (ImGui::Button("Play Main"))
		{
			ChessAPI::GetActiveGame().GoNextMove();
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::CloseCurrentPopup();
		}

		ImGui::PopStyleColor(3);

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

		if (ImGui::Button("Cancel"))
		{
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::CloseCurrentPopup();
		}

		ImGui::PopStyleColor(3);

		if (!ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
		{
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void BoardPanel::NewVariantPopup()
{
	ImGui::SetNextWindowPos(m_Center, ImGuiCond_Appearing, ImVec2(1, 1));
	if (ImGui::BeginPopupModal("New_Variant", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::Selectable("New Variation");

		if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_RightArrow))
		{
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::Selectable("Promote to MainLine")
			|| (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_RightArrow)))
		{
			auto movePathToPromote = ChessAPI::GetActiveGame().GetLastMoveKey();
			ChessAPI::GetActiveGame().GoPreviusMove();
			ChessAPI::GetActiveGame().GoNextMove();
			auto movePathToGo = ChessAPI::GetActiveGame().GetLastMoveKey();

			ChessAPI::GetActiveGame().EditVariation(movePathToPromote, Chess::GameManager::SWAP);
			ChessAPI::GetActiveGame().GoToPositionByKey(movePathToGo);

			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::Selectable("OverWrite")
			|| (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_RightArrow)))
		{
			auto movePath = ChessAPI::GetActiveGame().GetLastMoveKey();
			ChessAPI::GetActiveGame().GoPreviusMove();
			ChessAPI::GetActiveGame().GoNextMove();
			auto movePathToGo = ChessAPI::GetActiveGame().GetLastMoveKey();

			ChessAPI::GetActiveGame().EditVariation(movePath, Chess::GameManager::SWAP);
			movePath.back() = 0;
			ChessAPI::GetActiveGame().DeleteMove(movePath);
			ChessAPI::GetActiveGame().GoToPositionByKey(movePathToGo);

			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::CloseCurrentPopup();
		}

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.7f, 0.1f, 0.65f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.7f, 0.1f, 0.45f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 0.25f));

		if (ImGui::Button("Play Default"))
		{
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::CloseCurrentPopup();
		}

		ImGui::PopStyleColor(3);

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

		if (ImGui::Button("Cancel")
			|| (!ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_LeftArrow)))
		{
			auto movePath = ChessAPI::GetActiveGame().GetLastMoveKey();
			ChessAPI::GetActiveGame().GoPreviusMove();
			auto movePathToGo = ChessAPI::GetActiveGame().GetLastMoveKey();

			movePath.back() = 0;
			ChessAPI::GetActiveGame().DeleteMove(movePath);
			ChessAPI::GetActiveGame().GoToPositionByKey(movePathToGo);

			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::CloseCurrentPopup();
		}

		ImGui::PopStyleColor(3);

		ImGui::EndPopup();
	}
}

void BoardPanel::NewPiecePopup()
{
	if (ImGui::BeginPopup("New_Piece", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
	{
		if (m_PromoteMove.move == 0)
			ImGui::CloseCurrentPopup();

		bool color = ChessAPI::GetActiveGame().GetPlayerToPlay();
		int index = (color ? 1 : 0);

		for (int i = 0; i < 4; i++)
		{
			if (ImGui::ImageButton((uint32_t*)m_pieces[10-i - index * 6]->GetRendererID(), { 100, 100 }))
			{
				ChessAPI::GetActiveGame().MakeMove(m_PromoteMove, Chess::Piece(4 - i));
				m_PromoteMove.move = 0;
				ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
		{
			m_PromoteMove.move = 0;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void BoardPanel::EditorPopup()
{
	auto mainViewport = ImGui::GetMainViewport();
	
	ImGui::SetNextWindowSize(ImVec2(mainViewport->Size.x * 0.6, mainViewport->Size.y * 0.6), ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(ImVec2(mainViewport->GetWorkCenter().x - mainViewport->Size.x * 0.3, mainViewport->GetWorkCenter().y - mainViewport->Size.y * 0.3), ImGuiCond_Appearing);
	
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f / 255.0f, 225.0f / 255.0f, 135.0f / 255.0f, 255.0f / 255.0f));
	bool isOpen = ImGui::BeginPopupModal("Editor", NULL, ImGuiWindowFlags_NoResize);
	ImGui::PopStyleColor();

	if (isOpen)
	{
		ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;

		ImGui::Separator();

		{
			ImGui::BeginChild("Editor Board", ImVec2(ImGui::GetContentRegionAvail().x / 2, 0));

			auto editorSize = ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - 4 * ImGui::GetStyle().ItemSpacing.y - 2 * 60;

			ImVec2 bsize = { editorSize / 10, editorSize / 10 };

			static int pointIndex = -1;

			ImVec2 editorStartCursor;
			editorStartCursor.y = ImGui::GetCursorPosY();
			editorStartCursor.x = ImGui::GetWindowContentRegionWidth() / 2 - editorSize / 2;

			//render board
			ImGui::SetCursorPos(ImVec2(ImGui::GetWindowContentRegionWidth() / 2 - editorSize / 2, editorStartCursor.y));
			ImGui::Image((uint32_t*)m_board[0]->GetRendererID(), { editorSize, editorSize });

			auto cursorEnd = ImGui::GetCursorPos();

			//render Pieces

			float BoxSize = 100.5f / 900.0f * editorSize;
			float startBoxPos = 48.0f / 900.0f * editorSize + 0.5f * BoxSize;

			float xposition = startBoxPos;
			float yposition = startBoxPos;

			for (int j = 7; j > -1; --j)
			{
				for (int i = 0; i < 8; ++i)
				{
					if (m_Editorblock[i][j])
					{
						ImGui::SetCursorPos(ImVec2(xposition - bsize.x / 2 + editorStartCursor.x, yposition - bsize.y / 2 + editorStartCursor.y));
						ImGui::Image((ImTextureID)m_pieces[m_Editorblock[i][j] - 1]->GetRendererID(), bsize);
					}

					xposition += BoxSize;
				}
				xposition = startBoxPos;
				yposition += BoxSize;
			}

			ImGui::SetCursorPos(cursorEnd);
			ImGui::SetCursorPosX(cursorEnd.x + 25 + ImGui::GetStyle().FramePadding.x);

			for (int i = 0; i < 6; i++)
			{
				if (ImGui::ImageButton((uint32_t*)m_pieces[i]->GetRendererID(), { 50, 50 }))
				{
					pointIndex = i;
				}
				ImGui::SameLine();
			}

			ImGui::NewLine();
			ImGui::SetCursorPosX(cursorEnd.x + 25 + ImGui::GetStyle().FramePadding.x);

			for (int i = 6; i < 12; i++)
			{
				if (ImGui::ImageButton((uint32_t*)m_pieces[i]->GetRendererID(), { 50, 50 }))
				{
					pointIndex = i;
				}
				ImGui::SameLine();
			}

			ImGui::SetCursorPosY(cursorEnd.y + 25 + ImGui::GetStyle().FramePadding.y * 2.0f);

			if (ImGui::ImageButton((uint32_t*)m_RedX->GetRendererID(), { 50, 50 }, { 0, 1 }, { 1, 0 }))
			{
				pointIndex = -1;
			}

			cursorEnd = ImGui::GetCursorPos();

			ImGui::SetCursorPos(ImVec2(ImGui::GetMousePos().x - ImGui::GetWindowPos().x - bsize.x / 2, ImGui::GetMousePos().y - ImGui::GetWindowPos().y - bsize.y / 2));
			if (ImGui::GetCursorPos().y > cursorEnd.y - 50)
				ImGui::SetCursorPosY(cursorEnd.y - 50);

			if (pointIndex == -1)
			{
				ImGui::Image((ImTextureID)m_RedX->GetRendererID(), bsize);
			}
			else
			{
				ImGui::Image((ImTextureID)m_pieces[pointIndex]->GetRendererID(), bsize);
			}

			//mousePos
			static int numY = 0;
			static int numX = 0;

			numX = (ImGui::GetMousePos().x - editorStartCursor.x - ImGui::GetWindowPos().x - BoxSize / 2) / BoxSize;
			if (ImGui::GetMousePos().x - editorStartCursor.x - ImGui::GetWindowPos().x - BoxSize / 2 < 0)
				numX = -1;
			numY = (ImGui::GetMousePos().y - editorStartCursor.y - ImGui::GetWindowPos().y - BoxSize / 2) / BoxSize;
			if (ImGui::GetMousePos().y - editorStartCursor.y - ImGui::GetWindowPos().y - BoxSize / 2 < 0)
				numY = -1;

			ImVec2 MousePos = { (float)numX, 7 - (float)numY };

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				if (MousePos.x > -1 && MousePos.y > -1
					&& MousePos.x < 8 && MousePos.y < 8)
				{
					if (pointIndex + 1 == m_Editorblock[MousePos.x][MousePos.y])
						m_Editorblock[MousePos.x][MousePos.y] = 0;
					else
						m_Editorblock[MousePos.x][MousePos.y] = pointIndex + 1;
				}
			}

			ImGui::SetCursorPos(cursorEnd);

			ImGui::EndChild();
		}

		ImGui::SameLine();

		{
			ImGui::BeginChild("Editor Options", ImVec2(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2, 0));

			ImGui::PushFont(Walnut::Application::GetFont("Bold"));
			Walnut::UI::TextCentered("Options");
			ImGui::PopFont();

			ImGui::Separator();

			ImGui::Columns(2);

			ImGui::PushID("W");
			ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.2, 0.2, 0.2, 1));
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.8, 0.8, 0.8, 1));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.8, 0.8, 0.8, 1));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.8, 0.8, 0.8, 1));
			ImGui::Checkbox("0-0-0", &m_wBigRoke);
			ImGui::SameLine();
			ImGui::Checkbox("0-0", &m_wSmallRoke);
			ImGui::PopStyleColor(4);
			ImGui::PopID();

			ImGui::PushID("B");
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25, 0.25, 0.25, 1));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.25, 0.25, 0.25, 1));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.25, 0.25, 0.25, 1));
			ImGui::Checkbox("0-0-0", &m_bBigRoke);
			ImGui::SameLine();
			ImGui::Checkbox("0-0", &m_bSmallRoke);
			ImGui::PopStyleColor(3);
			ImGui::PopID();

			ImGui::NextColumn();

			ImGui::RadioButton("White", &m_EditorPlayerToPlay, 1);
			
			ImGui::SameLine();

			{
				float actualSize = ImGui::CalcTextSize("New Position").x + ImGui::GetStyle().FramePadding.x * 2.0f;
				float avail = ImGui::GetContentRegionAvail().x;

				float off = (avail - actualSize) * 1.0f;
				if (off > 0.0f)
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
			}

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.7, 0.1, 0.65));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1, 0.7, 0.1, 0.50));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1, 0.7, 0.1, 0.35));

			if (ImGui::SmallButton("New Position"))
			{
				Chess::PgnGame gamePgn;
				Chess::GameManager gameNew;
				gameNew.InitPgnGame(gamePgn);

				m_wBigRoke = true;
				m_wSmallRoke = true;
				m_bBigRoke = true;
				m_bSmallRoke = true;

				m_EditorPlayerToPlay = 1;
				m_anPanSanIndex = -1;

				for (int i = 0; i < 8; i++)
				{
					for (int j = 0; j < 8; j++)
					{
						Chess::GameManager::PieceID id = gameNew.GetPieceID(i + 8 * j);

						int ret = ((int)id.type + (id.color == Chess::WHITE ? 0 : 1) * 6 + 1);

						m_Editorblock[i][j] = (id.type != Chess::NONE ? ret : 0);
					}
				}
			}
			ImGui::PopStyleColor(3);

			ImGui::RadioButton("Black", &m_EditorPlayerToPlay, 0);

			ImGui::SameLine();
			
			{
				float actualSize = ImGui::CalcTextSize("Empty Board").x + ImGui::GetStyle().FramePadding.x * 2.0f;
				float avail = ImGui::GetContentRegionAvail().x;

				float off = (avail - actualSize) * 1.0f;
				if (off > 0.0f)
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
			}

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7, 0.1, 0.1, 0.5));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7, 0.1, 0.1, 0.35));
			if (ImGui::SmallButton("Empty Board"))
			{
				for (int i = 0; i < 8; i++)
				{
					for (int j = 0; j < 8; j++)
					{
						m_Editorblock[i][j] = 0;
					}
				}

				m_anPanSanIndex = -1;
			}
			ImGui::PopStyleColor(3);

			ImGui::Columns();

			ImGui::Separator();
			ImGui::NewLine();
			ImGui::Separator();

			auto CheckBoard = [this](std::string& fen)
				{
					std::string values = " PNBRQKpnbrqk";
					int boardValue[64];

					int indexBoard = 0;
					for (int i = 0; i < 8; i++)
					{
						for (int j = 0; j < 8; j++)
						{
							boardValue[indexBoard] = m_Editorblock[j][i];
							indexBoard++;
						}
					}

					fen = "";
					int empty = 0;
					bool once = false;

					//den paizei na exw graphei pio epikinduni function
					for (int i = 56; i > -1; i++)
					{
						if (i % 8 == 0 && once)
						{
							once = false;
							if (empty)
							{
								fen += std::to_string(empty);
								empty = 0;
							}
							if (i != 8)
								fen += '/';
							i -= 17;
							continue;
						}
						once = true;
						if (boardValue[i] == 0)
						{
							empty += 1;
							continue;
						}
						if (empty)
						{
							fen += std::to_string(empty);
							empty = 0;
						}
						fen += values[boardValue[i]];
					}

					fen += ' ';

					if (m_EditorPlayerToPlay == 1)
						fen += 'w';
					else
						fen += 'b';

					fen += ' ';

					if (m_wSmallRoke)
						fen += 'K';
					if (m_wBigRoke)
						fen += 'Q';
					if (m_bSmallRoke)
						fen += 'k';
					if (m_bBigRoke)
						fen += 'q';

					if (fen.back() == ' ')
						fen += '-';

					fen += ' ';

					if (m_anPanSanIndex == -1)
						fen += '-';
					else
					{
						fen += 'a' + m_anPanSanIndex;
						fen += m_EditorPlayerToPlay == 0 ? '3' : '6';
					}

					fen += ' ';
					fen += '0';
					fen += ' ';
					fen += '1';

					Chess::Board edBoard;
					return edBoard.NewPosition(fen);
				};

			static std::string currentFEN;
			CheckBoard(currentFEN);

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.58f, 0.97f, 1.0f));

			ImGui::Text("FEN");

			ImGui::PopStyleColor();

			ImGui::SameLine();

			ImGui::PushFont(Walnut::Application::GetFont("Bold"));
			ImGui::TextWrapped(currentFEN.c_str());
			ImGui::PopFont();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.58f, 0.97f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.58f, 0.97f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.58f, 0.97f, 0.3f));

			float actualSize = ImGui::CalcTextSize(" Copy ").x + ImGui::CalcTextSize(" Paste ").x + ImGui::GetStyle().FramePadding.x * 3.0f;
			float avail = ImGui::GetContentRegionAvail().x;

			float off = (avail - actualSize) * 0.5f;
			if (off > 0.0f)
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

			if (ImGui::SmallButton("Copy"))
			{
				ImGui::SetClipboardText(("CLF" + currentFEN).c_str());
			}

			ImGui::SameLine();

			if (ImGui::SmallButton("Paste"))
			{
				auto clipboardText = ImGui::GetClipboardText();

				if (!clipboardText)
					ImGui::OpenPopup("Error");

				currentFEN = clipboardText;

				if (currentFEN.size() < 4 || currentFEN.substr(0, 3) != "CLF")
				{
					ImGui::OpenPopup("Error");
				}

				std::string newfen = currentFEN.substr(3);
				Chess::Board edBoard;

				if (edBoard.NewPosition(newfen))
					OpenEditor(newfen);
				else
					ImGui::OpenPopup("Error");
			}

			ImGui::PopStyleColor(3);

			ImGui::Separator();

			ImGui::NewLine();

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - ImGui::GetStyle().FramePadding.y * 4.0f - ImGui::CalcTextSize("A").y);

			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.7, 0.1, 0.65));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1, 0.7, 0.1, 0.50));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1, 0.7, 0.1, 0.35));
			if (ImGui::Button("OverWrite"))
			{
				std::string fen;
				if (CheckBoard(fen))
				{
					std::vector<int> startPosition = { -1 };
					ChessAPI::GetActiveGame().GoToPositionByKey(startPosition);

					auto& PgnGame = ChessAPI::GetPgnGame();
					std::string oldDataRead = PgnGame.GetDataRead();
					PgnGame.Clear();
					PgnGame.SetDataRead(oldDataRead);
					PgnGame["FEN"] = fen;
					ChessAPI::GetActiveGame().InitPgnGame(PgnGame);

					ImGui::CloseCurrentPopup();
					ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = false;
				}
				else
				{
					ImGui::OpenPopup("Error");
				}

			}
			ImGui::SameLine();
			if (ImGui::Button("New Game"))
			{
				std::string fen;
				if (CheckBoard(fen))
				{
					ChessAPI::NewGameInFile();
					auto& PgnGame = ChessAPI::GetPgnGame();
					PgnGame.Clear();
					PgnGame["FEN"] = fen;
					ChessAPI::GetActiveGame().InitPgnGame(PgnGame);

					ImGui::CloseCurrentPopup();
					ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = false;
				}
				else
				{
					ImGui::OpenPopup("Error");
				}
			}

			ImGui::PopStyleColor(3);

			ImGui::SetNextWindowSize(ImVec2(mainViewport->Size.x * 0.12, mainViewport->Size.y * 0.15), ImGuiCond_Appearing);
			ImGui::SetNextWindowPos(mainViewport->GetWorkCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.1f, 0.1f, 0.85f));
			bool isOpenError = ImGui::BeginPopupModal("Error", 0, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::PopStyleColor();
			
			if (isOpenError)
			{
				Walnut::UI::TextCentered("       Invalid Board!       ");

				//ImGui::NewLine();

				ImGui::PushID("errorIB");

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7, 0.1, 0.1, 0.5));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7, 0.1, 0.1, 0.35));

				ImGuiStyle& style = ImGui::GetStyle();

				float actualSize = ImGui::CalcTextSize("Close").x + style.FramePadding.x * 2.0f;
				float avail = ImGui::GetContentRegionAvail().x;

				float off = (avail - actualSize) * 0.5f;
				if (off > 0.0f)
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

				if (ImGui::SmallButton("Close"))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::PopStyleColor(3);

				ImGui::PopID();

				ImGui::EndPopup();
			}

			ImGui::SameLine();

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Cancel").x - ImGui::GetStyle().FramePadding.x * 2.0f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7, 0.1, 0.1, 0.5));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7, 0.1, 0.1, 0.35));
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
				ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = false;
			}
			ImGui::PopStyleColor(3);
			
			ImGui::EndChild();
		}

		ImGui::EndPopup();
	}
}

void BoardPanel::FlipBoard()
{
	m_reverse = !m_reverse;
}

void BoardPanel::OpenEditor()
{
	m_ToOpenEditor = true;

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
			m_Editorblock[i][j] = ChessAPI::GetBlockID(i + 8 * j);
	}

	m_EditorPlayerToPlay = ChessAPI::GetActiveGame().GetPlayerToPlay() == Chess::WHITE ? 1 : 0;

	m_wSmallRoke = ChessAPI::GetActiveGame().GetBoard().GetKRoke();
	m_wBigRoke = ChessAPI::GetActiveGame().GetBoard().GetQRoke();
	m_bSmallRoke = ChessAPI::GetActiveGame().GetBoard().GetkRoke();
	m_bBigRoke = ChessAPI::GetActiveGame().GetBoard().GetqRoke();

	m_anPanSanIndex = ChessAPI::GetActiveGame().GetBoard().GetLastMoveIndex() % 8;
}

void BoardPanel::OpenEditor(const std::string& newFEN)
{
	m_ToOpenEditor = true;

	Chess::PgnGame pgngame;
	pgngame["FEN"] = newFEN;

	Chess::GameManager game;
	game.InitPgnGame(pgngame);

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			auto id = game.GetPieceID(i + 8 * j);

			int ret = ((int)id.type + (id.color == Chess::WHITE ? 0 : 1) * 6 + 1);

			m_Editorblock[i][j] = (id.type != Chess::NONE ? ret : 0);
		}
	}

	m_EditorPlayerToPlay = game.GetPlayerToPlay() == Chess::WHITE ? 1 : 0;

	m_wSmallRoke = game.GetBoard().GetKRoke();
	m_wBigRoke = game.GetBoard().GetQRoke();
	m_bSmallRoke = game.GetBoard().GetkRoke();
	m_bBigRoke = game.GetBoard().GetqRoke();

	m_anPanSanIndex = game.GetBoard().GetLastMoveIndex() % 8;
}

void BoardPanel::ShowArrowAt(int xPos, int yPos, int xDir, int yDir)
{
	if (!m_reverse)
		s_arrows.emplace_back(ArrowsData{ 4, ImVec2(7 - yPos, xPos), ImVec2(7 - yDir, xDir) });
	else
		s_arrows.emplace_back(ArrowsData{ 4, ImVec2(yPos, 7 - xPos), ImVec2(yDir, 7 - xDir) });
}

}