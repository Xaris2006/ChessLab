#include "Board.h"

#include <string>
#include <cmath>

//m_lastMoveIndex shound be intenger
//for roke should have four booleans

namespace Chess
{
	Board::Board()
	{
		NewPosition();
	}

	std::string Board::GetFen() const
	{
		std::array<Piece, 64> blocks;

		for (int i = 0; i < 64; i++)
		{
			for (uint8_t j = 0; j < 6; j++)
			{
				if (m_mapPieces[j].At(i))
				{
					blocks[i] = (Piece)j;
					break;
				}
				else
					blocks[i] = NONE;
			}
		}
			

		std::string Fenstr = "";

		std::string piecenames = "pnbrqkPNBRQK";

		for (int y = 7; y > -1; y--)
		{
			char empty = '0';
			for (int x = 0; x < 8; x++)
			{
				if (blocks[x + 8 * y] != NONE)
				{
					if (empty != '0') { Fenstr += empty; empty = '0'; }
					Fenstr += piecenames[blocks[x + 8 * y] + 6 * m_WhitePieces.At(x + 8 * y)];
				}
				else
					empty += 1;
			}
			if (empty != '0') { Fenstr += empty; }
			if (y != 0) { Fenstr += '/'; }
		}

		Fenstr += ' ';

		if (m_PlayerToPlay)
			Fenstr += 'w';
		else
			Fenstr += 'b';

		Fenstr += ' ';

		bool roke = false;

		if (m_mapWhitePieces[KING].At(4))
		{
			if (m_mapWhitePieces[ROOK].At(7) && m_K)
			{
				Fenstr += 'K';
				roke = true;
			}

			if (m_mapWhitePieces[ROOK].At(0) && m_Q)
			{
				Fenstr += 'Q';
				roke = true;
			}
		}

		if (m_mapBlackPieces[KING].At(60))
		{
			if (m_mapBlackPieces[ROOK].At(63) && m_k)
			{
				Fenstr += 'k';
				roke = true;
			}

			if (m_mapBlackPieces[ROOK].At(56) && m_q)
			{
				Fenstr += 'q';
				roke = true;
			}
		}

		if (!roke) { Fenstr += '-'; }

		Fenstr += ' ';

		if (m_LastMovedPieceIndex == -1)
		{
			Fenstr += '-';
		}
		else
		{
			Fenstr += 'a' + m_LastMovedPieceIndex % 8 + 1;
			
			if ((m_LastMovedPieceIndex / 8.0f) > 4)
				Fenstr += '6';
			else
				Fenstr += '3';
		}

		Fenstr += (' ' + std::to_string(m_FiftyMoveCounter));
		Fenstr += (' ' + std::to_string(m_BlackMovesCounter));

		return Fenstr;
	}

	std::vector<uint8_t>  Board::GetFormatedFen() const
	{
		//if (ERROR_EXIT) { return std::vector<uint8_t>(); }

		std::array<Piece, 64> blocks;

		for (int i = 0; i < 64; i++)
		{
			for (uint8_t j = 0; j < 6; j++)
			{
				if (m_mapPieces[j].At(i))
				{
					blocks[i] = (Piece)j;
					break;
				}
				else
					blocks[i] = NONE;
			}
		}

		std::vector<uint8_t> pos;
		int blockFilled = 0;
		bool firstfour = true;
		int blocks_empty = 0;

		constexpr uint8_t classnames[6] = { 0b00000000, 0b00000001, 0b00000010, 0b00000011, 0b00000100, 0b00000101 };
		constexpr uint8_t color[2] = { 0b00000000, 0b00001000 };
		constexpr uint8_t empty[4] = { 0b00000110, 0b00001110, 0b00000111, 0b00001111 };

		pos.emplace_back(0b00000000);
		for (int y = 7; y >= 0; y--)
		{
			for (int x = 0; x < 8; x++)
			{
				if (blockFilled == 8)
				{
					blockFilled = 0;
					pos.emplace_back(0b00000000);
				}
				if (!m_Pieces.At(x, y))
				{
					blocks_empty += 1;
					if (blocks_empty == 4)
					{
						size_t s = pos.size();
						pos[s - 1] |= empty[blocks_empty - 1];
						if (firstfour)
						{
							pos[s - 1] <<= 4;
							firstfour = false;
						}
						else
							firstfour = true;
						blockFilled += 4;
						blocks_empty = 0;
					}
					continue;
				}
				else if (blocks_empty)
				{
					size_t s = pos.size();
					pos[s - 1] |= empty[blocks_empty - 1];
					if (firstfour)
					{
						pos[s - 1] <<= 4;
						firstfour = false;
					}
					else
						firstfour = true;
					blockFilled += 4;
					blocks_empty = 0;
					if (blockFilled == 8)
					{
						blockFilled = 0;
						pos.emplace_back(0b00000000);
					}
				}
				size_t s = pos.size();
				pos[s - 1] |= classnames[blocks[x + 8*y]];
				pos[s - 1] |= color[(m_WhitePieces.At(x + 8 * y) ? 1 : 0)];
				if (firstfour)
				{
					pos[s - 1] <<= 4;
					firstfour = false;
				}
				else
					firstfour = true;
				blockFilled += 4;
			}

		}
		if (blocks_empty)
		{
			size_t s = pos.size();
			pos[s - 1] |= empty[blocks_empty - 1];
			if (firstfour)
				pos[s - 1] <<= 4;
		}
		
		pos.emplace_back(((uint8_t)((m_LastMovedPieceIndex == -1 ? 0 : m_LastMovedPieceIndex) % 8) | (m_PlayerToPlay == WHITE ? 0b00001000 : 0b00000000)) << 4);
		
		size_t s = pos.size();
		
		if (m_mapWhitePieces[KING].At(4))
		{
			if (m_mapWhitePieces[ROOK].At(7) && m_K)
				pos[s - 1] |= 0b00000001;

			if (m_mapWhitePieces[ROOK].At(0) && m_Q)
				pos[s - 1] |= 0b00000010;
		}

		if (m_mapBlackPieces[KING].At(60))
		{
			if (m_mapBlackPieces[ROOK].At(63) && m_k)
				pos[s - 1] |= 0b00000100;

			if (m_mapBlackPieces[ROOK].At(56) && m_q)
				pos[s - 1] |= 0b00001000;
		}

		return pos;
	}

