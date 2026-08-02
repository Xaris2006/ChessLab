#include "GameManager.h"

static constexpr std::string_view s_strMoveTypes = " NBRQK";
static constexpr std::string_view s_strMoveTypesSmall = " nbrqk";
static constexpr std::string_view s_strMoveIndexX = "abcdefgh";
static constexpr std::string_view s_strMoveIndexY = "12345678";

namespace Chess
{
	GameManager::~GameManager()
	{
		if (m_pgnGame)
			m_pgnGame->RemoveReference();
	}

	bool GameManager::InitPgnGame(PgnGame& pgnGame)
	{
		if (m_pgnGame)
			m_pgnGame->RemoveReference();

		pgnGame.AddReference();
		m_pgnGame = &pgnGame;

		bool fenLabelExist = m_pgnGame->IsLabelExist("FEN");
		bool ret = true;

		if (fenLabelExist)
		{
			ret = m_Board.NewPosition((*m_pgnGame)["FEN"]);

			if (!ret)
				(*m_pgnGame)["FEN"] = m_Board.GetFen();
		}
		else
			ret  = m_Board.NewPosition();

		m_mapMoves.clear();
		m_lastMoveKey.clear();

		m_lastMoveKey.emplace_back(-1);

		return ret;
	}

	void GameManager::Clear()
	{
		if (m_pgnGame)
			m_pgnGame->RemoveReference();

		m_Board.NewPosition();

		m_pgnGame = nullptr;
		m_mapMoves.clear();
		m_lastMoveKey.clear();
	}

	void GameManager::GetFen(std::string& fen) const
	{
		fen = m_Board.GetFen();
	}

	std::string GameManager::GetFen() const
	{
		return m_Board.GetFen();
	}

	std::vector<uint8_t> GameManager::GetFormatedFEN() const
	{
		return m_Board.GetFormatedFen();
	}

	Color GameManager::GetPlayerToPlay() const
	{
		return m_Board.GetPlayerToPlayColor();
	}

	GameManager::PieceID GameManager::GetPieceID(int index) const
	{
		return { m_Board.GetPieceType(index), m_Board.GetPieceColor(index) };
	}

	GameManager::PieceID GameManager::GetPieceID(int indexX, int indexY) const
	{
		return { m_Board.GetPieceType(indexX, indexY), m_Board.GetPieceColor(indexX, indexY) };
	}

	PgnGame::Detail& GameManager::GetNote(const MoveKey& moveKey)
	{
		if (!m_pgnGame || moveKey.empty())
			return *(PgnGame::Detail*)(void*)nullptr;

		auto currentPath = &m_pgnGame->GetMovePathbyRef();

		for (int i = 1; i < moveKey.size(); i += 2)
		{
			currentPath = &currentPath->children[moveKey[i]];
		}

		return currentPath->details[moveKey[moveKey.size() - 1]];
	}

	PgnGame::ChessMovesPath& GameManager::GetCurrentPgnMovePath()
	{
		if (!m_pgnGame)
			return *(PgnGame::ChessMovesPath*)nullptr;

		PgnGame::ChessMovesPath* path = &m_pgnGame->GetMovePathbyRef();

		for (int i = 1; i < m_lastMoveKey.size(); i += 2)
		{
			path = &path->children[m_lastMoveKey[i]];
		}

		return *path;
	}

	PgnGame::ChessMovesPath GameManager::GetMovesByStr() const
	{
		if(m_pgnGame)
			return m_pgnGame->GetMovePathbyCopy();
		return {};
	}

	GameManager::MoveKey GameManager::GetLastMoveKey() const
	{
		return m_lastMoveKey;
	}

	std::pair<Board::Move, Piece> GameManager::GetLastMove() const
	{
		if (!m_mapMoves.contains(m_lastMoveKey))
			return std::make_pair(Board::Move(), NONE);

		const auto& moveD = m_mapMoves.at(m_lastMoveKey);

		return std::make_pair(moveD.move, moveD.piecePromote);
	}

