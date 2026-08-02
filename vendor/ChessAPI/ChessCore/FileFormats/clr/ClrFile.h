#pragma once

#include "../base/Encoding.h"
#include "../base/ChessFile.h"
#include "ClrPairMoveHash.h"

namespace Chess
{
	class ClrFile
	{
	public:
		ClrFile();
		~ClrFile();

		void OpenFile(const std::filesystem::path& path, float* persentage = nullptr);

		FileManager::FileID GetID() const;
		size_t GetSize() const;
		MoveEncoding GetEncoding() const;
		
		PgnGame& operator[] (size_t index);
		const PgnGame& At(size_t index) const;

		ClrFile& operator=(const ClrFile& other) = delete;
		void Clear();

		void StartSearch(size_t index);
		void StopSearch(size_t index);

		std::shared_ptr<std::pair<SearchOptions, SearchResult>> GetSearch(size_t index);

		void GetCldGame(CldGame& game, size_t index);
		size_t GetWhiteWins() const;
		size_t GetBlackWins() const;
		size_t GetDraws() const;
		size_t GetAverageElo() const;
		std::vector<size_t> GetSearchTopGames() const;

		std::shared_ptr<SearchResultMoveData> GetMoveResults() const;

	private:
		void LoadDataPointers(float* persentage = nullptr);

	private:
		FileManager::FileID m_ID;

		std::shared_ptr<std::vector<std::string>> m_LabelNames;
		std::shared_ptr<std::vector<std::string>> m_LabelValues;
		std::shared_ptr<std::vector<size_t>> m_GamePointers;
		std::shared_ptr<uint8_t> m_typeName;
		std::shared_ptr<uint8_t> m_typeValue;
		std::shared_ptr<uint8_t> m_Settings;

		size_t m_LabelNamesPointer = 0;
		size_t m_LabelValuesPointer = 0;
		size_t m_GamePointersPointer = 0;

		std::pair<SearchID, std::shared_ptr<std::pair<SearchOptions, SearchResult>>> m_Search;

		//white, black, draw, average elo, games with elo, last Year played
		std::shared_ptr<SearchResultMoveData> m_SearchGameResults;
		std::shared_ptr<std::vector<size_t>> m_SearchTopGames;

		//result, white win, black win, draw, white elo, black elo, date
		std::shared_ptr<std::tuple<size_t, size_t, size_t, size_t, size_t, size_t, size_t>> m_SearchGameIndexes;
	};
}