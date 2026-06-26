#include "CldFile.h"

#include <cmath>
#include <fstream>
#include <mutex>

#include <iostream>

#include "../ChessFileManager.h"
#include "CldGame.h"
#include "../../GameManager.h"

extern std::filesystem::path g_cachedDirectory;

namespace Chess
{
	CldFile::CldFile()
	{
		m_GamePointers = std::make_shared<std::vector<size_t>>(); 
		m_LabelNames = std::make_shared<std::vector<std::string>>();
		m_LabelValues = std::make_shared<std::vector<std::string>>();
		m_typeName = std::make_shared<uint8_t>(1);
		m_typeValue = std::make_shared<uint8_t>(4);
		m_Settings = std::make_shared<uint8_t>(0);
		ChessFileManager::Get().AddCldFileReference(m_ID, m_GamePointers, m_LabelNames, m_LabelValues, m_typeName, m_typeValue, m_Settings);
	}

	CldFile::~CldFile()
	{
		for (auto& search : m_Searches)
		{
			ChessFileManager::Get().ClearSearch(std::get<0>(search));
		}

		FileManager::Get().RemoveFile(m_ID);
		ChessFileManager::Get().RemoveFileReference(m_ID);
	}

	void CldFile::OpenFile(const std::filesystem::path& path, float* persentage)
	{
		if (path.extension().string() != ".cld")
			return;

		for (auto& search : m_Searches)
		{
			ChessFileManager::Get().ClearSearch(std::get<0>(search));
		}

		FileManager::Get().RemoveFile(m_ID);
		ChessFileManager::Get().RemoveFileReference(m_ID);

		m_AddedGamesCount = 0;
		m_DeletedGames.clear();
		m_Searches.clear();
		m_GamePointers = std::make_shared<std::vector<size_t>>();
		m_LabelNames = std::make_shared<std::vector<std::string>>();
		m_LabelValues = std::make_shared<std::vector<std::string>>();
		m_typeName = std::make_shared<uint8_t>(1);
		m_typeValue = std::make_shared<uint8_t>(4);
		m_Settings = std::make_shared<uint8_t>(0);

		if (persentage)
			*persentage = 0.1f;

		m_ID = FileManager::Get().AddFile(path);
		ChessFileManager::Get().AddCldFileReference(m_ID, m_GamePointers, m_LabelNames, m_LabelValues, m_typeName, m_typeValue, m_Settings);

		LoadDataPointers(persentage);

		if (persentage)
			*persentage = 0.8f;

		auto cachePath = FileManager::Get().GetCachePath(m_ID);

		{
			std::vector<uint64_t> buffer;

			FileManager::FileID dFileID = FileManager::Get().AddFile(cachePath / "thisfile.dpgn");
			FileManager::Get().ReadBuffer(dFileID, 0, SIZE_MAX, buffer);

			for(auto& index : buffer)
				m_DeletedGames.insert(index);
		}

		if (persentage)
			*persentage = 0.9f;

		LoadSearchIndexes(cachePath);
	}

	void CldFile::SaveFile(const std::filesystem::path& path, float* persentage)
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

		for (int si = 0; si < m_Searches.size(); si++)
			StopSearch(si);

		auto cachePath = FileManager::Get().GetCachePath(nID);
		
		{
			std::ofstream outfile(cachePath / "thisfile.dpgn", std::ios::binary | std::ios::trunc);

			for (auto& gameDeleted : m_DeletedGames)
				outfile.write((char*)&gameDeleted, 8);

			outfile.close();
		}

		SaveSearchIndexes(cachePath);

		if (persentage)
			*persentage = 0.1f;

		std::vector<size_t> editedGames;
		ChessFileManager::Get().GetEditedGames(m_ID, editedGames);

		if (editedGames.empty() && !diffFile)
			return;

		if (editedGames.empty())
		{
			{
				std::ifstream source(FileManager::Get().GetFilePath(m_ID), std::ios::binary);
				std::ofstream destination(path, std::ios::binary);

				destination << source.rdbuf();

				source.close();
				destination.close();
			}
			
			return;
		}

		size_t oldSizeName = m_LabelNames->size();
		size_t oldSizeValue = m_LabelValues->size();

		std::vector<CldGame> EcldGames;
		EcldGames.resize(editedGames.size());