	void GameManager::GetAvailableMoves(std::vector<Board::Move>& moves) const
	{
		m_Board.GetAvailableMoves(moves);
	}

	bool GameManager::IsMoveValid(Board::Move move) const
	{
		return m_Board.IsMoveValid(move);
	}

	Board::MakeMoveStatus GameManager::MakeMove(Board::Move move, Piece	promotedType)
	{
		if (!m_Board.IsMoveValid(move))
			return Board::MOVEERROR;

		MoveData moveD;
		ConvertCoreMoveToMoveData(move, moveD, promotedType);

		if(moveD.pieceToMove == NONE)
			return Board::MOVEERROR;

		std::string strmove;
		ConvertMoveDataToPGNMove(moveD, strmove);

		auto ret = m_Board.MakeMove(move, promotedType);

		if(ret == Board::SUCCESS)
			AddMove(strmove, moveD);

		return ret;
	}

	Board::MakeMoveStatus GameManager::MakeMove(const std::string& move)
	{
		if(move.empty())
			return Board::MOVEERROR;

		MoveData moveD;
		ConvertPGNMoveToMoveData(move, moveD);

		if (!m_Board.IsMoveValid(moveD.move) || moveD.pieceToMove == NONE)
			return Board::MOVEERROR;
		
		auto ret = m_Board.MakeMove(moveD.move, moveD.piecePromote);

		if(ret == Board::SUCCESS)
			AddMove(move, moveD);

		return ret;
	}

	void GameManager::GoNextMove()
	{
		PgnGame::ChessMovesPath* currentPath = nullptr;
		GetCurrentPgnMovePath(currentPath);

		bool nextExists = false;

		int nextIndex = -1;

		for (int i = m_lastMoveKey[m_lastMoveKey.size() - 1] + 1; i < currentPath->move.size(); i++)
		{
			if (currentPath->move[i] != "child")
			{
				nextExists = true;
				nextIndex = i;
				break;
			}
		}

		if (!nextExists)
			return;
		
		int oldValue = m_lastMoveKey[m_lastMoveKey.size() - 1];

		m_lastMoveKey[m_lastMoveKey.size() - 1] = nextIndex;

		if (!m_mapMoves.contains(m_lastMoveKey))
		{
			MoveData moveD;
			ConvertPGNMoveToMoveData(currentPath->move[nextIndex], moveD);
			
			if (m_Board.MakeMove(moveD.move, moveD.piecePromote) != Board::SUCCESS)
			{
				m_lastMoveKey[m_lastMoveKey.size() - 1] = oldValue;
				return;
			}
			
			m_mapMoves[m_lastMoveKey] = moveD;
		}
		else
		{
			if (m_Board.MakeMove(m_mapMoves[m_lastMoveKey].move, m_mapMoves[m_lastMoveKey].piecePromote) != Board::SUCCESS)
			{
				m_lastMoveKey[m_lastMoveKey.size() - 1] = oldValue;
				GoInitialPosition();
				m_mapMoves.clear();
			}
		}
	}

