#include "SearchWork.h"

#include <optional>
#include <charconv>

static std::optional<double> getNumber(const std::string& s) {
	if (s.empty()) 
		return {};

	int value;
	auto result = std::from_chars(s.data(), s.data() + s.size(), value);

	if (result.ec == std::errc() && result.ptr == s.data() + s.size())
		return value;

	return {};
}

namespace Chess
{
	bool SearchOptions::IsGameValid(PgnGame& game)
	{
		for (auto& option : m_PgnOptions)
		{
			bool successed = true;

			for (auto& [name, value] : option)
			{
				if (!game.IsLabelExist(name))
				{
					successed = false;
					break;
				}

				if (auto textValueP = std::get_if<TextOption>(&value))
				{
					if (textValueP->pos == std::string::npos)
					{
						if (game[name].find(textValueP->value) == std::string::npos)
						{
							successed = false;
							break;
						}
					}
					else
					{
						if (game[name].find(textValueP->value) != textValueP->pos)
						{
							successed = false;
							break;
						}
					}					
				}
				else if (auto numberValueP = std::get_if<NumberOption>(&value))
				{
					if (auto number = getNumber(game[name]))
					{
						if (*number != numberValueP->value)
						{
							successed = false;
							break;
						}
					}
					else
					{
						successed = false;
						break;
					}
				}
				else if (auto rangeNumberValueP = std::get_if<RangeNumberOption>(&value))
				{
					if (auto number = getNumber(game[name]))
					{
						if (*number < rangeNumberValueP->valueMin || *number > rangeNumberValueP->valueMax)
						{
							successed = false;
							break;
						}
					}
					else
					{
						successed = false;
						break;
					}
				}
			}

			if (successed)
				return true;
		}

		return false;
	}

	bool SearchOptions::IsGameValid(CldGame& game)
	{
		for (auto& option : m_CldOptions)
		{
			bool successed = true;

			for (auto& [name, values] : option)
			{
				size_t valueIndex = game[name];
				if (!values.contains(valueIndex))
				{
					successed = false;
					break;
				}
			}

			if (successed)
				return true;
		}

		return false;
	}

	SearchOptions& SearchOptions::StartOption()
	{
		m_PgnOptions.emplace_back();
		return *this;
	}

	SearchOptions& SearchOptions::And(const std::string& name, const std::string& value, size_t pos)
	{
		if (m_PgnOptions.empty() || name.empty() || value.empty())
			return *this;

		auto& lastOption = m_PgnOptions.back();
		lastOption[name] = TextOption{ value, pos };

		return *this;
	}

	SearchOptions& SearchOptions::And(const std::string& name, int value)
	{
		if (m_PgnOptions.empty() || name.empty())
			return *this;

		auto& lastOption = m_PgnOptions.back();
		lastOption[name] = NumberOption{ value };

		return *this;
	}

	SearchOptions& SearchOptions::And(const std::string& name, int valueMin, int valueMax)
	{
		if (m_PgnOptions.empty() || name.empty() || valueMin > valueMax)
			return *this;

		auto& lastOption = m_PgnOptions.back();
		lastOption[name] = RangeNumberOption{ valueMin, valueMax };

		return *this;
	}

	void SearchOptions::EndOption()
	{
		if (m_PgnOptions.empty())
			return;

		if (m_PgnOptions.back().empty())
			m_PgnOptions.pop_back();
	}

	void SearchOptions::InitCldSearch(std::shared_ptr<std::vector<std::string>> labelNames, std::shared_ptr<std::vector<std::string>> labelValues)
	{
		m_CldOptions.clear();

		for (auto& option : m_PgnOptions)
		{
			m_CldOptions.emplace_back();

			for (auto& [name, value] : option)
			{
				size_t nameIndex = -1;
				
				for (size_t i = 0; i < labelNames->size(); i++)
				{
					if ((*labelNames)[i] == name)
					{
						nameIndex = i;
						break;
					}
				}

				if (nameIndex == -1)
				{
					m_CldOptions.pop_back();
					break;
				}

				if (auto textValueP = std::get_if<TextOption>(&value))
				{
					if (textValueP->pos == std::string::npos)
					{
						for (size_t i = 0; i < labelValues->size(); i++)
						{
							if ((*labelValues)[i].find(textValueP->value) != std::string::npos)
								m_CldOptions.back()[nameIndex].insert(i);
						}
					}
					else
					{
						for (size_t i = 0; i < labelValues->size(); i++)
						{
							if ((*labelValues)[i].find(textValueP->value) == textValueP->pos)
								m_CldOptions.back()[nameIndex].insert(i);
						}
					}
				}
				else if (auto numberValueP = std::get_if<NumberOption>(&value))
				{
					for (size_t i = 0; i < labelValues->size(); i++)
					{
						if (auto number = getNumber((*labelValues)[i]))
						{
							if (*number == numberValueP->value)
								m_CldOptions.back()[nameIndex].insert(i);
						}
					}
				}
				else if (auto rangeNumberValueP = std::get_if<RangeNumberOption>(&value))
				{
					for (size_t i = 0; i < labelValues->size(); i++)
					{
						if (auto number = getNumber((*labelValues)[i]))
						{
							if (*number >= rangeNumberValueP->valueMin && *number <= rangeNumberValueP->valueMax)
								m_CldOptions.back()[nameIndex].insert(i);
						}
					}
				}

				if (m_CldOptions.back()[nameIndex].empty())
				{
					m_CldOptions.pop_back();
					break;
				}
			}
		}
	}