		for (size_t i = 0; i < editedGames.size(); i++)
		{
			if (persentage)
			{
				*persentage = std::min(0.1f + float(i / (double)editedGames.size()) * 0.1f, 0.2f);
			}

			PgnGame& eGame = ChessFileManager::Get().GetGame(m_ID, editedGames[i]);
			auto labels = eGame.GetLabelNames();

			for (auto& label : labels)
			{
				size_t indexName = -1;
				for (size_t j = 0; j < m_LabelNames->size(); j++)
				{
					if ((*m_LabelNames)[j] == label)
					{
						indexName = j;
						break;
					}
				}

				if (indexName == -1)
				{
					indexName = m_LabelNames->size();
					m_LabelNames->emplace_back(label);
				}
				
				size_t indexValue = -1;
				for (size_t j = 0; j < m_LabelValues->size(); j++)
				{
					if ((*m_LabelValues)[j] == eGame[label])
					{
						indexValue = j;
						break;
					}
				}

				if (indexValue == -1)
				{
					indexValue = m_LabelValues->size();
					m_LabelValues->emplace_back(eGame[label]);
				}

				EcldGames[i][indexName] = indexValue;
			}

			ChessFileManager::ConvertPgnMovePathToCldMovePath(EcldGames[i].GetMovePathbyRef(), eGame.GetMovePathbyRef(), (*m_Settings) % 2 == 0 ? MoveEncoding::CLD : MoveEncoding::CORE, true);
		}

		std::vector<uint8_t> bufferNameNew, bufferValueNew;

		for (size_t i = oldSizeName; i < m_LabelNames->size(); i++)
		{
			for (char c : (*m_LabelNames)[i])
				bufferNameNew.emplace_back((uint8_t)c);
			bufferNameNew.emplace_back(0);
		}

		for (size_t i = oldSizeValue; i < m_LabelValues->size(); i++)
		{
			for (char c : (*m_LabelValues)[i])
				bufferValueNew.emplace_back((uint8_t)c);
			bufferValueNew.emplace_back(0);
		}

		if (persentage)
			*persentage = 0.3f;

		std::shared_ptr<std::vector<size_t>> NDataPointers = std::make_shared<std::vector<size_t>>();
		NDataPointers->reserve(GetSize());

