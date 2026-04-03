#include "SearchWork.h"

namespace Chess
{
	bool SearchOptions::IsGameValid(PgnGame& game)
	{
		for (auto& option : m_PgnOptions)
		{
			bool successed = true;

			for (auto& [name, value] : option)
			{
				if (game[name].find(value.first) == value.second)
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
		lastOption[name] = { value, pos };

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
				size_t valueIndex = -1;
				
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

				for (size_t i = 0; i < labelValues->size(); i++)
				{
					if (value.second == std::string::npos)
					{
						if ((*labelValues)[i].find(value.first) != std::string::npos)
						{
							valueIndex = i;
							m_CldOptions.back()[nameIndex].insert(i);
						}
					}
					else
					{
						if ((*labelValues)[i].find(value.first) == value.second)
						{
							valueIndex = i;
							m_CldOptions.back()[nameIndex].insert(i);
						}
					}
				}

				if (valueIndex == -1)
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
		std::string str, lastName;

		for (int i = 0; i < data.size(); i++)
		{
			if (data[i] == 0)
			{
				if (nameArea)
				{
					lastName = str;
					nameArea = false;
				}
				else
				{
					m_PgnOptions.back()[lastName].first = str;
					m_PgnOptions.back()[lastName].second = std::string::npos;

					nameArea = true;
				}

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

				for (int i = 0; i < value.first.size(); i++)
				{
					data.emplace_back(value.first[i]);
				}

				data.emplace_back(0);
			}
		}
	}

	std::string SearchOptions::GetOptionValue(const std::string& name) const
	{
		for (auto& option : m_PgnOptions)
		{
			if (option.contains(name))
				return option.at(name).first;
		}

		return "";
	}

}