#include "PgnFile.h"

#include <cmath>
#include <fstream>
#include <mutex>

#include "FileHash.h"

extern std::filesystem::path g_cachedDirectory;

namespace Chess
{
	PgnFile::PgnFile()
	{
		m_Data = std::make_shared<Chess::PgnManager::PgnFileData>();
		m_ID = PgnManager::Get().AddFile(m_Data);
	}

	PgnFile::~PgnFile()
	{
		PgnManager::Get().RemoveFile(m_ID);
	}

	void PgnFile::OpenFile(const std::filesystem::path& path)
	{
		Clear();

		m_Data->FilePath = path;
		auto fileHash = HashFile(m_Data->FilePath);

		PgnPath_Hash hasher;
		auto hash = hasher(path);
		auto cachePath = g_cachedDirectory / std::to_string(hash);

		if (std::filesystem::exists(cachePath) && std::filesystem::is_directory(cachePath))
		{
			bool changed = false;

			{
				std::ifstream infile(cachePath / "thisfile.hpgn", std::ios::binary);

				std::array<uint8_t, HASH_LENGTH> NFileHash;

				infile.read((char*)NFileHash.data(), HASH_LENGTH);

				if (NFileHash != fileHash)
					changed = true;

				infile.close();
			}

			if (!changed)
			{
				{
					std::ifstream infile(cachePath / "thisfile.ppgn", std::ios::binary);

					infile.seekg(0, std::ios_base::end);
					std::streampos maxSize = infile.tellg();
					infile.seekg(0, std::ios_base::beg);

					char* data = new char[maxSize];
					infile.read(data, maxSize);

					infile.close();

					m_Data->DataPointers.reserve(maxSize / 8);

					for (int i = 0; i < maxSize; i += 8)
						m_Data->DataPointers.emplace_back(*(size_t*)(&data[i]));

					delete[] data;
				}
				{
					std::ifstream infile(cachePath / "thisfile.dpgn", std::ios::binary);
					
					infile.seekg(0, std::ios_base::end);
					std::streampos maxSize = infile.tellg();
					infile.seekg(0, std::ios_base::beg);

					char* data = new char[maxSize];
					infile.read(data, maxSize);

					infile.close();

					m_DeletedGames.reserve(maxSize / 8);

					for (int i = 0; i < maxSize; i += 8)
						m_DeletedGames.emplace(*(size_t*)(&data[i]));

					delete[] data;
				}

				return;
			}
		}

		{
			std::ifstream infile(path, std::ios::binary);
			infile >> *this;
			infile.close();
		}

		std::filesystem::create_directories(cachePath);

		{
			std::ofstream outfile(cachePath / "thisfile.ppgn", std::ios::binary);
			outfile.write((char*)m_Data->DataPointers.data(), m_Data->DataPointers.size() * 8);
			outfile.close();
		}
		{
			std::ofstream outfile(cachePath / "thisfile.hpgn", std::ios::binary);
			outfile.write((char*)fileHash.data(), HASH_LENGTH);
			outfile.close();
		}
		{
			std::ofstream outfile(cachePath / "thisfile.dpgn", std::ios::binary);
			outfile.close();
		}
		
		//{
		//	auto deletedfileHash = HashFile(cachePath / "thisfile.dpgn");
		//
		//	std::ofstream outfile(cachePath / "deletedfile.hpgn", std::ios::binary);
		//	outfile.write((char*)deletedfileHash.data(), HASH_LENGTH);
		//	outfile.close();
		//}
	}

