#include "ImGuiBoard.h"

#include "ChessAPI.h"
#include "ChessCore/Board.h"
#include "ChessCore/GameManager.h"

#include "AppManagerChild.h"

#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#include <fstream>

#include "../../Walnut/Source/Walnut/Application.h"

extern bool g_AlreadyOpenedModalOpen;
extern float g_ChessEngineValue;
extern bool g_ChessEngineOpen;

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

void ImGuiBoard::OnAttach()
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
}

void ImGuiBoard::OnUIRender()
{
	ImGui::Begin("Game", 0, ImGuiWindowFlags_NoScrollbar);

	int tabRemove = -1;
	auto& opened = ChessAPI::GetOpenGames();

	if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_Reorderable))
	{
		cross = true;

		for (int n = 0; n < opened.size(); n++)
		{
			ImVec4 colorTab = ImGui::GetStyle().Colors[ImGuiCol_TabHovered];
			colorTab.x *= 0.7f;
			colorTab.y *= 0.7f;
			colorTab.z *= 0.7f;

			if (ChessAPI::GetActiveGame() == opened[n])
			{
				colorTab = ImGui::GetStyle().Colors[ImGuiCol_TabActive];
			}
			
			bool activeTab = false;
			
			ImGui::PushStyleColor(ImGuiCol_Tab, colorTab);
			ImGui::PushStyleColor(ImGuiCol_TabHovered, colorTab);
			ImGui::PushStyleColor(ImGuiCol_TabActive, colorTab);

			ImGui::PushID(opened[n]);

			bool* crossAddress = &cross;
			if (opened.size() == 1)
				crossAddress = nullptr;

			//rewrite
			if (ImGui::BeginTabItem((std::to_string(opened[n] + 1) + ": " + (*ChessAPI::GetPgnFile())[opened[n]]["White"] + " - " + (*ChessAPI::GetPgnFile())[opened[n]]["Black"]).c_str(),
				crossAddress, ImGuiTabItemFlags_None))
				activeTab = true;
			
			ImGui::PopID();

			ImGui::PopStyleColor(3);

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				ChessAPI::OpenChessGameInFile(opened[n]);
			}

			if (!cross && tabRemove == -1)
			{
				tabRemove = n;
			}

			if(activeTab)
			{
				ImGui::EndTabItem();
			}
		}
	}
	ImGui::EndTabBar();

	if (tabRemove != -1)
	{
		if (ChessAPI::GetActiveGame() == opened[tabRemove])
			ChessAPI::OpenChessGameInFile(opened[(0 == tabRemove ? 1 : 0)]);
	
		ChessAPI::CloseOpenGame(tabRemove);
		tabRemove = -1;
	}


	auto mif = (std::vector<int>)ChessAPI::GetMoveIntFormat();
	auto& note = ChessAPI::GetNote(mif);

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			s_tags[i][j] = 0;
		}
	}
	
	s_arrows.clear();

	if (note.find("%csl") != std::string::npos)
	{
		static const std::string hor = "abcdefgh";
		static const std::string ver = "12345678";
		static const std::string type = " RGY";
		
		for (int i = note.find("%csl") + 5; i < note.size(); i++)
		{
			if (note[i] == ',')
				continue;
			if (note[i] == ']')
				break;
			char t = note[i];
			char h = note[++i];
			char v = note[++i];

			if(!m_reverse)
				s_tags[7 - ver.find(v)][hor.find(h)] = type.find(t);
			else
				s_tags[ver.find(v)][7 - hor.find(h)] = type.find(t);
		}

	}

	if (note.find("%cal") != std::string::npos)
	{
		static const std::string hor = "abcdefgh";
		static const std::string ver = "12345678";
		static const std::string type = " RGY";
		
		for (int i = note.find("%cal") + 5; i < note.size(); i++)
		{
			if (note[i] == ',')
				continue;
			if (note[i] == ']')
				break;
			char t = note[i];
			char hs = note[++i];
			char vs = note[++i];
			char he = note[++i];
			char ve = note[++i];

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
				ChessAPI::PreviousSavedMove();
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

			auto MousePos = FindMousePos();

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
				if (redKey)
				{
					auto MousePos = FindMousePos();

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

					int endIndex;
					int startIndex = note.find("%csl");
					bool newTagArea = false;

					if (startIndex == std::string::npos)
					{
						newTagArea = true;

						note += "[%csl ]";
						startIndex = note.find("%csl");
					}

					endIndex = note.find(']', startIndex);

					int indexVH = note.find(vh, startIndex);
					if (indexVH != std::string::npos && indexVH < endIndex)
					{
						if (note[indexVH - 1] == 'R')
						{
							bool first = false;
							if (note[indexVH - 2] != ',')
								first = true;

							if (!first)
								note.erase(indexVH - 2, 4);
							else
							{
								bool last = false;
								if (note[indexVH + 2] == ']')
									last = true;

								if (last)
									note.erase(startIndex - 1, startIndex + 2 + 7);
								else
									note.erase(indexVH - 1, 4);
							}
						}
						else
							note[indexVH - 1] = 'R';
					}
					else
					{
						std::string noteToAdd = 'R' + vh;
						if (!newTagArea)
							noteToAdd += ',';
						note.insert(startIndex + 5, noteToAdd);

					}
					
				}
				if (greenKey)
				{
					auto MousePos = FindMousePos();

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

					int endIndex;
					int startIndex = note.find("%csl");
					bool newTagArea = false;

					if (startIndex == std::string::npos)
					{
						newTagArea = true;

						note += "[%csl ]";
						startIndex = note.find("%csl");
					}

					endIndex = note.find(']', startIndex);

					int indexVH = note.find(vh, startIndex);
					if (indexVH != std::string::npos && indexVH < endIndex)
					{
						if (note[indexVH - 1] == 'G')
						{
							bool first = false;
							if (note[indexVH - 2] != ',')
								first = true;

							if (!first)
								note.erase(indexVH - 2, 4);
							else
							{
								bool last = false;
								if (note[indexVH + 2] == ']')
									last = true;

								if (last)
									note.erase(startIndex - 1, startIndex + 2 + 7);
								else
									note.erase(indexVH - 1, 4);
							}
						}
						else
							note[indexVH - 1] = 'G';
					}
					else
					{
						std::string noteToAdd = 'G' + vh;
						if (!newTagArea)
							noteToAdd += ',';
						note.insert(startIndex + 5, noteToAdd);

					}
					
				}
				if (blueKey)
				{
					auto MousePos = FindMousePos();

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

					int endIndex;
					int startIndex = note.find("%csl");
					bool newTagArea = false;

					if (startIndex == std::string::npos)
					{
						newTagArea = true;

						note += "[%csl ]";
						startIndex = note.find("%csl");
					}

					endIndex = note.find(']', startIndex);

					int indexVH = note.find(vh, startIndex);
					if (indexVH != std::string::npos && indexVH < endIndex)
					{
						if (note[indexVH - 1] == 'Y')
						{
							bool first = false;
							if (note[indexVH - 2] != ',')
								first = true;

							if (!first)
								note.erase(indexVH - 2, 4);
							else
							{
								bool last = false;
								if (note[indexVH + 2] == ']')
									last = true;

								if (last)
									note.erase(startIndex - 1, startIndex + 2 + 7);
								else
									note.erase(indexVH - 1, 4);
							}
						}
						else
							note[indexVH - 1] = 'Y';
					}
					else
					{
						std::string noteToAdd = 'Y' + vh;
						if (!newTagArea)
							noteToAdd += ',';
						note.insert(startIndex + 5, noteToAdd);

					}
					
				}
			}

			if (doArrow)
			{
				if (redKey)
				{
					auto MousePos = FindMousePos();

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

					int endIndex;
					int startIndex = note.find("%cal");
					bool newTagArea = false;

					if (startIndex == std::string::npos)
					{
						newTagArea = true;

						note += "[%cal ]";
						startIndex = note.find("%cal");
					}

					endIndex = note.find(']', startIndex);

					int indexVH = note.find(vh, startIndex);
					if (indexVH != std::string::npos && indexVH < endIndex)
					{
						if (note[indexVH - 1] == 'R')
						{
							bool first = false;
							if (note[indexVH - 2] != ',')
								first = true;

							if (!first)
								note.erase(indexVH - 2, 6);
							else
							{
								bool last = false;
								if (note[indexVH + 4] == ']')
									last = true;

								if (last)
									note.erase(startIndex - 1, startIndex + 4 + 7);
								else
									note.erase(indexVH - 1, 6);
							}
						}
						else
							note[indexVH - 1] = 'R';
					}
					else
					{
						std::string noteToAdd = 'R' + vh;
						if (!newTagArea)
							noteToAdd += ',';
						note.insert(startIndex + 5, noteToAdd);

					}
				}
				else if (greenKey)
				{
					auto MousePos = FindMousePos();

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

					int endIndex;
					int startIndex = note.find("%cal");
					bool newTagArea = false;

					if (startIndex == std::string::npos)
					{
						newTagArea = true;

						note += "[%cal ]";
						startIndex = note.find("%cal");
					}

					endIndex = note.find(']', startIndex);

					int indexVH = note.find(vh, startIndex);
					if (indexVH != std::string::npos && indexVH < endIndex)
					{
						if (note[indexVH - 1] == 'G')
						{
							bool first = false;
							if (note[indexVH - 2] != ',')
								first = true;

							if (!first)
								note.erase(indexVH - 2, 6);
							else
							{
								bool last = false;
								if (note[indexVH + 4] == ']')
									last = true;

								if (last)
									note.erase(startIndex - 1, startIndex + 4 + 7);
								else
									note.erase(indexVH - 1, 6);
							}
						}
						else
							note[indexVH - 1] = 'G';
					}
					else
					{
						std::string noteToAdd = 'G' + vh;
						if (!newTagArea)
							noteToAdd += ',';
						note.insert(startIndex + 5, noteToAdd);

					}
				}
				else if (blueKey)
				{
					auto MousePos = FindMousePos();

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

					int endIndex;
					int startIndex = note.find("%cal");
					bool newTagArea = false;

					if (startIndex == std::string::npos)
					{
						newTagArea = true;

						note += "[%cal ]";
						startIndex = note.find("%cal");
					}

					endIndex = note.find(']', startIndex);

					int indexVH = note.find(vh, startIndex);
					if (indexVH != std::string::npos && indexVH < endIndex)
					{
						if (note[indexVH - 1] == 'Y')
						{
							bool first = false;
							if (note[indexVH - 2] != ',')
								first = true;

							if (!first)
								note.erase(indexVH - 2, 6);
							else
							{
								bool last = false;
								if (note[indexVH + 4] == ']')
									last = true;

								if (last)
									note.erase(startIndex - 1, startIndex + 4 + 7);
								else
									note.erase(indexVH - 1, 6);
							}
						}
						else
							note[indexVH - 1] = 'Y';
					}
					else
					{
						std::string noteToAdd = 'Y' + vh;
						if (!newTagArea)
							noteToAdd += ',';
						note.insert(startIndex + 5, noteToAdd);

					}
				}
			}
		}
	}

	m_size = ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - 4 * ImGui::GetStyle().ItemSpacing.y;

	m_startCursor.y = ImGui::GetCursorPosY();
	
	m_startCursor.x = ImGui::GetWindowContentRegionWidth() / 2 - m_size / 2;
	
	RenderPlayerColorBox();
	RenderBoard();

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
		{
			const wchar_t* path = (const wchar_t*)payload->Data;
			std::wstring wstrpath = path;
			std::string strpath = std::string(wstrpath.begin(), wstrpath.end());
			if (std::filesystem::path(strpath).extension().string() == ".pgn")
			{
				bool anwser = AppManagerChild::IsChessFileAvail(strpath);

				if (anwser)
				{
					ChessAPI::OpenChessFile(strpath);
					AppManagerChild::OwnChessFile(ChessAPI::GetPgnFilePath());
				}
				else
				{
					g_AlreadyOpenedModalOpen = true;
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	if (g_ChessEngineOpen)
		RenderBar();

	if(ShowTags)
		RenderTags();
	
	RenderPieces();

	if (m_CapturedPieceIndex > 0 && ShowPossibleMoves)
		RenderCirclesAtPossibleMoves();

	if(ShowArrows)
		RenderArrows();

	auto MousePos = FindMousePos();

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
				auto moveKey = ChessAPI::GetMoveIntFormat();

				if(AskNewVariation)
				{
					ChessAPI::GetMovesPgnFormat(PgnMoves);
					PtrPgnMoves = &PgnMoves;

					for (int i = 1; i < moveKey.size(); i += 2)
						PtrPgnMoves = &PtrPgnMoves->children[moveKey[i]];

					childAmount = PtrPgnMoves->children.size();
				}

				ChessAPI::MakeMove({ m_oldNumX, m_oldNumY }, { MousePos.x , MousePos.y });

				if (AskNewVariation)
				{
					ChessAPI::GetMovesPgnFormat(PgnMoves);
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
		std::vector<int> movePath = ChessAPI::GetMoveIntFormat();

		Chess::PgnGame::ChessMovesPath curMovesRef;
		ChessAPI::GetMovesPgnFormat(curMovesRef);
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
			ChessAPI::NextSavedMove();
		m_NextMove = false;
	}

	//check if a pawn is ready to be promoted
	if (ChessAPI::IsWaitingForNewType() && !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
		ImGui::OpenPopup("New_Piece");

	m_Center = ImGui::GetWindowPos();
	m_Center.x += ImGui::GetContentRegionMax().x;
	m_Center.y += ImGui::GetContentRegionMax().y * 0.25;

	//Open Editor
	if (m_ToOpenEditor)
	{
		m_ToOpenEditor = false;
		ImGui::OpenPopup("Editor");
	}

	NextMovePopup();
	NewVariantPopup();
	NewPiecePopup();
	EditorPopup();

	ImGui::End();

}

void ImGuiBoard::RenderPlayerColorBox()
{
	ImVec2 bsize = { m_size / 10, m_size / 10 };
	float blockSize = m_size / 9;

	float sizef = 1.43f;
	float xposition = blockSize;
	float yposition = blockSize;
	xposition = 8.2f * blockSize;
	yposition = 8.2f * blockSize;

	ImGui::SetCursorPos(ImVec2(xposition - bsize.x / 2 + m_startCursor.x, yposition - bsize.y / 2 + m_startCursor.y));

	if (ChessAPI::GetPlayerColor())
		ImGui::Image((ImTextureID)m_WhiteBox->GetRendererID(), { bsize.x * sizef, bsize.y * sizef });
	else
		ImGui::Image((ImTextureID)m_BlackBox->GetRendererID(), { bsize.x * sizef, bsize.y * sizef });
}

void ImGuiBoard::RenderBoard()
{
	auto& board = m_board[m_reverse];

	ImGui::SetCursorPos(m_startCursor);
	ImGui::Image((ImTextureID)board->GetRendererID(), { m_size, m_size });
}

void ImGuiBoard::RenderPieces()
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
					ImGui::Image((ImTextureID)m_pieces[m_block[i][j] - 1]->GetRendererID(), bsize);
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
					ImGui::Image((ImTextureID)m_pieces[m_block[i][j] - 1]->GetRendererID(), bsize);
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

void ImGuiBoard::RenderTags()
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

void ImGuiBoard::RenderArrows()
{
	ImVec2 bsize = { m_size / 10, m_size / 10 };
	float BoxSize = 100.5f / 900.0f * m_size;

	for(auto& arrowD : s_arrows)
	{
		auto arrow = m_RedArrow;
		auto colorLine = IM_COL32(161, 7, 13, 170);
		if (arrowD.type == 2)
		{
			arrow = m_GreenArrow;
			colorLine = IM_COL32(30, 172, 8, 170);
		}
		else if (arrowD.type == 3)
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

		RenderRotatedImage((ImTextureID)arrow->GetRendererID(), arrowPos, bsize, -yCor / glm::sqrt(glm::pow(xCor, 2) + glm::pow(yCor, 2)), xCor / glm::sqrt(glm::pow(xCor, 2) + glm::pow(yCor, 2)), IM_COL32(255, 255, 255, 210));
		
		ImGui::SetCursorPos(startPosCenter);
		ImVec2 startPosWindow = window->DC.CursorPos;
		//startPosWindow.x -= (bsize.x / 10);

		ImGui::SetCursorPos(endPosCenter);
		ImVec2 endPosWindow = window->DC.CursorPos;

		float len = glm::sqrt(glm::pow(endPosWindow.x - startPosWindow.x, 2) + glm::pow(endPosWindow.y - startPosWindow.y, 2));
		endPosWindow.y -= (BoxSize / 3 * (endPosWindow.y - startPosWindow.y) / len);
		endPosWindow.x -= (BoxSize / 3 * (endPosWindow.x - startPosWindow.x) / len);
		startPosWindow.y += (BoxSize / 6 * (endPosWindow.y - startPosWindow.y) / len);
		startPosWindow.x += (BoxSize / 6 * (endPosWindow.x - startPosWindow.x) / len);

		draw_list->AddLine(startPosWindow, endPosWindow, colorLine, bsize.x/7);
	}
}

void ImGuiBoard::RenderCirclesAtPossibleMoves()
{
	ImVec2 bsize = { m_size / 10, m_size / 10 };
	float BoxSize = 100.5f / 900.0f * m_size;
	float startBoxPos = 48.0f / 900.0f * m_size + 0.5f * BoxSize;

	float xposition = startBoxPos;
	float yposition = startBoxPos;

	std::vector<Chess::Board::Move> possibleMoves;
	ChessAPI::GetPossibleDirections(m_oldNumX + m_oldNumY * 8, possibleMoves);

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

void ImGuiBoard::RenderBar()
{
	float barSize = m_size / 1.3f;
	float boxSize = barSize / 10.0f;

	float zero = barSize / 2.0f - barSize * 0.076f;

	float plusOne = zero / 5.0f;

	static float barValue = zero;

	barValue += ((zero + std::max(-5.0f, std::min(g_ChessEngineValue, 5.0f)) * plusOne - barValue) / ImGui::GetIO().Framerate);

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

ImVec2 ImGuiBoard::FindMousePos()
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

void ImGuiBoard::UpdateBoardValues()
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
			m_block[i][j] = ChessAPI::GetBlockID(i + 8 * j);
	}
}

void ImGuiBoard::NextMovePopup()
{
	ImGui::SetNextWindowPos(m_Center, ImGuiCond_Appearing, ImVec2(1, 1));
	if (ImGui::BeginPopupModal("Move_Choose", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::PushFont(Walnut::Application::GetFont("Bold"));
		if (ImGui::Selectable(m_MainMove.c_str()) ||
			(ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_RightArrow)))
		{
			ChessAPI::NextSavedMove();
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopFont();

		for (auto it = m_PossibleNextMoves.begin(); it != m_PossibleNextMoves.end(); it++)
		{
			if (ImGui::Selectable(it->first.c_str()) ||
				(ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_RightArrow)))
			{
				ChessAPI::GoMoveByIntFormat(it->second);
				ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
				ImGui::CloseCurrentPopup();
			}
			
			ImGui::SameLine();

			{
				std::string childID;
				//int j = 1;

				//if (it->second[1] < 24)
				//{
				//	childID += (char)('A' + it->second[1]);
				//	childID += '.';
				//	j = 3;
				//}

				//for (; j < it->second.size(); j += 2)
				//{
				//	childID += std::to_string(it->second[j] + 1);
				//	childID += '.';
				//}

				//childID.pop_back();
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
			ChessAPI::NextSavedMove();
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::CloseCurrentPopup();
		}

		ImGui::PopStyleColor(3);

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 0.65f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 0.45f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 0.25f));

		if (ImGui::Button("Cansel"))
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

void ImGuiBoard::NewVariantPopup()
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
			auto movePathToPromote = ChessAPI::GetMoveIntFormat();
			ChessAPI::PreviousSavedMove();
			ChessAPI::NextSavedMove();
			auto movePathToGo = ChessAPI::GetMoveIntFormat();

			ChessAPI::PromoteVariation(movePathToPromote);
			ChessAPI::GoMoveByIntFormat(movePathToGo);

			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::Selectable("OverWrite")
			|| (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_RightArrow)))
		{
			auto movePath = ChessAPI::GetMoveIntFormat();
			ChessAPI::PreviousSavedMove();
			ChessAPI::NextSavedMove();
			auto movePathToGo = ChessAPI::GetMoveIntFormat();

			ChessAPI::PromoteVariation(movePath);
			ChessAPI::DeleteVariation(movePath);
			ChessAPI::GoMoveByIntFormat(movePathToGo);

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

		if (ImGui::Button("Cansel")
			|| (!ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_LeftArrow)))
		{
			auto movePath = ChessAPI::GetMoveIntFormat();
			ChessAPI::PreviousSavedMove();
			auto movePathToGo = ChessAPI::GetMoveIntFormat();

			ChessAPI::DeleteVariation(movePath);
			ChessAPI::GoMoveByIntFormat(movePathToGo);

			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::CloseCurrentPopup();
		}

		ImGui::PopStyleColor(3);

		ImGui::EndPopup();
	}
}

