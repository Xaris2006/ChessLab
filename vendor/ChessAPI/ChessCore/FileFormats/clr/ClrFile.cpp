#include "ClrFile.h"

#include <fstream>

#include "../ChessFileManager.h"
#include "../cld/CldGame.h"
#include "../../GameManager.h"

extern std::filesystem::path g_cachedDirectory;

namespace Chess
{
	ClrFile::ClrFile()
	{
		m_GamePointers = std::make_shared<std::vector<size_t>>();
		m_LabelNames = std::make_shared<std::vector<std::string>>();
		m_LabelValues = std::make_shared<std::vector<std::string>>();
		m_typeName = std::make_shared<uint8_t>(1);
		m_typeValue = std::make_shared<uint8_t>(4);
		m_Settings = std::make_shared<uint8_t>(0);
		m_Search = { SearchID(), std::make_shared<std::pair<SearchOptions, SearchResult>>() };
		m_SearchGameIndexes = std::make_shared<std::tuple<size_t, size_t, size_t, size_t, size_t, size_t, size_t>>();
		
		ChessFileManager::Get().AddClrFileReference(m_ID, m_GamePointers, m_LabelNames, m_LabelValues, m_typeName, m_typeValue, m_Settings, m_SearchGameIndexes);
	}

	ClrFile::~ClrFile()
	{
		ChessFileManager::Get().ClearSearch(std::get<0>(m_Search));
	
		FileManager::Get().RemoveFile(m_ID);
		ChessFileManager::Get().RemoveFileReference(m_ID);
	}

	void ClrFile::OpenFile(const std::filesystem::path& path, float* persentage)
	{
		if (path.extension().string() != ".cld")
			return;

		ChessFileManager::Get().ClearSearch(std::get<0>(m_Search));
		FileManager::Get().RemoveFile(m_ID);
		ChessFileManager::Get().RemoveFileReference(m_ID);

		m_GamePointers = std::make_shared<std::vector<size_t>>();
		m_LabelNames = std::make_shared<std::vector<std::string>>();
		m_LabelValues = std::make_shared<std::vector<std::string>>();
		m_typeName = std::make_shared<uint8_t>(1);
		m_typeValue = std::make_shared<uint8_t>(4);
		m_Settings = std::make_shared<uint8_t>(0);
		m_Search = { SearchID(), std::make_shared<std::pair<SearchOptions, SearchResult>>() };
		m_SearchGameResults.reset();
		m_SearchTopGames.reset();
		m_SearchGameIndexes = std::make_shared<std::tuple<size_t, size_t, size_t, size_t, size_t, size_t, size_t>>();

		if (persentage)
			*persentage = 0.1f;

		m_ID = FileManager::Get().AddFile(path);
		ChessFileManager::Get().AddClrFileReference(m_ID, m_GamePointers, m_LabelNames, m_LabelValues, m_typeName, m_typeValue, m_Settings, m_SearchGameIndexes);

		LoadDataPointers(persentage);

		if (persentage)
			*persentage = 0.8f;

		auto cachePath = FileManager::Get().GetCachePath(m_ID);

		if (persentage)
			*persentage = 0.9f;
	}