	void PgnFile::SaveFile(const std::filesystem::path& path)
	{
		bool diffFile = path != m_Data->FilePath;
		
		if (diffFile)
		{
			std::ofstream testfile(path, std::ios::trunc);
			testfile.close();
		}

		PgnPath_Hash hasher;
		auto hash = hasher(path);
		auto cachePath = g_cachedDirectory / std::to_string(hash);

		bool firstTime = !std::filesystem::exists(cachePath);

		if (!firstTime)
			firstTime = !std::filesystem::is_directory(cachePath);

		if (firstTime)
			std::filesystem::create_directories(cachePath);

		{
			std::ofstream outfile(cachePath / "thisfile.dpgn", std::ios::binary | std::ios::trunc);
			
			for (auto& gameDeleted : m_DeletedGames)
				outfile.write((char*)&gameDeleted, 8);

			outfile.close();
		}

		if (m_Data->EditedGames.empty() && !diffFile)
			return;

		if (m_Data->EditedGames.empty())
		{
			auto fileHash = HashFile(m_Data->FilePath);

			{
				std::ifstream source(m_Data->FilePath, std::ios::binary);
				std::ofstream destination(path, std::ios::binary);

				destination << source.rdbuf();

				source.close();
				destination.close();
			}
			{
				std::ofstream outfile(cachePath / "thisfile.ppgn", std::ios::binary);
				outfile.write((char*)m_Data->DataPointers.data(), m_Data->DataPointers.size() * 8);
				outfile.close();
			}
			{
				std::ofstream outfile(cachePath / "thisfile.hpgn", std::ios::binary);
				outfile.write((char*)fileHash.data(), HASH_LENGTH);
				outfile.close();
			}

			return;
		}

		std::vector<size_t> NDataPointers;
		NDataPointers.reserve(GetSize());
		
		{
			std::ifstream source(m_Data->FilePath, std::ios::binary);
			std::ofstream outfile(cachePath / "helper", std::ios::binary, std::ios::trunc);

			size_t lastIndex = 0;
			size_t NPointersIndex = 0;
			
			std::vector<size_t> EditedGamesSorted;

			auto loadFileByChunk = [this, &source, &outfile, &lastIndex, &NPointersIndex, &NDataPointers](size_t endPoint, size_t gameIndex)
				{
					if (m_Data->DataPointers.empty())
						return;

					const std::streamsize chunkSize = 1024 * 1024;

					std::vector<char> buffer(chunkSize);
					std::streamsize remaining = endPoint - m_Data->DataPointers[lastIndex];

					source.seekg(m_Data->DataPointers[lastIndex]);
					//outfile.seekp(std::ios::app);

					while (remaining > 0)
					{
						std::streamsize currentChunk = std::min(remaining, chunkSize);

						source.read(buffer.data(), currentChunk);
						//std::streamsize bytesRead = source.gcount();
						//if (bytesRead == 0)
						//	continue;

						//outfile.write(buffer.data(), bytesRead);
						outfile.write(buffer.data(), currentChunk);

						//remaining -= bytesRead;
						remaining -= currentChunk;
					}

					for (int i = 0; i < gameIndex - lastIndex; i++)
						NDataPointers.emplace_back(m_Data->DataPointers[lastIndex + i] - m_Data->DataPointers[lastIndex] + NPointersIndex);

					NPointersIndex += (endPoint - m_Data->DataPointers[lastIndex]);
				};
			
			for (auto& game : m_Data->EditedGames)
			{
				bool founded = false;

				for (int i = 0; i < EditedGamesSorted.size(); i++)
				{
					if (game < EditedGamesSorted[i])
					{
						EditedGamesSorted.insert(EditedGamesSorted.begin() + i, game);
						founded = true;
						break;
					}
				}

				if (!founded)
					EditedGamesSorted.emplace_back(game);
			}

			for (auto& game : EditedGamesSorted)
			{
				if (game > lastIndex && game < m_Data->DataPointers.size())
					loadFileByChunk(m_Data->DataPointers[game], game);
				
				if (game == m_Data->DataPointers.size())
				{
					source.seekg(0, std::ios::end);
					loadFileByChunk((size_t)source.tellg(), m_Data->DataPointers.size());
				}
				
				lastIndex = game + 1;

				if (NDataPointers.empty())
					NDataPointers.emplace_back(0);
				else
				{
					if (game == m_Data->DataPointers.size())
						NDataPointers.emplace_back(NPointersIndex);
					else if(m_Data->EditedGames.contains(game - 1))
						NDataPointers.emplace_back(NDataPointers[NDataPointers.size() - 1] + m_Data->Games[game - 1].GetData().size() + 1);
					else
						NDataPointers.emplace_back(NDataPointers[NDataPointers.size() - 1] + m_Data->DataPointers[game] - m_Data->DataPointers[game - 1]);
				}

				std::string strData = m_Data->Games[game].GetData() + '\n';
				NPointersIndex += strData.size();
				//outfile.seekp(std::ios::app);
				outfile.write(strData.data(), strData.size());
			}

			if (EditedGamesSorted[EditedGamesSorted.size() - 1] < m_Data->DataPointers.size() - 1)
			{
				source.seekg(0, std::ios::end);
				loadFileByChunk((size_t)source.tellg(), m_Data->DataPointers.size());
			}

			outfile.close();
			source.close();
		}

		{
			std::ifstream source(cachePath / "helper", std::ios::binary);
			std::ofstream destination(path, std::ios::binary);

			destination << source.rdbuf();

			source.close();
			destination.close();
		}

		{
			std::unique_lock<std::shared_mutex> ul(m_Data->FileMutex);
			m_Data->EditedGames.clear();
			m_Data->FilePath = path;
			m_Data->DataPointers = NDataPointers;
			m_AddedGamesCount = 0;
		}

		auto fileHash = HashFile(m_Data->FilePath);
		
		{
			std::ofstream outfile(cachePath / "thisfile.ppgn", std::ios::binary);
			outfile.write((char*)m_Data->DataPointers.data(), m_Data->DataPointers.size() * 8);
			outfile.close();
		}
		{
			std::ofstream outfile(cachePath / "thisfile.hpgn", std::ios::binary);
			outfile.write((char*)fileHash.data(), HASH_LENGTH);
			outfile.close();
		}
	}

