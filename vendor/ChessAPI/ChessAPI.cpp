#include "ChessAPI.h"

#include "fstream"

static Chess::PgnFile* s_PngFile;
static std::string s_PngPath;
static std::string s_PngName;

static std::unordered_map<size_t, Chess::GameManager> s_Games;
static std::unordered_map<size_t, std::vector<int>> s_MovePathIntFormat;

static size_t s_ActiveGame = 0;
static std::vector<int> s_OpenGames;


static std::string s_PieceName[7];
static std::array<int, 64> s_OldMovePath;

static bool m_IsLastPointedPanelNotes = false;

static Chess::Board::Move s_movePromotion;

namespace ChessAPI
{
	void Init()
	{
		s_movePromotion.index = -1;

		s_PieceName[0] = "";
		s_PieceName[1] = "";
		s_PieceName[2] = "K";
		s_PieceName[3] = "B";
		s_PieceName[4] = "R";
		s_PieceName[5] = "Q";
		s_PieceName[6] = "K";

		s_PngFile = new Chess::PgnFile();
		s_PngFile->CreateGame();

		s_PngPath = "";
		s_PngName = "New Game";

		s_ActiveGame = 0;
		s_OpenGames.emplace_back(0);

		s_Games[0].InitPgnGame(s_PngFile->operator[](0));
		s_MovePathIntFormat[0] = s_Games[0].GetLastMoveKey();
	}

	int GetActiveGame()
	{
		return s_ActiveGame;
	}

	std::vector<int>& GetOpenGames()
	{
		return s_OpenGames;
	}

	bool IsGameOpen(size_t index)
	{
		return s_Games.contains(index);
	}

	void CloseOpenGame(int index)
	{
		s_Games.erase(s_OpenGames[index]);
		s_MovePathIntFormat.erase(s_OpenGames[index]);

		s_OpenGames.erase(s_OpenGames.begin() + index);
	}

	Chess::PgnFile* GetPgnFile()
	{
		return s_PngFile;
	}
	Chess::PgnGame* GetPgnGame()
	{
		return &s_Games[s_ActiveGame].GetPgnGame();
	}

	std::string GetPieceName(int type)
	{
		return s_PieceName[type];
	}
	std::string& GetPgnFileName()
	{
		return s_PngName;
	}
	std::string& GetPgnFilePath()
	{
		return s_PngPath;
	}
	void GetMovesPgnFormat(Chess::PgnGame::ChessMovesPath& moves)
	{
		moves = s_Games[s_ActiveGame].GetMovesByStr();
	}
	Chess::PgnGame::Detail& GetNote(const Chess::GameManager::MoveKey& pathmove)
	{
		return s_Games[s_ActiveGame].GetNote(pathmove);
	}
	Chess::GameManager::MoveKey& GetMoveIntFormat()
	{
		return s_MovePathIntFormat[s_ActiveGame];
	}

	std::string GetFEN()
	{
		return s_Games[s_ActiveGame].GetFen();
	}
	int GetBlockID(int BlockIndex)
	{
		auto id = s_Games[s_ActiveGame].GetPieceID(BlockIndex);

		int ret = (int)id.type + (id.color == Chess::WHITE ? 0 : 1) * 6 + 1;

		return id.type != Chess::NONE ? ret : 0;
	}
	std::vector<std::string> GetCurPgnLabelNames()
	{
		return s_Games[s_ActiveGame].GetPgnGame().GetLabelNames();
	}
	std::string& GetCurPgnLabelValue(const std::string& name)
	{
		return s_Games[s_ActiveGame].GetPgnGame()[name];
	}
	bool GetPlayerColor()
	{
		return s_Games[s_ActiveGame].GetPlayerToPlay();
	}

	void GetPossibleDirections(int pos, std::vector<Chess::Board::Move>& moves)
	{
		moves.clear();

		s_Games[s_ActiveGame].GetAvailableMoves(moves);

		for (int i = 0; i < moves.size(); ++i)
		{
			if (pos != moves[i].index)
			{
				moves.erase(moves.begin() + i);
				i--;
			}
		}
	}
	Chess::OpeningBook::PositionID GetFormatedPosition()
	{
		return s_Games[s_ActiveGame].GetFormatedFEN();
	}

