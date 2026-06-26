#pragma once

#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <filesystem>
#include <functional>
#include <array>

#include "pgn/PgnGame.h"
#include "cld/CldGame.h"

#include "SearchWork.h"
#include "FileManager.h"

namespace Chess
{
	class ChessFileManager
	{
	public:
		static ChessFileManager& Get();
		static void Init();
		static void Shutdown();

	public:
		void AddFileReference(FileManager::FileID fileID, std::shared_ptr<std::vector<size_t>> pointers);
		void AddCldFileReference(FileManager::FileID fileID, 
			std::shared_ptr<std::vector<size_t>> gamePointers,
			std::shared_ptr<std::vector<std::string>> labelNames,
			std::shared_ptr<std::vector<std::string>> labelValues,
			std::shared_ptr<uint8_t> typeName,
			std::shared_ptr<uint8_t> typeValue, 
			std::shared_ptr<uint8_t> settings);

		void RemoveFileReference(FileManager::FileID fileID);
		
		PgnGame& GetGame(FileManager::FileID fileID, size_t index);
		void GetGames(FileManager::FileID fileID, size_t index, size_t size, std::vector<PgnGame*>& games);
		
		void RemoveFromEditedGames(FileManager::FileID fileID, size_t index);
		void GetEditedGames(FileManager::FileID fileID, std::vector<size_t>& indexes) const;
		bool IsGameEdited(FileManager::FileID fileID, size_t index) const;

		void ClearSearch(SearchID id);
		void StartSearch(SearchID id, FileManager::FileID fileID, std::shared_ptr<std::pair<SearchOptions, SearchResult>> seachPtr);

	public:
		static void ConvertCldMovePathToPgnMovePath(PgnGame::ChessMovesPath& pgnMovePath, const CldGame::CldMovesPath& cldMovePath, MoveEncoding encoding, int moveIndex = 0, bool reset = false);
		static void ConvertPgnMovePathToCldMovePath(CldGame::CldMovesPath& cldMovePath, const PgnGame::ChessMovesPath& pgnMovePath, MoveEncoding encoding, bool reset = false);

	private:
		struct UUIDIndexHash
		{
			std::size_t operator()(const std::pair<UUID, std::size_t>& p) const noexcept {
				std::size_t h1 = std::hash<UUID>{}(p.first);
				std::size_t h2 = std::hash<std::size_t>{}(p.second);

				// High-quality hash combine (64-bit friendly)
				return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
			}
		};
		
		struct PgnSharedData
		{
			std::shared_ptr<std::vector<size_t>> gamePointers;
		};

		struct CldSharedData
		{
			std::shared_ptr<std::vector<size_t>> gamePointers;
			std::shared_ptr<std::vector<std::string>> labelNames;
			std::shared_ptr<std::vector<std::string>> labelValues;
			std::shared_ptr<uint8_t> typeName;
			std::shared_ptr<uint8_t> typeValue;
			std::shared_ptr<uint8_t> settings;
		};

	private:
		//make it shared ptr pgngames
		std::unordered_map<std::pair<FileManager::FileID, size_t>, PgnGame, UUIDIndexHash> m_Games;
		std::unordered_map<std::pair<FileManager::FileID, size_t>, std::chrono::high_resolution_clock::time_point, UUIDIndexHash> m_GamesTimer;
		std::unordered_map<FileManager::FileID, std::vector<size_t>> m_EditedGames;
		std::unordered_map<FileManager::FileID, PgnSharedData> m_PgnData;
		std::unordered_map<FileManager::FileID, CldSharedData> m_CldData;
		
		std::unordered_map<SearchID, std::tuple<FileManager::FileID, std::shared_ptr<std::pair<SearchOptions, SearchResult>>, std::shared_ptr<std::mutex>, size_t>> m_Searches;

		std::thread* m_ThreadFileHandler = nullptr;
		bool m_endThread = true;

		static constexpr size_t s_amountOfWorkersMax = 20;
		static constexpr size_t s_amountOfWorkersMin = 1;

		std::array<std::thread*, s_amountOfWorkersMax> m_SearchWorkers;
		std::array<bool, s_amountOfWorkersMax> m_endWorkers;
	};
}