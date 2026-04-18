#pragma once

#include <filesystem>
#include <unordered_set>

#include "../FileManager.h"
#include "../pgn/PgnGame.h"

#include "../SearchWork.h"

namespace Chess
{
	class ChessFile
	{
	public:
		virtual ~ChessFile() = default;

		virtual void OpenFile(const std::filesystem::path& path, float* persentage = nullptr) = 0;
		virtual void SaveFile(const std::filesystem::path& path, float* persentage = nullptr) = 0;

		virtual FileManager::FileID GetID() const = 0;
		virtual size_t GetSize() const = 0;
		virtual std::unordered_set<size_t> GetDeletedGames() const = 0;
		virtual const PgnGame& At(size_t index) const = 0;
		virtual PgnGame& operator[] (size_t index) = 0;

		virtual void Clear() = 0;

		virtual void CreateGame(size_t index = -1) = 0; //index = -1 -> placed at the end
		virtual void DeleteGame(size_t index) = 0;
		virtual void RecoverGame(size_t index) = 0;
		virtual void MoveGame(size_t position, size_t direction) = 0;

		virtual bool IsGameDeleted(size_t index) const = 0;
		virtual bool IsGameEdited(size_t index) const = 0;

		virtual std::shared_ptr<std::pair<SearchOptions, SearchResult>> AddSearch() = 0;
		virtual void RemoveSearch(size_t index) = 0;
		virtual void StartSearch(size_t index) = 0;
		virtual void StopSearch(size_t index) = 0;

		virtual size_t GetSearchesCount() const = 0;
		virtual std::shared_ptr<std::pair<SearchOptions, SearchResult>> GetSearch(size_t index) = 0;
		virtual std::string* GetSearchName(size_t index) = 0;

	private:
		virtual void LoadDataPointers(float* persentage = nullptr) = 0;

		virtual void LoadSearchIndexes(const std::filesystem::path& cachePath) = 0;
		virtual void SaveSearchIndexes(const std::filesystem::path& cachePath) = 0;
	};

}