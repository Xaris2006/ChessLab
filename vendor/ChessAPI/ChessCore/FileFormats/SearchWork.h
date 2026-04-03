#pragma once

#include "UUID/UUID.h"

#include "pgn/PgnGame.h"
#include "cld/CldGame.h"

#include <memory>
#include <unordered_set>

namespace Chess
{
	using SearchID = UUID;
	
	class SearchOptions
	{
	public:
		bool IsGameValid(PgnGame& game);
		bool IsGameValid(CldGame& game);

		SearchOptions& StartOption();
		SearchOptions& And(const std::string& name, const std::string& value, size_t pos = std::string::npos);
		void EndOption();

		void InitCldSearch(std::shared_ptr<std::vector<std::string>> labelNames, std::shared_ptr<std::vector<std::string>> labelValues);
		void Clear() { m_PgnOptions.clear(); m_CldOptions.clear(); }

		void SetOptionByData(const std::vector<uint8_t>& data);
		void GetOptionData(std::vector<uint8_t>& data) const;
		std::string GetOptionValue(const std::string& name) const;

	private:
		std::vector<std::unordered_map<std::string, std::pair<std::string, size_t>>> m_PgnOptions;
		std::vector<std::unordered_map<size_t, std::unordered_set<size_t>>> m_CldOptions;
	};

	//only Read
	struct SearchResult
	{
		float Persentage = 0.0f;
		std::vector<size_t> PossitiveIndexes;
	};
}