#pragma once

#include <vector>
#include <string>

namespace Chess
{
	void InitializeMoveTables();

	void GetMoveTablesNames(std::vector<std::string>& names);

	uint8_t GetMoveIntex(std::string_view name, uint16_t move, size_t count);
	uint16_t GetMoveByIndex(std::string_view name, uint8_t index, size_t count);
}