	bool Board::NewPosition(const std::string& fenStr)
	{
		std::string fen = fenStr == "?" || fenStr == "" || fenStr == "default" ? "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" : fenStr;

		m_Pieces.Data() = 0x00;
		m_WhitePieces.Data() = 0x00;
		m_BlackPieces.Data() = 0x00;
		for(auto& piece : m_mapPieces)
			piece.Data() = 0x00;
		for(auto& piece : m_mapWhitePieces)
			piece.Data() = 0x00;
		for(auto& piece : m_mapBlackPieces)
			piece.Data() = 0x00;

		m_K = false;
		m_Q = false;
		m_k = false;
		m_q = false;

		m_LastMovedPieceIndex = -1;

		//Reading FEN 

		static const std::string WhiteTypePiecies = "PNBRQK";
		static const std::string BlackTypePiecies = "pnbrqk";
		std::string Roke;
		std::string FiftyMoveRule = "";
		std::string Blackmove = "";

		int ylevel = 7, xlevel = 0;

		for (size_t i = 0; i < fen.size(); i++)
		{

			if (fen[i] == '/' || fen[i] == ' ') { ylevel -= 1; xlevel = 0; }
			else if (ylevel >= 0)
			{
				if (fen[i] > 'A')
				{
					m_Pieces.Set(xlevel, ylevel, true);
					if (fen[i] > 'a') 
					{
						m_BlackPieces.Set(xlevel, ylevel, true);
						m_mapPieces[BlackTypePiecies.find(fen[i])].Set(xlevel, ylevel, true);
						m_mapBlackPieces[BlackTypePiecies.find(fen[i])].Set(xlevel, ylevel, true);
					}
					else
					{
						m_WhitePieces.Set(xlevel, ylevel, true);
						m_mapPieces[WhiteTypePiecies.find(fen[i])].Set(xlevel, ylevel, true);
						m_mapWhitePieces[WhiteTypePiecies.find(fen[i])].Set(xlevel, ylevel, true);
					}

					xlevel += 1;
				}
				else
				{
					xlevel += (int)fen[i] - 48;
				}
			}
			else if (ylevel == -1)
			{
				if (fen[i] == 'w') { m_PlayerToPlay = WHITE; }
				else { m_PlayerToPlay = BLACK; }
			}
			else if (ylevel == -2)
			{
				if (fen[i] == '-')
					continue;
				else
					Roke += fen[i];
			}
			else if (ylevel == -3)
			{
				if (fen[i] == '-')
					continue;
				
				if (fen[i] >= 'a')
					m_LastMovedPieceIndex = (int)(fen[i] - 'a');
				else if (fen[i] == '6')
					m_LastMovedPieceIndex += (5 * 8);
				else
					m_LastMovedPieceIndex += (2 * 8);
			}
			else if (ylevel == -4)
			{
				FiftyMoveRule += fen[i];
			}
			else
			{
				Blackmove += fen[i];
			}

		}

		if (Roke.find('K') != std::string::npos)
			m_K = true;
		if (Roke.find('Q') != std::string::npos)
			m_Q = true;
		if (Roke.find('k') != std::string::npos)
			m_k = true;
		if (Roke.find('q') != std::string::npos)
			m_q = true;

		m_FiftyMoveCounter = std::stoi(FiftyMoveRule);
		m_BlackMovesCounter = std::stoi(Blackmove);

		bool ret = IsBoardValid();

		if (!ret)
			NewPosition();

		return ret;
	}

	bool Board::NewPosition(const std::vector<uint8_t>& ffen)
	{
		return false;
	}

	void Board::GetAvailableMoves(std::vector<Move>& moves) const
	{
		moves.clear();

		FindPawnMoves(moves);
		FindKngihtMoves(moves);
		FindBishopMoves(moves);
		FindRookMoves(moves);
		FindQueenMoves(moves);
		FindKingMoves(moves);

		for (int i = 0; i < moves.size(); i++)
		{
			if (!IsMoveKingSecured(moves[i]))
			{
				moves.erase(moves.begin() + i);
				i--;
			}
		}
	}

	void Board::GetAvailableMoves(std::vector<Move>& moves, Piece type) const
	{
		moves.clear();

		switch (type)
		{
		case Chess::PAWN:
			FindPawnMoves(moves);
			break;
		case Chess::KNIGHT:
			FindKngihtMoves(moves);
			break;
		case Chess::BISHOP:
			FindBishopMoves(moves);
			break;
		case Chess::ROOK:
			FindRookMoves(moves);
			break;
		case Chess::QUEEN:
			FindQueenMoves(moves);
			break;
		case Chess::KING:
			FindKingMoves(moves);
			break;
		default:
			break;
		}

		for (int i = 0; i < moves.size(); i++)
		{
			if (!IsMoveKingSecured(moves[i]))
			{
				moves.erase(moves.begin() + i);
				i--;
			}
		}
	}

	Piece Board::GetPieceType(int index) const
	{
		for (int i = 0; i < 6; i++)
		{
			if (m_mapPieces[i].At(index))
				return (Piece)i;
		}

		return NONE;
	}

	Piece Board::GetPieceType(int indexX, int indexY) const
	{
		for (int i = 0; i < 6; i++)
		{
			if (m_mapPieces[i].At(indexX + 8 * indexY))
				return (Piece)i;
		}

		return NONE;
	}

	Color Board::GetPieceColor(int index) const
	{
		if (m_WhitePieces.At(index))
			return WHITE;
		else
			return BLACK;
	}

	Color Board::GetPieceColor(int indexX, int indexY) const
	{
		if (m_WhitePieces.At(indexX + 8 * indexY))
			return WHITE;
		else
			return BLACK;
	}

