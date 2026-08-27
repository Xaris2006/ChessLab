#pragma once

#include <filesystem>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <memory>

#include "UUID/UUID.h"

namespace Chess
{
	class FileManager
	{
	public:
		using FileID = UUID;

	public:
		static FileManager& Get();
		static void Init();
		static void Shutdown();

	public:
		struct PathHash
		{
			size_t operator() (const std::filesystem::path& path) const
			{
				std::size_t hash = 0;

				if (path.empty())
					return hash;
				
				std::hash<std::string> hasher;

				std::error_code ec;
				auto cpath = std::filesystem::canonical(path, ec);
				
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

	public:
		FileID AddFile(const std::filesystem::path& filepath);
		void RemoveFile(FileID id);
		bool HasFile(FileID id) const;

		std::filesystem::path GetFilePath(FileID id) const;
		std::filesystem::path GetCachePath(FileID id) const;
		size_t GetMaxFileSize(FileID id) const;

		bool ReadBuffer(FileID id, size_t startIndex, size_t amount, std::vector<uint8_t>& data) const;
		bool ReadBuffer(FileID id, size_t startIndex, size_t amount, std::vector<uint64_t>& data) const;
		bool WriteBuffer(FileID id, size_t startIndex, size_t amount, std::vector<uint8_t>& data);

	private:
		FileManager() = default;

	private:
		std::unordered_map<FileID, std::filesystem::path> m_File;
		std::unordered_map<FileID, std::unique_ptr<std::shared_mutex>> m_FileMutex;
	};

}