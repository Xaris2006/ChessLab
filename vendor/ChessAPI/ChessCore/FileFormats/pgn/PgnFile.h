#pragma once

#include "../base/ChessFile.h"
#include "../base/Encoding.h"

namespace Chess
{
	class PgnFile : public ChessFile
	{
	public:
		PgnFile();
		virtual ~PgnFile();

		virtual void OpenFile(const std::filesystem::path& path, float* persentage = nullptr) override;
		virtual void SaveFile(const std::filesystem::path& path, float* persentage = nullptr) override;

		virtual FileManager::FileID GetID() const override;
		virtual size_t GetSize() const override;
		virtual std::unordered_set<size_t> GetDeletedGames() const override;
		virtual PgnGame& operator[] (size_t index) override;
		virtual const PgnGame& At (size_t index) const override;

		PgnFile& operator=(const PgnFile& other) = delete;
		virtual void Clear() override;

		virtual void CreateGame(size_t index = -1) override; //index = -1 -> placed at the end
		virtual void DeleteGame(size_t index) override;
		virtual void RecoverGame(size_t index) override;
		virtual void RemoveFromEdited(size_t index) override;
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

	public:
		static void RemoveDeletedGames(const std::filesystem::path& path);
		friend void ConvertToCld(const PgnFile& pgnFile, const std::filesystem::path& destination, MoveEncoding encoding, bool useTable, float* persentage);

	private:
		virtual void LoadDataPointers(float* persentage = nullptr) override;

		virtual void LoadSearchIndexes(const std::filesystem::path& cachePath) override;
		virtual void SaveSearchIndexes(const std::filesystem::path& cachePath) override;

	private:
		FileManager::FileID m_ID;

		std::unordered_set<size_t> m_DeletedGames;
		std::shared_ptr<std::vector<size_t>> m_DataPointers;

		int m_AddedGamesCount = 0;		

		std::vector<std::tuple<SearchID, std::string, std::shared_ptr<std::pair<SearchOptions, SearchResult>>>> m_Searches;
	};

	void ConvertToCld(const PgnFile& pgnFile, const std::filesystem::path& destination, MoveEncoding encoding, bool useTable, float* persentage = nullptr);
}