	std::istream& operator>>(std::istream& stream, PgnFile& f)
	{
		stream.seekg(0, std::ios_base::end);
		std::streampos maxSize = stream.tellg();
		stream.seekg(0, std::ios_base::beg);

		if (maxSize <= 0)
			return stream;

		size_t maxBufferSize = std::min((size_t)std::streamoff(maxSize), 100'000'000ui64);
		char* data = new char[maxBufferSize];

		size_t lastPointer = -1;

		while (true)
		{
			if (!f.m_Data->DataPointers.empty())
				lastPointer = f.m_Data->DataPointers[f.m_Data->DataPointers.size() - 1];

			stream.seekg(lastPointer + 1, std::ios_base::beg);

			size_t bufferSize = std::min(maxBufferSize, (size_t)std::streamoff(maxSize) - lastPointer - 1);

			stream.read(data, bufferSize);

			bool NotNoteOpen = true;
			bool NotPosibleLabelOpen = true;

			bool readyToAdd = true;

			for (size_t i = 0; i < bufferSize; i++)
			{
				if (readyToAdd && NotNoteOpen && NotPosibleLabelOpen && data[i] == '[')
					f.m_Data->DataPointers.emplace_back(lastPointer + 1 + i);

				if (data[i] >= 32 && data[i] <= 126)
				{
					if (NotPosibleLabelOpen)
					{
						if (data[i] == '{')
							NotNoteOpen = false;
						if (data[i] == '}')
							NotNoteOpen = true;
					}

					//if (NotNoteOpen)
					//{
					//	if (data[i] == '"')
					//		NotPosibleLabelOpen = !NotPosibleLabelOpen;
					//}

					if (data[i] != ']')
					{
						readyToAdd = true;
						continue;
					}

					readyToAdd = false;
					continue;
				}
			}

			if (lastPointer + 1 + bufferSize == (size_t)std::streamoff(maxSize))
				break;
		}

		delete[] data;

		return stream;
	}