	void SetNotePanelPointed(const bool& ispointed)
	{
		m_IsLastPointedPanelNotes = ispointed;
	}

	void OpenChessFile(const std::string& path)
	{
		s_movePromotion.index = -1;

		s_Games.clear();
		s_MovePathIntFormat.clear();
		s_OpenGames.clear();

		if (path == "")
		{
			//empty game
			s_PngFile->Clear();
			s_PngFile->CreateGame();

			s_PngPath = "";
			s_PngName = "New Game";

			s_ActiveGame = 0;
			s_OpenGames.emplace_back(0);

			s_Games[0].Clear();

			s_Games[0].InitPgnGame(s_PngFile->operator[](0));
			s_MovePathIntFormat[0] = s_Games[0].GetLastMoveKey();
			return;
		}

		s_PngFile->Clear();

		s_PngPath = path;
		//std::ifstream infile(path, std::ios::binary);
		//infile >> s_PngFile;
		//infile.close();
		s_PngFile->OpenFile(s_PngPath);

		size_t index = path.find_last_of("\\") + 1;
		s_PngName = path.substr(index);

		if (!s_PngFile->GetSize())
			s_PngFile->CreateGame();

		s_ActiveGame = 0;
		s_OpenGames.emplace_back(0);

		s_Games[0].Clear();

		s_Games[0].InitPgnGame(s_PngFile->operator[](0));
		s_MovePathIntFormat[0] = s_Games[0].GetLastMoveKey();
	}

	void OpenChessGameInFile(int index)
	{
		if (index >= s_PngFile->GetSize() || index < 0)
			return;

		s_movePromotion.index = -1;

		s_ActiveGame = index;
		bool alreadyOpen = false;
		for (int i = 0; i < s_OpenGames.size(); i++)
		{
			if (s_OpenGames[i] == index)
				alreadyOpen = true;
		}
		if(!alreadyOpen)
			s_OpenGames.emplace_back(index);
		s_Games[s_ActiveGame].Clear();

		s_Games[s_ActiveGame].InitPgnGame(s_PngFile->operator[](s_ActiveGame));

		if(alreadyOpen)
			GoMoveByIntFormat(s_MovePathIntFormat[s_ActiveGame]);
		else
			s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetLastMoveKey();
	}

	void OverWriteChessFile(const std::string& filepath)
	{
		s_movePromotion.index = -1;

		if (filepath == "")
		{
			if (s_PngPath == "")
			{
				auto& pgngame = s_Games[0].GetPgnGame();
				s_Games.clear();
				s_MovePathIntFormat.clear();
				s_Games[0].Clear();
				//s_Games[0].InitPgnGame(pgngame);
				s_MovePathIntFormat[0] = s_Games[0].GetLastMoveKey();
				return;
			}

			s_PngFile->SaveFile(s_PngPath);

			//std::ofstream outfile(s_PngPath);
			//outfile << s_PngFile;
			//outfile.close();

			return;
		}

		s_PngPath = filepath;

		s_PngFile->SaveFile(s_PngPath);

		//std::ofstream outfile(filepath);
		//outfile << s_PngFile;
		//outfile.close();
	}

	void NewGameInFile()
	{
		s_movePromotion.index = -1;

		s_PngFile->CreateGame();
		s_ActiveGame = s_PngFile->GetSize() - 1;
		s_OpenGames.emplace_back(s_ActiveGame);
		s_Games[s_ActiveGame].Clear();

		s_Games[s_ActiveGame].InitPgnGame(s_PngFile->operator[](s_ActiveGame));
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetLastMoveKey();
		
	}

