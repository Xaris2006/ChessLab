#pragma once

#include "PgnManager.h"

namespace Chess
{
	class PgnFile
	{
	public:

		PgnFile();
		~PgnFile();

		void OpenFile(const std::filesystem::path& path);
		void SaveFile(const std::filesystem::path& path);

		PgnManager::FileID GetID() const;
		size_t GetSize() const;
		std::unordered_set<size_t> GetDeletedGames() const;
		PgnGame& operator[] (size_t index);

		PgnFile& operator=(const PgnFile& other) = delete;
		void Clear();

		void CreateGame(size_t index = -1); //index = -1 -> placed at the end
		void DeleteGame(size_t index);
		void RecoverGame(size_t index);
		void MoveGame(size_t position, size_t direction);

		bool IsGameDeleted(size_t index) const;
		bool IsGameEdited(size_t index) const;

	public:
		static void RemoveDeletedGames(const std::filesystem::path& path);

	public:
		struct PgnPath_Hash
		{
			size_t operator() (const std::filesystem::path& path) const
			{
				std::size_t hash = 0;
				std::hash<std::string> hasher;
				
				std::error_code ec;

				auto cpath = std::filesystem::canonical(path, ec);

				if (cpath.extension() != ".pgn")
					return hash;

				hash ^= hasher(cpath.filename().u8string()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

				while (cpath.has_parent_path())
				{
					// Combine the hash values (simple XOR + shift approach)
					hash ^= hasher(cpath.parent_path().u8string()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);

					auto parent = cpath.parent_path();

					if (cpath == parent)
						break;

					cpath = parent;
				}
				return hash;
			}
		};

	private:
		//stream should be opened in binary mode
		friend std::istream& operator>>(std::istream& stream, PgnFile& f);

	private:
		PgnManager::FileID m_ID;
		std::shared_ptr<PgnManager::PgnFileData> m_Data;

		std::unordered_set<size_t> m_DeletedGames;

		int m_AddedGamesCount = 0;		
	};
}