	void ClrFile::LoadDataPointers(float* persentage)
	{
		auto& [resIndex, w, b, d, WEloIndex, BEloIndex, DateIndex] = *m_SearchGameIndexes;

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

		if (version == 0x01)
		{
			{
				std::vector<uint8_t> bufferNames;
				FileManager::Get().ReadBuffer(m_ID, m_LabelNamesPointer, m_LabelValuesPointer - m_LabelNamesPointer, bufferNames);

				std::string name;
				for (size_t i = 0; i < bufferNames.size(); i++)
				{
					if (bufferNames[i] == 0)
					{
						if (name == "Result")
							resIndex = m_LabelNames->size();
						else if (name == "WhiteElo")
							WEloIndex = m_LabelNames->size();
						else if (name == "BlackElo")
							BEloIndex = m_LabelNames->size();
						else if (name == "Date")
							DateIndex = m_LabelNames->size();

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
						if (value == "1-0")
							w = m_LabelValues->size();
						else if (value == "0-1")
							b = m_LabelValues->size();
						else if (value == "1/2-1/2")
							d = m_LabelValues->size();

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
	}

	FileManager::FileID ClrFile::GetID() const
	{
		return m_ID;
	}

	size_t ClrFile::GetSize() const
	{
		return m_GamePointers->size();
	}

	MoveEncoding ClrFile::GetEncoding() const
	{
		bool wencode = ((*m_Settings) % 2) == 0;
		if (!wencode)
			return MoveEncoding::CORE;

		return  MoveEncoding::CLD;		
	}

	PgnGame& ClrFile::operator[] (size_t index)
	{
		return ChessFileManager::Get().GetGame(m_ID, index);
	}

	const PgnGame& ClrFile::At(size_t index) const
	{
		return ChessFileManager::Get().GetGame(m_ID, index);
	}

	void ClrFile::Clear()
	{
		ChessFileManager::Get().ClearSearch(std::get<0>(m_Search));
		m_Search = { SearchID(), std::make_shared<std::pair<SearchOptions, SearchResult>>() };

		FileManager::Get().RemoveFile(m_ID);
		ChessFileManager::Get().RemoveFileReference(m_ID);

		m_ID = UUID();
		m_GamePointers = std::make_shared<std::vector<size_t>>();
		m_LabelNames = std::make_shared<std::vector<std::string>>();
		m_LabelValues = std::make_shared<std::vector<std::string>>();
		m_typeName = std::make_shared<uint8_t>(1);
		m_typeValue = std::make_shared<uint8_t>(4);
		m_Settings = std::make_shared<uint8_t>(0);
		m_SearchGameResults.reset();
		m_SearchTopGames.reset();
		m_SearchGameIndexes = std::make_shared<std::tuple<size_t, size_t, size_t, size_t, size_t, size_t, size_t>>();

		ChessFileManager::Get().AddClrFileReference(m_ID, m_GamePointers, m_LabelNames, m_LabelValues, m_typeName, m_typeValue, m_Settings, m_SearchGameIndexes);
	}

	void ClrFile::StartSearch(size_t index)
	{
		m_SearchGameResults = std::make_shared<SearchResultMoveData>();
		m_SearchTopGames = std::make_shared<std::vector<size_t>>();
		(*m_SearchGameResults)[{0, 0}] = { 0, 0, 0, 0, 0, 0 };

		auto& [id, data] = m_Search;

		data->first.InitClrSearch(m_ID, m_LabelNames, m_LabelValues);

		ChessFileManager::Get().SetUpClrResults(id, m_SearchGameResults, m_SearchTopGames);
		ChessFileManager::Get().StartSearch(id, m_ID, data);		
	}

	void ClrFile::StopSearch(size_t index)
	{
		auto& [id, data] = m_Search;
		ChessFileManager::Get().ClearSearch(id);
		id = SearchID();
		data = std::make_shared<std::pair<SearchOptions, SearchResult>>();
		m_SearchGameResults.reset();
		m_SearchTopGames.reset();
	}

	std::shared_ptr<std::pair<SearchOptions, SearchResult>> ClrFile::GetSearch(size_t index)
	{
		return m_Search.second;
	}

	void ClrFile::GetCldGame(CldGame& game, size_t index)
	{
		std::vector<uint8_t> data;
		FileManager::Get().ReadBuffer(m_ID, (*m_GamePointers)[index], ((index + 1 < m_GamePointers->size()) ? ((*m_GamePointers)[index + 1] - (*m_GamePointers)[index]) : SIZE_MAX), (std::vector<uint8_t>&)data);

		game.Parse(data, (*m_Settings) % 2 == 0 ? MoveEncoding::CLD : MoveEncoding::CORE, (*m_typeName), (*m_typeValue), true, true, false, false);
	}

	size_t ClrFile::GetWhiteWins() const
	{
		if (m_SearchGameResults)
			return std::get<0>((*m_SearchGameResults)[{0, 0}]);
		else
			return 0;
	}

	size_t ClrFile::GetBlackWins() const
	{
		if (m_SearchGameResults)
			return std::get<1>((*m_SearchGameResults)[{0, 0}]);
		else
			return 0;
	}

	size_t ClrFile::GetDraws() const
	{
		if (m_SearchGameResults)
			return std::get<2>((*m_SearchGameResults)[{0, 0}]);
		else
			return 0;
	}

	size_t ClrFile::GetAverageElo() const
	{
		if (m_SearchGameResults)
			return std::get<3>((*m_SearchGameResults)[{0, 0}]);
		else
			return 0;
	}

	std::vector<size_t> ClrFile::GetSearchTopGames() const
	{
		if (m_SearchTopGames)
			return *m_SearchTopGames;
		else
			return std::vector<size_t>();
	}

	std::shared_ptr<SearchResultMoveData> ClrFile::GetMoveResults() const
	{
		return m_SearchGameResults;
	}
}