		if (FileManager::Get().HasFile(m_ID))
		{
			std::ifstream source(FileManager::Get().GetFilePath(m_ID), std::ios::binary);
			
			std::ofstream destination(cachePath / "helper", std::ios::binary, std::ios::trunc);
			std::ofstream destinationGames(cachePath / "helperGames", std::ios::binary, std::ios::trunc);

			const uint8_t title[8] = { 'C', 'L', 'D', 0x01, (*m_Settings), 0x00, 0x01, 0x04 };
			uint64_t indexName = m_LabelNamesPointer;
			uint64_t indexValue = m_LabelValuesPointer + bufferNameNew.size();
			//check if we have no games
			uint64_t indexGamePointers = m_GamePointersPointer + bufferNameNew.size() + bufferValueNew.size();
			size_t numberOfGames = GetSize();

			destination.write((char*)title, 8);
			destination.write((char*)&indexName, 8);
			destination.write((char*)&indexValue, 8);
			destination.write((char*)&indexGamePointers, 8);
			destination.write((char*)&numberOfGames, 8);

			std::vector<uint8_t> buffer;

			buffer.resize(m_LabelValuesPointer - m_LabelNamesPointer);
			source.seekg(m_LabelNamesPointer);
			source.read((char*)buffer.data(), buffer.size());
			destination.write((char*)buffer.data(), buffer.size());
			if (bufferNameNew.size() > 0)
				destination.write((char*)bufferNameNew.data(), bufferNameNew.size());
			
			buffer.resize(m_GamePointersPointer - m_LabelValuesPointer);
			source.seekg(m_LabelValuesPointer);
			source.read((char*)buffer.data(), buffer.size());
			destination.write((char*)buffer.data(), buffer.size());
			if (bufferValueNew.size() > 0)
				destination.write((char*)bufferValueNew.data(), bufferValueNew.size());

			if (persentage)
				*persentage = 0.4f;

			m_LabelValuesPointer = indexValue;
			m_GamePointersPointer = indexGamePointers;

			size_t lastIndex = 0;
			size_t NPointersIndex = indexGamePointers + GetSize() * 8;

			auto loadFileByChunk = [this, &source, &destinationGames, &lastIndex, &NPointersIndex, &NDataPointers](size_t endPoint, size_t gameIndex)
				{
					if (m_GamePointers->empty())
						return;

					const std::streamsize chunkSize = 1024 * 1024;

					std::vector<char> buffer(chunkSize);
					std::streamsize remaining = endPoint - (*m_GamePointers)[lastIndex];

					source.seekg((*m_GamePointers)[lastIndex]);
					//outfile.seekp(std::ios::app);

					while (remaining > 0)
					{
						std::streamsize currentChunk = std::min(remaining, chunkSize);

						source.read(buffer.data(), currentChunk);
						//std::streamsize bytesRead = source.gcount();
						//if (bytesRead == 0)
						//	continue;

						//outfile.write(buffer.data(), bytesRead);
						destinationGames.write(buffer.data(), currentChunk);

						//remaining -= bytesRead;
						remaining -= currentChunk;
					}

					for (int i = 0; i < gameIndex - lastIndex; i++)
						NDataPointers->emplace_back((*m_GamePointers)[lastIndex + i] - (*m_GamePointers)[lastIndex] + NPointersIndex);

					NPointersIndex += (endPoint - (*m_GamePointers)[lastIndex]);
				};

			for (size_t i = 0; i < editedGames.size(); i++)
			{
				if (persentage)
				{
					*persentage = std::min(0.4f + float(i / (double)editedGames.size()) * 0.4f, 0.8f);
				}

				if (editedGames[i] > lastIndex && editedGames[i] < m_GamePointers->size())
					loadFileByChunk((*m_GamePointers)[editedGames[i]], editedGames[i]);

				if (editedGames[i] == m_GamePointers->size())
				{
					source.seekg(0, std::ios::end);
					loadFileByChunk((size_t)source.tellg(), m_GamePointers->size());
				}

				lastIndex = editedGames[i] + 1;

				if (NDataPointers->empty())
					NDataPointers->emplace_back(NPointersIndex);
				else
				{
					if (editedGames[i] == m_GamePointers->size())
						NDataPointers->emplace_back(NPointersIndex);
					else if (i > 0 && editedGames[i] - 1 == editedGames[i - 1])
					{
						std::vector<uint8_t> data;
						EcldGames[i].GetData(data, (*m_typeName), (*m_typeValue));
						NDataPointers->emplace_back(NDataPointers->back() + data.size());
					}
					else
						NDataPointers->emplace_back(NDataPointers->back() + (*m_GamePointers)[editedGames[i]] - (*m_GamePointers)[editedGames[i] - 1]);
				}

				std::vector<uint8_t> data;
				EcldGames[i].GetData(data, (*m_typeName), (*m_typeValue));
				NPointersIndex += data.size();
				destinationGames.write((char*)data.data(), data.size());
			}

			if (editedGames.back() < m_GamePointers->size() - 1)
			{
				source.seekg(0, std::ios::end);
				loadFileByChunk((size_t)source.tellg(), m_GamePointers->size());
			}

			destination.write((char*)NDataPointers->data(), NDataPointers->size() * 8);

			destination.close();
			destinationGames.close();
			source.close();
		}

		if (persentage)
			*persentage = 0.9f;

		{
			std::ifstream source(cachePath / "helper", std::ios::binary);
			std::ifstream sourceGames(cachePath / "helperGames", std::ios::binary);
			std::ofstream destination(path, std::ios::binary);

			destination << source.rdbuf() << sourceGames.rdbuf();

			source.close();
			sourceGames.close();
			destination.close();
		}

