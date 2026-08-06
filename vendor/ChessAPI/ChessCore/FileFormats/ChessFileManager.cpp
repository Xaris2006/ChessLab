#include "ChessFileManager.h"

#include <span>

#include "../Board.h"
#include "MoveTables.h"

static Chess::ChessFileManager* s_ChessFileManager = nullptr;

static std::shared_mutex s_ChessManagerDataAccessMutex;
static std::shared_mutex s_ChessManagerSearchAccessMutex;

static std::optional<size_t> getNumber(std::string_view s) {
	if (s.empty())
		return {};

	size_t value;
	auto result = std::from_chars(s.data(), s.data() + s.size(), value);

	if (result.ec == std::errc() && result.ptr == s.data() + s.size())
		return value;

	return {};
}

namespace Chess
{
	void ChessFileManager::Init()
	{
		InitializeMoveTables();

		s_ChessFileManager = new ChessFileManager();

		s_ChessFileManager->m_endThread = false;

		s_ChessFileManager->m_ThreadFileHandler = new std::thread(
			[]()
			{
				while (!s_ChessFileManager->m_endThread)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));

					std::unique_lock uniqueLock(s_ChessManagerDataAccessMutex);

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

						if ((time > 1000 && !hasRef) 
							|| (!s_ChessFileManager->m_PgnData.contains(key.first) 
								&& !s_ChessFileManager->m_CldData.contains(key.first)
								&& !s_ChessFileManager->m_ClrData.contains(key.first)
								))
						{
							s_ChessFileManager->m_Games.erase(key);
							it = s_ChessFileManager->m_GamesTimer.erase(it);

							continue;
						}

						std::hash<std::string> hasher;

						if (hasRef && !s_ChessFileManager->m_ClrData.contains(key.first) && hasher(s_ChessFileManager->m_Games[key].GetData()) != hasher(s_ChessFileManager->m_Games[key].GetDataRead()))
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
					size_t f_wWins, f_bWins, f_Draws, f_Elo, f_EloCounter;
					SearchResultMoveData f_localClrResults;
					std::vector<std::pair<size_t, uint16_t>> f_localClrTopGames;

					SearchID f_currentSearchID = 0;
					FileManager::FileID f_fileID;
					
					std::shared_ptr<std::pair<SearchOptions, SearchResult>> f_searchPtr;
					std::shared_ptr<std::mutex> f_mtx;
					std::shared_ptr<std::vector<size_t>> f_pointers;
					std::shared_ptr<SearchResultMoveData> f_clrResults;
					std::shared_ptr<std::vector<size_t>> f_clrTopGames;

					while (!s_ChessFileManager->m_endWorkers[threadIndex])
					{
						{
							std::unique_lock lock(s_ChessManagerSearchAccessMutex);

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
										std::shared_lock sharedLock(s_ChessManagerDataAccessMutex);

										if (!FileManager::Get().HasFile(fileID) 
											|| (!s_ChessFileManager->m_PgnData.contains(fileID)
												&& !s_ChessFileManager->m_CldData.contains(fileID)
												&& !s_ChessFileManager->m_ClrData.contains(fileID)
												))
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
										else if (s_ChessFileManager->m_ClrData.contains(fileID))
											pointers = s_ChessFileManager->m_ClrData[fileID].gamePointers;
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
									else if (s_ChessFileManager->m_ClrData.contains(fileID) 
										&& indexRef >= s_ChessFileManager->m_ClrData[fileID].gamePointers->size())
										continue;

									f_startIndex = indexRef;
									f_endIndex = f_startIndex + 5'000 + 1;//5'000 + 1;
									indexRef = f_endIndex;

									f_fileID = fileID;
									f_searchPtr = searchPtr;
									f_mtx = searchMutex;

									f_currentSearchID = searchID;

									if (s_ChessFileManager->m_ClrData.contains(fileID))
									{
										f_clrResults = s_ChessFileManager->m_ClrSearchResults[searchID];
										f_clrTopGames = s_ChessFileManager->m_ClrSearchTopGames[searchID].first;
									}
									else
									{
										f_clrResults.reset();
										f_clrTopGames.reset();
									}

									break;
								}
							}
						}

