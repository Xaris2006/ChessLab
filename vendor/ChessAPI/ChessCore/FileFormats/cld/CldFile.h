#pragma once

#include "../base/ChessFile.h"

namespace Chess
{
	class CldFile : public ChessFile
	{
	public:
		CldFile();
		virtual ~CldFile();

		virtual void OpenFile(const std::filesystem::path& path) override;
		virtual void SaveFile(const std::filesystem::path& path) override;

		virtual FileManager::FileID GetID() const override;
		virtual size_t GetSize() const override;
		virtual std::unordered_set<size_t> GetDeletedGames() const override;
		virtual PgnGame& operator[] (size_t index) override;
		virtual const PgnGame& At (size_t index) const override;

		CldFile& operator=(const CldFile& other) = delete;
		virtual void Clear();

		virtual void CreateGame(size_t index = -1) override; //index = -1 -> placed at the end
		virtual void DeleteGame(size_t index) override;
		virtual void RecoverGame(size_t index) override;
		virtual void MoveGame(size_t position, size_t direction) override;

		virtual bool IsGameDeleted(size_t index) const override;
		virtual bool IsGameEdited(size_t index) const override;

		virtual std::shared_ptr<std::pair<SearchOptions, SearchResult>> AddSearch() override;
		virtual void RemoveSearch(size_t index) override;
		virtual void StartSearch(size_t index) override;
		virtual void StopSearch(size_t index) override;

		virtual size_t GetSearchesCount() const override;
		virtual std::shared_ptr<std::pair<SearchOptions, SearchResult>> GetSearch(size_t index) override;
		virtual std::string* GetSearchName(size_t index) override;

		void GetCldGame(CldGame& game, size_t index);

	public:
		static void RemoveDeletedGames(const std::filesystem::path& path);
		friend void ConvertToPgn(const CldFile& cldFile, const std::filesystem::path& destination);

	private:
		virtual void LoadDataPointers() override;

		virtual void LoadSearchIndexes(const std::filesystem::path& cachePath) override;
		virtual void SaveSearchIndexes(const std::filesystem::path& cachePath) override;

	private:
		FileManager::FileID m_ID;

		std::unordered_set<size_t> m_DeletedGames;

		std::shared_ptr<std::vector<std::string>> m_LabelNames;
		std::shared_ptr<std::vector<std::string>> m_LabelValues;
		std::shared_ptr<std::vector<size_t>> m_GamePointers;
		std::shared_ptr<uint8_t> m_typeName;
		std::shared_ptr<uint8_t> m_typeValue;

		size_t m_LabelNamesPointer = 0;
		size_t m_LabelValuesPointer = 0;
		size_t m_GamePointersPointer = 0;

		int m_AddedGamesCount = 0;

		std::vector<std::tuple<SearchID, std::string, std::shared_ptr<std::pair<SearchOptions, SearchResult>>>> m_Searches;
	};

	void ConvertToPgn(const CldFile& cldFile, const std::filesystem::path& destination);
}