	void GameManager::GoPreviusMove()
	{
		if (!m_mapMoves.contains(m_lastMoveKey))
			return;//Something Bad Happened here

		MoveData moveD = m_mapMoves[m_lastMoveKey];

		if(m_Board.GetPlayerToPlayColor() == WHITE)
			m_Board.SetBlackMovesCount(m_Board.GetBlackMovesCount() - 1);

		m_Board.SetFiftyMoveCount(moveD.FiftyMoveCounter);
		m_Board.SetLastMoveIndex(moveD.lastMoveIndex);

		m_Board.SetKRoke(moveD.K);
		m_Board.SetQRoke(moveD.Q);
		m_Board.SetkRoke(moveD.k);
		m_Board.SetqRoke(moveD.q);
		
		m_Board.RemovePiece(moveD.move.index + moveD.move.move);

		if (moveD.pieceToMove == PAWN && moveD.move.index + moveD.move.move == moveD.lastMoveIndex)
			m_Board.AddPiece(PAWN, m_Board.GetPlayerToPlayColor(), moveD.move.index + moveD.move.move + (m_Board.GetPlayerToPlayColor() == BLACK ? -8 : +8));
		else if (moveD.pieceOnDirection != NONE)
			m_Board.AddPiece(moveD.pieceOnDirection, m_Board.GetPlayerToPlayColor(), moveD.move.index + moveD.move.move);

		m_Board.SwapPlayerToPlay();

		m_Board.AddPiece(moveD.pieceToMove, m_Board.GetPlayerToPlayColor(), moveD.move.index);

		if (moveD.pieceToMove == KING && std::abs(moveD.move.move) == 2)
		{
			m_Board.RemovePiece(moveD.move.index + moveD.move.move/2);
			m_Board.AddPiece(ROOK, m_Board.GetPlayerToPlayColor(), moveD.move.index + (moveD.move.move > 0 ? +3 : -4));
		}

		PgnGame::ChessMovesPath* currentPath = nullptr;
		GetCurrentPgnMovePath(currentPath);

		do
		{
			m_lastMoveKey[m_lastMoveKey.size() - 1]--;

			if (m_lastMoveKey[0] == -1)
				return;
			if (m_lastMoveKey[m_lastMoveKey.size() - 1] == -1)
				break;

		} while (currentPath->move[m_lastMoveKey[m_lastMoveKey.size() - 1]] == "child");

		if (m_lastMoveKey[m_lastMoveKey.size() - 1] != -1)
			return;

		//m_lastMoveKey.erase(m_lastMoveKey.end() - 1);
		//m_lastMoveKey.erase(m_lastMoveKey.end() - 1);
		m_lastMoveKey.pop_back();
		m_lastMoveKey.pop_back();

		GetCurrentPgnMovePath(currentPath);

		if (m_lastMoveKey[m_lastMoveKey.size() - 1] < 0)
		{
			m_lastMoveKey[m_lastMoveKey.size() - 1] = -1;
			return;
		}

		do
		{
			m_lastMoveKey[m_lastMoveKey.size() - 1]--;

		} while (m_lastMoveKey[m_lastMoveKey.size() - 1] > -1 && currentPath->move[m_lastMoveKey[m_lastMoveKey.size() - 1]] == "child");

		if (m_lastMoveKey[m_lastMoveKey.size() - 1] < 0)
		{
			m_lastMoveKey[m_lastMoveKey.size() - 1] = -1;
			return;
		}

		do
		{
			m_lastMoveKey[m_lastMoveKey.size() - 1]--;

		} while (m_lastMoveKey[m_lastMoveKey.size() - 1] > -1 && currentPath->move[m_lastMoveKey[m_lastMoveKey.size() - 1]] == "child");
	}

	void GameManager::GoInitialPosition()
	{
		while (m_lastMoveKey[0] != -1)
			GoPreviusMove();
	}

	void GameManager::GoToPositionByKey(const MoveKey& moveKey)
	{
		GoInitialPosition();
		
		PgnGame::ChessMovesPath* currentPath = &m_pgnGame->GetMovePathbyRef();

		for (int i = 0; i < moveKey.size(); i++)
		{
			if (i % 2 == 1)
			{
				GoPreviusMove();
				currentPath = &currentPath->children[moveKey[i]];
			}
			else
			{
				for (int j = 0; j < moveKey[i] + 1; j++)
				{
					if (currentPath->move[j] != "child")
					{
						if (MakeMove(currentPath->move[j]) != Board::SUCCESS)
							return;
					}
				}
			}
		}
	}

