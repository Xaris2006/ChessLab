#include "ChessFileManager.h"

#include <span>
#include <fstream>
#include <iostream>

static Chess::ChessFileManager* s_ChessFileManager = nullptr;

static std::shared_mutex s_PgnManagerDataAccessMutex;
static std::shared_mutex s_PgnManagerSearchAccessMutex;

static std::unordered_map<uint16_t, uint8_t> s_tableRainE;
static std::unordered_map<uint16_t, uint8_t> s_tableRainM;
static std::unordered_map<uint16_t, uint8_t> s_tableRainL;
static std::vector<uint16_t> s_tableRainI;

namespace Chess
{
	void ChessFileManager::Init()
	{
		if (true)
		{
			std::ifstream infileTable("table.clt", std::ios::binary);

			std::vector<uint8_t> data;

			infileTable.seekg(0, std::ios_base::end);
			std::streampos maxIndex = infileTable.tellg();
			infileTable.seekg(0, std::ios::beg);

			data.resize(maxIndex);
			infileTable.read(reinterpret_cast<char*>(data.data()), data.size());

			infileTable.close();

			s_tableRainI.resize(189 * 3);

			for (int i = 0; i < data.size(); i += 2)
			{
				uint16_t move = (((uint16_t)data[i + 1] << 8) | data[i]);

				if (i < 189 * 2)
					s_tableRainE[move] = (i / 2) % 189;
				else if (i < 189 * 4)
					s_tableRainM[move] = (i / 2) % 189;
				else
					s_tableRainL[move] = (i / 2) % 189;

				s_tableRainI[i / 2] = move;
			}
		}

		s_ChessFileManager = new ChessFileManager();

		s_ChessFileManager->m_endThread = false;

		s_ChessFileManager->m_ThreadFileHandler = new std::thread(
			[]()
			{
				while (!s_ChessFileManager->m_endThread)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));

					std::unique_lock uniqueLock(s_PgnManagerDataAccessMutex);

					for (auto it = s_ChessFileManager->m_GamesTimer.begin(); it != s_ChessFileManager->m_GamesTimer.end();)
					{
						auto& [key, value] = *it;

						if (s_ChessFileManager->m_EditedGames.contains(key.first))
						{
							bool found = false;

							for (auto index : s_ChessFileManager->m_EditedGames.at(key.first))
							{
								if (index == key.second)
								{
									found = true;
									break;
								}
							}
							
							if (found)
							{
								++it;
								continue;
							}
						}

						auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - value).count();

						bool hasRef = !s_ChessFileManager->m_Games[key].IsFree();

						if (time > 1000 && !hasRef)
						{
							s_ChessFileManager->m_Games.erase(key);
							it = s_ChessFileManager->m_GamesTimer.erase(it);

							continue;
						}

						std::hash<std::string> hasher;

						if (hasRef && hasher(s_ChessFileManager->m_Games[key].GetData()) != hasher(s_ChessFileManager->m_Games[key].GetDataRead()))
						{
							bool added = false;

							for(int i = 0; i < s_ChessFileManager->m_EditedGames[key.first].size(); i++)
							{
								if (s_ChessFileManager->m_EditedGames[key.first][i] > key.second)
								{
									s_ChessFileManager->m_EditedGames[key.first].insert(s_ChessFileManager->m_EditedGames[key.first].begin() + i, key.second);
									
									added = true;
									break;
								}
							}

							if (!added)
								s_ChessFileManager->m_EditedGames[key.first].emplace_back(key.second);

						}

						++it;
					}
				}

				return;
			});

		s_ChessFileManager->m_SearchWorkers.fill(nullptr);

		const size_t amountOfWorkers = std::max(s_ChessFileManager->s_amountOfWorkersMin, std::min(s_ChessFileManager->s_amountOfWorkersMax, size_t(std::thread::hardware_concurrency() / 4)));

		for (int threadIndex = 0; threadIndex < amountOfWorkers; threadIndex++)
		{
			s_ChessFileManager->m_endWorkers[threadIndex] = false;

			s_ChessFileManager->m_SearchWorkers[threadIndex] = new std::thread(
				[threadIndex]()
				{
					size_t f_startIndex = 0;
					size_t f_endIndex = 0;
					std::vector<size_t> f_positiveIndexes;

					SearchID f_currentSearchID = 0;
					FileManager::FileID f_fileID;
					
					std::shared_ptr<std::pair<SearchOptions, SearchResult>> f_searchPtr;
					std::shared_ptr<std::mutex> f_mtx;
					std::shared_ptr<std::vector<size_t>> f_pointers;

					while (!s_ChessFileManager->m_endWorkers[threadIndex])
					{
						{
							std::unique_lock lock(s_PgnManagerSearchAccessMutex);

							for (auto& [searchID, searchData] : s_ChessFileManager->m_Searches)
							{
								FileManager::FileID fileID = std::get<0>(searchData);
								std::shared_ptr<std::pair<SearchOptions, SearchResult>>& searchPtr = std::get<1>(searchData);
								std::shared_ptr<std::mutex>& searchMutex = std::get<2>(searchData);
								size_t& indexRef = std::get<3>(searchData);

								if (threadIndex == 0)
								{
									std::shared_ptr<std::vector<size_t>> pointers;
									
									{
										std::shared_lock sharedLock(s_PgnManagerDataAccessMutex);

										if (!FileManager::Get().HasFile(fileID) 
											|| (!s_ChessFileManager->m_PgnData.contains(fileID) && !s_ChessFileManager->m_CldData.contains(fileID)))
										{
											searchPtr->first.Clear();
											searchPtr->second.Persentage = 0.0f;

											s_ChessFileManager->m_Searches.erase(searchID);

											break;
										}

										if (s_ChessFileManager->m_PgnData.contains(fileID))
											pointers = s_ChessFileManager->m_PgnData[fileID].gamePointers;
										else if (s_ChessFileManager->m_CldData.contains(fileID))
											pointers = s_ChessFileManager->m_CldData[fileID].gamePointers;
									}

									if (searchPtr->second.Persentage < 1.0f)
										searchPtr->second.Persentage = std::min(1.0f, (float)indexRef / (float)pointers->size());

									if (searchPtr->second.Persentage >= 1.0f)
									{
										searchPtr->second.Persentage = 1.0f;

										s_ChessFileManager->m_Searches.erase(searchID);
										break;
									}
								}

								if (searchPtr->second.Persentage < 1.0f)
								{
									if (s_ChessFileManager->m_PgnData.contains(fileID) 
										&& indexRef >= s_ChessFileManager->m_PgnData[fileID].gamePointers->size())
										continue;
									else if (s_ChessFileManager->m_CldData.contains(fileID) 
										&& indexRef >= s_ChessFileManager->m_CldData[fileID].gamePointers->size())
										continue;

									f_startIndex = indexRef;
									f_endIndex = f_startIndex + 5'000 + 1;
									indexRef = f_endIndex;

									f_fileID = fileID;
									f_searchPtr = searchPtr;
									f_mtx = searchMutex;

									f_currentSearchID = searchID;

									break;
								}
							}
						}

						if (f_currentSearchID != 0)
						{
							f_positiveIndexes.clear();
							
							if (false)
							{
								std::shared_lock sharedLock(s_PgnManagerSearchAccessMutex);

								if ( s_ChessFileManager->m_Searches.contains(f_currentSearchID) == false)
								{
									f_currentSearchID = 0;

									continue;
								}

								auto& search = s_ChessFileManager->m_Searches[f_currentSearchID];
								f_fileID = std::get<0>(search);
								f_searchPtr = std::get<1>(search);
								f_mtx = std::get<2>(search);
							}
							{
								std::shared_lock sharedLock(s_PgnManagerDataAccessMutex);

								if (!FileManager::Get().HasFile(f_fileID)
									|| (!s_ChessFileManager->m_PgnData.contains(f_fileID) && !s_ChessFileManager->m_CldData.contains(f_fileID)))
								{
									f_currentSearchID = 0;

									continue;
								}

								if (s_ChessFileManager->m_PgnData.contains(f_fileID))
									f_pointers = s_ChessFileManager->m_PgnData[f_fileID].gamePointers;
								else if (s_ChessFileManager->m_CldData.contains(f_fileID))
									f_pointers = s_ChessFileManager->m_CldData[f_fileID].gamePointers;

								if (f_startIndex >= f_pointers->size())
								{
									f_currentSearchID = 0;

									continue;
								}
							}

							size_t fileStartIndex = f_pointers->at(f_startIndex);
							size_t fileEndIndex = ((f_endIndex < f_pointers->size()) ? f_pointers->at(f_endIndex) : SIZE_MAX);

							static thread_local std::vector<char> data;
							FileManager::Get().ReadBuffer(f_fileID, fileStartIndex, fileEndIndex - fileStartIndex, (std::vector<uint8_t>&)data);

							size_t nextDataIndex = 0;

							for (int j = f_startIndex; j < f_endIndex; j++)
							{
								size_t endDataIndex = data.size();

								if (f_pointers->size() > j + 1)
									endDataIndex = f_pointers->at(j + 1) - fileStartIndex;

								if (endDataIndex == nextDataIndex)
									continue;

								if (s_ChessFileManager->m_PgnData.contains(f_fileID))
								{
									static thread_local PgnGame game;

									game.Parse(std::string_view{ &data[nextDataIndex], endDataIndex - nextDataIndex }, true, false);
									nextDataIndex = endDataIndex;

									if (f_searchPtr->first.IsGameValid(game))
										f_positiveIndexes.emplace_back(j);
								}
								else if (s_ChessFileManager->m_CldData.contains(f_fileID))
								{
									static thread_local CldGame game;
									game.Parse(std::span<uint8_t>((uint8_t*)data.data() + nextDataIndex, endDataIndex - nextDataIndex),
										(*s_ChessFileManager->m_CldData.at(f_fileID).typeName),
										(*s_ChessFileManager->m_CldData.at(f_fileID).typeValue),
										true, 
										false);
									nextDataIndex = endDataIndex;

									if (f_searchPtr->first.IsGameValid(game))
										f_positiveIndexes.emplace_back(j);
								}
							}

							//still there is a possibility that the search was cleared meanwhile

							{
								std::scoped_lock lock(*f_mtx);

								for (int j = 0; j < f_positiveIndexes.size(); j++)
									f_searchPtr->second.PossitiveIndexes.emplace_back(f_positiveIndexes[j]);
							}

							f_currentSearchID = 0;

							continue;
						}

						std::this_thread::sleep_for(std::chrono::milliseconds(50));
					}

					return;
				});
		}
	}

	void ChessFileManager::Shutdown()
	{
		s_ChessFileManager->m_endWorkers.fill(true);

		for (int i = 0; i < s_amountOfWorkersMax; i++)
		{
			if (s_ChessFileManager->m_SearchWorkers[i] == nullptr)
				continue;

			s_ChessFileManager->m_SearchWorkers[i]->join();
			delete s_ChessFileManager->m_SearchWorkers[i];
		}

		s_ChessFileManager->m_endThread = true;
		s_ChessFileManager->m_ThreadFileHandler->join();
		delete s_ChessFileManager->m_ThreadFileHandler;
		s_ChessFileManager->m_ThreadFileHandler = nullptr;

		//delete s_PgnManager;
		//s_PgnManager = nullptr;
	}

	ChessFileManager& ChessFileManager::Get()
	{
		return *s_ChessFileManager;
	}

	void ChessFileManager::AddFileReference(FileManager::FileID fileID, std::shared_ptr<std::vector<size_t>> pointers)
	{
		std::unique_lock lock(s_PgnManagerDataAccessMutex);

		m_PgnData[fileID].gamePointers = pointers;
	}

	void ChessFileManager::AddCldFileReference(FileManager::FileID fileID,
		std::shared_ptr<std::vector<size_t>> gamePointers, 
		std::shared_ptr<std::vector<std::string>> labelNames, 
		std::shared_ptr<std::vector<std::string>> labelValues, 
		std::shared_ptr<uint8_t> typeName,
		std::shared_ptr<uint8_t> typeValue)
	{
		std::unique_lock lock(s_PgnManagerDataAccessMutex);

		auto& cldD = m_CldData[fileID];
		cldD.gamePointers = gamePointers;
		cldD.labelNames = labelNames;
		cldD.labelValues = labelValues;
		cldD.typeName = typeName;
		cldD.typeValue = typeValue;
	}

	void ChessFileManager::RemoveFileReference(FileManager::FileID fileID)
	{
		std::unique_lock lock(s_PgnManagerDataAccessMutex);

		if (s_ChessFileManager->m_PgnData.contains(fileID))
			m_PgnData.erase(fileID);
		else if (s_ChessFileManager->m_CldData.contains(fileID))
			m_CldData.erase(fileID);

		m_EditedGames.erase(fileID);
	}

	PgnGame& ChessFileManager::GetGame(FileManager::FileID fileID, size_t index)
	{
		std::unique_lock lock(s_PgnManagerDataAccessMutex);

		if (m_Games.contains({ fileID, index }))
		{
			m_GamesTimer[{ fileID, index }] = std::chrono::high_resolution_clock::now();
			return m_Games.at({ fileID, index });
		}

		if (m_PgnData.contains(fileID) && m_PgnData.at(fileID).gamePointers->size() <= index)
		{
			m_EditedGames[fileID].emplace_back(index);
			m_GamesTimer[{ fileID, index }] = std::chrono::high_resolution_clock::now();
			return m_Games[{ fileID, index }];
		}

		if (m_CldData.contains(fileID) && m_CldData.at(fileID).gamePointers->size() <= index)
		{
			m_EditedGames[fileID].emplace_back(index);
			m_GamesTimer[{ fileID, index }] = std::chrono::high_resolution_clock::now();
			return m_Games[{ fileID, index }];
		}

		if (!FileManager::Get().HasFile(fileID))
			return (PgnGame&)*(PgnGame*)(nullptr);

		if (m_PgnData.contains(fileID))
		{
			std::vector<uint8_t> data;
			FileManager::Get().ReadBuffer(fileID, (*m_PgnData[fileID].gamePointers)[index], ((index + 1 < m_PgnData[fileID].gamePointers->size()) ? ((*m_PgnData[fileID].gamePointers)[index + 1] - (*m_PgnData[fileID].gamePointers)[index]) : SIZE_MAX), (std::vector<uint8_t>&)data);

			m_Games[{ fileID, index }].Parse(std::string_view{ (char*)data.data(), data.size() });
			m_GamesTimer[{ fileID, index }] = std::chrono::high_resolution_clock::now();

			return m_Games.at({ fileID, index });
		}

		if (m_CldData.contains(fileID))
		{
			std::vector<uint8_t> data;
			FileManager::Get().ReadBuffer(fileID, (*m_CldData[fileID].gamePointers)[index], ((index + 1 < m_CldData[fileID].gamePointers->size()) ? ((*m_CldData[fileID].gamePointers)[index + 1] - (*m_CldData[fileID].gamePointers)[index]) : SIZE_MAX), (std::vector<uint8_t>&)data);

			CldGame cldGame;
			cldGame.Parse(data, (*m_CldData[fileID].typeName), (*m_CldData[fileID].typeValue));

			auto& newGame = m_Games[{ fileID, index }];
			
			for (auto& nameIndex : cldGame.GetLabelNames())
				newGame[(*m_CldData[fileID].labelNames)[nameIndex]] = (*m_CldData[fileID].labelValues)[cldGame[nameIndex]];
			
			ConvertCldMovePathToPgnMovePath(newGame.GetMovePathbyRef(), cldGame.GetMovePathbyRef());
			newGame.SetCurrentAsInitial();

			m_GamesTimer[{ fileID, index }] = std::chrono::high_resolution_clock::now();
			return m_Games.at({ fileID, index });
		}
	}

	void ChessFileManager::GetGames(FileManager::FileID fileID, size_t index, size_t size, std::vector<PgnGame*> games)
	{
		std::unique_lock lock(s_PgnManagerDataAccessMutex);

		games.clear();

		if (m_Games.contains({ fileID, index }))
		{
			m_GamesTimer[{ fileID, index }] = std::chrono::high_resolution_clock::now();
			return; m_Games.at({ fileID, index });
		}

		if (m_PgnData.contains(fileID) && m_PgnData.at(fileID).gamePointers->size() <= index)
		{
			m_EditedGames[fileID].emplace_back(index);
			m_GamesTimer[{ fileID, index }] = std::chrono::high_resolution_clock::now();
			return; m_Games[{ fileID, index }];
		}

		if (m_CldData.contains(fileID) && m_CldData.at(fileID).gamePointers->size() <= index)
		{
			m_EditedGames[fileID].emplace_back(index);
			m_GamesTimer[{ fileID, index }] = std::chrono::high_resolution_clock::now();
			return; m_Games[{ fileID, index }];
		}

		if (!FileManager::Get().HasFile(fileID))
			return;

		if (m_PgnData.contains(fileID))
		{
			std::vector<uint8_t> data;
			FileManager::Get().ReadBuffer(fileID, (*m_PgnData[fileID].gamePointers)[index], ((index + 1 < m_PgnData[fileID].gamePointers->size()) ? ((*m_PgnData[fileID].gamePointers)[index + 1] - (*m_PgnData[fileID].gamePointers)[index]) : SIZE_MAX), (std::vector<uint8_t>&)data);

			m_Games[{ fileID, index }].Parse(std::string_view{ (char*)data.data(), data.size() });
			m_GamesTimer[{ fileID, index }] = std::chrono::high_resolution_clock::now();

			return; m_Games.at({ fileID, index });
		}

		if (m_CldData.contains(fileID))
		{
			std::vector<uint8_t> data;
			FileManager::Get().ReadBuffer(fileID, (*m_CldData[fileID].gamePointers)[index], ((index + 1 < m_CldData[fileID].gamePointers->size()) ? ((*m_CldData[fileID].gamePointers)[index + 1] - (*m_CldData[fileID].gamePointers)[index]) : SIZE_MAX), (std::vector<uint8_t>&)data);

			CldGame cldGame;
			cldGame.Parse(data, (*m_CldData[fileID].typeName), (*m_CldData[fileID].typeValue));

			auto& newGame = m_Games[{ fileID, index }];

			for (auto& nameIndex : cldGame.GetLabelNames())
				newGame[(*m_CldData[fileID].labelNames)[nameIndex]] = (*m_CldData[fileID].labelValues)[cldGame[nameIndex]];

			ConvertCldMovePathToPgnMovePath(newGame.GetMovePathbyRef(), cldGame.GetMovePathbyRef());
			newGame.SetCurrentAsInitial();

			m_GamesTimer[{ fileID, index }] = std::chrono::high_resolution_clock::now();
			return; m_Games.at({ fileID, index });
		}
	}

	void ChessFileManager::GetEditedGames(FileManager::FileID fileID, std::vector<size_t>& indexes) const
	{
		std::unique_lock lock(s_PgnManagerDataAccessMutex);

		if (!m_EditedGames.contains(fileID))
			return;

		indexes = m_EditedGames.at(fileID);
	}

	bool ChessFileManager::IsGameEdited(FileManager::FileID fileID, size_t index) const
	{
		std::unique_lock lock(s_PgnManagerDataAccessMutex);

		if (!m_EditedGames.contains(fileID))
			return false;

		for (int i = 0; i < m_EditedGames.at(fileID).size(); i++)
		{
			if (m_EditedGames.at(fileID)[i] == index)
				return true;
		}

		return false;
	}

	void ChessFileManager::ClearSearch(SearchID id)
	{
		std::unique_lock lock(s_PgnManagerSearchAccessMutex);

		if (!m_Searches.contains(id))
			return;

		m_Searches.erase(id);
	}

	void ChessFileManager::StartSearch(SearchID id, FileManager::FileID fileID, std::shared_ptr<std::pair<SearchOptions, SearchResult>> seachPtr)
	{
		std::unique_lock lock(s_PgnManagerSearchAccessMutex);

		if (m_Searches.contains(id))
			return;
		
		if (!FileManager::Get().HasFile(fileID))
			return;
		
		auto& [fID, ptr, mtx, index] = m_Searches[id];
		
		mtx = std::make_shared<std::mutex>();

		{
			std::scoped_lock<std::mutex> ul(*mtx.get());

			fID = fileID;
			ptr = seachPtr;

			ptr->second.Persentage = 0.0f;
			ptr->second.PossitiveIndexes.clear();

			index = 0;
		}
	}

	void ChessFileManager::ConvertCldMovePathToPgnMovePath(PgnGame::ChessMovesPath& pgnMovePath, const CldGame::CldMovesPath& cldMovePath, int moveIndex)
	{
		static const char* pieceNames = "NBRQPK";

		if (cldMovePath.details.contains(-1))
		{
			auto& detail = pgnMovePath.details[-1];
			detail.cmds = cldMovePath.details.at(-1).cmds;
			detail.note = cldMovePath.details.at(-1).note;
		}

		size_t index = 0;

		for (int i = 0; i < cldMovePath.move.size(); i++)
		{
			const std::pair<uint8_t, uint8_t>* move = &cldMovePath.move[i];

			if (move->first == UINT8_MAX)
			{
				pgnMovePath.move.emplace_back("child");

				int moveIndexToPass = moveIndex;

				if (moveIndexToPass > 0)
					moveIndexToPass -= 1;
				else
					moveIndexToPass *= -1;

				ConvertCldMovePathToPgnMovePath(pgnMovePath.children.emplace_back(), cldMovePath.children[index], moveIndexToPass);
				index++;

				continue;
			}

			std::string moveToAdd;

			moveIndex *= -1;
			
			if (moveIndex >= 0)
			{
				moveIndex += 1;
				moveToAdd += (std::to_string(moveIndex) + ". ");
			}

			std::pair<uint8_t, uint8_t> tableMove;

			if (move->first >= 64)
			{
				uint8_t indexOfMove = move->first - 64;

				if (i - index >= 70)
					indexOfMove += (189 * 2);
				else if (i - index >= 45)
					indexOfMove += 189;

				tableMove.first = uint8_t(s_tableRainI.at(indexOfMove) >> 8);
				tableMove.second = uint8_t(s_tableRainI.at(indexOfMove) & 0x00FF);

				move = &tableMove;
			}

			uint8_t moveDir = move->first & 0b00111111;
			uint8_t movePartOne = (move->second & 0b11100000) >> 5;
			uint8_t movePartSecond = (move->second & 0b00011100) >> 2;
			uint8_t movePartThird = move->second & 0b00000011;
			
			char DirX = 'a' + (moveDir >> 3);
			char DirY = '1' + (moveDir & 0b00000111);

			if (movePartOne == 0)
			{
				if ((moveDir >> 3) != movePartSecond)
				{
					moveToAdd += ('a' + movePartSecond);
					moveToAdd += 'x';
				}
	
				moveToAdd += DirX;
				moveToAdd += DirY;

				if (DirY == '1' || DirY == '8')
				{
					moveToAdd += '=';
					moveToAdd += pieceNames[movePartThird];
				}
			}
			else if (movePartOne == 1)
			{
				if (movePartSecond == 0)
				{
					moveToAdd += 'K';
					moveToAdd += DirX;
					moveToAdd += DirY;
				}
				else if (movePartSecond == 1)
				{
					moveToAdd += 'K';
					moveToAdd += 'x';
					moveToAdd += DirX;
					moveToAdd += DirY;
				}
				else if (movePartSecond == 2)
				{
					moveToAdd += pieceNames[movePartThird];
					moveToAdd += DirX;
					moveToAdd += DirY;
				}
				else if (movePartSecond == 3)
				{
					moveToAdd += pieceNames[movePartThird];
					moveToAdd += 'x';
					moveToAdd += DirX;
					moveToAdd += DirY;
				}
				else if (movePartSecond == 4)
				{
					moveToAdd += "O-O";
				}
				else if (movePartSecond == 5)
				{
					moveToAdd += "O-O-O";
				}
			}
			else if (movePartOne == 2)
			{
				moveToAdd += pieceNames[movePartThird];
				moveToAdd += ('a' + movePartSecond);
				moveToAdd += DirX;
				moveToAdd += DirY;
			}
			else if (movePartOne == 3)
			{
				moveToAdd += pieceNames[movePartThird];
				moveToAdd += ('a' + movePartSecond);
				moveToAdd += 'x';
				moveToAdd += DirX;
				moveToAdd += DirY;
			}
			else if (movePartOne == 4)
			{
				moveToAdd += pieceNames[movePartThird];
				moveToAdd += ('1' + movePartSecond);
				moveToAdd += DirX;
				moveToAdd += DirY;
			}
			else if (movePartOne == 5)
			{
				moveToAdd += pieceNames[movePartThird];
				moveToAdd += ('1' + movePartSecond);
				moveToAdd += 'x';
				moveToAdd += DirX;
				moveToAdd += DirY;
			}
			else if (movePartOne == 6)
			{
				static const int converter[] = {-2, -1, 1, 2};
				char pN;
				char posX;
				char posY;

				if (movePartThird == 0)
				{
					int converted = converter[movePartSecond % 4];
					posX = DirX + converted;

					converted = (std::abs(converted) == 2 ? 1 : 2);
					
					if (movePartSecond / 4 == 0)
						posY = DirY - converted;
					else
						posY = DirY + converted;
					
					pN = pieceNames[movePartThird];
				}
				else if (movePartThird == 1 || movePartThird == 2)
				{
					int converted = converter[movePartSecond % 4];
					posX = DirX + converted;

					if (movePartSecond / 4 == 0)
						posY = DirY - converted;
					else
						posY = DirY + converted;

					pN = pieceNames[movePartThird];
				}
				else
				{
					int pos3 = movePartSecond % 4;
					
					if (pos3 == 0)
					{
						posX = DirX - 3;
						posY = DirY - 3;
					}
					else if (pos3 == 1)
					{
						posX = DirX + 3;
						posY = DirY - 3;
					}
					else if (pos3 == 2)
					{
						posX = DirX - 3;
						posY = DirY + 3;
					}
					else 
					{
						posX = DirX + 3;
						posY = DirY + 3;
					}

					if (movePartSecond / 4 == 0)
						pN = 'B';
					else
						pN = 'Q';
				}

				moveToAdd += pN;
				moveToAdd += posX;
				moveToAdd += posY;
				moveToAdd += DirX;
				moveToAdd += DirY;
			}
			else if (movePartOne == 7)
			{
				static const int converter[] = { -2, -1, 1, 2 };
				char pN;
				char posX;
				char posY;

				if (movePartThird == 0)
				{
					int converted = converter[movePartSecond % 4];
					posX = DirX + converted;

					converted = (std::abs(converted) == 2 ? 1 : 2);

					if (movePartSecond / 4 == 0)
						posY = DirY - converted;
					else
						posY = DirY + converted;

					pN = pieceNames[movePartThird];
				}
				else if (movePartThird == 1 || movePartThird == 2)
				{
					int converted = converter[movePartSecond % 4];
					posX = DirX + converted;

					if (movePartSecond / 4 == 0)
						posY = DirY - converted;
					else
						posY = DirY + converted;

					pN = pieceNames[movePartThird];
				}
				else
				{
					int pos3 = movePartSecond % 4;

					if (pos3 == 0)
					{
						posX = DirX - 3;
						posY = DirY - 3;
					}
					else if (pos3 == 1)
					{
						posX = DirX + 3;
						posY = DirY - 3;
					}
					else if (pos3 == 2)
					{
						posX = DirX - 3;
						posY = DirY + 3;
					}
					else
					{
						posX = DirX + 3;
						posY = DirY + 3;
					}

					if (movePartSecond / 4 == 0)
						pN = 'B';
					else
						pN = 'Q';
				}

				moveToAdd += pN;
				moveToAdd += posX;
				moveToAdd += posY;
				moveToAdd += 'x';
				moveToAdd += DirX;
				moveToAdd += DirY;
			}

			pgnMovePath.move.emplace_back(moveToAdd);

			if (cldMovePath.details.contains(i))
			{
				auto& detail = pgnMovePath.details[i];
				detail.cmds = cldMovePath.details.at(i).cmds;
				detail.note = cldMovePath.details.at(i).note;
			}
		}
	}

	void ChessFileManager::ConvertPgnMovePathToCldMovePath(CldGame::CldMovesPath& cldMovePath, const PgnGame::ChessMovesPath& pgnMovePath)
	{
		const uint8_t converter[] = { 0, 1, 0, 2, 3 };
		const uint8_t childIndexID = 255;

		if (pgnMovePath.details.contains(-1) && (pgnMovePath.details.at(-1).note != "" || !pgnMovePath.details.at(-1).cmds.empty()))
		{
			auto& detail = pgnMovePath.details.at(-1);
			cldMovePath.details[-1].cmds = detail.cmds;
			cldMovePath.details[-1].note = detail.note;
		}

		size_t index = 0;

		for (size_t i = 0; i < pgnMovePath.move.size(); i++)
		{
			const std::string& move = pgnMovePath.move[i];

			if (move == "child")
			{
				cldMovePath.move.emplace_back(childIndexID, childIndexID);
				
				auto& child = cldMovePath.children.emplace_back(&cldMovePath);
				ConvertPgnMovePathToCldMovePath(child, pgnMovePath.children[index]);
				index++;

				continue;
			}

			uint8_t firstPart, secondPart;

			size_t moveStart = -1, moveDirStart = -1, yPosIndex = -1, xPosIndex = -1;
			bool xPos = false, yPos = false, taking = false, Prom = false;

			bool bigRoke = move.find("O-O-O") != std::string::npos || move.find("0-0-0") != std::string::npos;
			bool smallRoke = move.find("O-O") != std::string::npos || move.find("0-0") != std::string::npos;

			if (bigRoke)
			{
				firstPart = 0;
				secondPart = 0b00110100;
			}
			else if (smallRoke)
			{
				firstPart = 0;
				secondPart = 0b00110000;
			}
			else
			{
				moveStart = move.find(' ');

				if (moveStart == std::string::npos)
					moveStart = 0;
				else
					moveStart += 1;

				moveDirStart = moveStart;

				for (int k = move.size() - 1; k > moveStart; k--)
				{
					if (move[k] >= '1' && move[k] <= '8')
					{
						moveDirStart = k - 1;
						break;
					}
				}

				Prom = move.find('=') != std::string::npos;
				taking = move.find('x') != std::string::npos;
				uint8_t dirX = move[moveDirStart] - 'a';
				uint8_t dirY = move[moveDirStart + 1] - '1';

				firstPart = (dirX << 3) | dirY;

				if (move[moveStart] > 'A' && move[moveStart] < 'Z')
				{
					int checkIfPos = moveDirStart - moveStart;

					if (taking)
						checkIfPos -= 1;

					for (int k = 1; k < checkIfPos; k++)
					{
						char c = move[k + moveStart];
						if (c >= 'a' && c <= 'h')
						{
							xPosIndex = c - 'a';
							xPos = true;
						}
						else if (c >= '1' && c <= '8')
						{
							yPosIndex = c - '1';
							yPos = true;
						}
					}

					if (!xPos && !yPos)
					{
						if (move[moveStart] == 'K')
						{
							if (taking)
								secondPart = 0b00100100;
							else
								secondPart = 0b00100000;
						}
						else if (move[moveStart] == 'N')
						{
							if (taking)
								secondPart = 0b00101100;
							else
								secondPart = 0b00101000;
						}
						else if (move[moveStart] == 'B')
						{
							if (taking)
								secondPart = 0b00101101;
							else
								secondPart = 0b00101001;
						}
						else if (move[moveStart] == 'R')
						{
							if (taking)
								secondPart = 0b00101110;
							else
								secondPart = 0b00101010;
						}
						else if (move[moveStart] == 'Q')
						{
							if (taking)
								secondPart = 0b00101111;
							else
								secondPart = 0b00101011;
						}
					}
					else if (xPos && !yPos && !taking)
					{
						if (move[moveStart] == 'N')
						{
							secondPart = 0b01000000 | (xPosIndex << 2);
						}
						else if (move[moveStart] == 'B')
						{
							secondPart = 0b01000001 | (xPosIndex << 2);
						}
						else if (move[moveStart] == 'R')
						{
							secondPart = 0b01000010 | (xPosIndex << 2);
						}
						else if (move[moveStart] == 'Q')
						{
							secondPart = 0b01000011 | (xPosIndex << 2);
						}
					}
					else if (xPos && !yPos && taking)
					{
						if (move[moveStart] == 'N')
						{
							secondPart = 0b01100000 | (xPosIndex << 2);
						}
						else if (move[moveStart] == 'B')
						{
							secondPart = 0b01100001 | (xPosIndex << 2);
						}
						else if (move[moveStart] == 'R')
						{
							secondPart = 0b01100010 | (xPosIndex << 2);
						}
						else if (move[moveStart] == 'Q')
						{
							secondPart = 0b01100011 | (xPosIndex << 2);
						}
					}
					else if (!xPos && yPos && !taking)
					{
						if (move[moveStart] == 'N')
						{
							secondPart = 0b10000000 | (yPosIndex << 2);
						}
						else if (move[moveStart] == 'B')
						{
							secondPart = 0b10000001 | (yPosIndex << 2);
						}
						else if (move[moveStart] == 'R')
						{
							secondPart = 0b10000010 | (yPosIndex << 2);
						}
						else if (move[moveStart] == 'Q')
						{
							secondPart = 0b10000011 | (yPosIndex << 2);
						}
					}
					else if (!xPos && yPos && taking)
					{
						if (move[moveStart] == 'N')
						{
							secondPart = 0b10100000 | (yPosIndex << 2);
						}
						else if (move[moveStart] == 'B')
						{
							secondPart = 0b01010001 | (yPosIndex << 2);
						}
						else if (move[moveStart] == 'R')
						{
							secondPart = 0b10100010 | (yPosIndex << 2);
						}
						else if (move[moveStart] == 'Q')
						{
							secondPart = 0b10100011 | (yPosIndex << 2);
						}
					}
					else if (xPos && yPos && !taking)
					{
						int xDiff = dirX - xPosIndex;
						int yDiff = dirY - yPosIndex;

						if (move[moveStart] == 'N')
						{
							secondPart = 0b11000000 | ((converter[xDiff + 2] + (0 ? yDiff > 0 : 4)) << 2);
						}
						else if (move[moveStart] == 'B')
						{
							if (std::abs(xDiff) > 2)
							{
								if (xDiff < 0 && yDiff < 0)
								{
									secondPart = 0b11000010;
								}
								else if (xDiff > 0 && yDiff < 0)
								{
									secondPart = 0b11000110;
								}
								else if (xDiff < 0 && yDiff > 0)
								{
									secondPart = 0b11001010;
								}
								else
								{
									secondPart = 0b11001110;
								}
							}
							else
							{
								secondPart = 0b11000001 | ((converter[xDiff + 2] + (yDiff > 0 ? 0 : 4)) << 2);
							}
						}
						else if (move[moveStart] == 'Q')
						{
							if (std::abs(xDiff) > 2)
							{
								if (xDiff < 0 && yDiff < 0)
								{
									secondPart = 0b11010010;
								}
								else if (xDiff > 0 && yDiff < 0)
								{
									secondPart = 0b11010110;
								}
								else if (xDiff < 0 && yDiff > 0)
								{
									secondPart = 0b11011010;
								}
								else
								{
									secondPart = 0b11011110;
								}
							}
							else
							{
								secondPart = 0b11000011 | ((converter[xDiff + 2] + (yDiff > 0 ?  0 : 4)) << 2);
							}
						}
					}
					else if (xPos && yPos && taking)
					{
						int xDiff = dirX - xPosIndex;
						int yDiff = dirY - yPosIndex;

						if (move[moveStart] == 'N')
						{
							secondPart = 0b11100000 | ((converter[xDiff + 2] + (yDiff > 0 ? 0 : 4)) << 2);
						}
						else if (move[moveStart] == 'B')
						{
							if (std::abs(xDiff) > 2)
							{
								if (xDiff < 0 && yDiff < 0)
								{
									secondPart = 0b11100010;
								}
								else if (xDiff > 0 && yDiff < 0)
								{
									secondPart = 0b11100110;
								}
								else if (xDiff < 0 && yDiff > 0)
								{
									secondPart = 0b11101010;
								}
								else
								{
									secondPart = 0b11101110;
								}
							}
							else
							{
								secondPart = 0b11100001 | ((converter[xDiff + 2] + (yDiff > 0 ?  0 : 4)) << 2);
							}
						}
						else if (move[moveStart] == 'Q')
						{
							if (std::abs(xDiff) > 2)
							{
								if (xDiff < 0 && yDiff < 0)
								{
									secondPart = 0b11110010;
								}
								else if (xDiff > 0 && yDiff < 0)
								{
									secondPart = 0b11110110;
								}
								else if (xDiff < 0 && yDiff > 0)
								{
									secondPart = 0b11111010;
								}
								else
								{
									secondPart = 0b11111110;
								}
							}
							else
							{
								secondPart = 0b11100011 | ((converter[xDiff + 2] + (yDiff > 0 ? 0 : 4)) << 2);
							}
						}
					}
				}
				else
				{
					if (taking)
					{
						secondPart = 0b00000000 | ((move[moveStart] - 'a') << 2);
					}
					else
					{
						secondPart = 0b00000000 | (dirX << 2);
					}

					if (Prom)
					{
						char type = move[moveDirStart + 3];
						
						if (type == 'B')
							secondPart |= 1;
						else if (type == 'R')
							secondPart |= 2;
						else if (type == 'Q')
							secondPart |= 3;
					}
				}
			}

			if (i - index < 45)
			{
				if (s_tableRainE.contains((firstPart << 8) | secondPart))
					cldMovePath.move.emplace_back(s_tableRainE[(firstPart << 8) | secondPart] + 64, 0);
				else
					cldMovePath.move.emplace_back(firstPart, secondPart);
			}
			else if (i - index < 70)
			{
				if (s_tableRainM.contains((firstPart << 8) | secondPart))
					cldMovePath.move.emplace_back(s_tableRainM[(firstPart << 8) | secondPart] + 64, 0);
				else
					cldMovePath.move.emplace_back(firstPart, secondPart);
			}
			else if (s_tableRainL.contains((firstPart << 8) | secondPart))
				cldMovePath.move.emplace_back(s_tableRainL[(firstPart << 8) | secondPart] + 64, 0);
			else
				cldMovePath.move.emplace_back(firstPart, secondPart);

			if (pgnMovePath.details.contains(i) && (pgnMovePath.details.at(i).note != "" || !pgnMovePath.details.at(i).cmds.empty()))
			{
				auto& detail = pgnMovePath.details.at(i);
				cldMovePath.details[i].cmds = detail.cmds;
				cldMovePath.details[i].note = detail.note;
			}
		}		
	}
}