	void DeleteGamesInFile()
	{
		return;

		s_movePromotion.index = -1;

		for (auto& index : s_PngFile->GetDeletedGames())
		{
			s_Games.erase(index);
			s_MovePathIntFormat.erase(index);

			for (int i = 0; i < s_OpenGames.size(); i++)
			{
				if (s_OpenGames[i] == index)
				{
					s_OpenGames.erase(s_OpenGames.begin() + i);
					break;
				}
			}

			if (!s_PngFile->GetSize())
			{
				s_PngFile->CreateGame();

				s_ActiveGame = 0;
				s_OpenGames.emplace_back(0);

				s_Games[0].Clear();
				s_Games[0].InitPgnGame(s_PngFile->operator[](0));
				s_MovePathIntFormat[0] = s_Games[0].GetLastMoveKey();

				return;
			}

			if (s_OpenGames.empty())
			{
				s_ActiveGame = 0;
				s_OpenGames.emplace_back(0);

				s_Games[0].Clear();
				s_Games[0].InitPgnGame(s_PngFile->operator[](0));
				s_MovePathIntFormat[0] = s_Games[0].GetLastMoveKey();

				return;
			}

			for (auto& game : s_OpenGames)
			{
				if (game > index)
					game--;
			}
			if (s_ActiveGame > index)
				s_ActiveGame--;

			if (s_ActiveGame == index)
			{
				s_ActiveGame = s_OpenGames[0];

				s_Games[s_ActiveGame].Clear();
				s_Games[s_ActiveGame].InitPgnGame(s_PngFile->operator[](s_ActiveGame));
				s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetLastMoveKey();
			}
		}

		s_PngFile->SaveFile(s_PngPath);

		OpenChessFile(s_PngPath);
	}

	void SetNewPieceType(int type)
	{
		s_Games[s_ActiveGame].MakeMove(s_movePromotion, (Chess::Piece)(type - 1));
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetLastMoveKey();
		s_movePromotion.index = -1;
	}
	void GoMoveByIntFormat(Chess::GameManager::MoveKey& pathmoves)
	{
		s_movePromotion.index = -1;

		s_Games[s_ActiveGame].GoToPositionByKey(pathmoves);
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetLastMoveKey();
	}
	void GoMoveByStr(const std::string& movename)
	{
		s_movePromotion.index = -1;

		s_Games[s_ActiveGame].MakeMove(movename);
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetLastMoveKey();
	}
	void NextSavedMove()
	{
		s_movePromotion.index = -1;

		s_Games[s_ActiveGame].GoNextMove();
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetLastMoveKey();
	}
	void PreviousSavedMove()
	{
		s_movePromotion.index = -1;

		s_Games[s_ActiveGame].GoPreviusMove();
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetLastMoveKey();
	}
	
	bool IsMoveValid(glm::vec2 pos, glm::vec2 dir)
	{
		Chess::Board::Move move;
		move.index = pos.x + pos.y * 8;
		move.move = (dir - pos).x + (dir - pos).y * 8;

		auto retval = s_Games[s_ActiveGame].IsMoveValid(move);

		//if (retval == Chess::Board::PROMOTION)
		//	s_movePromotion = move;

		return retval == Chess::Board::SUCCESS;
	}

	bool MakeMove(glm::vec2 pos, glm::vec2 dir)
	{
		s_movePromotion.index = -1;

		Chess::Board::Move move;
		move.index = pos.x + pos.y * 8;
		move.move = (dir - pos).x + (dir - pos).y * 8;

		auto retval = s_Games[s_ActiveGame].MakeMove(move);

		if (retval == Chess::Board::PROMOTION)
			s_movePromotion = move;

		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetLastMoveKey();
		return retval == Chess::Board::SUCCESS;
	}

	bool IsBoardChanged()
	{
		bool output = false;
		for (int i = 0; i < 64; i++)
		{
			int id = GetBlockID(i);
			if (s_OldMovePath[i] != id)
			{
				s_OldMovePath[i] = id;
				output = true;
			}
		}
		return output;
	}

	void DeleteMove(const Chess::GameManager::MoveKey& movepath)
	{
		s_Games[s_ActiveGame].DeleteMove(movepath);
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetLastMoveKey();
	}
	void DeleteVariation(const Chess::GameManager::MoveKey& movepath)
	{
		Chess::GameManager::MoveKey fixedMovepath = movepath;
		fixedMovepath[fixedMovepath.size() - 1] = 0;

		s_Games[s_ActiveGame].DeleteMove(fixedMovepath);
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetLastMoveKey();
	}
	void  PromoteVariation(const Chess::GameManager::MoveKey& movepath)
	{
		s_Games[s_ActiveGame].EditVariation(movepath, Chess::GameManager::SWAP);
		s_MovePathIntFormat[s_ActiveGame] = s_Games[s_ActiveGame].GetLastMoveKey();
	}

	bool IsWaitingForNewType()
	{
		return s_movePromotion.index != -1;
	}
}