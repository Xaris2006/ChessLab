#include "ChessAPI.h"

#include "ChessCore/FileFormats/base/ChessFile.h"
#include "ChessCore/FileFormats/pgn/PgnFile.h"
#include "ChessCore/FileFormats/cld/CldFile.h"

#include "ChessCore/FileFormats/cld/CldGame.h"

#include "../ChessLabUtils.h"
#include "../AppManagerChild.h"

#include <fstream>

static Chess::ChessFile* s_ChessFile;
static std::filesystem::path s_FilePath;
static std::string s_FileName;

static std::shared_ptr<Chess::ClrFile> s_ClrFile;

static std::unordered_map<int64_t, Chess::GameManager> s_Games;

static int64_t s_ActiveGame = 0;
static std::vector<int> s_OpenGames;

namespace ChessAPI
{
	void Init()
	{
		s_ChessFile = new Chess::PgnFile();
		s_ChessFile->CreateGame();

		s_FilePath = "";
		s_FileName = "New Game";

		s_ActiveGame = 0;
		s_OpenGames.emplace_back(0);

		s_Games[0].InitPgnGame(s_ChessFile->operator[](0));
	}

	void ShareClrFile(std::shared_ptr<Chess::ClrFile> file)
	{
		s_ClrFile = file;
	}

	int GetActiveGameIndex()
	{
		return s_ActiveGame;
	}

	std::vector<int>& GetOpenGameIndexes()
	{
		return s_OpenGames;
	}

	bool IsGameOpen(int index)
	{
		return s_Games.contains(index);
	}

	void OpenChessGameInFile(int index)
	{
		if (!s_Games.contains(index))
		{			
			if (index >= 0)
			{
				if (index >= s_ChessFile->GetSize())
					return;

				s_OpenGames.emplace_back(index);
				s_ActiveGame = index;
				s_Games[s_ActiveGame].InitPgnGame(s_ChessFile->operator[](s_ActiveGame));
			}
			else
			{
				if (index < -1 * s_ClrFile->GetSize())
					return;

				s_OpenGames.emplace_back(index);
				s_ActiveGame = index;
				auto& game = s_ClrFile->operator[](s_ActiveGame * -1 - 1);
				s_Games[s_ActiveGame].InitPgnGame(game);
			}
		}
		else
			s_ActiveGame = index;
	}

	void NewGameInFile()
	{
		s_ChessFile->CreateGame();
		s_ActiveGame = s_ChessFile->GetSize() - 1;
		s_OpenGames.emplace_back(s_ActiveGame);
		s_Games[s_ActiveGame].Clear();

		s_Games[s_ActiveGame].InitPgnGame(s_ChessFile->operator[](s_ActiveGame));
	}

	void CloseOpenGame(int index)
	{
		bool found = false;

		for(int i = 0; i < s_OpenGames.size(); i++)
		{
			if (s_OpenGames[i] == index)
			{
				s_OpenGames.erase(s_OpenGames.begin() + i);
				found = true;
				break;
			}
		}

		if (found)
		{
			s_Games.erase(index);
		}
	}

	Chess::ChessFile& GetChessFile()
	{
		return *s_ChessFile;
	}
	Chess::PgnGame& GetPgnGame()
	{
		return s_Games[s_ActiveGame].GetPgnGame();
	}
	Chess::GameManager& GetActiveGame()
	{
		return s_Games[s_ActiveGame];
	}
	std::string& GetChessFileName()
	{
		return s_FileName;
	}
	std::filesystem::path& GetChessFilePath()
	{
		return s_FilePath;
	}

	Chess::PgnGame& GetPgnGameByIndex(int index)
	{
		return s_Games[index].GetPgnGame();
	}