void ImGuiBoard::NewPiecePopup()
{
	if (ImGui::BeginPopup("New_Piece", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
	{
		if (!ChessAPI::IsWaitingForNewType())
			ImGui::CloseCurrentPopup();

		bool color = ChessAPI::GetPlayerColor();
		int index = (color ? 1 : 0);

		for (int i = 0; i < 4; i++)
		{
			if (ImGui::ImageButton((uint32_t*)m_pieces[10-i - index * 6]->GetRendererID(), { 100, 100 }))
			{
				ChessAPI::SetNewPieceType(5 - i);
				ImGui::CloseCurrentPopup();
			}
		}

		if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
		{
			ChessAPI::PreviousSavedMove();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void ImGuiBoard::EditorPopup()
{
	ImGui::SetNextWindowPos(m_Center, ImGuiCond_Appearing);
	if (ImGui::BeginPopupModal("Editor"))
	{
		ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		ImVec2 ViewportSize = { viewportPanelSize.x, viewportPanelSize.x };

		auto editorSize = ImGui::GetWindowWidth() - ImGui::GetCursorPosX() - 6 * ImGui::GetStyle().ItemSpacing.x;

		ImVec2 bsize = { editorSize / 10, editorSize / 10 };

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

		static int pointIndex = -1;

		ImGui::SetCursorPos(cursorEnd);

		for (int i = 0; i < 6; i++)
		{
			if (ImGui::ImageButton((uint32_t*)m_pieces[i]->GetRendererID(), {50, 50}))
			{
				pointIndex = i;
			}
			ImGui::SameLine();
		}

		ImGui::NewLine();
		for (int i = 6; i < 12; i++)
		{
			if (ImGui::ImageButton((uint32_t*)m_pieces[i]->GetRendererID(), { 50, 50 }))
			{
				pointIndex = i;
			}
			ImGui::SameLine();
		}
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

		//ImGui::SeparatorText("Settings");
		ImGui::Separator();
		ImGui::Text("Settings:");

		ImGui::Columns(2);

		static bool w_BigRoke = false, b_BigRoke = false, w_SmallRoke = false, b_SmallRoke = false;

		ImGui::PushID("W");
		ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.2, 0.2, 0.2, 1));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.8, 0.8, 0.8, 1));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.8, 0.8, 0.8, 1));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.8, 0.8, 0.8, 1));
		ImGui::Checkbox("0-0-0", &w_BigRoke);
		ImGui::SameLine();
		ImGui::Checkbox("0-0", &w_SmallRoke);
		ImGui::PopStyleColor(4);
		ImGui::PopID();

		ImGui::PushID("B");
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25, 0.25, 0.25, 1));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.25, 0.25, 0.25, 1));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.25, 0.25, 0.25, 1));
		ImGui::Checkbox("0-0-0", &b_BigRoke);
		ImGui::SameLine();
		ImGui::Checkbox("0-0", &b_SmallRoke);
		ImGui::PopStyleColor(3);
		ImGui::PopID();

		ImGui::NextColumn();

		static int player = 1;
		ImGui::RadioButton("White", &player, 1);
		ImGui::SameLine();
		ImGui::RadioButton("Black", &player, 0);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.7, 0.1, 0.65));
		if (ImGui::Button("New"))
		{
			Chess::PgnGame gamePgn;
			Chess::GameManager gameNew;
			gameNew.InitPgnGame(gamePgn);

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
		ImGui::PopStyleColor();

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
		if (ImGui::Button("Clear"))
		{
			for (int i = 0; i < 8; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					m_Editorblock[i][j] = 0;
				}
			}
		}
		ImGui::PopStyleColor();

		ImGui::Columns();

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

				if (player == 1)
					fen += 'w';
				else
					fen += 'b';

				fen += ' ';

				if (w_SmallRoke)
					fen += 'K';
				if (w_BigRoke)
					fen += 'Q';
				if (b_SmallRoke)
					fen += 'k';
				if (b_BigRoke)
					fen += 'q';

				if (fen[fen.size() - 1] == ' ')
					fen += '-';

				fen += ' ';
				fen += '-';
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

		ImGui::TextWrapped(currentFEN.c_str());

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.58f, 0.97f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.58f, 0.97f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.58f, 0.97f, 0.3f));

		if (ImGui::Button("Copy"))
		{
			ImGui::SetClipboardText(currentFEN.c_str());
		}

		ImGui::SameLine();

		if (ImGui::Button("Paste"))
		{
			currentFEN = ImGui::GetClipboardText();
			Chess::Board edBoard;

			if (edBoard.NewPosition(currentFEN))
				OpenEditor(currentFEN);
			else
				ImGui::OpenPopup("Error");
		}

		ImGui::PopStyleColor(3);

		ImGui::Separator();

		ImGui::NewLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.7, 0.1, 0.65));
		if (ImGui::Button("OverWrite"))
		{
			std::string fen;
			if (CheckBoard(fen))
			{
				std::vector<int> startPosition = { -1 };
				ChessAPI::GoMoveByIntFormat(startPosition);

				auto& PgnGame = *ChessAPI::GetPgnGame();
				PgnGame.Clear();
				PgnGame["FEN"] = fen;

				//ChessAPI::OverWriteChessFile("");

				ChessAPI::OpenChessGameInFile(ChessAPI::GetActiveGame());

				ImGui::CloseCurrentPopup();
				ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = false;
			}
			else
			{
				ImGui::OpenPopup("Error");
			}

		}
		ImGui::SameLine();
		if (ImGui::Button("Create As New Game"))
		{
			std::string fen;
			if (CheckBoard(fen))
			{
				ChessAPI::NewGameInFile();
				auto& PgnGame = *ChessAPI::GetPgnGame();
				PgnGame.Clear();
				PgnGame["FEN"] = fen;
				
				//ChessAPI::OverWriteChessFile("");

				ChessAPI::OpenChessGameInFile(ChessAPI::GetActiveGame());

				ImGui::CloseCurrentPopup();
				ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = false;
			}
			else
			{
				ImGui::OpenPopup("Error");
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Create As New File"))
		{
			s_fen.clear();
			if (CheckBoard(s_fen))
			{
				ImGui::OpenPopup("New Chess File");

				ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = false;
			}
			else
			{
				ImGui::OpenPopup("Error");
			}
		}
		ImGui::PopStyleColor();

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Error", 0, ImGuiWindowFlags_NoResize))
		{
			ImGui::TextWrapped("Invalid Board!");

			ImGui::NewLine();

			ImGui::PushID("in");

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize("Close").x - 12);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor();

			ImGui::PopID();

			ImGui::EndPopup();
		}

		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("New Chess File", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
		{
			static std::string s_inputNName = "NewFile.pgn";
			ImGui::InputText("Name", &s_inputNName);

			ImGui::NewLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1, 0.7, 0.1, 0.65));
			if (ImGui::Button("Create"))
			{
				std::filesystem::path nPath = std::filesystem::path() / "chess_working_directory" / s_inputNName;
				nPath.replace_extension(".pgn");

				std::string strNPath = nPath.string();

				bool anwser = AppManagerChild::IsChessFileAvail(strNPath);

				if (anwser)
				{
					Chess::PgnFile NPgnFile;
					NPgnFile.CreateGame();
					NPgnFile[0]["FEN"] = s_fen;
					NPgnFile.SaveFile(strNPath);

					AppManagerChild::OpenChessFileInOtherApp(strNPath);
					ImGui::ClosePopupToLevel(0, true);
				}
				else
				{
					g_AlreadyOpenedModalOpen = true;
					ImGui::ClosePopupToLevel(0, true);
				}

			}
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
			if (ImGui::Button("Cansel"))
				ImGui::CloseCurrentPopup();
			ImGui::PopStyleColor();
			ImGui::EndPopup();
		}

		ImGui::SameLine();

		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize("Cansel").x - 13);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7, 0.1, 0.1, 0.65));
		if (ImGui::Button("Cansel"))
		{
			ImGui::CloseCurrentPopup();
			ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = false;
		}
		ImGui::PopStyleColor();

		ImGui::EndPopup();
	}
}

void ImGuiBoard::FlipBoard()
{
	m_reverse = !m_reverse;
}

void ImGuiBoard::OpenEditor()
{
	m_ToOpenEditor = true;

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
			m_Editorblock[i][j] = ChessAPI::GetBlockID(i + 8 * j);
	}
}

void ImGuiBoard::OpenEditor(const std::string& newFEN)
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
}