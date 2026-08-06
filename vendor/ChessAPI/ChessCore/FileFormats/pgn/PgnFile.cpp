#include "PgnFile.h"

#include <fstream>

#include "../ChessFileManager.h"

#include "FileHash.h"

extern std::filesystem::path g_cachedDirectory;

namespace Chess
{
	PgnFile::PgnFile()
	{
		m_DataPointers = std::make_shared<std::vector<size_t>>();
		ChessFileManager::Get().AddFileReference(m_ID, m_DataPointers);
	}

	PgnFile::~PgnFile()
	{
		for (auto& search : m_Searches)
		{
			ChessFileManager::Get().ClearSearch(std::get<0>(search));
		}

		FileManager::Get().RemoveFile(m_ID);
		ChessFileManager::Get().RemoveFileReference(m_ID);
	}

	void PgnFile::OpenFile(const std::filesystem::path& path, float* persentage)
	{
		//remove clear
		Clear();

		ChessFileManager::Get().RemoveFileReference(m_ID);

		m_ID = FileManager::Get().AddFile(path);
		ChessFileManager::Get().AddFileReference(m_ID, m_DataPointers);

		auto fileHash = HashFile(path, persentage);

		auto cachePath = FileManager::Get().GetCachePath(m_ID);

		if (std::filesystem::exists(cachePath / "thisfile.hpgn") && !std::filesystem::is_directory(cachePath / "thisfile.hpgn"))
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
				if (persentage)
					*persentage = 0.4f;

				{
					std::ifstream infile(cachePath / "thisfile.ppgn", std::ios::binary);

					infile.seekg(0, std::ios_base::end);
					std::streampos maxSize = infile.tellg();
					infile.seekg(0, std::ios_base::beg);

					char* data = new char[maxSize];
					infile.read(data, maxSize);

					infile.close();

					m_DataPointers->reserve(maxSize / 8);

					for (int i = 0; i < maxSize; i += 8)
						m_DataPointers->emplace_back(*(size_t*)(&data[i]));

					delete[] data;
				}

				if (persentage)
					*persentage = 0.7f;

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

				if (persentage)
					*persentage = 0.8f;

				LoadSearchIndexes(cachePath);

				if (persentage)
					*persentage = 0.9f;

				return;
			}
		}

		if (persentage)
			*persentage = 0.4f;

		LoadDataPointers(persentage);

		{
			std::ofstream outfile(cachePath / "thisfile.ppgn", std::ios::binary | std::ios::trunc);
			outfile.write((char*)m_DataPointers->data(), m_DataPointers->size() * 8);
			outfile.close();
		}
		{
			std::ofstream outfile(cachePath / "thisfile.hpgn", std::ios::binary | std::ios::trunc);
			outfile.write((char*)fileHash.data(), HASH_LENGTH);
			outfile.close();
		}
		{
			std::ofstream outfile(cachePath / "thisfile.dpgn", std::ios::binary | std::ios::trunc);
			outfile.close();
		}

		if (persentage)
			*persentage = 0.9f;

		SaveSearchIndexes(cachePath);
	}

	void PgnFile::SaveFile(const std::filesystem::path& path, float* persentage)
	{
		std::filesystem::path oldPath = "";

		if (FileManager::Get().HasFile(m_ID))
			oldPath = FileManager::Get().GetFilePath(m_ID);

		bool diffFile = path.lexically_normal() != oldPath.lexically_normal();
		FileManager::FileID nID = m_ID;

		if (diffFile)
		{
			std::ofstream testfile(path, std::ios::trunc);
			testfile.close();

			nID = FileManager::Get().AddFile(path);
		}

		auto cachePath = FileManager::Get().GetCachePath(nID);

		{
			std::ofstream outfile(cachePath / "thisfile.dpgn", std::ios::binary | std::ios::trunc);
			
			for (auto& gameDeleted : m_DeletedGames)
				outfile.write((char*)&gameDeleted, 8);

			outfile.close();
		}

		for (int si = 0; si < m_Searches.size(); si++)
			StopSearch(si);

		SaveSearchIndexes(cachePath);

		if (persentage)
			*persentage = 0.1f;

		std::vector<size_t> editedGames;
		ChessFileManager::Get().GetEditedGames(m_ID, editedGames);

		if (editedGames.empty())
		{
			if (diffFile)
			{
				auto fileHash = HashFile(FileManager::Get().GetFilePath(m_ID), persentage);

				{
					std::ifstream source(FileManager::Get().GetFilePath(m_ID), std::ios::binary);
					std::ofstream destination(path, std::ios::binary);

					destination << source.rdbuf();

					source.close();
					destination.close();
				}
				{
					std::ofstream outfile(cachePath / "thisfile.ppgn", std::ios::binary);
					outfile.write((char*)m_DataPointers->data(), m_DataPointers->size() * 8);
					outfile.close();
				}
				{
					std::ofstream outfile(cachePath / "thisfile.hpgn", std::ios::binary);
					outfile.write((char*)fileHash.data(), HASH_LENGTH);
					outfile.close();
				}
			}

			if (persentage)
				*persentage = 0.9f;

			return;
		}

		std::shared_ptr<std::vector<size_t>> NDataPointers = std::make_shared<std::vector<size_t>>();
		NDataPointers->reserve(GetSize());
		
		{
			std::ifstream source;
			
			if (FileManager::Get().HasFile(m_ID))
				source.open(FileManager::Get().GetFilePath(m_ID), std::ios::binary);
			
			std::ofstream outfile(cachePath / "helper", std::ios::binary, std::ios::trunc);

			size_t lastIndex = 0;
			size_t NPointersIndex = 0;
			
			auto loadFileByChunk = [this, &source, &outfile, &lastIndex, &NPointersIndex, &NDataPointers](size_t endPoint, size_t gameIndex)
				{
					if (m_DataPointers->empty())
						return;

					const std::streamsize chunkSize = 1024 * 1024;

					std::vector<char> buffer(chunkSize);
					std::streamsize remaining = endPoint - (*m_DataPointers)[lastIndex];

					source.seekg((*m_DataPointers)[lastIndex]);

					while (remaining > 0)
					{
						std::streamsize currentChunk = std::min(remaining, chunkSize);

						source.read(buffer.data(), currentChunk);
						outfile.write(buffer.data(), currentChunk);

						remaining -= currentChunk;
					}

					for (int i = 0; i < gameIndex - lastIndex; i++)
						NDataPointers->emplace_back((*m_DataPointers)[lastIndex + i] - (*m_DataPointers)[lastIndex] + NPointersIndex);

					NPointersIndex += (endPoint - (*m_DataPointers)[lastIndex]);
				};
			
			for (auto& game : editedGames)
			{
				if (game > lastIndex)
				{
					if (game < m_DataPointers->size())
						loadFileByChunk((*m_DataPointers)[game], game);
					else if (game == m_DataPointers->size())
					{
						source.seekg(0, std::ios::end);
						loadFileByChunk((size_t)source.tellg(), m_DataPointers->size());
					}
				}
				
				lastIndex = game + 1;

				if (NDataPointers->empty())
					NDataPointers->emplace_back(0);
				else
				{
					if (game == m_DataPointers->size())
						NDataPointers->emplace_back(NPointersIndex);
					else if(ChessFileManager::Get().IsGameEdited(m_ID, game - 1))
						NDataPointers->emplace_back((*NDataPointers)[NDataPointers->size() - 1] + ChessFileManager::Get().GetGame(m_ID, game - 1).GetData().size());
					else
						NDataPointers->emplace_back((*NDataPointers)[NDataPointers->size() - 1] + (*m_DataPointers)[game] - (*m_DataPointers)[game - 1]);
				}

				//there should be a better way, two continues edited games, we get their data twice
				std::string strData = ChessFileManager::Get().GetGame(m_ID, game).GetData();
				NPointersIndex += strData.size();

				outfile.write(strData.data(), strData.size());
			}

			if (editedGames[editedGames.size() - 1] < m_DataPointers->size() - 1)
			{
				source.seekg(0, std::ios::end);
				loadFileByChunk((size_t)source.tellg(), m_DataPointers->size());
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
			m_DataPointers = NDataPointers;
			m_AddedGamesCount = 0;

			ChessFileManager::Get().RemoveFileReference(m_ID);

			if (diffFile)
			{
				FileManager::Get().RemoveFile(m_ID);
				m_ID = nID;
			}

			ChessFileManager::Get().AddFileReference(m_ID, NDataPointers);
		}

		{
			std::ofstream outfile(cachePath / "thisfile.ppgn", std::ios::binary);
			outfile.write((char*)m_DataPointers->data(), m_DataPointers->size() * 8);
			outfile.close();
		}
		{
			auto fileHash = HashFile(path, persentage);

			std::ofstream outfile(cachePath / "thisfile.hpgn", std::ios::binary);
			outfile.write((char*)fileHash.data(), HASH_LENGTH);
			outfile.close();
		}
	}

	void PgnFile::LoadDataPointers(float* persentage)
	{
		size_t maxBufferSize = 100'000'000ui64;
		
		std::vector<char> data;
		data.reserve(maxBufferSize);

		size_t lastPointer = -1;

		while (true)
		{
			if (!m_DataPointers->empty())
				lastPointer = (*m_DataPointers)[m_DataPointers->size() - 1];

			if (persentage)
			{
				size_t fileSize = FileManager::Get().GetMaxFileSize(m_ID);
				*persentage = std::min(0.4f + float((lastPointer + maxBufferSize) / (double)fileSize) * 0.5f, 0.9f);
			}

			FileManager::Get().ReadBuffer(m_ID, lastPointer + 1, maxBufferSize, (std::vector<uint8_t>&)data);

			bool NotNoteOpen = true;

			for (size_t i = 0; i < data.size(); i++)
			{
				if (data[i] == '[')
				{
					if (i > 0 && (data[i - 1] == '\\' || data[i - 1] == ']'))
						continue;

					if (i > 1 && data[i - 2] == ']')
						continue;

					if (i > 2 && data[i - 3] == ']')
						continue;

					if (NotNoteOpen)
						m_DataPointers->emplace_back(lastPointer + 1 + i);

					continue;
				}

				if (data[i] == '{')
				{
					if (i > 0 && data[i - 1] == '\\')
						continue;

					NotNoteOpen = false;
				}
				if (data[i] == '}')
				{
					if (i > 0 && data[i - 1] == '\\')
						continue;

					NotNoteOpen = true;
				}
			}

			if (lastPointer + 1 + data.size() == FileManager::Get().GetMaxFileSize(m_ID))
				break;
		}
	}

	FileManager::FileID PgnFile::GetID() const
	{
		return m_ID;
	}

	size_t PgnFile::GetSize() const
	{
		return m_DataPointers->size() + m_AddedGamesCount;
	}

	std::unordered_set<size_t> PgnFile::GetDeletedGames() const
	{
		return m_DeletedGames;
	}

	PgnGame& PgnFile::operator[] (size_t index)
	{
		return ChessFileManager::Get().GetGame(m_ID, index);
	}

	const PgnGame& PgnFile::At (size_t index) const
	{
		return ChessFileManager::Get().GetGame(m_ID, index);
	}

	void PgnFile::Clear()
	{
		FileManager::Get().RemoveFile(m_ID);
		ChessFileManager::Get().RemoveFileReference(m_ID);

		m_ID = UUID();
		m_AddedGamesCount = 0;
		m_DeletedGames.clear();
		m_DataPointers = std::make_shared<std::vector<size_t>>();
		ChessFileManager::Get().AddFileReference(m_ID, m_DataPointers);

		for (auto& search : m_Searches)
		{
			ChessFileManager::Get().ClearSearch(std::get<0>(search));
		}

		m_Searches.clear();
	}

	void PgnFile::CreateGame(size_t index)
	{
		if (index == -1)
		{
			ChessFileManager::Get().GetGame(m_ID, GetSize());
			m_AddedGamesCount++;
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
	
	void PgnFile::RemoveFromEdited(size_t index)
	{
		ChessFileManager::Get().RemoveFromEditedGames(m_ID, index);
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
		return ChessFileManager::Get().IsGameEdited(m_ID, index);
	}

	void PgnFile::RemoveDeletedGames(const std::filesystem::path& path)
	{
		PgnFile file;
		file.OpenFile(path);

		if (file.m_DeletedGames.empty())
			return;

		for (auto& [id, name, data] : file.m_Searches)
		{
			for (int i = 0; i < data->second.PossitiveIndexes.size(); i++)
			{
				if (file.m_DeletedGames.contains(data->second.PossitiveIndexes[i]))
				{
					data->second.PossitiveIndexes.erase(data->second.PossitiveIndexes.begin() + i);
					i--;
				}
			}
		}

		auto cachePath = FileManager::Get().GetCachePath(file.m_ID);

		std::vector<size_t> NewDataPointers;
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
			std::ifstream source(FileManager::Get().GetFilePath(file.m_ID), std::ios::binary);
			std::ofstream outfile(cachePath / "helper", std::ios::binary, std::ios::trunc);

			size_t indexMove = 0;

			for (int i = 0; i < DeletedGamesSorted.size(); i++)
			{
				if (file.m_DataPointers->size() == DeletedGamesSorted[i] + 1)
					break;


				if (DeletedGamesSorted[i] == 0)
				{
					indexMove += ((*file.m_DataPointers)[DeletedGamesSorted[i] + 1] - (*file.m_DataPointers)[DeletedGamesSorted[i]]);

					continue;
				}

				const std::streamsize chunkSize = 1024 * 1024;
				
				size_t lastIndex = (*file.m_DataPointers)[DeletedGamesSorted[i]];
				//if (i + 1 == DeletedGamesSorted.size())
				//{
				//	source.seekg(0, std::ios::end);
				//	lastIndex = (size_t)source.tellg();
				//}

				size_t startIndex = 0; 
				if (i > 0)
					startIndex = (*file.m_DataPointers)[DeletedGamesSorted[i - 1] + 1];
				
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
					NewDataPointers.emplace_back((*file.m_DataPointers)[j] - indexMove);

				indexMove += ((*file.m_DataPointers)[DeletedGamesSorted[i] + 1] - (*file.m_DataPointers)[DeletedGamesSorted[i]]);
			}

			if (DeletedGamesSorted[DeletedGamesSorted.size() - 1] < file.m_DataPointers->size() - 1)
			{
				const std::streamsize chunkSize = 1024 * 1024;

				source.seekg(0, std::ios::end);
				size_t lastIndex = (size_t)source.tellg();

				size_t startIndex = (*file.m_DataPointers)[DeletedGamesSorted[DeletedGamesSorted.size() - 1] + 1];

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

				for (int j = DeletedGamesSorted[DeletedGamesSorted.size() - 1] + 1; j < file.m_DataPointers->size(); j++)
					NewDataPointers.emplace_back((*file.m_DataPointers)[j] - indexMove);
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

		auto fileHash = HashFile(path, nullptr);

		{
			std::ofstream outfile(cachePath / "thisfile.ppgn", std::ios::binary);
			outfile.write((char*)NewDataPointers.data(), NewDataPointers.size() * 8);
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

		file.SaveSearchIndexes(cachePath);
	}

	std::shared_ptr<std::pair<SearchOptions, SearchResult>> PgnFile::AddSearch()
	{
		SearchID id;

		std::shared_ptr<std::pair<SearchOptions, SearchResult>> search = std::make_shared<std::pair<SearchOptions, SearchResult>>();

		m_Searches.emplace_back(id, "Table " + std::to_string(m_Searches.size() + 1), search);

		return search;
	}

	void PgnFile::RemoveSearch(size_t index)
	{
		if (index < m_Searches.size())
		{
			ChessFileManager::Get().ClearSearch(std::get<0>(m_Searches[index]));

			m_Searches.erase(m_Searches.begin() + index);
		}
	}

	void PgnFile::StartSearch(size_t index)
	{
		if (index < m_Searches.size())
		{
			auto& [id, name, data] = m_Searches.at(index);

			ChessFileManager::Get().StartSearch(id,	m_ID, data);
		}
	}

	void PgnFile::StopSearch(size_t index)
	{
		if (index < m_Searches.size())
		{
			auto& [id, name, data] = m_Searches.at(index);
			ChessFileManager::Get().ClearSearch(id);

			if (data->second.Persentage != 1.0f)
			{
				data->second.PossitiveIndexes.clear();
				data->second.Persentage = 0.0f;
			}
		}
	}

	size_t PgnFile::GetSearchesCount() const
	{
		return m_Searches.size();
	}

	std::shared_ptr<std::pair<SearchOptions, SearchResult>> PgnFile::GetSearch(size_t index)
	{
		if (index < m_Searches.size())
			return std::get<2>(m_Searches[index]);

		return nullptr;
	}

	std::string* PgnFile::GetSearchName(size_t index)
	{
		if (index < m_Searches.size())
			return &std::get<1>(m_Searches[index]);

		return nullptr;
	}

	void PgnFile::LoadSearchIndexes(const std::filesystem::path& cachePath)
	{
		std::ifstream infile(cachePath / "thisfile.spgn", std::ios::binary | std::ios::ate);

		if (!infile.is_open() || infile.tellg() == 0)
			return;

		infile.seekg(0, std::ios::beg);

		size_t size = 0;
		std::vector<uint8_t> optionData;

		while (!infile.fail())
		{
			infile.read((char*)&size, sizeof(size_t));

			if(size == SIZE_MAX)
				break;

			auto& [id, name, data] = m_Searches.emplace_back();
			data = std::make_shared<std::pair<SearchOptions, SearchResult>>();

			data->second.Persentage = 1.0f;
			data->second.PossitiveIndexes.resize(size);

			infile.read((char*)data->second.PossitiveIndexes.data(), size * sizeof(size_t));
			
			char nc;
			infile.read(&nc, 1);

			if (nc == '\0')
			{
				infile.read(&nc, 1);

				while (nc != '\0')
				{
					name += nc;
					infile.read(&nc, 1);
				}
			}
			else
				infile.seekg(-1, std::ios::cur);
						
			infile.read((char*)&size, sizeof(size_t));
			optionData.resize(size);
			infile.read((char*)optionData.data(), size);

			data->first.SetOptionByData(optionData);
		}

		infile.close();
	}

	void PgnFile::SaveSearchIndexes(const std::filesystem::path& cachePath)
	{
		size_t size = 0;
		std::vector<uint8_t> optionsData;

		std::ofstream outfile(cachePath / "thisfile.spgn", std::ios::binary | std::ios::trunc);

		for (auto& [id, name, data] : m_Searches)
		{
			if (data->second.Persentage < 1.0f)
				continue;

			size = data->second.PossitiveIndexes.size();
			outfile.write((char*)&size, sizeof(size_t));
			outfile.write((char*)data->second.PossitiveIndexes.data(), data->second.PossitiveIndexes.size() * sizeof(size_t));

			outfile.put('\0');
			outfile.write(name.data(), name.size());
			outfile.put('\0');
			
			optionsData.clear();
			data->first.GetOptionData(optionsData);
			
			size = optionsData.size();
			outfile.write((char*)&size, sizeof(size_t));
			outfile.write((char*)optionsData.data(), optionsData.size());
		}

		size = SIZE_MAX;
		outfile.write((char*)&size, sizeof(size_t));

		outfile.close();
	}
}