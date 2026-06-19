#include "FileManager.h"

#include <fstream>
#include "../external.h"

static Chess::FileManager* s_Instance = nullptr;
static std::mutex s_InstanceMutex;

namespace Chess
{
	FileManager& FileManager::Get()
	{
		if (!s_Instance)
			s_Instance = new FileManager();
		return *s_Instance;
	}

	void FileManager::Init()
	{
		if (!s_Instance)
			s_Instance = new FileManager();
	}

	void FileManager::Shutdown()
	{
		if (s_Instance)
		{
			delete s_Instance;
			s_Instance = nullptr;
		}
	}

	FileManager::FileID FileManager::AddFile(const std::filesystem::path& filepath)
	{
		std::scoped_lock InstanceLock(s_InstanceMutex);
		
		std::error_code ec;
		auto cpath = std::filesystem::canonical(filepath, ec);
		
		if (ec || !std::filesystem::exists(cpath))
			return FileID();
		
		FileID id = FileID();
		m_FileMutex[id] = std::make_unique<std::shared_mutex>();

		std::scoped_lock lock(*m_FileMutex[id].get());
		m_File[id] = cpath;
		
		return id;
	}

	void FileManager::RemoveFile(FileID id)
	{
		std::scoped_lock InstanceLock(s_InstanceMutex);

		if (m_File.contains(id))
			m_File.erase(id);
		
		if (m_FileMutex.contains(id))
			m_FileMutex.erase(id);
	}

	bool FileManager::HasFile(FileID id) const
	{
		std::scoped_lock InstanceLock(s_InstanceMutex);

		return m_File.contains(id);
	}

	std::filesystem::path FileManager::GetFilePath(FileID id) const
	{
		if (!m_FileMutex.contains(id))
			return "";

		std::scoped_lock lock(*m_FileMutex.at(id).get());

		if (m_File.contains(id))
			return m_File.at(id);
		
		return "";
	}

	std::filesystem::path FileManager::GetCachePath(FileID id) const
	{
		if (!m_FileMutex.contains(id))
			return "";

		std::scoped_lock lock(*m_FileMutex.at(id).get());

		if (m_File.contains(id))
		{
			PathHash hasher;
			auto hash = hasher(m_File.at(id));
			auto cachePath = GetCacheDirectory() / std::to_string(hash);

			bool firstTime = !std::filesystem::exists(cachePath);

			if (!firstTime)
				firstTime = !std::filesystem::is_directory(cachePath);

			if (firstTime)
				std::filesystem::create_directories(cachePath);

			return cachePath;
		}
		
		return "";
	}


	bool FileManager::ReadBuffer(FileID id, size_t startIndex, size_t amount, std::vector<uint8_t>& data) const
	{
		data.clear();

		if (!m_FileMutex.contains(id))
			return false;

		std::shared_lock lock(*m_FileMutex.at(id).get());
		
		if(!m_File.contains(id))
			return false;

		std::ifstream file(m_File.at(id), std::ios::binary);
		
		if (!file)
			return false;

		file.seekg(0, std::ios_base::end);
		std::streampos maxIndex = file.tellg();
		
		size_t maxAmount = std::streamoff(maxIndex) - startIndex;
		data.resize(std::min(amount, maxAmount));

		file.seekg(startIndex, std::ios::beg);

		file.read(reinterpret_cast<char*>(data.data()), data.size());

		return true;
	}

	bool FileManager::ReadBuffer(FileID id, size_t startIndex, size_t amount, std::vector<uint64_t>& data) const
	{
		data.clear();

		if (!m_FileMutex.contains(id))
			return false;

		std::shared_lock lock(*m_FileMutex.at(id).get());

		if (!m_File.contains(id))
			return false;

		std::ifstream file(m_File.at(id), std::ios::binary);

		if (!file)
			return false;

		file.seekg(0, std::ios_base::end);
		std::streampos maxIndex = file.tellg();

		size_t maxAmount = std::streamoff(maxIndex) - startIndex;
		size_t readAmount = std::min(amount, maxAmount);
		data.resize(std::ceil(readAmount / 8.0));
		
		file.seekg(startIndex, std::ios::beg);

		file.read(reinterpret_cast<char*>(data.data()), readAmount);

		return true;
	}

	size_t FileManager::GetMaxFileSize(FileID id) const
	{
		if (!m_FileMutex.contains(id))
			return 0;

		std::scoped_lock lock(*m_FileMutex.at(id).get());

		if (!m_File.contains(id))
			return 0;

		std::ifstream file(m_File.at(id), std::ios::binary);

		if (!file)
			return 0;

		file.seekg(0, std::ios_base::end);
		std::streampos maxIndex = file.tellg();

		return std::streamoff(maxIndex);
	}

}