	int GetBlockID(int BlockIndex)
	{
		auto id = s_Games[s_ActiveGame].GetPieceID(BlockIndex);

		int ret = (int)id.type + (id.color == Chess::WHITE ? 0 : 1) * 6 + 1;

		return id.type != Chess::NONE ? ret : 0;
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

	void OpenChessFile(const std::filesystem::path& path, float* persentage)
	{
		s_Games.clear();
		s_OpenGames.clear();

		if (path == "")
		{
			//empty game
			delete s_ChessFile;
			s_ChessFile = new Chess::PgnFile();
			s_ChessFile->CreateGame();

			s_FilePath = "";
			s_FileName = "New Game";

			s_ActiveGame = 0;
			s_OpenGames.emplace_back(0);

			s_Games[0].InitPgnGame(s_ChessFile->operator[](0));

			return;
		}

		s_FilePath = path;
		s_FileName = path.filename().u8string();
		std::string extension = path.extension().u8string();

		//find a way to avoid removing and removing again the chess file, just waist of time
		delete s_ChessFile;

		if (extension == ".pgn")
			s_ChessFile = new Chess::PgnFile();
		else if (extension == ".cld")
			s_ChessFile = new Chess::CldFile();
		else
		{
			s_ChessFile = new Chess::PgnFile();
			s_ChessFile->CreateGame();

			s_FilePath = "";
			s_FileName = "New Game";

			s_ActiveGame = 0;
			s_OpenGames.emplace_back(0);

			s_Games[0].InitPgnGame(s_ChessFile->operator[](0));

			return;
		}

		s_ChessFile->OpenFile(s_FilePath, persentage);

		if (!s_ChessFile->GetSize())
			s_ChessFile->CreateGame();

		s_ActiveGame = 0;
		s_OpenGames.emplace_back(0);

		s_Games[0].InitPgnGame(s_ChessFile->operator[](0));

		if (false)
		{
			for (int i = 0; i < s_ChessFile->GetSize(); i++)
			{
				Chess::CldGame game;
				((Chess::CldFile*)s_ChessFile)->GetCldGame(game, i);
				//s_ChessFile->operator[](i);
			}
		}

		if (false)
		{
			std::unordered_map<uint16_t, std::pair<size_t, size_t>> movesCounterE;
			std::unordered_map<uint16_t, std::pair<size_t, size_t>> movesCounterM;
			std::unordered_map<uint16_t, std::pair<size_t, size_t>> movesCounterL;

			std::vector<uint16_t> orderedE;
			std::vector<uint16_t> orderedM;
			std::vector<uint16_t> orderedL;

			for (int i = 0; i < s_ChessFile->GetSize(); i++)
			{
				Chess::CldGame game;
				((Chess::CldFile*)s_ChessFile)->GetCldGame(game, i);

				size_t childIndex = 0;

				auto& movePath = game.GetMovePathbyRef();
				for (int j = 0; j < movePath.move.size(); j++)
				{
					if (movePath.move[j].first == UINT8_MAX)
					{
						childIndex++;
						continue;
					}

					uint16_t m = ((movePath.move[j].first << 8) | movePath.move[j].second);

					if (j < 45 + childIndex)
					{
						if (!movesCounterE.contains(m))
						{
							if (i < 1'000'000)
							{
								movesCounterE[m].first = 0;
								movesCounterE[m].second = 0;
							}
							else
								continue;
						}

						movesCounterE.at(m).first += 1;
						movesCounterE.at(m).second += j;
					}
					else if (j < 70 + childIndex)
					{
						if (!movesCounterM.contains(m))
						{
							if (i < 2'000'000)
							{
								movesCounterM[m].first = 0;
								movesCounterM[m].second = 0;
							}
							else
								continue;
						}

						movesCounterM.at(m).first += 1;
						movesCounterM.at(m).second += j;
					}
					else
					{
						if (!movesCounterL.contains(m))
						{
							if (i < 3'000'000)
							{
								movesCounterL[m].first = 0;
								movesCounterL[m].second = 0;
							}
							else
								continue;
						}

						movesCounterL.at(m).first += 1;
						movesCounterL.at(m).second += j;
					}
				}
			}

			orderedE.reserve(movesCounterE.size());
			orderedM.reserve(movesCounterM.size());
			orderedL.reserve(movesCounterL.size());

			bool finded = false;

			for (auto& [move, count] : movesCounterE)
			{
				finded = false;
				for (int i = 0; i < orderedE.size(); i++)
				{
					if (movesCounterE[orderedE[i]].first < count.first)
					{
						orderedE.insert(orderedE.begin() + i, move);
						finded = true;
						break;
					}
				}

				if (!finded)
					orderedE.emplace_back(move);
			}

			for (auto& [move, count] : movesCounterM)
			{
				finded = false;
				for (int i = 0; i < orderedM.size(); i++)
				{
					if (movesCounterM[orderedM[i]].first < count.first)
					{
						orderedM.insert(orderedM.begin() + i, move);
						finded = true;
						break;
					}
				}

				if (!finded)
					orderedM.emplace_back(move);
			}

			for (auto& [move, count] : movesCounterL)
			{
				finded = false;
				for (int i = 0; i < orderedL.size(); i++)
				{
					if (movesCounterL[orderedL[i]].first < count.first)
					{
						orderedL.insert(orderedL.begin() + i, move);
						finded = true;
						break;
					}
				}

				if (!finded)
					orderedL.emplace_back(move);
			}

			std::ofstream outfileT("tableCore.clt", std::ios::binary | std::ios::trunc);

			std::cout << "\n ----- [ E ] ----- \n";

			{
				size_t index = 0, overral = 0;
				
				outfileT.write((char*)orderedE.data(), sizeof(uint16_t) * 189);

				for (int i = 0; i < orderedE.size() && i < 189; i++)
				{
					auto& [count, sum] = movesCounterE[orderedE[i]];

					std::cout << " -- [ " << count << " ] -- " << " -- [ " << sum / count << " ] -- " << (orderedE[i] >> 8) << "  --  " << (orderedE[i] & 0x00ff) << '\n';
					index += 1;
					overral += count;
				}

				std::cout << '\n' << index << '\n';
				std::cout << '\n' << overral << '\n';
			}

			std::cout << "\n ----- [ M ] ----- \n";

			{
				size_t index = 0, overral = 0;
				
				outfileT.write((char*)orderedM.data(), sizeof(uint16_t) * 189);

				for (int i = 0; i < orderedM.size() && i < 189; i++)
				{
					auto& [count, sum] = movesCounterM[orderedM[i]];

					std::cout << " -- [ " << count << " ] -- " << " -- [ " << sum / count << " ] -- " << '\n';// << (move >> 8) << "  --  " << (move & 0x00ff) << '\n';
					index += 1;
					overral += count;
				}

				std::cout << '\n' << index << '\n';
				std::cout << '\n' << overral << '\n';
			}

			std::cout << "\n ----- [ L ] ----- \n";

			{
				size_t index = 0, overral = 0;

				outfileT.write((char*)orderedL.data(), sizeof(uint16_t) * 189);

				for (int i = 0; i < orderedL.size() && i < 189; i++)
				{
					auto& [count, sum] = movesCounterL[orderedL[i]];

					std::cout << " -- [ " << count << " ] -- " << " -- [ " << sum / count << " ] -- " << '\n';// << (move >> 8) << "  --  " << (move & 0x00ff) << '\n';
					index += 1;
					overral += count;
				}

				std::cout << '\n' << index << '\n';
				std::cout << '\n' << overral << '\n';
			}
		}
	}

	void OverWriteChessFile(const std::filesystem::path& filepath, float* persentage)
	{
		//save

		if (filepath == "")
		{
			if (s_FilePath == "")
				return;

			s_ChessFile->SaveFile(s_FilePath, persentage);
			return;
		}

		//save as

		s_Games.clear();
		s_OpenGames.clear();

		std::string extension = filepath.extension().u8string();

		if (extension == ".pgn")
		{
			if (s_FilePath.extension().u8string() == ".pgn")
			{
				s_ChessFile->SaveFile(filepath, persentage);
			}
			else
			{
				Chess::ConvertToPgn(*(Chess::CldFile*)s_ChessFile, filepath, persentage);

				delete s_ChessFile;
				s_ChessFile = new Chess::PgnFile();

				s_ChessFile->OpenFile(filepath);
			}			
		}
		else if (extension == ".cld")
		{
			if (s_FilePath.extension().u8string() == ".cld")
			{
				s_ChessFile->SaveFile(filepath, persentage);
			}
			else
			{
				Chess::ConvertToCld(*(Chess::PgnFile*)s_ChessFile, filepath, Chess::MoveEncoding::CORE, persentage);

				delete s_ChessFile;
				s_ChessFile = new Chess::CldFile();

				s_ChessFile->OpenFile(filepath);
			}
		}

		s_FilePath = filepath;
		s_FileName = filepath.filename().u8string();

		s_ActiveGame = 0;
		s_OpenGames.emplace_back(0);

		if (!s_ChessFile->GetSize())
			s_ChessFile->CreateGame();

		s_Games[0].Clear();
		s_Games[0].InitPgnGame(s_ChessFile->operator[](0));
	}
}