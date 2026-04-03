#include "pgn/PgnFile.h"
#include "cld/CldFile.h"
#include "ChessFileManager.h"

#include <fstream>

namespace Chess
{
	void ConvertToPgn(const CldFile& cldFile, const std::filesystem::path& destination)
	{
		if (destination.extension().string() != ".pgn")
			return;

		auto id = FileManager::Get().AddFile(destination);
		auto cachePath = FileManager::Get().GetCachePath(id);
		FileManager::Get().RemoveFile(id);

		std::ofstream pointerFile(cachePath / "thisfile.ppgn", std::ios::binary | std::ios::trunc);
		std::ofstream outfile(destination, std::ios::binary | std::ios::trunc);

		size_t index = 0;
		std::string dataToWrite;

		for (size_t i = 0; i < cldFile.GetSize(); i++)
		{
			pointerFile.write((char*)&index, 8);
			dataToWrite = cldFile.At(i).GetData();
			outfile.write(dataToWrite.c_str(), dataToWrite.size());
			index += dataToWrite.size();
		}

		outfile.close();
		pointerFile.close();
	}

	void ConvertToCld(const PgnFile& pgnFile, const std::filesystem::path& destination)
	{
		if (destination.extension().string() != ".cld")
			return;

		auto id = FileManager::Get().AddFile(destination);
		auto cachePath = FileManager::Get().GetCachePath(id);
		FileManager::Get().RemoveFile(id);

		std::vector<uint8_t> dataToWrite;
		CldGame cldGameToAdd;
		PgnGame pgnGame;

		std::vector<size_t> gameIndexes;
		std::vector<std::string> labelNames, labelValues;
		std::unordered_map<std::string, size_t> labelNamesMap, labelValuesMap;
		uint8_t typeName = 1, typeValue = 4;

		dataToWrite.reserve(10'000);

		gameIndexes.reserve(pgnFile.GetSize());
		gameIndexes.emplace_back(0);

		labelNames.reserve(100);
		labelValues.reserve(pgnFile.GetSize() * 2);
		labelNamesMap.reserve(100);
		labelValuesMap.reserve(pgnFile.GetSize() * 2);

		labelNames.emplace_back("?");
		labelValues.emplace_back("?");
		labelNamesMap["?"] = 0;
		labelValuesMap["?"] = 0;

		std::ofstream helpOutfile(cachePath / "helper", std::ios::binary | std::ios::trunc);

		for (size_t i = 0; i < pgnFile.GetSize(); i++)
		{
			cldGameToAdd.Clear();

			pgnGame = pgnFile.At(i);

			if (pgnGame.IsLabelExist("Variant") && pgnGame["Variant"] == "chess960")
				continue;

			auto labels = pgnGame.GetLabelNames();

			for (auto& label : labels)
			{
				std::string value = pgnGame[label];
				if (value == "?" || value.empty())
					continue;

				size_t indexName = -1;
				if (!labelNamesMap.contains(label))
				{
					indexName = labelNames.size();
					labelNamesMap[label] = labelNames.size();
					labelNames.emplace_back(label);
				}
				else
					indexName = labelNamesMap[label];

				size_t indexValue = -1;
				if (!labelValuesMap.contains(value))
				{
					indexValue = labelValues.size();
					labelValuesMap[value] = labelValues.size();
					labelValues.emplace_back(value);
				}
				else
					indexValue = labelValuesMap[value];

				cldGameToAdd[indexName] = indexValue;
			}

			ChessFileManager::ConvertPgnMovePathToCldMovePath(cldGameToAdd.GetMovePathbyRef(), pgnGame.GetMovePathbyRef());

			cldGameToAdd.GetData(dataToWrite, typeName, typeValue);
			gameIndexes.emplace_back(dataToWrite.size() + gameIndexes.back());

			helpOutfile.write((char*)dataToWrite.data(), dataToWrite.size());
		}

		helpOutfile.close();
		gameIndexes.pop_back();

		std::vector<char> NameBuffer, ValueBuffer;

		for (auto& str : labelNames)
		{
			for(auto& c : str)
				NameBuffer.emplace_back(c);
			NameBuffer.emplace_back(0);
		}

		for (auto& str : labelValues)
		{
			for (auto& c : str)
				ValueBuffer.emplace_back(c);
			ValueBuffer.emplace_back(0);
		}

		const uint8_t title[8] = { 'C', 'L', 'D', 0x01, 0x00, 0x00, typeName, typeValue };
		uint64_t indexName = 40;
		uint64_t indexValue = indexName + NameBuffer.size();
		//check if we have no games
		uint64_t indexGamePointers = indexValue + ValueBuffer.size();
		size_t numberOfGames = gameIndexes.size();

		size_t indexGames = indexGamePointers + gameIndexes.size() * 8;
		for (auto& index : gameIndexes)
			index += indexGames;

		std::ofstream outfile(destination, std::ios::binary | std::ios::trunc);

		outfile.write((char*)title, 8);
		outfile.write((char*)&indexName, 8);
		outfile.write((char*)&indexValue, 8);
		outfile.write((char*)&indexGamePointers, 8);
		outfile.write((char*)&numberOfGames, 8);

		outfile.write(NameBuffer.data(), NameBuffer.size());
		outfile.write(ValueBuffer.data(), ValueBuffer.size());
		outfile.write((char*)gameIndexes.data(), gameIndexes.size() * 8);

		std::ifstream helpInfile(cachePath / "helper", std::ios::binary);

		std::vector<char> bufferToCopy(40'000'000);

		helpInfile.seekg(0, std::ios_base::end);
		std::streampos maxIndex = helpInfile.tellg();
		helpInfile.seekg(0, std::ios::beg);

		while (helpInfile.good())
		{
			size_t maxAmount = std::streamoff(maxIndex) - helpInfile.tellg();
			size_t sizeToCopy = std::min(40'000'000ull, maxAmount);
			bufferToCopy.resize(sizeToCopy);

			helpInfile.read(bufferToCopy.data(), sizeToCopy);
			outfile.write(bufferToCopy.data(), sizeToCopy);

			if (maxAmount <= 40'000'000ull)
				break;
		}

		helpInfile.close();

		outfile.close();
	}
}