#pragma once

#include "UUID/UUID.h"

#include "pgn/PgnGame.h"
#include "cld/CldGame.h"

#include <memory>
#include <unordered_set>
#include <variant>

namespace Chess
{
	using SearchID = UUID;
	
	class SearchOptions
	{
	public:
		struct TextOption
		{
			std::string value;
			size_t pos;
		};

		struct NumberOption
		{
			int value;
		};

		struct RangeNumberOption
		{
			int valueMin;
			int valueMax;
		};

	public:
		bool IsGameValid(PgnGame& game);
		bool IsGameValid(CldGame& game);

		SearchOptions& StartOption();
		SearchOptions& And(const std::string& name, const std::string& value, size_t pos = std::string::npos);
		SearchOptions& And(const std::string& name, int value);
		SearchOptions& And(const std::string& name, int valueMin, int valueMax);
		void EndOption();

		void InitCldSearch(std::shared_ptr<std::vector<std::string>> labelNames, std::shared_ptr<std::vector<std::string>> labelValues);
		void Clear() { m_PgnOptions.clear(); m_CldOptions.clear(); }

		void SetOptionByData(const std::vector<uint8_t>& data);
		void GetOptionData(std::vector<uint8_t>& data) const;

		TextOption GetOptionText(const std::string& name) const;
		NumberOption GetOptionNumber(const std::string& name) const;
		RangeNumberOption GetOptionRangeNumber(const std::string& name) const;
			
	private:
		std::vector<std::unordered_map<std::string, std::variant<TextOption, NumberOption, RangeNumberOption>>> m_PgnOptions;
		std::vector<std::unordered_map<size_t, std::unordered_set<size_t>>> m_CldOptions;
	};

	//only Read
	struct SearchResult
	{
		float Persentage = 0.0f;
		std::vector<size_t> PossitiveIndexes;
	};
}