	void SearchOptions::SetOptionByData(const std::vector<uint8_t>& data)
	{
		m_PgnOptions.clear();

		bool nameArea = true;
		bool textArea = false;
		bool numberArea = false;
		bool rangeNumberArea = false;

		std::string str, lastName;

		for (int i = 0; i < data.size(); i++)
		{
			if (!nameArea && !textArea && !numberArea && !rangeNumberArea)
			{
				if (data[i] == 1)
					textArea = true;
				else if (data[i] == 2)
					numberArea = true;
				else if (data[i] == 3)
					rangeNumberArea = true;

				continue;
			}

			if (textArea)
			{
				if (data[i] == 0)
				{
					size_t pos;
					memcpy(&pos, &data[i + 1], sizeof(size_t));

					m_PgnOptions.back()[lastName] = TextOption{ str, pos };
					str.clear();
					textArea = false;
					nameArea = true;
					i += 8;
				}
				else
					str += data[i];
				
				continue;
			}
			else if (numberArea)
			{
				int indeger;
				memcpy(&indeger, &data[i], sizeof(int));
				
				m_PgnOptions.back()[lastName] = NumberOption{ indeger };
				numberArea = false;
				nameArea = true;
				i += 3;

				continue;
			}
			else if (rangeNumberArea)
			{
				int indegerMin, indegerMax;
				memcpy(&indegerMin, &data[i], sizeof(int));
				memcpy(&indegerMax, &data[i + 4], sizeof(int));

				m_PgnOptions.back()[lastName] = RangeNumberOption{ indegerMin, indegerMax };
				rangeNumberArea = false;
				nameArea = true;
				i += 7;

				continue;
			}

			if (data[i] == 0 && nameArea)
			{
				lastName = str;
				nameArea = false;
				str.clear();
				
				continue;
			}

			if (data[i] == 255)
			{
				m_PgnOptions.emplace_back();
				continue;
			}

			str += data[i];
		}
	}

	void SearchOptions::GetOptionData(std::vector<uint8_t>& data) const
	{
		data.clear();

		for (auto& option : m_PgnOptions)
		{			
			data.emplace_back(255);
			
			for (auto& [name, value] : option)
			{
				for (int i = 0; i < name.size(); i++)
				{
					data.emplace_back(name[i]);
				}

				data.emplace_back(0);

				if (auto textValueP = std::get_if<TextOption>(&value))
				{
					data.emplace_back(1);

					for (int i = 0; i < textValueP->value.size(); i++)
					{
						data.emplace_back(textValueP->value[i]);
					}

					data.emplace_back(0);

					data.reserve(data.size() + 8);
					data.insert(data.end(), (uint8_t*)&textValueP->pos, (uint8_t*)&textValueP->pos + sizeof(size_t));
				}
				else if (auto numberValueP = std::get_if<NumberOption>(&value))
				{
					data.emplace_back(2);

					data.reserve(data.size() + 4);
					data.insert(data.end(), (uint8_t*)&numberValueP->value, (uint8_t*)&numberValueP->value + sizeof(int));
				}
				else if (auto rangeNumberValueP = std::get_if<RangeNumberOption>(&value))
				{
					data.emplace_back(3);

					data.reserve(data.size() + 8);
					data.insert(data.end(), (uint8_t*)&rangeNumberValueP->valueMin, (uint8_t*)&rangeNumberValueP->valueMin + sizeof(int));
					data.insert(data.end(), (uint8_t*)&rangeNumberValueP->valueMax, (uint8_t*)&rangeNumberValueP->valueMax + sizeof(int));
				}
			}
		}
	}

	SearchOptions::TextOption SearchOptions::GetOptionText(const std::string& name) const
	{
		for (auto& option : m_PgnOptions)
		{
			if (option.contains(name))
			{
				if (auto textValueP = std::get_if<TextOption>(&option.at(name)))
					return *textValueP;
			}
		}

		return { "", std::string::npos };
	}

	SearchOptions::NumberOption SearchOptions::GetOptionNumber(const std::string& name) const
	{
		for (auto& option : m_PgnOptions)
		{
			if (option.contains(name))
			{
				if (auto numberValueP = std::get_if<NumberOption>(&option.at(name)))
					return *numberValueP;
			}
		}
		return { 0 };
	}

	SearchOptions::RangeNumberOption SearchOptions::GetOptionRangeNumber(const std::string& name) const
	{
		for (auto& option : m_PgnOptions)
		{
			if (option.contains(name))
			{
				if (auto rangeNumberValueP = std::get_if<RangeNumberOption>(&option.at(name)))
					return *rangeNumberValueP;
			}
		}
		return { 0, 0 };
	}
}