	void GameManager::EditVariation(const MoveKey& moveKey, VariationEdit editType)
	{
		std::vector<MoveData> oldMoves;
		
		while (m_lastMoveKey[0] != -1)
		{
			if (m_mapMoves.contains(m_lastMoveKey))
				oldMoves.emplace_back(m_mapMoves[m_lastMoveKey]);
			else
			{
				//something bad happened

				return;
			}

			GoPreviusMove();
		}

		m_mapMoves.clear();

		PgnGame::ChessMovesPath* currentPath = &m_pgnGame->GetMovePathbyRef();

		for (int i = 1; i < moveKey.size(); i += 2)
		{
			currentPath = &currentPath->children[moveKey[i]];
		}

		switch (editType)
		{
		case Chess::GameManager::SWAP:
		{
			if (moveKey.size() == 1)//has parent
				break;

			std::unordered_map<size_t, PgnGame::Detail> detailsNew;
			std::vector<std::string> moveNew;
			std::vector<PgnGame::ChessMovesPath> childrenNew;

			int amountOfChildrenInside = 0;
			int oldParentChildrenStay = 0;
			int oldParentStart = 0;
			int oldParentNextMove = -1;

			for (int i = moveKey[moveKey.size() - 3] - 1; i > 0; --i)
			{
				if (currentPath->parent->move[i] != "child")
				{
					oldParentStart = i;
					break;
				}
			}

			for (int i = oldParentStart + 1; i < currentPath->parent->move.size(); ++i)
			{
				bool child = currentPath->parent->move[i] == "child";

				if (child)
					amountOfChildrenInside++;
				else if (oldParentNextMove == -1)
					oldParentNextMove = i;

				if (child && oldParentNextMove == -1)
					oldParentChildrenStay++;
			}
			
			//copy parent moves
			moveNew.emplace_back(currentPath->parent->move[oldParentStart]);
			if (currentPath->parent->details.contains(oldParentStart))
			{
				detailsNew[0] = currentPath->parent->details[oldParentStart];
				currentPath->parent->details.erase(oldParentStart);
			}
			for (int i = oldParentNextMove; i < currentPath->parent->move.size(); ++i)
			{
				moveNew.emplace_back(currentPath->parent->move[i]);
				if (currentPath->parent->details.contains(i))
				{
					detailsNew[i - oldParentNextMove + 1] = currentPath->parent->details[i];
					currentPath->parent->details.erase(i);
				}
			}

			//delete old parent moves
			if(oldParentNextMove != -1)
				currentPath->parent->move.resize(oldParentNextMove);
			currentPath->parent->move[oldParentStart] = currentPath->move[0];

			if (currentPath->details.contains(0))
				currentPath->parent->details[oldParentStart] = currentPath->details[0];

			//copy current moves to parent
			for (int i = 1; i < currentPath->move.size(); ++i)
			{
				currentPath->parent->move.emplace_back(currentPath->move[i]);
				if (currentPath->details.contains(i))
					currentPath->parent->details[currentPath->parent->move.size() - 1] = currentPath->details[i];
			}

			//copy parent children
			for (int i = currentPath->parent->children.size() - amountOfChildrenInside + oldParentChildrenStay; 
					 i < currentPath->parent->children.size(); ++i)
			{
				childrenNew.emplace_back(currentPath->parent->children[i]);
			}

			//delete old parent children
			currentPath->parent->children.resize(currentPath->parent->children.size() - amountOfChildrenInside + oldParentChildrenStay);

			auto childrenToCopy = currentPath->children;
			auto parent = currentPath->parent;

			currentPath->children = childrenNew;
			currentPath->details = detailsNew;
			currentPath->move = moveNew;

			for (auto& c : childrenToCopy)
				parent->children.emplace_back(c);

			m_pgnGame->GetMovePathbyRef().ReloadChildren();
			m_pgnGame->GetMovePathbyRef().parent = nullptr;			

			break;
		}
		case Chess::GameManager::SWAP_MAIN:
		{

			break;
		}
		case Chess::GameManager::INSIDE_UP:
		{

			break;
		}
		case Chess::GameManager::INSIDE_DOWN:
		{

			break;
		}
		case Chess::GameManager::INSIDE_MAX:
		{

			break;
		}
		case Chess::GameManager::INSIDE_MIN:
		{

			break;
		}
		default:
			break;
		}

		//Go back to our original moveKey;

		for (int i = oldMoves.size() - 1; i >= 0; --i)
		{
			if (MakeMove(oldMoves[i].move, oldMoves[i].piecePromote) != Board::SUCCESS)
			{
				//Something bad happened
				return;
			}
		}
	}

