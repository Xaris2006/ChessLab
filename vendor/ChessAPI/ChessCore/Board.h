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
		uint64_t GetHash(bool recalculate = false) const;
		
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

		int GetAmountOfPieces() const;
		int GetAmountOfPieces(Piece type) const;
		Piece GetPieceType(int index) const;
		Piece GetPieceType(int indexX, int indexY) const;
		BitBoard GetBitBoard(Piece type, Color color) const;

		Color GetPieceColor(int index) const;
		Color GetPieceColor(int indexX, int indexY) const;
		Color GetPlayerToPlayColor() const { return m_PlayerToPlay; }

		KingStatus GetKingStatus() const;

		MakeMoveStatus MakeMove(Move move, Piece piecePromotion = NONE);
		bool IsMoveValid(Move move) const;

		void SetBlackMovesCount(int BlackMovesCount) { m_BlackMovesCounter = BlackMovesCount; }
		void SetFiftyMoveCount(int FiftyMoveCount) { m_FiftyMoveCounter = FiftyMoveCount; }
		void SetLastMoveIndex(int LastMoveIndex);
			 
		void SetKRoke(bool K);
		void SetQRoke(bool Q);
		void SetkRoke(bool k);
		void SetqRoke(bool q);

		void SwapPlayerToPlay();

		//Be Carefull with these
		//-
		void AddPiece(Piece type, Color color, int index);
		void AddPiece(Piece type, Color color, int indexX, int indexY);

		void RemovePiece(int index);
		void RemovePiece(int indexX, int indexY);
		//-

		std::string ConvertUCIMoveToPGNMove(const std::string& uciMove) const;
		std::string ConvertPGNMoveToUCIMove(const std::string& pgnMove) const;

		std::string ConvertCLDMoveToUCIMove(uint16_t cldMove) const;
		uint16_t ConvertUCIMoveToCLDMove(const std::string& uciMove) const;

		//we dont need board var to convert this formats
		static std::string ConvertCLDMoveToPGNMove(uint16_t cldMove);
		static std::string ConvertCLDMoveToPGNMove(std::pair<uint8_t, uint8_t> cldMove);
		//we dont need board var to convert this formats
		static std::pair<uint8_t, uint8_t> ConvertPGNMoveToCLDMove(const std::string& pgnMove);
		
		std::string ConvertCoreMoveToUCIMove(const Board::Move& move, Piece promotedType = NONE) const;
		std::string ConvertCoreMoveToPGNMove(const Board::Move& move, Piece promotedType = NONE) const;
		uint16_t ConvertCoreMoveToCLDMove(const Board::Move& move, Piece promotedType = NONE) const;

		void ConvertUCIMoveToCoreMove(Board::Move& move, Piece& promotedType, const std::string& uciMove) const;
		void ConvertPGNMoveToCoreMove(Board::Move& move, Piece& promotedType, const std::string& pgnMove) const;
		void ConvertCLDMoveToCoreMove(Board::Move& move, Piece& promotedType, uint16_t cldMove) const;

	private:
		bool IsBoardValid() const;

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

	private:
		//BitBoard				m_Pieces;
		//std::array<BitBoard, 6> m_mapPieces;
		BitBoard				m_WhitePieces;
		std::array<BitBoard, 6> m_mapWhitePieces;
		BitBoard				m_BlackPieces;
		std::array<BitBoard, 6> m_mapBlackPieces;
		
		bool m_K = false, m_Q = false, m_k = false, m_q = false;

		int m_LastMovedPieceIndex = -1;

		Color m_PlayerToPlay = WHITE;

		int m_FiftyMoveCounter = 0;
		int m_BlackMovesCounter = 1;

		uint64_t m_Hash = 0;

		std::array<Piece, 64> m_BoardPieces;

		//virtual:

		mutable BitBoard				m_Virtual_Pieces;
		mutable std::array<BitBoard, 6> m_Virtual_mapPieces;
		mutable BitBoard				m_Virtual_WhitePieces;
		mutable std::array<BitBoard, 6> m_Virtual_mapWhitePieces;
		mutable BitBoard				m_Virtual_BlackPieces;
		mutable std::array<BitBoard, 6> m_Virtual_mapBlackPieces;
	};
}