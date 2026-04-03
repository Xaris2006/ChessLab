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
	void CloseOpenGame(int index);

	Chess::ChessFile& GetPgnFile();
	Chess::PgnGame& GetPgnGame();
	Chess::GameManager& GetActiveGame();
	std::filesystem::path& GetPgnFilePath();
	std::string& GetPgnFileName();

	int	GetBlockID(int BlockIndex);
	
	void OpenChessFile(const std::filesystem::path& path);
	void OverWriteChessFile(const std::filesystem::path& filepath);
	void OpenChessGameInFile(int index);
	void NewGameInFile();
}