	void GameManager::DeleteMove(const MoveKey& moveKey)
	{
		GoInitialPosition();

		PgnGame::ChessMovesPath* currentPath = &m_pgnGame->GetMovePathbyRef();

		for (int i = 1; i < moveKey.size(); i += 2)
		{
			currentPath = &currentPath->children[moveKey[i]];
		}

		if (currentPath->move.size() > moveKey[moveKey.size() - 1] + 1
			&& currentPath->move[moveKey[moveKey.size() - 1] + 1] == "child")
		{
			int chindIndex = 0;
			auto moveKeyNew = moveKey;

			for (int i = 0; i < moveKey[moveKey.size() - 1]; ++i)
			{
				if (currentPath->move[i] == "child")
					chindIndex++;
			}

			moveKeyNew[moveKeyNew.size() - 1]++;
			moveKeyNew.emplace_back(chindIndex);
			moveKeyNew.emplace_back(0);

			EditVariation(moveKeyNew, SWAP);
			DeleteMove(moveKeyNew);

			return;
		}

		currentPath->move.resize(moveKey[moveKey.size() - 1]);

		if (currentPath->move.empty() && currentPath->parent)
		{
			auto Parent = currentPath->parent;
			Parent->children.erase(Parent->children.begin() + moveKey[moveKey.size() - 2]);
			Parent->ReloadChildren();
			Parent->move.erase(Parent->move.begin() + moveKey[moveKey.size() - 3]);
		}

		m_mapMoves.clear();
	}

	void GameManager::ConvertMoveDataToPGNMove(const MoveData& move, std::string& strmove) const
	{
		strmove = m_Board.ConvertCoreMoveToPGNMove(move.move, move.piecePromote);
	}

	void GameManager::ConvertPGNMoveToMoveData(const std::string& strmove, MoveData& move) const
	{
		m_Board.ConvertPGNMoveToCoreMove(move.move, move.piecePromote, strmove);

	
		move.FiftyMoveCounter = m_Board.GetFiftyMoveCount();
		move.lastMoveIndex = m_Board.GetLastMoveIndex();

		move.K = m_Board.GetKRoke();
		move.Q = m_Board.GetQRoke();
		move.k = m_Board.GetkRoke();
		move.q = m_Board.GetqRoke();

		move.pieceToMove = m_Board.GetPieceType(move.move.index);
		move.pieceOnDirection = m_Board.GetPieceType(move.move.index + move.move.move);
	}

	void GameManager::ConvertCoreMoveToMoveData(const Board::Move& move, MoveData& moveData, Piece promotedType) const
	{
		moveData.pieceToMove = m_Board.GetPieceType(move.index);
		moveData.pieceOnDirection = m_Board.GetPieceType(move.index + move.move);

		moveData.FiftyMoveCounter = m_Board.GetFiftyMoveCount();
		moveData.lastMoveIndex = m_Board.GetLastMoveIndex();
		moveData.piecePromote = promotedType;

		moveData.K = m_Board.GetKRoke();
		moveData.Q = m_Board.GetQRoke();
		moveData.k = m_Board.GetkRoke();
		moveData.q = m_Board.GetqRoke();

		moveData.move = move;

		if (moveData.pieceToMove == PAWN && moveData.move.index + moveData.move.move == moveData.lastMoveIndex)
			moveData.pieceOnDirection = PAWN;
	}