		{
			m_GamePointers = NDataPointers;
			m_AddedGamesCount = 0;

			ChessFileManager::Get().RemoveFileReference(m_ID);

			if (diffFile)
			{
				FileManager::Get().RemoveFile(m_ID);
				m_ID = nID;
			}

			ChessFileManager::Get().AddCldFileReference(m_ID, m_GamePointers, m_LabelNames, m_LabelValues, m_typeName, m_typeValue, m_Settings);
		}
	}

	void CldFile::LoadDataPointers(float* persentage)
	{
		//$$CLD(3B)$version(1B)$settings(1B)$idtable(1B)$typeIndexNAME(1B)$typeIndexVALUE(1B)$IndexNAME(8B)$IndexVALUE(8B)$IndexGAME(8B)$NumberOfGames(8B)$$

		std::vector<uint8_t> buffer;
		FileManager::Get().ReadBuffer(m_ID, 0, 40, buffer);

		if (buffer.size() < 40)
			return;

		if (buffer[0] != 'C' || buffer[1] != 'L' || buffer[2] != 'D')
			return;

		uint8_t version = buffer[3];
		(*m_Settings) = buffer[4];
		uint8_t idTable = buffer[5];
		(*m_typeName) = buffer[6];
		(*m_typeValue) = buffer[7];

		m_LabelNamesPointer = 40;// *(uint64_t*)&buffer[8];
		m_LabelValuesPointer = *(uint64_t*)&buffer[16];
		m_GamePointersPointer = *(uint64_t*)&buffer[24];
		size_t numberOfGames = *(uint64_t*)&buffer[32];
		
		if (version != 0x01)
			return;

		if ((*m_Settings) != 0x00 && (*m_Settings) != 0x01)
			return;

		{
			std::vector<uint8_t> bufferNames;
			FileManager::Get().ReadBuffer(m_ID, m_LabelNamesPointer, m_LabelValuesPointer - m_LabelNamesPointer, bufferNames);

			std::string name;
			for (size_t i = 0; i < bufferNames.size(); i++)
			{
				if (bufferNames[i] == 0)
				{
					m_LabelNames->emplace_back(name);
					name.clear();
					continue;
				}

				name += (char)bufferNames[i];
			}
		}

		if (persentage)
			*persentage = 0.2f;

		{
			std::vector<uint8_t> bufferValues;
			FileManager::Get().ReadBuffer(m_ID, m_LabelValuesPointer, m_GamePointersPointer - m_LabelValuesPointer, bufferValues);

			std::string value;
			for (size_t i = 0; i < bufferValues.size(); i++)
			{
				if (bufferValues[i] == 0)
				{
					m_LabelValues->emplace_back(value);
					value.clear();
					continue;
				}

				value += (char)bufferValues[i];
			}
		}

		if (persentage)
			*persentage = 0.6f;

		FileManager::Get().ReadBuffer(m_ID, m_GamePointersPointer, numberOfGames * 8, (*m_GamePointers));
	}

	FileManager::FileID CldFile::GetID() const
	{
		return m_ID;
	}

	size_t CldFile::GetSize() const
	{
		return m_GamePointers->size() + m_AddedGamesCount;
	}

	std::unordered_set<size_t> CldFile::GetDeletedGames() const
	{
		return m_DeletedGames;
	}

	PgnGame& CldFile::operator[] (size_t index)
	{
		return ChessFileManager::Get().GetGame(m_ID, index);
	}

	const PgnGame& CldFile::At (size_t index) const
	{
		return ChessFileManager::Get().GetGame(m_ID, index);
	}

	void CldFile::Clear()
	{
		for (auto& search : m_Searches)
		{
			ChessFileManager::Get().ClearSearch(std::get<0>(search));
		}

		FileManager::Get().RemoveFile(m_ID);
		ChessFileManager::Get().RemoveFileReference(m_ID);

		m_ID = UUID();
		m_AddedGamesCount = 0;
		m_DeletedGames.clear();
		m_Searches.clear();
		m_GamePointers = std::make_shared<std::vector<size_t>>();
		m_LabelNames = std::make_shared<std::vector<std::string>>();
		m_LabelValues = std::make_shared<std::vector<std::string>>();
		m_typeName = std::make_shared<uint8_t>(1);
		m_typeValue = std::make_shared<uint8_t>(4);
		m_Settings = std::make_shared<uint8_t>(0);

		ChessFileManager::Get().AddCldFileReference(m_ID, m_GamePointers, m_LabelNames, m_LabelValues, m_typeName, m_typeValue, m_Settings);
	}

	void CldFile::CreateGame(size_t index)
	{
		if (index == -1)
		{
			ChessFileManager::Get().GetGame(m_ID, GetSize());
			m_AddedGamesCount++;
		}
	}

	void CldFile::DeleteGame(size_t index)
	{
		m_DeletedGames.emplace(index);
	}

	void CldFile::RecoverGame(size_t index)
	{
		if (m_DeletedGames.contains(index))
			m_DeletedGames.erase(index);
	}
	
	void CldFile::RemoveFromEdited(size_t index)
	{
		ChessFileManager::Get().RemoveFromEditedGames(m_ID, index);
	}

	void CldFile::MoveGame(size_t position, size_t direction)
	{
		//m_Games.insert(m_Games.begin() + direction, m_Games[position]);
		//m_Games.erase(m_Games.begin() + direction);
	}


	bool CldFile::IsGameDeleted(size_t index) const
	{
		if (m_DeletedGames.contains(index))
			return true;
		return false;
	}

	bool CldFile::IsGameEdited(size_t index) const
	{
		return ChessFileManager::Get().IsGameEdited(m_ID, index);
	}

	void CldFile::RemoveDeletedGames(const std::filesystem::path& path)
	{
		CldFile file;
		file.OpenFile(path);

		if (file.m_DeletedGames.empty())
			return;

		file.m_Searches.clear();

		auto cachePath = FileManager::Get().GetCachePath(file.m_ID);

		std::vector<size_t> NewDataPointers;
		std::vector<size_t> DeletedGamesSorted;

		{
			FileManager::FileID dFileID = FileManager::Get().AddFile(cachePath / "thisfile.dpgn");
			FileManager::Get().ReadBuffer(dFileID, 0, SIZE_MAX, DeletedGamesSorted);

			std::sort(DeletedGamesSorted.begin(), DeletedGamesSorted.end());
		}

		size_t startPointerGames = file.m_GamePointersPointer + (file.GetSize() - DeletedGamesSorted.size()) * 8;
		NewDataPointers.reserve(file.GetSize() - DeletedGamesSorted.size() + 1);
		NewDataPointers.emplace_back(startPointerGames);

		{
			std::ifstream source(FileManager::Get().GetFilePath(file.m_ID), std::ios::binary);
			std::ofstream outfile(cachePath / "helperGames", std::ios::binary, std::ios::trunc);

			for (int i = 0; i < DeletedGamesSorted.size(); i++)
			{
				if (file.m_GamePointers->size() == DeletedGamesSorted[i] + 1)
					break;

				if (i == 0 && DeletedGamesSorted[i] == 0)
					continue;

				if (i > 0 && DeletedGamesSorted[i] - 1 == DeletedGamesSorted[i - 1])
					continue;

				size_t lastIndex = (*file.m_GamePointers)[DeletedGamesSorted[i]];				
				size_t startIndex = (*file.m_GamePointers)[0];
				if (i > 0)
					startIndex = (*file.m_GamePointers)[DeletedGamesSorted[i - 1] + 1];

				const std::streamsize chunkSize = 1024 * 1024 * 64;
				static std::vector<char> buffer(chunkSize);
				std::streamsize remaining = lastIndex - startIndex;
				source.seekg(startIndex);

				while (remaining > 0)
				{
					std::streamsize currentChunk = std::min(remaining, chunkSize);

					source.read(buffer.data(), currentChunk);
					outfile.write(buffer.data(), currentChunk);

					remaining -= currentChunk;
				}

				for (int j = (i == 0 ? 1 : (DeletedGamesSorted[i - 1] + 2)); j < DeletedGamesSorted[i] + 1; j++)
				{
					NewDataPointers.emplace_back((*file.m_GamePointers)[j] - (*file.m_GamePointers)[j - 1] + NewDataPointers.back());
				}
			}

			if (DeletedGamesSorted.back() < file.m_GamePointers->size() - 1)
			{
				source.seekg(0, std::ios::end);
				size_t lastIndex = (size_t)source.tellg();
				size_t startIndex = (*file.m_GamePointers)[DeletedGamesSorted.back() + 1];

				const std::streamsize chunkSize = 1024 * 1024 * 64;
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

				for (int j = DeletedGamesSorted.back() + 2; j < file.m_GamePointers->size(); j++)
					NewDataPointers.emplace_back((*file.m_GamePointers)[j] - (*file.m_GamePointers)[j - 1] + NewDataPointers.back());
			}

			NewDataPointers.pop_back();

			source.close();
			outfile.close();
		}

		{
			std::ifstream source(FileManager::Get().GetFilePath(file.m_ID), std::ios::binary);
			std::ofstream outfile(cachePath / "helper", std::ios::binary, std::ios::trunc);

			const uint8_t title[8] = { 'C', 'L', 'D', 0x01, 0x00, 0x00, 0x01, 0x04 };
			size_t numberOfGames = NewDataPointers.size();

			outfile.write((char*)title, 8);
			outfile.write((char*)&file.m_LabelNamesPointer, 8);
			outfile.write((char*)&file.m_LabelValuesPointer, 8);
			outfile.write((char*)&file.m_GamePointersPointer, 8);
			outfile.write((char*)&numberOfGames, 8);

			std::vector<uint8_t> buffer;
			buffer.resize(file.m_LabelValuesPointer - file.m_LabelNamesPointer);
			source.seekg(file.m_LabelNamesPointer);
			source.read((char*)buffer.data(), buffer.size());
			outfile.write((char*)buffer.data(), buffer.size());

			buffer.resize(file.m_GamePointersPointer - file.m_LabelValuesPointer);
			source.seekg(file.m_LabelValuesPointer);
			source.read((char*)buffer.data(), buffer.size());
			outfile.write((char*)buffer.data(), buffer.size());

			outfile.write((char*)NewDataPointers.data(), NewDataPointers.size() * 8);


			source.close();
			outfile.close();
		}

		{
			std::ifstream sourceGames(cachePath / "helperGames", std::ios::binary);
			std::ifstream source(cachePath / "helper", std::ios::binary);
			std::ofstream destination(path, std::ios::binary);

			destination << source.rdbuf() << sourceGames.rdbuf();

			source.close();
			sourceGames.close();
			destination.close();
		}

		{
			std::ofstream outfile(cachePath / "thisfile.dpgn", std::ios::binary, std::ios::trunc);
			outfile.close();
		}

		file.SaveSearchIndexes(cachePath);
	}

	std::shared_ptr<std::pair<SearchOptions, SearchResult>> CldFile::AddSearch()
	{
		SearchID id;

		std::shared_ptr<std::pair<SearchOptions, SearchResult>> search = std::make_shared<std::pair<SearchOptions, SearchResult>>();

		m_Searches.emplace_back(id, "Table " + std::to_string(m_Searches.size() + 1), search);

		return search;
	}

	void CldFile::RemoveSearch(size_t index)
	{
		if (index < m_Searches.size())
		{
			ChessFileManager::Get().ClearSearch(std::get<0>(m_Searches[index]));

			m_Searches.erase(m_Searches.begin() + index);
		}
	}

	void CldFile::StartSearch(size_t index)
	{
		if (index < m_Searches.size())
		{
			auto& [id, name, data] = m_Searches.at(index);

			data->first.InitCldSearch(m_LabelNames, m_LabelValues);

			ChessFileManager::Get().StartSearch(id, m_ID, data);
		}
	}

	void CldFile::StopSearch(size_t index)
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

	size_t CldFile::GetSearchesCount() const
	{
		return m_Searches.size();
	}

	std::shared_ptr<std::pair<SearchOptions, SearchResult>> CldFile::GetSearch(size_t index)
	{
		if (index < m_Searches.size())
			return std::get<2>(m_Searches[index]);

		return nullptr;
	}

	std::string* CldFile::GetSearchName(size_t index)
	{
		if (index < m_Searches.size())
			return &std::get<1>(m_Searches[index]);

		return nullptr;
	}

	void CldFile::LoadSearchIndexes(const std::filesystem::path& cachePath)
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

			if (size == SIZE_MAX)
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

	void CldFile::SaveSearchIndexes(const std::filesystem::path& cachePath)
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

	void CldFile::GetCldGame(CldGame& game, size_t index)
	{
		std::vector<uint8_t> data;
		FileManager::Get().ReadBuffer(m_ID, (*m_GamePointers)[index], ((index + 1 < m_GamePointers->size()) ? ((*m_GamePointers)[index + 1] - (*m_GamePointers)[index]) : SIZE_MAX), (std::vector<uint8_t>&)data);

		game.Parse(data, (*m_Settings) % 2 == 0 ? MoveEncoding::CLD : MoveEncoding::CORE, (*m_typeName), (*m_typeValue), true, false, true, false);
	}

}