	PgnManager::FileID PgnFile::GetID() const
	{
		return m_ID;
	}

	size_t PgnFile::GetSize() const
	{
		return m_Data->DataPointers.size() + m_AddedGamesCount;
	}

	std::unordered_set<size_t> PgnFile::GetDeletedGames() const
	{
		return m_DeletedGames;
	}

	PgnGame& PgnFile::operator[] (size_t index)
	{
		if (!m_Data->Games.contains(index))
		{
			if (m_Data->FilePath.empty())
			{
				__debugbreak();
				return m_Data->Games[0];
			}

			std::ifstream infile(m_Data->FilePath, std::ios::binary);

			size_t size;

			if (m_Data->DataPointers.size() == index + 1)
			{
				infile.seekg(0, std::ios_base::end);
				std::streampos maxSize = infile.tellg();
				infile.seekg(0, std::ios_base::beg);

				size = std::streamoff(maxSize) - m_Data->DataPointers[index];
			}
			else
				size = m_Data->DataPointers[index + 1] - m_Data->DataPointers[index];

			char* data = new char[size];

			infile.seekg(m_Data->DataPointers[index], std::ios_base::beg);
			infile.read(data, size);

			infile.close();

			std::string strData;

			for (int i = 0; i < size; i++)
			{
				if (data[i] == '\n')
					strData += data[i];

				if (data[i] >= 32 && data[i] <= 126)
				{
					strData += data[i];
				}
			}

			m_Data->Games[index].Parse(strData);

			delete[] data;
		}

		{
			std::shared_lock<std::shared_mutex> sl(m_Data->FileMutex);
			m_Data->GamesTimer[index] = std::chrono::high_resolution_clock::now();
		}

		return m_Data->Games[index];
	}

	void PgnFile::Clear()
	{
		PgnManager::Get().RemoveFile(m_ID);

		m_Data = std::make_shared<Chess::PgnManager::PgnFileData>();
		m_ID = PgnManager::Get().AddFile(m_Data);

		m_AddedGamesCount = 0;
		m_DeletedGames.clear();
	}

	void PgnFile::CreateGame(size_t index)
	{
		if (index == -1)
		{
			std::unique_lock<std::shared_mutex> ul(m_Data->FileMutex);
			m_Data->Games[GetSize()];
			m_Data->EditedGames.insert(GetSize());
			m_AddedGamesCount++;

			return;
		}
	}

	void PgnFile::DeleteGame(size_t index)
	{
		m_DeletedGames.emplace(index);
	}

	void PgnFile::RecoverGame(size_t index)
	{
		if (m_DeletedGames.contains(index))
			m_DeletedGames.erase(index);
	}

	void PgnFile::MoveGame(size_t position, size_t direction)
	{
		//m_Games.insert(m_Games.begin() + direction, m_Games[position]);
		//m_Games.erase(m_Games.begin() + direction);
	}


	bool PgnFile::IsGameDeleted(size_t index) const
	{
		if (m_DeletedGames.contains(index))
			return true;
		return false;
	}

	bool PgnFile::IsGameEdited(size_t index) const
	{
		if (m_Data->EditedGames.contains(index))
			return true;
		return false;
	}