	void GameManager::GetCurrentPgnMovePath(PgnGame::ChessMovesPath*& path) const
	{
		path = &m_pgnGame->GetMovePathbyRef();

		for (int i = 1; i < m_lastMoveKey.size(); i += 2)
		{
			path = &path->children[m_lastMoveKey[i]];
		}
	}

	void GameManager::AddMove(const std::string& strmove, const MoveData& move)
	{
		PgnGame::ChessMovesPath* currentPath = &m_pgnGame->GetMovePathbyRef();
		
		for (int i = 1; i < m_lastMoveKey.size(); i += 2)
		{
			currentPath = &currentPath->children[m_lastMoveKey[i]];
		}

		bool nextExists = false;

		int nextIndex = -1;

		for (int i = m_lastMoveKey[m_lastMoveKey.size() - 1] + 1; i < currentPath->move.size(); i++)
		{
			if (currentPath->move[i] != "child")
			{
				nextExists = true;
				nextIndex = i;
				break;
			}
		}

		if (!nextExists)
		{
			currentPath->move.emplace_back(strmove);
			m_lastMoveKey[m_lastMoveKey.size() - 1] = currentPath->move.size() - 1;

			m_mapMoves[m_lastMoveKey] = move;

			return;
		}

		bool smallRoke = move.pieceToMove == KING && move.move.move == 2;

		if (currentPath->move[nextIndex].find(strmove) == 0 
			&& (!smallRoke 
				|| (smallRoke 
					&& currentPath->move[nextIndex].find("0-0-0") == std::string::npos
					&& currentPath->move[nextIndex].find("O-O-O") == std::string::npos)))
		{
			m_lastMoveKey[m_lastMoveKey.size() - 1] = nextIndex;

			m_mapMoves[m_lastMoveKey] = move;

			return;
		}

		int childIndex = 0;

		for (int i = 0; i < nextIndex; i++)
		{
			if (currentPath->move[i] == "child")
				childIndex++;
		}

		int childAmount = 0;

		for (int i = nextIndex + 1; i < currentPath->move.size(); i++)
		{
			if (currentPath->move[i] != "child")
				break;
			else
				childAmount++;
		}

		for (int i = 0; i < childAmount; i++)
		{
			if (currentPath->children[childIndex + i].move[0].find(strmove) == 0 
				&& (!smallRoke
					|| (smallRoke
						&& currentPath->children[childIndex + i].move[0].find("0-0-0") == std::string::npos
						&& currentPath->children[childIndex + i].move[0].find("O-O-O") == std::string::npos)))
			{
				m_lastMoveKey[m_lastMoveKey.size() - 1] = nextIndex + 1 + i;
				m_lastMoveKey.emplace_back(childIndex + i);
				m_lastMoveKey.emplace_back(0);

				m_mapMoves[m_lastMoveKey] = move;

				return;
			}
		}

		currentPath->move.insert(currentPath->move.begin() + nextIndex + 1, "child");
		currentPath->children.insert(currentPath->children.begin() + childIndex + childAmount, PgnGame::ChessMovesPath());
		currentPath->ReloadChildren();
		currentPath->children[childIndex + childAmount].move.emplace_back(strmove);
		//currentPath->children[childIndex + childAmount].parent = currentPath;

		m_lastMoveKey[m_lastMoveKey.size() - 1] = nextIndex + childAmount + 1;
		m_lastMoveKey.emplace_back(childIndex + childAmount);
		m_lastMoveKey.emplace_back(0);

		m_mapMoves[m_lastMoveKey] = move;

		MoveKey toGo = m_lastMoveKey;

		GoInitialPosition();

		m_mapMoves.clear();

		GoToPositionByKey(toGo);
	}
}