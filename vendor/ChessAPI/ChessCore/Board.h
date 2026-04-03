#pragma once

#include "BitBoard.h"
#include "Piece.h"

#include <vector>
#include <unordered_map>
#include <array>

namespace Chess
{
	class Board
	{
	public:
		struct Move
		{
			Move() = default;
			Move(uint8_t otherIndex, MoveDifference otherMove) : index(otherIndex), move(otherMove)	{}

			uint8_t index = 0;
			MoveDifference move = 0;
		};

		enum MakeMoveStatus
		{
			MOVEERROR = 0,
			SUCCESS,
			PROMOTION
		};

		enum KingStatus : uint8_t
		{
			SECURE = 0x00,
			CHECKED = 0x01,
			MATED = 0x02
		};

	public:
		Board();
		~Board() = default;

		std::string GetFen() const;
		std::vector<uint8_t>  GetFormatedFen() const;
		
		bool NewPosition(const std::string& fenStr = "default");
		bool NewPosition(const std::vector<uint8_t>& ffen);

		void GetAvailableMoves(std::vector<Move>& moves) const;
		void GetAvailableMoves(std::vector<Move>& moves, Piece type) const;

		int GetBlackMovesCount() const { return m_BlackMovesCounter; }
		int GetFiftyMoveCount() const { return m_FiftyMoveCounter; }
		int GetLastMoveIndex() const { return m_LastMovedPieceIndex; }
		
		bool GetKRoke() const { return m_K; }
		bool GetQRoke() const { return m_Q; }
		bool GetkRoke() const { return m_k; }
		bool GetqRoke() const { return m_q; }

		Piece GetPieceType(int index) const;
		Piece GetPieceType(int indexX, int indexY) const;

		Color GetPieceColor(int index) const;
		Color GetPieceColor(int indexX, int indexY) const;
		Color GetPlayerToPlayColor() const { return m_PlayerToPlay; }

		KingStatus GetKingStatus() const;

		MakeMoveStatus MakeMove(Move move, Piece piecePromotion = NONE);
		bool IsMoveValid(Move move) const;

		//Be Carefull with these
		//-
		void AddPiece(Piece type, Color color, int index);
		void AddPiece(Piece type, Color color, int indexX, int indexY);

		void RemovePiece(int index);
		void RemovePiece(int indexX, int indexY);
		//-

		void SetBlackMovesCount(int BlackMovesCount) { m_BlackMovesCounter = BlackMovesCount; }
		void SetFiftyMoveCount(int FiftyMoveCount) { m_FiftyMoveCounter = FiftyMoveCount; }
		void SetLastMoveIndex(int LastMoveIndex) { m_LastMovedPieceIndex= LastMoveIndex; }
			 
		void SetKRoke(bool K) { m_K = K; }
		void SetQRoke(bool Q) { m_Q = Q; }
		void SetkRoke(bool k) { m_k = k; }
		void SetqRoke(bool q) { m_q = q; }

		void SwapPlayerToPlay() { m_PlayerToPlay = m_PlayerToPlay == WHITE ? BLACK : WHITE; }

	private:
		void UpdateVitualValues() const;
		MakeMoveStatus VirtualMakeMove(Move move) const;	

		KingStatus GetVirtualKingStatus(Color playerColor) const;

		bool IsMoveKingSecured(Move move) const;

		void FindPawnMoves(std::vector<Move>& moves) const;
		void FindKngihtMoves(std::vector<Move>& moves) const;
		void FindBishopMoves(std::vector<Move>& moves) const;
		void FindRookMoves(std::vector<Move>& moves) const;
		void FindQueenMoves(std::vector<Move>& moves) const;
		void FindKingMoves(std::vector<Move>& moves) const;

		bool IsBoardValid() const;

	private:
		BitBoard				m_Pieces;
		std::array<BitBoard, 6> m_mapPieces;
		BitBoard				m_WhitePieces;
		std::array<BitBoard, 6> m_mapWhitePieces;
		BitBoard				m_BlackPieces;
		std::array<BitBoard, 6> m_mapBlackPieces;
		
		bool m_K = false, m_Q = false, m_k = false, m_q = false;

		int m_LastMovedPieceIndex = -1;

		Color m_PlayerToPlay = WHITE;

		int m_FiftyMoveCounter = 0;
		int m_BlackMovesCounter = 1;

		//virtual:

		mutable BitBoard				m_Virtual_Pieces;
		mutable std::array<BitBoard, 6> m_Virtual_mapPieces;
		mutable BitBoard				m_Virtual_WhitePieces;
		mutable std::array<BitBoard, 6> m_Virtual_mapWhitePieces;
		mutable BitBoard				m_Virtual_BlackPieces;
		mutable std::array<BitBoard, 6> m_Virtual_mapBlackPieces;
	};
}