						if (f_currentSearchID != 0)
						{
							f_positiveIndexes.clear();
							f_wWins = 0;
							f_bWins = 0;
							f_Draws = 0;
							f_Elo = 0;
							f_EloCounter = 0;
							f_localClrResults.clear();
							f_localClrTopGames.clear();

							if (false)
							{
								std::shared_lock sharedLock(s_ChessManagerSearchAccessMutex);

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
								std::shared_lock sharedLock(s_ChessManagerDataAccessMutex);

								if (!FileManager::Get().HasFile(f_fileID)
									|| (!s_ChessFileManager->m_PgnData.contains(f_fileID) 
										&& !s_ChessFileManager->m_CldData.contains(f_fileID)
										&& !s_ChessFileManager->m_ClrData.contains(f_fileID)
										))
								{
									f_currentSearchID = 0;

									continue;
								}

								if (s_ChessFileManager->m_PgnData.contains(f_fileID))
									f_pointers = s_ChessFileManager->m_PgnData[f_fileID].gamePointers;
								else if (s_ChessFileManager->m_CldData.contains(f_fileID))
									f_pointers = s_ChessFileManager->m_CldData[f_fileID].gamePointers;
								else if (s_ChessFileManager->m_ClrData.contains(f_fileID))
									f_pointers = s_ChessFileManager->m_ClrData[f_fileID].gamePointers;

								if (f_startIndex >= f_pointers->size())
								{
									f_currentSearchID = 0;

									continue;
								}
							}

							bool readLabels = f_searchPtr->first.IsLabelUsed();
							bool readMoves = f_searchPtr->first.IsMovesUsed();

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

									game.Parse(std::string_view{ &data[nextDataIndex], endDataIndex - nextDataIndex }, true, true);
									nextDataIndex = endDataIndex;

									if (f_searchPtr->first.IsGameValid(game))
										f_positiveIndexes.emplace_back(j);
								}
								else if (s_ChessFileManager->m_CldData.contains(f_fileID))
								{
									bool wencode = ((*s_ChessFileManager->m_CldData.at(f_fileID).settings)) % 2 == 0;
									MoveEncoding encoding = MoveEncoding::CLD;
									if (!wencode)
										encoding = MoveEncoding::CORE;

									static thread_local CldGame game;
									game.Parse(std::span<uint8_t>((uint8_t*)data.data() + nextDataIndex, endDataIndex - nextDataIndex),
										encoding,
										(*s_ChessFileManager->m_CldData.at(f_fileID).typeName),
										(*s_ChessFileManager->m_CldData.at(f_fileID).typeValue),
										true, 
										readLabels,
										readMoves,
										false);

									nextDataIndex = endDataIndex;

									if (f_searchPtr->first.IsGameValid(game))
										f_positiveIndexes.emplace_back(j);
								}
								else if (s_ChessFileManager->m_ClrData.contains(f_fileID))
								{
									bool wencode = ((*s_ChessFileManager->m_ClrData.at(f_fileID).settings)) % 2 == 0;
									MoveEncoding encoding = MoveEncoding::CLD;
									if (!wencode)
										encoding = MoveEncoding::CORE;

									static thread_local CldGame game;
									game.Parse(std::span<uint8_t>((uint8_t*)data.data() + nextDataIndex, endDataIndex - nextDataIndex),
										encoding,
										(*s_ChessFileManager->m_ClrData.at(f_fileID).typeName),
										(*s_ChessFileManager->m_ClrData.at(f_fileID).typeValue),
										true, 
										true,
										true,
										false);

									nextDataIndex = endDataIndex;

									int moveIndex = f_searchPtr->first.IsGameValidClr(game);

									if (moveIndex > -2)
									{
										f_positiveIndexes.emplace_back(j);

										auto& [resIndex, w, b, d, WEloIndex, BEloIndex, DateIndex] = *s_ChessFileManager->m_ClrData.at(f_fileID).searchGameIndexes;

										size_t res = game.At(resIndex);

										if (res == w)
											f_wWins++;
										else if (res == b)
											f_bWins++;
										else if (res == d)
											f_Draws++;
										
										size_t WElo = game.At(WEloIndex);
										size_t BElo = game.At(BEloIndex);
										size_t Date = game.At(DateIndex);

										std::string WEloStr = (*s_ChessFileManager->m_ClrData.at(f_fileID).labelValues)[WElo];
										std::string BEloStr = (*s_ChessFileManager->m_ClrData.at(f_fileID).labelValues)[BElo];
										std::string DateStr = (*s_ChessFileManager->m_ClrData.at(f_fileID).labelValues)[Date];

										auto WEloToAdd = getNumber(WEloStr);
										auto BEloToAdd = getNumber(BEloStr);
										auto DateToAdd = getNumber({ DateStr.begin(), DateStr.begin() + 4 });

										uint16_t averElo = 0;

										if (WEloToAdd)
										{
											f_Elo = (f_Elo * f_EloCounter + WEloToAdd.value()) / (f_EloCounter + 1);
											f_EloCounter++;

											averElo = WEloToAdd.value();
										}
										if (BEloToAdd)
										{
											f_Elo = (f_Elo * f_EloCounter + BEloToAdd.value()) / (f_EloCounter + 1);
											f_EloCounter++;

											if (averElo)
												averElo = (averElo + BEloToAdd.value()) / 2;
											else
												averElo = BEloToAdd.value();
										}

										bool isGameSorted = false;

										for (size_t fI = 0; fI < f_localClrTopGames.size(); fI++)
										{
											const auto& [index, elo] = f_localClrTopGames[fI];

											if (elo < averElo)
											{
												f_localClrTopGames.insert(f_localClrTopGames.begin() + fI, std::make_pair((size_t)j, averElo));
												isGameSorted = true;
												break;
											}
										}

										if (!isGameSorted && f_localClrTopGames.size() < 21)
											f_localClrTopGames.emplace_back((size_t)j, averElo);

										for (int nextMove = moveIndex + 1; nextMove < game.GetMovePathbyRef().move.size(); nextMove++)
										{
											if (game.GetMovePathbyRef().move[nextMove].first != 255ui8)
											{
												auto& [lw, lb, ld, lAE, lLP, lLD] = f_localClrResults[game.GetMovePathbyRef().move[nextMove]];

												if (res == w)
													lw++;
												else if (res == b)
													lb++;
												else if (res == d)
													ld++;

												if (WEloToAdd)
												{
													lAE = (lAE * lLP + WEloToAdd.value()) / (lLP + 1);
													lLP++;
												}
												if (BEloToAdd)
												{
													lAE = (lAE * lLP + BEloToAdd.value()) / (lLP + 1);
													lLP++;
												}
												if (DateToAdd)
												{
													if (lLD < DateToAdd.value())
														lLD = DateToAdd.value();
												}

												break;
											}
										}
									}
								}								
							}

							//still there is a possibility that the search was cleared meanwhile

							{
								std::scoped_lock lock(*f_mtx);

								for (int j = 0; j < f_positiveIndexes.size(); j++)
									f_searchPtr->second.PossitiveIndexes.emplace_back(f_positiveIndexes[j]);

								if (s_ChessFileManager->m_ClrData.contains(f_fileID) && f_clrResults && f_clrTopGames)
								{
									auto& [w, b, d, AE, LP, LD] = (*f_clrResults)[{0, 0}];
									w += f_wWins;
									b += f_bWins;
									d += f_Draws;

									if (f_EloCounter > 0)
									{
										AE = (AE * LP + f_Elo * f_EloCounter) / (LP + f_EloCounter);
										LP += f_EloCounter;
									}

									for (auto& [move, local] : f_localClrResults)
									{
										auto& [mw, mb, md, mAE, mLP, mLD] = (*f_clrResults)[move];
										auto& [lmw, lmb, lmd, lmAE, lmLP, lmLD] = local;
										mw += lmw;
										mb += lmb;
										md += lmd;

										if (lmLP > 0)
										{
											mAE = (mAE * mLP + lmAE * lmLP) / (mLP + lmLP);
											mLP += lmLP;
										}
										if (lmLD > mLD)
											mLD = lmLD;
									}

									if (f_localClrTopGames.size() > 20)
										f_localClrTopGames.resize(20);

									for (auto& [index, elo] : f_localClrTopGames)
									{
										bool isGameSorted = false;
										for (int gameIndex = 0; gameIndex < f_clrTopGames->size(); gameIndex++)
										{
											if (s_ChessFileManager->m_ClrSearchTopGames[f_currentSearchID].second[gameIndex] < elo)
											{
												f_clrTopGames->insert(f_clrTopGames->begin() + gameIndex, index);
												s_ChessFileManager->m_ClrSearchTopGames[f_currentSearchID].second.insert(s_ChessFileManager->m_ClrSearchTopGames[f_currentSearchID].second.begin() + gameIndex, elo);

												isGameSorted = true;
												break;
											}
										}

										if (!isGameSorted && f_clrTopGames->size() < 21)
										{
											f_clrTopGames->emplace_back(index);
											s_ChessFileManager->m_ClrSearchTopGames[f_currentSearchID].second.emplace_back(elo);
										}
									}

									if (f_clrTopGames->size() > 20)
										f_clrTopGames->resize(20);

									if (s_ChessFileManager->m_ClrSearchTopGames[f_currentSearchID].second.size() > 20)
										s_ChessFileManager->m_ClrSearchTopGames[f_currentSearchID].second.resize(20);
								}								
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
	}

	ChessFileManager& ChessFileManager::Get()
	{
		return *s_ChessFileManager;
	}

	void ChessFileManager::AddFileReference(FileManager::FileID fileID, std::shared_ptr<std::vector<size_t>> pointers)
	{
		std::unique_lock lock(s_ChessManagerDataAccessMutex);

		m_PgnData[fileID].gamePointers = pointers;
	}

	void ChessFileManager::AddCldFileReference(FileManager::FileID fileID,
		std::shared_ptr<std::vector<size_t>> gamePointers, 
		std::shared_ptr<std::vector<std::string>> labelNames, 
		std::shared_ptr<std::vector<std::string>> labelValues, 
		std::shared_ptr<uint8_t> typeName,
		std::shared_ptr<uint8_t> typeValue,
		std::shared_ptr<uint8_t> settings)
	{
		std::unique_lock lock(s_ChessManagerDataAccessMutex);

		auto& cldD = m_CldData[fileID];
		cldD.gamePointers = gamePointers;
		cldD.labelNames = labelNames;
		cldD.labelValues = labelValues;
		cldD.typeName = typeName;
		cldD.typeValue = typeValue;
		cldD.settings = settings;
	}

	void ChessFileManager::AddClrFileReference(FileManager::FileID fileID,
		std::shared_ptr<std::vector<size_t>> gamePointers,
		std::shared_ptr<std::vector<std::string>> labelNames,
		std::shared_ptr<std::vector<std::string>> labelValues,
		std::shared_ptr<uint8_t> typeName,
		std::shared_ptr<uint8_t> typeValue,
		std::shared_ptr<uint8_t> settings,
		std::shared_ptr<std::tuple<size_t, size_t, size_t, size_t, size_t, size_t, size_t>> searchGameIndexes)
	{
		std::unique_lock lock(s_ChessManagerDataAccessMutex);

		auto& clrD = m_ClrData[fileID];
		clrD.gamePointers = gamePointers;
		clrD.labelNames = labelNames;
		clrD.labelValues = labelValues;
		clrD.typeName = typeName;
		clrD.typeValue = typeValue;
		clrD.settings = settings;
		clrD.searchGameIndexes = searchGameIndexes;
	}

	void ChessFileManager::RemoveFileReference(FileManager::FileID fileID)
	{
		std::unique_lock lock(s_ChessManagerDataAccessMutex);

		if (s_ChessFileManager->m_PgnData.contains(fileID))
			m_PgnData.erase(fileID);
		else if (s_ChessFileManager->m_CldData.contains(fileID))
			m_CldData.erase(fileID);
		else if (s_ChessFileManager->m_ClrData.contains(fileID))
			m_ClrData.erase(fileID);

		m_EditedGames.erase(fileID);

		//make it diff function
		for (auto it = s_ChessFileManager->m_GamesTimer.begin(); it != s_ChessFileManager->m_GamesTimer.end(); ++it)
		{
			auto& [key, value] = *it;

			if (key.first == fileID)
			{
				s_ChessFileManager->m_Games[key].SetCurrentAsInitial();
			}
		}
	}

	PgnGame& ChessFileManager::GetGame(FileManager::FileID fileID, size_t index)
	{
		std::unique_lock lock(s_ChessManagerDataAccessMutex);

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

		if (m_ClrData.contains(fileID) && m_ClrData.at(fileID).gamePointers->size() <= index)
		{
			return (PgnGame&)*(PgnGame*)(nullptr);
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

			bool wencode = ((*s_ChessFileManager->m_CldData.at(fileID).settings) % 2) == 0;
			MoveEncoding encoding = MoveEncoding::CLD;
			if (!wencode)
				encoding = MoveEncoding::CORE;

			CldGame cldGame;
			cldGame.Parse(data, encoding, (*m_CldData[fileID].typeName), (*m_CldData[fileID].typeValue), true);

			auto& newGame = m_Games[{ fileID, index }];
			
			for (auto& nameIndex : cldGame.GetLabelNames())
				newGame[(*m_CldData[fileID].labelNames)[nameIndex]] = (*m_CldData[fileID].labelValues)[cldGame[nameIndex]];
			
			ConvertCldMovePathToPgnMovePath(newGame.GetMovePathbyRef(), cldGame.GetMovePathbyRef(), encoding, 0, true);
			newGame.SetCurrentAsInitial();

			m_GamesTimer[{ fileID, index }] = std::chrono::high_resolution_clock::now();
			return m_Games.at({ fileID, index });
		}

		if (m_ClrData.contains(fileID))
		{
			std::vector<uint8_t> data;
			FileManager::Get().ReadBuffer(fileID, (*m_ClrData[fileID].gamePointers)[index], ((index + 1 < m_ClrData[fileID].gamePointers->size()) ? ((*m_ClrData[fileID].gamePointers)[index + 1] - (*m_ClrData[fileID].gamePointers)[index]) : SIZE_MAX), (std::vector<uint8_t>&)data);

			bool wencode = ((*s_ChessFileManager->m_ClrData.at(fileID).settings) % 2) == 0;
			MoveEncoding encoding = MoveEncoding::CLD;
			if (!wencode)
				encoding = MoveEncoding::CORE;

			CldGame cldGame;
			cldGame.Parse(data, encoding, (*m_ClrData[fileID].typeName), (*m_ClrData[fileID].typeValue), true);

			auto& newGame = m_Games[{ fileID, index }];

			for (auto& nameIndex : cldGame.GetLabelNames())
				newGame[(*m_ClrData[fileID].labelNames)[nameIndex]] = (*m_ClrData[fileID].labelValues)[cldGame[nameIndex]];

			ConvertCldMovePathToPgnMovePath(newGame.GetMovePathbyRef(), cldGame.GetMovePathbyRef(), encoding, 0, true);
			newGame.SetCurrentAsInitial();

			m_GamesTimer[{ fileID, index }] = std::chrono::high_resolution_clock::now();
			return m_Games.at({ fileID, index });
		}
	}

	void ChessFileManager::GetGames(FileManager::FileID fileID, size_t index, size_t size, std::vector<PgnGame*>& games)
	{
		std::unique_lock lock(s_ChessManagerDataAccessMutex);

		games.clear();

		std::unordered_set<size_t> alreadyLoadedIndexes;

		for (int i = 0; i < size; i++)
		{
			if (m_Games.contains({ fileID, index + i }))
			{
				alreadyLoadedIndexes.insert(index + i);
			}
		}

		if (alreadyLoadedIndexes.size() == size)
		{
			for (int i = 0; i < size; i++)
			{
				games.emplace_back(&m_Games.at({ fileID, index + i }));
				m_GamesTimer[{ fileID, index + i }] = std::chrono::high_resolution_clock::now();
			}

			return;
		}

		if (m_PgnData.contains(fileID))
		{
			if (index >= m_PgnData.at(fileID).gamePointers->size())
			{
				for (int i = 0; i < size; i++)
				{
					m_EditedGames[fileID].emplace_back(index + i);
					m_GamesTimer[{ fileID, index + i }] = std::chrono::high_resolution_clock::now();
					games.emplace_back(&m_Games[{ fileID, index + i }]);
				}

				return;
			}

		}

		if (m_CldData.contains(fileID))
		{
			if (index >= m_CldData.at(fileID).gamePointers->size())
			{
				for (int i = 0; i < size; i++)
				{
					m_EditedGames[fileID].emplace_back(index + i);
					m_GamesTimer[{ fileID, index + i }] = std::chrono::high_resolution_clock::now();
					games.emplace_back(&m_Games[{ fileID, index + i }]);
				}

				return;
			}
		}

		if (m_ClrData.contains(fileID))
		{
			if (index >= m_ClrData.at(fileID).gamePointers->size())
			{
				return;
			}
		}

		if (!FileManager::Get().HasFile(fileID))
			return;

		if (m_PgnData.contains(fileID))
		{
			size_t maxIndex = std::min(index + size, m_PgnData.at(fileID).gamePointers->size());

			std::vector<uint8_t> data;
			FileManager::Get().ReadBuffer(fileID, (*m_PgnData[fileID].gamePointers)[index], ((maxIndex < m_PgnData[fileID].gamePointers->size()) ? ((*m_PgnData[fileID].gamePointers)[maxIndex] - (*m_PgnData[fileID].gamePointers)[index]) : SIZE_MAX), (std::vector<uint8_t>&)data);

			for (size_t i = index; i < maxIndex; i++)
			{
				if (alreadyLoadedIndexes.contains(i))
				{
					m_GamesTimer[{ fileID, i }] = std::chrono::high_resolution_clock::now();
					games.emplace_back(&m_Games.at({ fileID, i }));
				}
				else
				{
					m_Games[{ fileID, i }].Parse(std::string_view{ (char*)data.data() + (*m_PgnData[fileID].gamePointers)[i] - (*m_PgnData[fileID].gamePointers)[index], (i == m_PgnData.at(fileID).gamePointers->size() - 1 ? (data.size()+ (*m_PgnData[fileID].gamePointers)[index] - (*m_PgnData[fileID].gamePointers)[i]) : ((*m_PgnData[fileID].gamePointers)[i + 1] - (*m_PgnData[fileID].gamePointers)[i])) });
					m_GamesTimer[{ fileID, i }] = std::chrono::high_resolution_clock::now();
					
					games.emplace_back(&m_Games.at({ fileID, i }));
				}
			}

			if (maxIndex >= m_PgnData.at(fileID).gamePointers->size())
			{
				for (int i = 0; i < index + size - m_PgnData.at(fileID).gamePointers->size(); i++)
				{
					m_EditedGames[fileID].emplace_back(maxIndex + i);
					m_GamesTimer[{ fileID, maxIndex + i }] = std::chrono::high_resolution_clock::now();
					games.emplace_back(&m_Games[{ fileID, maxIndex + i }]);
				}
			}

			return;
		}

		if (m_CldData.contains(fileID))
		{
			size_t maxIndex = std::min(index + size, m_CldData.at(fileID).gamePointers->size());

			std::vector<uint8_t> data;
			FileManager::Get().ReadBuffer(fileID, (*m_CldData[fileID].gamePointers)[index], ((maxIndex < m_CldData[fileID].gamePointers->size()) ? ((*m_CldData[fileID].gamePointers)[maxIndex] - (*m_CldData[fileID].gamePointers)[index]) : SIZE_MAX), (std::vector<uint8_t>&)data);

			for (size_t i = index; i < maxIndex; i++)
			{
				if (alreadyLoadedIndexes.contains(i))
				{
					m_GamesTimer[{ fileID, i }] = std::chrono::high_resolution_clock::now();
					games.emplace_back(&m_Games.at({ fileID, i }));
				}
				else
				{
					bool wencode = ((*s_ChessFileManager->m_CldData.at(fileID).settings) % 2) == 0;
					MoveEncoding encoding = MoveEncoding::CLD;
					if (!wencode)
						encoding = MoveEncoding::CORE;

					CldGame cldGame;
					cldGame.Parse(std::span(data.data() + (*m_CldData[fileID].gamePointers)[i] - (*m_CldData[fileID].gamePointers)[index], i == m_CldData.at(fileID).gamePointers->size() - 1 ? (data.size() + (*m_CldData[fileID].gamePointers)[index] - (*m_CldData[fileID].gamePointers)[i]) : ((*m_CldData[fileID].gamePointers)[i + 1] - (*m_CldData[fileID].gamePointers)[i])), encoding, (*m_CldData[fileID].typeName), (*m_CldData[fileID].typeValue));

					auto& newGame = m_Games[{ fileID, i }];

					for (auto& nameIndex : cldGame.GetLabelNames())
						newGame[(*m_CldData[fileID].labelNames)[nameIndex]] = (*m_CldData[fileID].labelValues)[cldGame[nameIndex]];

					ConvertCldMovePathToPgnMovePath(newGame.GetMovePathbyRef(), cldGame.GetMovePathbyRef(), encoding, 0, true);
					newGame.SetCurrentAsInitial();

					m_GamesTimer[{ fileID, i }] = std::chrono::high_resolution_clock::now();
					games.emplace_back(&m_Games.at({ fileID, i }));
				}
			}

			if (maxIndex >= m_CldData.at(fileID).gamePointers->size())
			{
				for (int i = 0; i < index + size - m_CldData.at(fileID).gamePointers->size(); i++)
				{
					m_EditedGames[fileID].emplace_back(maxIndex + i);
					m_GamesTimer[{ fileID, maxIndex + i }] = std::chrono::high_resolution_clock::now();
					games.emplace_back(&m_Games[{ fileID, maxIndex + i }]);
				}
			}

			return;
		}

		if (m_ClrData.contains(fileID))
		{
			size_t maxIndex = std::min(index + size, m_ClrData.at(fileID).gamePointers->size());

			std::vector<uint8_t> data;
			FileManager::Get().ReadBuffer(fileID, (*m_ClrData[fileID].gamePointers)[index], ((maxIndex < m_ClrData[fileID].gamePointers->size()) ? ((*m_ClrData[fileID].gamePointers)[maxIndex] - (*m_ClrData	[fileID].gamePointers)[index]) : SIZE_MAX), (std::vector<uint8_t>&)data);

			for (size_t i = index; i < maxIndex; i++)
			{
				if (alreadyLoadedIndexes.contains(i))
				{
					m_GamesTimer[{ fileID, i }] = std::chrono::high_resolution_clock::now();
					games.emplace_back(&m_Games.at({ fileID, i }));
				}
				else
				{
					bool wencode = ((*s_ChessFileManager->m_ClrData.at(fileID).settings) % 2) == 0;
					MoveEncoding encoding = MoveEncoding::CLD;
					if (!wencode)
						encoding = MoveEncoding::CORE;

					CldGame cldGame;
					cldGame.Parse(std::span(data.data() + (*m_ClrData[fileID].gamePointers)[i] - (*m_ClrData[fileID].gamePointers)[index], i == m_ClrData.at(fileID).gamePointers->size() - 1 ? (data.size() + (*m_ClrData[fileID].gamePointers)[index] - (*m_ClrData[fileID].gamePointers)[i]) : ((*m_ClrData[fileID].gamePointers)[i + 1] - (*m_ClrData[fileID].gamePointers)[i])), encoding, (*m_ClrData[fileID].typeName), (*m_ClrData[fileID].typeValue));

					auto& newGame = m_Games[{ fileID, i }];

					for (auto& nameIndex : cldGame.GetLabelNames())
						newGame[(*m_ClrData[fileID].labelNames)[nameIndex]] = (*m_ClrData[fileID].labelValues)[cldGame[nameIndex]];

					ConvertCldMovePathToPgnMovePath(newGame.GetMovePathbyRef(), cldGame.GetMovePathbyRef(), encoding, 0, true);
					newGame.SetCurrentAsInitial();

					m_GamesTimer[{ fileID, i }] = std::chrono::high_resolution_clock::now();
					games.emplace_back(&m_Games.at({ fileID, i }));
				}
			}

			return;
		}
	}

	void ChessFileManager::RemoveFromEditedGames(FileManager::FileID fileID, size_t index)
	{
		std::unique_lock lock(s_ChessManagerDataAccessMutex);
		
		if (!m_EditedGames.contains(fileID))
			return;
		
		for (int i = 0; i < m_EditedGames.at(fileID).size(); i++)
		{
			if (m_EditedGames.at(fileID)[i] == index)
			{
				m_EditedGames.at(fileID).erase(m_EditedGames.at(fileID).begin() + i);
				return;
			}
		}
	}

	void ChessFileManager::GetEditedGames(FileManager::FileID fileID, std::vector<size_t>& indexes) const
	{
		std::unique_lock lock(s_ChessManagerDataAccessMutex);

		if (!m_EditedGames.contains(fileID))
			return;

		indexes = m_EditedGames.at(fileID);
	}

	bool ChessFileManager::IsGameEdited(FileManager::FileID fileID, size_t index) const
	{
		std::unique_lock lock(s_ChessManagerDataAccessMutex);

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
		std::unique_lock lock(s_ChessManagerSearchAccessMutex);

		if (m_Searches.contains(id))
			m_Searches.erase(id);

		if (m_ClrSearchResults.contains(id))
			m_ClrSearchResults.erase(id);

		if (m_ClrSearchTopGames.contains(id))
			m_ClrSearchTopGames.erase(id);
	}

	void ChessFileManager::StartSearch(SearchID id, FileManager::FileID fileID, std::shared_ptr<std::pair<SearchOptions, SearchResult>> seachPtr)
	{
		std::unique_lock lock(s_ChessManagerSearchAccessMutex);

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

	void ChessFileManager::SetUpClrResults(SearchID id, std::shared_ptr<SearchResultMoveData> searchResults, std::shared_ptr<std::vector<size_t>> searchTopGames)
	{
		std::unique_lock lock(s_ChessManagerSearchAccessMutex);

		if (m_ClrSearchResults.contains(id))
			return;

		m_ClrSearchResults[id] = searchResults;
		m_ClrSearchTopGames[id] = { searchTopGames, {} };
	}


	void ChessFileManager::ConvertCldMovePathToPgnMovePath(PgnGame::ChessMovesPath& pgnMovePath, const CldGame::CldMovesPath& cldMovePath, MoveEncoding encoding, int moveIndex, bool reset)
	{
		static const char* pieceNames = "NBRQPK";
		static Board staticBoard;

		if (reset)
			staticBoard.NewPosition();

		Board myBoard = staticBoard;
		Board::Move prevMove;
		Piece prevProm = NONE;

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
				Board oldBoard = staticBoard;

				pgnMovePath.move.emplace_back("child");

				int moveIndexToPass = moveIndex;

				if (moveIndexToPass > 0)
					moveIndexToPass -= 1;
				else
					moveIndexToPass *= -1;

				ConvertCldMovePathToPgnMovePath(pgnMovePath.children.emplace_back(), cldMovePath.children[index], encoding, moveIndexToPass);
				index++;

				staticBoard = oldBoard;

				continue;
			}

			std::string moveToAdd;

			moveIndex *= -1;
			
			if (moveIndex >= 0)
			{
				moveIndex += 1;
				moveToAdd += (std::to_string(moveIndex) + ". ");
			}

			if (encoding == MoveEncoding::CLD)
			{
				moveToAdd += Board::ConvertCLDMoveToPGNMove(*move);
				pgnMovePath.move.emplace_back(moveToAdd);
			}
			else if (encoding == MoveEncoding::CORE)
			{
				Board::Move coreMove;
				Piece promType = NONE;

				coreMove.index = move->first;
				coreMove.move = uint8_t(move->second >> 2) - coreMove.index;
				promType = Piece((move->second & 0b00000011) + 1);

				moveToAdd = myBoard.ConvertCoreMoveToPGNMove(coreMove, promType);

				pgnMovePath.move.emplace_back(moveToAdd);
				if (myBoard.MakeMove(coreMove, promType) != Board::SUCCESS)
					break;
				if (prevMove.move != 0)
					staticBoard.MakeMove(prevMove, prevProm);
				
				prevMove = coreMove;
				prevProm = promType;
			}

			if (cldMovePath.details.contains(i))
			{
				auto& detail = pgnMovePath.details[i];
				detail.cmds = cldMovePath.details.at(i).cmds;
				detail.note = cldMovePath.details.at(i).note;
			}
		}

		if (reset)
			pgnMovePath.ReloadChildren();
	}

	void ChessFileManager::ConvertPgnMovePathToCldMovePath(CldGame::CldMovesPath& cldMovePath, const PgnGame::ChessMovesPath& pgnMovePath, MoveEncoding encoding, bool reset)
	{
		const uint8_t childIndexID = 255;
		static Board staticBoard;

		if (reset)
			staticBoard.NewPosition();

		Board myBoard = staticBoard;
		Board::Move prevMove = { 0, 0 };
		Piece prevProm = NONE;

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
				Board oldBoard = staticBoard;

				cldMovePath.move.emplace_back(childIndexID, childIndexID);

				auto& child = cldMovePath.children.emplace_back(&cldMovePath);
				ConvertPgnMovePathToCldMovePath(child, pgnMovePath.children[index], encoding);
				index++;

				staticBoard = oldBoard;

				continue;
			}

			if (encoding == MoveEncoding::CLD)
			{
				auto moveCld = Board::ConvertPGNMoveToCLDMove(move);
				cldMovePath.move.emplace_back(moveCld.first, moveCld.second);
			}
			else if (encoding == MoveEncoding::CORE)
			{
				Board::Move coreMove;
				Piece promType;
				myBoard.ConvertPGNMoveToCoreMove(coreMove, promType, move);

				uint8_t promotionBits = (promType - 1) & 0b00000011;

				cldMovePath.move.emplace_back(coreMove.index, (uint8_t(coreMove.move + coreMove.index) << 2) | promotionBits);

				if (myBoard.MakeMove(coreMove, promType) != Board::SUCCESS)
					break;
				if (prevMove.move != 0)
					staticBoard.MakeMove(prevMove, prevProm);

				prevMove = coreMove;
				prevProm = promType;
			}

			if (pgnMovePath.details.contains(i) && (pgnMovePath.details.at(i).note != "" || !pgnMovePath.details.at(i).cmds.empty()))
			{
				auto& detail = pgnMovePath.details.at(i);
				cldMovePath.details[i].cmds = detail.cmds;
				cldMovePath.details[i].note = detail.note;
			}
		}

		if (reset)
			cldMovePath.ReloadChildren();
	}
}