	Board::KingStatus Board::GetKingStatus() const
	{
		if (m_PlayerToPlay == WHITE)
		{
			int kingIndex = (int)std::log2(m_mapWhitePieces[KING].Data());
			

			int yLevel = kingIndex / 8;
			int xLevel = kingIndex % 8;

			const int right = 7 - xLevel;
			int left = xLevel;
			int up = 7 - yLevel;
			int down = yLevel;

			bool ok_right = right >= 2;
			bool ok_left = left >= 2;
			bool ok_up = up >= 2;
			bool ok_down = down >= 2;

			BitBoard enemyQueenBishop = m_mapBlackPieces[QUEEN] + m_mapBlackPieces[BISHOP];
			BitBoard enemyQueenRook = m_mapBlackPieces[QUEEN] + m_mapBlackPieces[ROOK];

			for (int y = 1; y < std::min(up, right) + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex + y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 9))
					goto skip;

				if (m_BlackPieces.At(kingIndex + y * 9))
					break;
			}
			for (int y = 1; y < std::min(up, left) + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex + y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 7))
					goto skip;

				if (m_BlackPieces.At(kingIndex + y * 7))
					break;
			}
			for (int y = 1; y < std::min(down, left) + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex - y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 9))
					goto skip;

				if (m_BlackPieces.At(kingIndex - y * 9))
					break;
			}
			for (int y = 1; y < std::min(down, right) + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex - y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 7))
					goto skip;

				if (m_BlackPieces.At(kingIndex - y * 7))
					break;
			}

			for (int y = 1; y < up + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex + y * 8))
					break;

				if (enemyQueenRook.At(kingIndex + y * 8))
					goto skip;

				if (m_BlackPieces.At(kingIndex + y * 8))
					break;
			}
			for (int y = 1; y < left + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex - y))
					break;

				if (enemyQueenRook.At(kingIndex - y))
					goto skip;

				if (m_BlackPieces.At(kingIndex - y))
					break;
			}
			for (int y = 1; y < down + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex - y * 8))
					break;

				if (enemyQueenRook.At(kingIndex - y * 8))
					goto skip;

				if (m_BlackPieces.At(kingIndex - y * 8))
					break;
			}
			for (int y = 1; y < right + 1; y++)
			{
				if (m_WhitePieces.At(kingIndex + y))
					break;

				if (enemyQueenRook.At(kingIndex + y))
					goto skip;

				if (m_BlackPieces.At(kingIndex + y))
					break;
			}

			if ((m_mapBlackPieces[PAWN].At(kingIndex + 7) && left) || (m_mapBlackPieces[PAWN].At(kingIndex + 9) && right))
				goto skip;

			if (ok_up)
			{
				if (left && m_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[3]))
					goto skip;
				if (right && m_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[2]))
					goto skip;
			}
			if (ok_down)
			{
				if (left && m_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[2]))
					goto skip;
				if (right && m_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[3]))
					goto skip;
			}
			if (ok_left)
			{
				if (down && m_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[1]))
					goto skip;
				if (up && m_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[0]))
					goto skip;
			}
			if (ok_right)
			{
				if (down && m_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[0]))
					goto skip;
				if (up && m_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[1]))
					goto skip;
			}
		}
		else
		{
			int kingIndex = (int)std::log2(m_mapBlackPieces[KING].Data());

			int yLevel = kingIndex / 8;
			int xLevel = kingIndex % 8;

			int right = 7 - xLevel;
			int left = xLevel;
			int up = 7 - yLevel;
			int down = yLevel;

			bool ok_right = right >= 2;
			bool ok_left = left >= 2;
			bool ok_up = up >= 2;
			bool ok_down = down >= 2;

			BitBoard enemyQueenBishop = m_mapWhitePieces[QUEEN] + m_mapWhitePieces[BISHOP];
			BitBoard enemyQueenRook = m_mapWhitePieces[QUEEN] + m_mapWhitePieces[ROOK];

			for (int y = 1; y < std::min(up, right) + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex + y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 9))
					goto skip;

				if (m_WhitePieces.At(kingIndex + y * 9))
					break;
			}
			for (int y = 1; y < std::min(up, left) + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex + y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 7))
					goto skip;

				if (m_WhitePieces.At(kingIndex + y * 7))
					break;
			}
			for (int y = 1; y < std::min(down, left) + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex - y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 9))
					goto skip;

				if (m_WhitePieces.At(kingIndex - y * 9))
					break;
			}
			for (int y = 1; y < std::min(down, right) + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex - y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 7))
					goto skip;

				if (m_WhitePieces.At(kingIndex - y * 7))
					break;
			}

			for (int y = 1; y < up + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex + y * 8))
					break;

				if (enemyQueenRook.At(kingIndex + y * 8))
					goto skip;

				if (m_WhitePieces.At(kingIndex + y * 8))
					break;
			}
			for (int y = 1; y < left + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex - y))
					break;

				if (enemyQueenRook.At(kingIndex - y))
					goto skip;

				if (m_WhitePieces.At(kingIndex - y))
					break;
			}
			for (int y = 1; y < down + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex - y * 8))
					break;

				if (enemyQueenRook.At(kingIndex - y * 8))
					goto skip;

				if (m_WhitePieces.At(kingIndex - y * 8))
					break;
			}
			for (int y = 1; y < right + 1; y++)
			{
				if (m_BlackPieces.At(kingIndex + y))
					break;

				if (enemyQueenRook.At(kingIndex + y))
					goto skip;

				if (m_WhitePieces.At(kingIndex + y))
					break;
			}

			if ((m_mapWhitePieces[PAWN].At(kingIndex - 9) && left) || (m_mapWhitePieces[PAWN].At(kingIndex - 7) && right))
				goto skip;

			if (ok_up)
			{
				if (left && m_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[3]))
					goto skip;
				if (right && m_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[2]))
					goto skip;
			}
			if (ok_down)
			{
				if (left && m_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[2]))
					goto skip;
				if (right && m_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[3]))
					goto skip;
			}
			if (ok_left)
			{
				if (down && m_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[1]))
					goto skip;
				if (up && m_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[0]))
					goto skip;
			}
			if (ok_right)
			{
				if (down && m_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[0]))
					goto skip;
				if (up && m_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[1]))
					goto skip;
			}
		}

		return SECURE;

		skip:

		std::vector<Move> moves;
		GetAvailableMoves(moves);

		if (moves.empty())
			return MATED;
		
		return CHECKED;
	}

	Board::MakeMoveStatus Board::MakeMove(Move move, Piece piecePromotion)
	{
		//safe
		if (!IsMoveValid(move))
			return MOVEERROR;

		Color nextToPlay = m_PlayerToPlay;
		Piece type = NONE, enemyType = NONE;
		int direction = move.index + move.move;

		for (int i = 0; i < 6; i++)
		{
			if (m_mapPieces[i].At(move.index))
				type = (Piece)i;
			if (m_mapPieces[i].At(direction))
				enemyType = (Piece)i;
		}

		//safe
		if (type == NONE)
			return MOVEERROR;

		//safe
		if (type == PAWN && ((direction / 8) == 0 || (direction / 8) == 7) && (piecePromotion > QUEEN || piecePromotion == PAWN))
			return PROMOTION;

		//Roke
		if (type == KING && std::abs(move.move) == 2)
		{
			if (MakeMove({ (move.move > 0 ? (move.index + 3) : (move.index - 4)), (move.move > 0 ? -2 : 3) }) != SUCCESS)
				return MOVEERROR;//safe
			m_PlayerToPlay = nextToPlay;
			if (m_PlayerToPlay == BLACK)
				m_BlackMovesCounter--;
			m_FiftyMoveCounter--;
		}

		m_Pieces.Set(move.index, false);
		m_mapPieces[type].Set(move.index, false);

		if (m_PlayerToPlay == WHITE)
		{
			nextToPlay = BLACK;

			//Moves king or rook so no roke any more
			if (type == KING)
			{
				m_K = false;
				m_Q = false;
			}
			if (type == ROOK && move.index == 7)
				m_K = false;
			if (type == ROOK && move.index == 0)
				m_Q = false;

			m_WhitePieces.Set(move.index, false);
			m_mapWhitePieces[type].Set(move.index, false);

			m_WhitePieces.Set(direction, true);
			m_mapWhitePieces[type].Set(direction, true);

			if (enemyType != NONE)
			{
				if (type != enemyType)
				{
					m_mapPieces[type].Set(direction, true);
					m_mapPieces[enemyType].Set(direction, false);
				}

				m_BlackPieces.Set(direction, false);
				m_mapBlackPieces[enemyType].Set(direction, false);
			}
			else
			{
				m_Pieces.Set(direction, true);
				m_mapPieces[type].Set(direction, true);
			}
			
			//White AN PAN SAN
			if (type == PAWN && direction == m_LastMovedPieceIndex)
			{
				m_Pieces.Set(direction - 8, false);
				m_mapPieces[PAWN].Set(direction - 8, false);
				m_BlackPieces.Set(direction - 8, false);
				m_mapBlackPieces[PAWN].Set(direction - 8, false);
			}

			//Promosion
			if (piecePromotion < KING && piecePromotion > PAWN)
			{
				m_mapPieces[PAWN].Set(direction, false);
				m_mapWhitePieces[PAWN].Set(direction, false);
				
				m_mapPieces[piecePromotion].Set(direction, true);
				m_mapWhitePieces[piecePromotion].Set(direction, true);
			}
		}
		else
		{
			nextToPlay = WHITE;

			//Moves king or rook so no roke any more
			if (type == KING)
			{
				m_k = false;
				m_q = false;
			}
			if (type == ROOK && move.index == 63)
				m_k = false;
			if (type == ROOK && move.index == 56)
				m_q = false;

			m_BlackPieces.Set(move.index, false);
			m_mapBlackPieces[type].Set(move.index, false);

			m_BlackPieces.Set(direction, true);
			m_mapBlackPieces[type].Set(direction, true);

			if (enemyType != NONE)
			{
				if (type != enemyType)
				{
					m_mapPieces[type].Set(direction, true);
					m_mapPieces[enemyType].Set(direction, false);
				}

				m_WhitePieces.Set(direction, false);
				m_mapWhitePieces[enemyType].Set(direction, false);
			}
			else
			{
				m_Pieces.Set(direction, true);
				m_mapPieces[type].Set(direction, true);
			}

			//Black AN PAN SAN
			if (type == PAWN && direction == m_LastMovedPieceIndex)
			{
				m_Pieces.Set(direction + 8, false);
				m_mapPieces[PAWN].Set(direction + 8, false);
				m_WhitePieces.Set(direction + 8, false);
				m_mapWhitePieces[PAWN].Set(direction + 8, false);
			}

			//Promosion
			if (piecePromotion < KING && piecePromotion > PAWN)
			{
				m_mapPieces[PAWN].Set(direction, false);
				m_mapBlackPieces[PAWN].Set(direction, false);

				m_mapPieces[piecePromotion].Set(direction, true);
				m_mapBlackPieces[piecePromotion].Set(direction, true);
			}
			
			m_BlackMovesCounter++;
		}

		//Update last move for an pan san
		if (type == PAWN && std::abs(move.move) == 16)
			m_LastMovedPieceIndex = direction  - move.move/ 2;
		else
			m_LastMovedPieceIndex = -1;

		if (type == PAWN || enemyType != NONE)
			m_FiftyMoveCounter++;

		m_PlayerToPlay = nextToPlay;

		return SUCCESS;
	}

	bool Board::IsMoveValid(Move move) const
	{
		std::vector<Move> PossibleMoves;
		GetAvailableMoves(PossibleMoves);
		for (auto& m : PossibleMoves)
		{
			if (m.index == move.index && m.move == move.move)
				return true;
		}

		return false;
	}

	void Board::AddPiece(Piece type, Color color, int index)
	{
		Piece old = GetPieceType(index);
		if (old != NONE)
		{
			m_mapPieces[old].Set(index, false);
			m_WhitePieces.Set(index, false);
			m_mapWhitePieces[old].Set(index, false);
			m_BlackPieces.Set(index, false);
			m_mapBlackPieces[old].Set(index, false);
		}

		m_Pieces.Set(index, true);
		m_mapPieces[type].Set(index, true);

		if (color == WHITE)
		{
			m_WhitePieces.Set(index, true);
			m_mapWhitePieces[type].Set(index, true);
		}
		else
		{
			m_BlackPieces.Set(index, true);
			m_mapBlackPieces[type].Set(index, true);
		}
	}

	void Board::AddPiece(Piece type, Color color, int indexX, int indexY)
	{
		Piece old = GetPieceType(indexX, indexY);
		if (old != NONE)
		{
			m_mapPieces[old].Set(indexX, indexY, false);
			m_WhitePieces.Set(indexX, indexY, false);
			m_mapWhitePieces[old].Set(indexX, indexY, false);
			m_BlackPieces.Set(indexX, indexY, false);
			m_mapBlackPieces[old].Set(indexX, indexY, false);
		}

		m_Pieces.Set(indexX, indexY, true);
		m_mapPieces[type].Set(indexX, indexY, true);

		if (color == WHITE)
		{
			m_WhitePieces.Set(indexX, indexY, true);
			m_mapWhitePieces[type].Set(indexX, indexY, true);
		}
		else
		{
			m_BlackPieces.Set(indexX, indexY, true);
			m_mapBlackPieces[type].Set(indexX, indexY, true);
		}
	}

	void Board::RemovePiece(int index)
	{
		Piece type = GetPieceType(index);

		if (type == NONE)
			return;

		m_Pieces.Set(index, false);
		m_mapPieces[type].Set(index, false);

		if (GetPieceColor(index) == WHITE)
		{
			m_WhitePieces.Set(index, false);
			m_mapWhitePieces[type].Set(index, false);
		}
		else
		{
			m_BlackPieces.Set(index, false);
			m_mapBlackPieces[type].Set(index, false);
		}
	}

	void Board::RemovePiece(int indexX, int indexY)
	{
		Piece type = GetPieceType(indexX, indexY);

		if (type == NONE)
			return;

		m_Pieces.Set(indexX, indexY, false);
		m_mapPieces[type].Set(indexX, indexY, false);

		if (GetPieceColor(indexX, indexY) == WHITE)
		{
			m_WhitePieces.Set(indexX, indexY, false);
			m_mapWhitePieces[type].Set(indexX, indexY, false);
		}
		else
		{
			m_BlackPieces.Set(indexX, indexY, false);
			m_mapBlackPieces[type].Set(indexX, indexY, false);
		}
	}

	void Board::UpdateVitualValues() const
	{
		m_Virtual_Pieces = m_Pieces;
		m_Virtual_WhitePieces = m_WhitePieces;
		m_Virtual_BlackPieces = m_BlackPieces;
		m_Virtual_mapPieces = m_mapPieces;
		m_Virtual_mapWhitePieces = m_mapWhitePieces;
		m_Virtual_mapBlackPieces = m_mapBlackPieces;
	}

	Board::MakeMoveStatus Board::VirtualMakeMove(Move move) const
	{
		Piece type = NONE, enemyType = NONE;
		int direction = move.index + move.move;

		for (int i = 0; i < 6; i++)
		{
			if (m_Virtual_mapPieces[i].At(move.index))
				type = (Piece)i;
			if (m_Virtual_mapPieces[i].At(direction))
				enemyType = (Piece)i;
		}

		if (type == NONE)
			return MOVEERROR;

		m_Virtual_Pieces.Set(move.index, false);
		m_Virtual_mapPieces[type].Set(move.index, false);

		if (m_PlayerToPlay == WHITE)
		{
			m_Virtual_WhitePieces.Set(move.index, false);
			m_Virtual_mapWhitePieces[type].Set(move.index, false);

			m_Virtual_WhitePieces.Set(direction, true);
			m_Virtual_mapWhitePieces[type].Set(direction, true);

			if (enemyType != NONE)
			{
				if (type != enemyType)
				{
					m_Virtual_mapPieces[type].Set(direction, true);
					m_Virtual_mapPieces[enemyType].Set(direction, false);
				}

				m_Virtual_BlackPieces.Set(direction, false);
				m_Virtual_mapBlackPieces[enemyType].Set(direction, false);
			}
			else
			{
				m_Virtual_Pieces.Set(direction, true);
				m_Virtual_mapPieces[type].Set(direction, true);
			}
		}
		else
		{
			m_Virtual_BlackPieces.Set(move.index, false);
			m_Virtual_mapBlackPieces[type].Set(move.index, false);

			m_Virtual_BlackPieces.Set(direction, true);
			m_Virtual_mapBlackPieces[type].Set(direction, true);

			if (enemyType != NONE)
			{
				if (type != enemyType)
				{
					m_Virtual_mapPieces[type].Set(direction, true);
					m_Virtual_mapPieces[enemyType].Set(direction, false);
				}

				m_Virtual_WhitePieces.Set(direction, false);
				m_Virtual_mapWhitePieces[enemyType].Set(direction, false);
			}
			else
			{
				m_Virtual_Pieces.Set(direction, true);
				m_Virtual_mapPieces[type].Set(direction, true);
			}
		}

		return SUCCESS;
	}

	Board::KingStatus Board::GetVirtualKingStatus(Color playerColor) const
	{
		if (playerColor == WHITE)
		{
			int kingIndex = (int)std::log2(m_Virtual_mapWhitePieces[KING].Data());

			int yLevel = kingIndex / 8;
			int xLevel = kingIndex % 8;

			int right = 7 - xLevel;
			int left = xLevel;
			int up = 7 - yLevel;
			int down = yLevel;

			bool ok_right = right >= 2;
			bool ok_left = left >= 2;
			bool ok_up = up >= 2;
			bool ok_down = down >= 2;

			BitBoard enemyQueenBishop = m_Virtual_mapBlackPieces[QUEEN] + m_Virtual_mapBlackPieces[BISHOP];
			BitBoard enemyQueenRook = m_Virtual_mapBlackPieces[QUEEN] + m_Virtual_mapBlackPieces[ROOK];

			for (int y = 1; y < std::min(up, right) + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex + y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 9))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex + y * 9))
					break;
			}
			for (int y = 1; y < std::min(up, left) + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex + y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 7))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex + y * 7))
					break;
			}
			for (int y = 1; y < std::min(down, left) + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex - y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 9))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex - y * 9))
					break;
			}
			for (int y = 1; y < std::min(down, right) + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex - y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 7))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex - y * 7))
					break;
			}

			for (int y = 1; y < up + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex + y * 8))
					break;

				if (enemyQueenRook.At(kingIndex + y * 8))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex + y * 8))
					break;
			}
			for (int y = 1; y < left + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex - y))
					break;

				if (enemyQueenRook.At(kingIndex - y))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex - y))
					break;
			}
			for (int y = 1; y < down + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex - y * 8))
					break;

				if (enemyQueenRook.At(kingIndex - y * 8))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex - y * 8))
					break;
			}
			for (int y = 1; y < right + 1; y++)
			{
				if (m_Virtual_WhitePieces.At(kingIndex + y))
						break;

				if (enemyQueenRook.At(kingIndex + y))
					return CHECKED;

				if (m_Virtual_BlackPieces.At(kingIndex + y))
					break;
			}

			if ((m_Virtual_mapBlackPieces[PAWN].At(kingIndex + 7) && left) || (m_Virtual_mapBlackPieces[PAWN].At(kingIndex + 9) && right))
				return CHECKED;

			if (ok_up)
			{
				if (left && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[3]))
					return CHECKED;
				if (right && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[2]))
					return CHECKED;
			}
			if (ok_down)
			{
				if (left && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[2]))
					return CHECKED;
				if (right && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[3]))
					return CHECKED;
			}
			if (ok_left)
			{
				if (down && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[1]))
					return CHECKED;
				if (up && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[0]))
					return CHECKED;
			}
			if (ok_right)
			{
				if (down && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex - KnightMoves[0]))
					return CHECKED;
				if (up && m_Virtual_mapBlackPieces[KNIGHT].At(kingIndex + KnightMoves[1]))
					return CHECKED;
			}

			if (ok_up)
			{
				if (m_Virtual_mapBlackPieces[KING].At(kingIndex + KingMoves[2]))
					return CHECKED;
				if (ok_left && m_Virtual_mapBlackPieces[KING].At(kingIndex + KingMoves[1]))
					return CHECKED;
				if (ok_right && m_Virtual_mapBlackPieces[KING].At(kingIndex + KingMoves[3]))
					return CHECKED;
			}
			if (ok_down)
			{
				if (m_Virtual_mapBlackPieces[KING].At(kingIndex - KingMoves[2]))
					return CHECKED;
				if (ok_left && m_Virtual_mapBlackPieces[KING].At(kingIndex - KingMoves[3]))
					return CHECKED;
				if (ok_right && m_Virtual_mapBlackPieces[KING].At(kingIndex - KingMoves[1]))
					return CHECKED;
			}
			if (ok_left && m_Virtual_mapBlackPieces[KING].At(kingIndex - KingMoves[0]))
				return CHECKED;
			if (ok_right && m_Virtual_mapBlackPieces[KING].At(kingIndex + KingMoves[0]))
				return CHECKED;
		}
		else
		{
			int kingIndex = (int)std::log2(m_Virtual_mapBlackPieces[KING].Data());

			int yLevel = kingIndex / 8;
			int xLevel = kingIndex % 8;

			int right = 7 - xLevel;
			int left = xLevel;
			int up = 7 - yLevel;
			int down = yLevel;

			bool ok_right = right >= 2;
			bool ok_left = left >= 2;
			bool ok_up = up >= 2;
			bool ok_down = down >= 2;

			BitBoard enemyQueenBishop = m_Virtual_mapWhitePieces[QUEEN] + m_Virtual_mapWhitePieces[BISHOP];
			BitBoard enemyQueenRook = m_Virtual_mapWhitePieces[QUEEN] + m_Virtual_mapWhitePieces[ROOK];

			for (int y = 1; y < std::min(up, right) + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex + y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 9))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex + y * 9))
					break;
			}
			for (int y = 1; y < std::min(up, left) + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex + y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex + y * 7))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex + y * 7))
					break;
			}
			for (int y = 1; y < std::min(down, left) + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex - y * 9))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 9))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex - y * 9))
					break;
			}
			for (int y = 1; y < std::min(down, right) + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex - y * 7))
					break;

				if (enemyQueenBishop.At(kingIndex - y * 7))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex - y * 7))
					break;
			}

			for (int y = 1; y < up + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex + y * 8))
					break;

				if (enemyQueenRook.At(kingIndex + y * 8))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex + y * 8))
					break;
			}
			for (int y = 1; y < left + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex - y))
					break;

				if (enemyQueenRook.At(kingIndex - y))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex - y))
					break;
			}
			for (int y = 1; y < down + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex - y * 8))
					break;

				if (enemyQueenRook.At(kingIndex - y * 8))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex - y * 8))
					break;
			}
			for (int y = 1; y < right + 1; y++)
			{
				if (m_Virtual_BlackPieces.At(kingIndex + y))
					break;

				if (enemyQueenRook.At(kingIndex + y))
					return CHECKED;

				if (m_Virtual_WhitePieces.At(kingIndex + y))
					break;
			}

			if ((m_Virtual_mapWhitePieces[PAWN].At(kingIndex - 9) && left) || (m_Virtual_mapWhitePieces[PAWN].At(kingIndex - 7) && right))
				return CHECKED;

			if (ok_up)
			{
				if (left && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[3]))
					return CHECKED;
				if (right && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[2]))
					return CHECKED;
			}
			if (ok_down)
			{
				if (left && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[2]))
					return CHECKED;
				if (right && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[3]))
					return CHECKED;
			}
			if (ok_left)
			{
				if (down && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[1]))
					return CHECKED;
				if (up && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[0]))
					return CHECKED;
			}
			if (ok_right)
			{
				if (down && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex - KnightMoves[0]))
					return CHECKED;
				if (up && m_Virtual_mapWhitePieces[KNIGHT].At(kingIndex + KnightMoves[1]))
					return CHECKED;
			}

			if (ok_up)
			{
				if (m_Virtual_mapWhitePieces[KING].At(kingIndex + KingMoves[2]))
					return CHECKED;
				if (ok_left && m_Virtual_mapWhitePieces[KING].At(kingIndex + KingMoves[1]))
					return CHECKED;
				if (ok_right && m_Virtual_mapWhitePieces[KING].At(kingIndex + KingMoves[3]))
					return CHECKED;
			}
			if (ok_down)
			{
				if (m_Virtual_mapWhitePieces[KING].At(kingIndex - KingMoves[2]))
					return CHECKED;
				if (ok_left && m_Virtual_mapWhitePieces[KING].At(kingIndex - KingMoves[3]))
					return CHECKED;
				if (ok_right && m_Virtual_mapWhitePieces[KING].At(kingIndex - KingMoves[1]))
					return CHECKED;
			}
			if (ok_left && m_Virtual_mapWhitePieces[KING].At(kingIndex - KingMoves[0]))
				return CHECKED;
			if (ok_right && m_Virtual_mapWhitePieces[KING].At(kingIndex + KingMoves[0]))
				return CHECKED;

		}
		
		return SECURE;
	}

	bool Board::IsMoveKingSecured(Move move) const
	{
		UpdateVitualValues();
		VirtualMakeMove(move);

		if (m_PlayerToPlay == WHITE)
			return GetVirtualKingStatus(WHITE) == SECURE;
		else
			return GetVirtualKingStatus(BLACK) == SECURE;
	}

	void Board::FindPawnMoves(std::vector<Move>& moves) const
	{
		auto& whitePawns = m_mapWhitePieces[PAWN];
		auto& blackPawns = m_mapBlackPieces[PAWN];

		if (m_PlayerToPlay == WHITE)
		{
			for (int i = 8; i < 56; i++)
			{
				if (whitePawns.At(i))
				{
					if (!m_Pieces.At(i + PawnMoves[0]))
					{
						moves.emplace_back(i, PawnMoves[0]);

						if (i < 16 && !m_Pieces.At(i + PawnMoves[1]))
							moves.emplace_back(i, PawnMoves[1]);
					}

					if (i % 8 < 7 && (m_BlackPieces.At(i + PawnMoves[2]) || (i + PawnMoves[2]) == m_LastMovedPieceIndex))
						moves.emplace_back(i, PawnMoves[2]);

					if (i % 8 > 0 && (m_BlackPieces.At(i + PawnMoves[3]) || (i + PawnMoves[3]) == m_LastMovedPieceIndex))
						moves.emplace_back(i, PawnMoves[3]);
				}
			}
		}
		else
		{
			for (int i = 8; i < 56; i++)
			{
				if (blackPawns.At(i))
				{
					if (!m_Pieces.At(i - PawnMoves[0]))
					{
						moves.emplace_back(i, -PawnMoves[0]);

						if (i > 47 && !m_Pieces.At(i - PawnMoves[1]))
							moves.emplace_back(i, -PawnMoves[1]);
					}

					if (i % 8 > 0 && (m_WhitePieces.At(i - PawnMoves[2]) || (i - PawnMoves[2]) == m_LastMovedPieceIndex))
						moves.emplace_back(i, -PawnMoves[2]);

					if (i % 8 < 7 && (m_WhitePieces.At(i - PawnMoves[3]) || (i - PawnMoves[3]) == m_LastMovedPieceIndex))
						moves.emplace_back(i, -PawnMoves[3]);
				}
			}
		}
	}

	void Board::FindKngihtMoves(std::vector<Move>& moves) const
	{
		auto& whiteKngihts = m_mapWhitePieces[KNIGHT];
		auto& blackKngihts = m_mapBlackPieces[KNIGHT];

		if (m_PlayerToPlay == WHITE)
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				bool ok_right = right >= 2;
				bool ok_left = left >= 2;
				bool ok_up = up >= 2;
				bool ok_down = down >= 2;

				if (whiteKngihts.At(i))
				{
					if (ok_up)
					{
						if (left && !m_WhitePieces.At(i + KnightMoves[3]))
							moves.emplace_back(i, KnightMoves[3]);
						if (right && !m_WhitePieces.At(i + KnightMoves[2]))
							moves.emplace_back(i, KnightMoves[2]);
					}
					if (ok_down)
					{
						if (left && !m_WhitePieces.At(i - KnightMoves[2]))
							moves.emplace_back(i, -KnightMoves[2]);
						if (right && !m_WhitePieces.At(i - KnightMoves[3]))
							moves.emplace_back(i, -KnightMoves[3]);
					}
					if (ok_left)
					{
						if (down && !m_WhitePieces.At(i - KnightMoves[1]))
							moves.emplace_back(i, -KnightMoves[1]);
						if (up && !m_WhitePieces.At(i + KnightMoves[0]))
							moves.emplace_back(i, KnightMoves[0]);
					}
					if (ok_right)
					{
						if (down && !m_WhitePieces.At(i - KnightMoves[0]))
							moves.emplace_back(i, -KnightMoves[0]);
						if (up && !m_WhitePieces.At(i + KnightMoves[1]))
							moves.emplace_back(i, KnightMoves[1]);
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				bool ok_right = right >= 2;
				bool ok_left = left >= 2;
				bool ok_up = up >= 2;
				bool ok_down = down >= 2;

				if (blackKngihts.At(i))
				{
					if (ok_up)
					{
						if (left && !m_BlackPieces.At(i + KnightMoves[3]))
							moves.emplace_back(i, KnightMoves[3]);
						if (right && !m_BlackPieces.At(i + KnightMoves[2]))
							moves.emplace_back(i, KnightMoves[2]);
					}
					if (ok_down)
					{
						if (left && !m_BlackPieces.At(i - KnightMoves[2]))
							moves.emplace_back(i, -KnightMoves[2]);
						if (right && !m_BlackPieces.At(i - KnightMoves[3]))
							moves.emplace_back(i, -KnightMoves[3]);
					}
					if (ok_left)
					{
						if (down && !m_BlackPieces.At(i - KnightMoves[1]))
							moves.emplace_back(i, -KnightMoves[1]);
						if (up && !m_BlackPieces.At(i + KnightMoves[0]))
							moves.emplace_back(i, KnightMoves[0]);
					}
					if (ok_right)
					{
						if (down && !m_BlackPieces.At(i - KnightMoves[0]))
							moves.emplace_back(i, -KnightMoves[0]);
						if (up && !m_BlackPieces.At(i + KnightMoves[1]))
							moves.emplace_back(i, KnightMoves[1]);
					}
				}
			}
		}

		
	}

	void Board::FindBishopMoves(std::vector<Move>& moves) const
	{
		auto& whiteBishops = m_mapWhitePieces[BISHOP];
		auto& blackBishops = m_mapBlackPieces[BISHOP];

		if (m_PlayerToPlay == WHITE)
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				if (whiteBishops.At(i))
				{
					for (int y = 1; y < std::min(up, right) + 1; y++)
					{
						if (m_WhitePieces.At(i + y * 9))
							break;

						moves.emplace_back(i, y * 9);

						if (m_BlackPieces.At(i + y * 9))
							break;
					}
					for (int y = 1; y < std::min(up, left) + 1; y++)
					{
						if (m_WhitePieces.At(i + y * 7))
							break;

						moves.emplace_back(i, y * 7);

						if (m_BlackPieces.At(i + y * 7))
							break;
					}
					for (int y = 1; y < std::min(down, left) + 1; y++)
					{
						if (m_WhitePieces.At(i - y * 9))
							break;

						moves.emplace_back(i, -y * 9);

						if (m_BlackPieces.At(i - y * 9))
							break;
					}
					for (int y = 1; y < std::min(down, right) + 1; y++)
					{
						if (m_WhitePieces.At(i - y * 7))
							break;

						moves.emplace_back(i, -y * 7);

						if (m_BlackPieces.At(i - y * 7))
							break;
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				if (blackBishops.At(i))
				{
					for (int y = 1; y < std::min(up, right) + 1; y++)
					{
						if (m_BlackPieces.At(i + y * 9))
							break;

						moves.emplace_back(i, y * 9);

						if (m_WhitePieces.At(i + y * 9))
							break;
					}
					for (int y = 1; y < std::min(up, left) + 1; y++)
					{
						if (m_BlackPieces.At(i + y * 7))
							break;

						moves.emplace_back(i, y * 7);

						if (m_WhitePieces.At(i + y * 7))
							break;
					}
					for (int y = 1; y < std::min(down, left) + 1; y++)
					{
						if (m_BlackPieces.At(i - y * 9))
							break;

						moves.emplace_back(i, -y * 9);

						if (m_WhitePieces.At(i - y * 9))
							break;
					}
					for (int y = 1; y < std::min(down, right) + 1; y++)
					{
						if (m_BlackPieces.At(i - y * 7))
							break;

						moves.emplace_back(i, -y * 7);

						if (m_WhitePieces.At(i - y * 7))
							break;
					}
				}
			}
		}
	}

	void Board::FindRookMoves(std::vector<Move>& moves) const
	{
		auto& whiteRooks = m_mapWhitePieces[ROOK];
		auto& blackRooks = m_mapBlackPieces[ROOK];

		if (m_PlayerToPlay == WHITE)
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				if (whiteRooks.At(i))
				{
					for (int y = 1; y < up + 1; y++)
					{
						if (m_WhitePieces.At(i + y * 8))
							break;

						moves.emplace_back(i, y * 8);

						if (m_BlackPieces.At(i + y * 8))
							break;
					}
					for (int y = 1; y < left + 1; y++)
					{
						if (m_WhitePieces.At(i - y))
							break;

						moves.emplace_back(i, -y);

						if (m_BlackPieces.At(i - y))
							break;
					}
					for (int y = 1; y < down + 1; y++)
					{
						if (m_WhitePieces.At(i - y * 8))
							break;

						moves.emplace_back(i, -y * 8);

						if (m_BlackPieces.At(i - y * 8))
							break;
					}
					for (int y = 1; y < right + 1; y++)
					{
						if (m_WhitePieces.At(i + y))
							break;

						moves.emplace_back(i, y);

						if (m_BlackPieces.At(i + y))
							break;
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				if (blackRooks.At(i))
				{
					for (int y = 1; y < up + 1; y++)
					{
						if (m_BlackPieces.At(i + y * 8))
							break;

						moves.emplace_back(i, y * 8);

						if (m_WhitePieces.At(i + y * 8))
							break;
					}
					for (int y = 1; y < left + 1; y++)
					{
						if (m_BlackPieces.At(i - y))
							break;

						moves.emplace_back(i, -y);

						if (m_WhitePieces.At(i - y))
							break;
					}
					for (int y = 1; y < down + 1; y++)
					{
						if (m_BlackPieces.At(i - y * 8))
							break;

						moves.emplace_back(i, -y * 8);

						if (m_WhitePieces.At(i - y * 8))
							break;
					}
					for (int y = 1; y < right + 1; y++)
					{
						if (m_BlackPieces.At(i + y))
							break;

						moves.emplace_back(i, y);

						if (m_WhitePieces.At(i + y))
							break;
					}
				}
			}
		}

		
	}

	void Board::FindQueenMoves(std::vector<Move>& moves) const
	{
		auto& whiteQueens = m_mapWhitePieces[QUEEN];
		auto& blackQueens = m_mapBlackPieces[QUEEN];

		if (m_PlayerToPlay == WHITE)
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				if (whiteQueens.At(i))
				{
					for (int y = 1; y < std::min(up, right) + 1; y++)
					{
						if (m_WhitePieces.At(i + y * 9))
							break;

						moves.emplace_back(i, y * 9);

						if (m_BlackPieces.At(i + y * 9))
							break;
					}
					for (int y = 1; y < std::min(up, left) + 1; y++)
					{
						if (m_WhitePieces.At(i + y * 7))
							break;

						moves.emplace_back(i, y * 7);

						if (m_BlackPieces.At(i + y * 7))
							break;
					}
					for (int y = 1; y < std::min(down, left) + 1; y++)
					{
						if (m_WhitePieces.At(i - y * 9))
							break;

						moves.emplace_back(i, -y * 9);

						if (m_BlackPieces.At(i - y * 9))
							break;
					}
					for (int y = 1; y < std::min(down, right) + 1; y++)
					{
						if (m_WhitePieces.At(i - y * 7))
							break;

						moves.emplace_back(i, -y * 7);

						if (m_BlackPieces.At(i - y * 7))
							break;
					}

					for (int y = 1; y < up + 1; y++)
					{
						if (m_WhitePieces.At(i + y * 8))
							break;

						moves.emplace_back(i, y * 8);

						if (m_BlackPieces.At(i + y * 8))
							break;
					}
					for (int y = 1; y < left + 1; y++)
					{
						if (m_WhitePieces.At(i - y))
							break;

						moves.emplace_back(i, -y);

						if (m_BlackPieces.At(i - y))
							break;
					}
					for (int y = 1; y < down + 1; y++)
					{
						if (m_WhitePieces.At(i - y * 8))
							break;

						moves.emplace_back(i, -y * 8);

						if (m_BlackPieces.At(i - y * 8))
							break;
					}
					for (int y = 1; y < right + 1; y++)
					{
						if (m_WhitePieces.At(i + y))
							break;

						moves.emplace_back(i, y);

						if (m_BlackPieces.At(i + y))
							break;
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < 64; i++)
			{
				int yLevel = i / 8;
				int xLevel = i % 8;

				int right = 7 - xLevel;
				int left = xLevel;
				int up = 7 - yLevel;
				int down = yLevel;

				if (blackQueens.At(i))
				{
					for (int y = 1; y < std::min(up, right) + 1; y++)
					{
						if (m_BlackPieces.At(i + y * 9))
							break;

						moves.emplace_back(i, y * 9);

						if (m_WhitePieces.At(i + y * 9))
							break;
					}
					for (int y = 1; y < std::min(up, left) + 1; y++)
					{
						if (m_BlackPieces.At(i + y * 7))
							break;

						moves.emplace_back(i, y * 7);

						if (m_WhitePieces.At(i + y * 7))
							break;
					}
					for (int y = 1; y < std::min(down, left) + 1; y++)
					{
						if (m_BlackPieces.At(i - y * 9))
							break;

						moves.emplace_back(i, -y * 9);

						if (m_WhitePieces.At(i - y * 9))
							break;
					}
					for (int y = 1; y < std::min(down, right) + 1; y++)
					{
						if (m_BlackPieces.At(i - y * 7))
							break;

						moves.emplace_back(i, -y * 7);

						if (m_WhitePieces.At(i - y * 7))
							break;
					}

					for (int y = 1; y < up + 1; y++)
					{
						if (m_BlackPieces.At(i + y * 8))
							break;

						moves.emplace_back(i, y * 8);

						if (m_WhitePieces.At(i + y * 8))
							break;
					}
					for (int y = 1; y < left + 1; y++)
					{
						if (m_BlackPieces.At(i - y))
							break;

						moves.emplace_back(i, -y);

						if (m_WhitePieces.At(i - y))
							break;
					}
					for (int y = 1; y < down + 1; y++)
					{
						if (m_BlackPieces.At(i - y * 8))
							break;

						moves.emplace_back(i, -y * 8);

						if (m_WhitePieces.At(i - y * 8))
							break;
					}
					for (int y = 1; y < right + 1; y++)
					{
						if (m_BlackPieces.At(i + y))
							break;

						moves.emplace_back(i, y);

						if (m_WhitePieces.At(i + y))
							break;
					}
				}
			}
		}		
	}

	void Board::FindKingMoves(std::vector<Move>& moves) const
	{
		if (m_PlayerToPlay == WHITE)
		{
			int i = (int)std::log2(m_mapWhitePieces[KING].Data());

			int yLevel = i / 8;
			int xLevel = i % 8;

			int right = 7 - xLevel;
			int left = xLevel;
			int up = 7 - yLevel;
			int down = yLevel;

			bool ok_right = right >= 1;
			bool ok_left = left >= 1;
			bool ok_up = up >= 1;
			bool ok_down = down >= 1;

			
			if (ok_up)
			{
				if (!m_WhitePieces.At(i + KingMoves[2]))
					moves.emplace_back(i, KingMoves[2]);
				if (ok_left && !m_WhitePieces.At(i + KingMoves[1]))
					moves.emplace_back(i, KingMoves[1]);
				if (ok_right && !m_WhitePieces.At(i + KingMoves[3]))
					moves.emplace_back(i, KingMoves[3]);
			}
			if (ok_down)
			{
				if (!m_WhitePieces.At(i - KingMoves[2]))
					moves.emplace_back(i, -KingMoves[2]);
				if (ok_left && !m_WhitePieces.At(i - KingMoves[3]))
					moves.emplace_back(i, -KingMoves[3]);
				if (ok_right && !m_WhitePieces.At(i - KingMoves[1]))
					moves.emplace_back(i, -KingMoves[1]);
			}
			if (ok_left && !m_WhitePieces.At(i - KingMoves[0]))
				moves.emplace_back(i, -KingMoves[0]);
			if (ok_right && !m_WhitePieces.At(i + KingMoves[0]))
				moves.emplace_back(i, KingMoves[0]);

			if (m_K)
			{
				if (!m_Pieces.At(4 + 1) && !m_Pieces.At(4 + 2) && IsMoveKingSecured({ 4, 0 }) && IsMoveKingSecured({ 4, 1 }))
					moves.emplace_back(4, 2);
			}
			if (m_Q)
			{
				if (!m_Pieces.At(4 - 1) && !m_Pieces.At(4 - 3) && IsMoveKingSecured({ 4, 0 }) && IsMoveKingSecured({ 4, -1 }))
					moves.emplace_back(4, -2);
			}
		}
		else
		{
			int i = (int)std::log2(m_mapBlackPieces[KING].Data());

			int yLevel = i / 8;
			int xLevel = i % 8;

			int right = 7 - xLevel;
			int left = xLevel;
			int up = 7 - yLevel;
			int down = yLevel;

			bool ok_right = right >= 1;
			bool ok_left = left >= 1;
			bool ok_up = up >= 1;
			bool ok_down = down >= 1;

			if (ok_up)
			{
				if (!m_BlackPieces.At(i + KingMoves[2]))
					moves.emplace_back(i, KingMoves[2]);
				if (ok_left && !m_BlackPieces.At(i + KingMoves[1]))
					moves.emplace_back(i, KingMoves[1]);
				if (ok_right && !m_BlackPieces.At(i + KingMoves[3]))
					moves.emplace_back(i, KingMoves[3]);
			}
			if (ok_down)
			{
				if (!m_BlackPieces.At(i - KingMoves[2]))
					moves.emplace_back(i, -KingMoves[2]);
				if (ok_left && !m_BlackPieces.At(i - KingMoves[3]))
					moves.emplace_back(i, -KingMoves[3]);
				if (ok_right && !m_BlackPieces.At(i - KingMoves[1]))
					moves.emplace_back(i, -KingMoves[1]);
			}
			if (ok_left && !m_BlackPieces.At(i - KingMoves[0]))
				moves.emplace_back(i, -KingMoves[0]);
			if (ok_right && !m_BlackPieces.At(i + KingMoves[0]))
					moves.emplace_back(i, KingMoves[0]);

			if (m_k)
			{
				if (!m_Pieces.At(60 + 1) && !m_Pieces.At(60 + 2) && IsMoveKingSecured({ 60, 0 }) && IsMoveKingSecured({ 60, 1 }))
					moves.emplace_back(60, 2);
			}
			if (m_q)
			{
				if (!m_Pieces.At(60 - 1) && !m_Pieces.At(60 - 3) && IsMoveKingSecured({ 60, 0 }) && IsMoveKingSecured({ 60, -1 }))
					moves.emplace_back(60, -2);
			}
		}
	}

	bool Board::IsBoardValid() const
	{
		int allWhitepecies = 0;
		int allBlackpecies = 0;
		
		for (int i = 0; i < 64; ++i)
		{
			if (GetPieceType(i) != NONE)
			{
				if (GetPieceColor(i) == WHITE)
					allWhitepecies += 1;
				else
					allBlackpecies += 1;
			}
		}

		if (allWhitepecies > 16 || allBlackpecies > 16)
			return false;

		int WkingAmount = 0, BkingAmount = 0;

		for (int i = 0; i < 64; ++i)
		{
			if (GetPieceType(i) == KING)
			{
				if (GetPieceColor(i) == WHITE)
					WkingAmount += 1;
				else
					BkingAmount += 1;
			}
		}

		if (WkingAmount != 1 || BkingAmount != 1)
			return false;

		int kingsDistance = std::abs((int)std::log2(m_mapWhitePieces[KING].Data()) - (int)std::log2(m_mapBlackPieces[KING].Data()));

		if (kingsDistance == 1 || kingsDistance == 7 || kingsDistance == 8 || kingsDistance == 9)
			return false;

		UpdateVitualValues();

		if (GetVirtualKingStatus((m_PlayerToPlay == WHITE ? BLACK : WHITE)) != SECURE)
			return false;
		
		return true;
	}
}