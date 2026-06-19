#pragma once

#include <string>
#include <filesystem>

#include "ChessCore/GameManager.h"
#include "ChessCore/FileFormats/base/ChessFile.h"

namespace ChessAPI
{
	void Init();

	int GetActiveGameIndex();
	std::vector<int>& GetOpenGameIndexes();
	bool IsGameOpen(size_t index);
	
	void NewGameInFile();
	void OpenChessGameInFile(int index);
	void CloseOpenGame(int index);

	Chess::ChessFile& GetChessFile();
	Chess::PgnGame& GetPgnGame();
	Chess::GameManager& GetActiveGame();
	std::filesystem::path& GetChessFilePath();
	std::string& GetChessFileName();

	int	GetBlockID(int BlockIndex);
	
	void OpenChessFile(const std::filesystem::path& path, float* persentage = nullptr);
	void OverWriteChessFile(const std::filesystem::path& filepath, float* persentage = nullptr);
}