	void PgnFile::RemoveDeletedGames(const std::filesystem::path& path)
	{
		PgnFile file;
		file.OpenFile(path);

		if (file.m_DeletedGames.empty())
			return;

		PgnPath_Hash hasher;
		auto hash = hasher(path);
		auto cachePath = g_cachedDirectory / std::to_string(hash);

		std::vector<size_t> NDataPointers;
		std::vector<size_t> DeletedGamesSorted;

		for (auto& game : file.m_DeletedGames)
		{
			bool founded = false;

			for (int i = 0; i < DeletedGamesSorted.size(); i++)
			{
				if (game < DeletedGamesSorted[i])
				{
					DeletedGamesSorted.insert(DeletedGamesSorted.begin() + i, game);
					founded = true;
					break;
				}
			}

			if (!founded)
				DeletedGamesSorted.emplace_back(game);
		}

		{
			std::ifstream source(file.m_Data->FilePath, std::ios::binary);
			std::ofstream outfile(cachePath / "helper", std::ios::binary, std::ios::trunc);

			size_t indexMove = 0;

			for (int i = 0; i < DeletedGamesSorted.size(); i++)
			{
				if (file.m_Data->DataPointers.size() == DeletedGamesSorted[i] + 1)
					break;


				if (DeletedGamesSorted[i] == 0)
				{
					indexMove += (file.m_Data->DataPointers[DeletedGamesSorted[i] + 1] - file.m_Data->DataPointers[DeletedGamesSorted[i]]);

					continue;
				}

				const std::streamsize chunkSize = 1024 * 1024;
				
				size_t lastIndex = file.m_Data->DataPointers[DeletedGamesSorted[i]];
				//if (i + 1 == DeletedGamesSorted.size())
				//{
				//	source.seekg(0, std::ios::end);
				//	lastIndex = (size_t)source.tellg();
				//}

				size_t startIndex = 0; 
				if (i > 0)
					startIndex = file.m_Data->DataPointers[DeletedGamesSorted[i - 1] + 1];
				
				std::vector<char> buffer(chunkSize);
				std::streamsize remaining = lastIndex - startIndex;
				source.seekg(startIndex);

				while (remaining > 0)
				{
					std::streamsize currentChunk = std::min(remaining, chunkSize);

					source.read(buffer.data(), currentChunk);
					outfile.write(buffer.data(), currentChunk);

					remaining -= currentChunk;
				}

				for (int j = (i == 0 ? 0 : DeletedGamesSorted[i - 1] + 1); j < DeletedGamesSorted[i]; j++)
					NDataPointers.emplace_back(file.m_Data->DataPointers[j] - indexMove);

				indexMove += (file.m_Data->DataPointers[DeletedGamesSorted[i] + 1] - file.m_Data->DataPointers[DeletedGamesSorted[i]]);
			}

			if (DeletedGamesSorted[DeletedGamesSorted.size() - 1] < file.m_Data->DataPointers.size() - 1)
			{
				const std::streamsize chunkSize = 1024 * 1024;

				source.seekg(0, std::ios::end);
				size_t lastIndex = (size_t)source.tellg();

				size_t startIndex = file.m_Data->DataPointers[DeletedGamesSorted[DeletedGamesSorted.size() - 1] + 1];

				std::vector<char> buffer(chunkSize);
				std::streamsize remaining = lastIndex - startIndex;
				source.seekg(startIndex);

				while (remaining > 0)
				{
					std::streamsize currentChunk = std::min(remaining, chunkSize);

					source.read(buffer.data(), currentChunk);
					outfile.write(buffer.data(), currentChunk);

					remaining -= currentChunk;
				}

				for (int j = DeletedGamesSorted[DeletedGamesSorted.size() - 1] + 1; j < file.m_Data->DataPointers.size(); j++)
					NDataPointers.emplace_back(file.m_Data->DataPointers[j] - indexMove);
			}

			source.close();
			outfile.close();
		}

		{
			std::ifstream source(cachePath / "helper", std::ios::binary);
			std::ofstream destination(path, std::ios::binary);

			destination << source.rdbuf();

			source.close();
			destination.close();
		}

		auto fileHash = HashFile(path);

		{
			std::ofstream outfile(cachePath / "thisfile.ppgn", std::ios::binary);
			outfile.write((char*)NDataPointers.data(), NDataPointers.size() * 8);
			outfile.close();
		}
		{
			std::ofstream outfile(cachePath / "thisfile.hpgn", std::ios::binary);
			outfile.write((char*)fileHash.data(), HASH_LENGTH);
			outfile.close();
		}
		{
			std::ofstream outfile(cachePath / "thisfile.dpgn", std::ios::binary, std::ios::trunc);
			outfile.close();
		}
	}
}