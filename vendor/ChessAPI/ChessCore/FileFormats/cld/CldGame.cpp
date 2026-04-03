#include "CldGame.h"

namespace Chess
{
	CldGame::~CldGame()
	{

	}

	const CldGame::CldMovesPath CldGame::GetMovePathbyCopy() const
	{
		return m_MovesPath;
	}

	CldGame::CldMovesPath& CldGame::GetMovePathbyRef()
	{
		return m_MovesPath;
	}

	std::vector<size_t> CldGame::GetLabelNames() const
	{
		std::vector<size_t> LabelNames;
		LabelNames.reserve(m_Labels.size());

		for (auto& [name, value] : m_Labels)
			LabelNames.emplace_back(name);
		
		return LabelNames;
	}

	void CldGame::Clear()
	{
		m_Labels.clear();
		m_MovesPath.move.clear();
		m_MovesPath.children.clear();
		m_MovesPath.details.clear();
	}

	bool CldGame::IsLabelExist(size_t name) const
	{
		return m_Labels.contains(name);
	}


	void CldGame::RemoveLabel(size_t name)
	{
		if (m_Labels.contains(name))
			m_Labels.erase(name);
	}

	size_t& CldGame::operator[](size_t label)
	{
		if (!m_Labels.contains(label))
			m_Labels[label] = 0;

		return m_Labels[label];
	}

	void CldGame::GetData(std::vector<uint8_t>& data, uint8_t nameType, uint8_t valueType) const
	{
		data.clear();
		data.resize(m_Labels.size() * (nameType + valueType) + 4, UINT8_MAX);

		uint8_t* ptr = data.data();

		for (auto& [name, value] : m_Labels)
		{
			if (nameType == 1)
			{
				*ptr = *(uint8_t*)&name;
				ptr++;
			}
			else if (nameType == 2)
			{
				*(uint16_t*)ptr = *(uint16_t*)&name;
				ptr += 2;
			}
			else if (nameType == 4)
			{
				*(uint32_t*)ptr = *(uint32_t*)&name;
				ptr += 4;
			}

			if (valueType == 1)
			{
				*ptr = *(uint8_t*)&value;
				ptr++;
			}
			else if (valueType == 2)
			{
				*(uint16_t*)ptr = *(uint16_t*)&value;
				ptr += 2;
			}
			else if (valueType == 4)
			{
				*(uint32_t*)ptr = *(uint32_t*)&value;
				ptr += 4;
			}
		}
				
		WriteMoves(data, m_MovesPath);
	}

	void CldGame::GetDataRead(std::vector<uint8_t>& data) const
	{
		data = m_DataRead;
	}

	void CldGame::Parse(std::span<uint8_t> data, uint8_t nameType, uint8_t valueType, bool onlyRead, bool readMoves)
	{
		Clear();

		if (data.empty())
			return;
		
		size_t movesStartIndex = 0;

		for (size_t i = 0; i < data.size(); i += (nameType + valueType))
		{
			if (*(uint32_t*)&data[i] == UINT32_MAX)
			{
				movesStartIndex = i + 4;
				break;
			}

			uint32_t labelNameToAdd = 0;
			uint32_t labelValueToAdd = 0;

			if (nameType == 1)
			{
				labelNameToAdd = *(uint8_t*)&data[i];
			}
			else if (nameType == 2)
			{
				labelNameToAdd = *(uint16_t*)&data[i];
			}
			else if (nameType == 4)
			{
				labelNameToAdd = *(uint32_t*)&data[i];
			}

			if (valueType == 1)
			{
				labelValueToAdd = *(uint8_t*)&data[i + nameType];
			}
			else if (valueType == 2)
			{
				labelValueToAdd = *(uint16_t*)&data[i + nameType];
			}
			else if (valueType == 4)
			{
				labelValueToAdd = *(uint32_t*)&data[i + nameType];
			}

			m_Labels[labelNameToAdd] = labelValueToAdd;
		}
		
		if (!readMoves)
			return;
		
		const uint8_t child = 255;
		const uint8_t regulaMove	 = 64;
		const uint8_t openVariation  = 253;
		const uint8_t closeVariation = 254;
		const uint8_t detail         = 255;
		//everything else is table moves

		CldMovesPath* Parent = &m_MovesPath;
		Parent->details.reserve(20);
		Parent->move.reserve(80);
		
		bool detailsOpen = false;
		uint8_t startMovePart = 64;

		for (size_t i = movesStartIndex; i < data.size(); i++)
		{
			//regular move started
			if (startMovePart < regulaMove)
			{
				Parent->move.emplace_back(startMovePart, data[i]);
				startMovePart = regulaMove;
				continue;
			}

			//details
			{
				if (data[i] == detail && !detailsOpen)
				{
					detailsOpen = true;
					continue;
				}

				if (data[i] == detail && detailsOpen)
				{
					size_t detailIndex = Parent->move.size() - 1;

					Parent->details[detailIndex].note += ' ';

					std::string realNote;
					bool lastPer = false;
					bool cmdSection = false, cmdSectionValue = false;

					std::string cmdName, cmdValue;

					for (int i = 0; i < Parent->details[detailIndex].note.size() - 1; i++)
					{
						if (Parent->details[detailIndex].note[i] == '[' && Parent->details[detailIndex].note[i + 1] == '%')
						{
							cmdSection = true;
							i++;
							continue;
						}

						if (cmdSection && !cmdSectionValue && (Parent->details[detailIndex].note[i] == ' ' || Parent->details[detailIndex].note[i] == '\n'))
						{
							cmdSectionValue = true;
							continue;
						}

						if (cmdSectionValue && Parent->details[detailIndex].note[i] == ']')
						{
							cmdSection = false;
							cmdSectionValue = false;

							Parent->details[detailIndex].cmds[cmdName] = cmdValue;

							cmdName.clear();
							cmdValue.clear();
							continue;
						}

						if (cmdSection)
						{
							if (cmdSectionValue)
								cmdValue += Parent->details[detailIndex].note[i];
							else
								cmdName += Parent->details[detailIndex].note[i];
							continue;
						}

						if (Parent->details[detailIndex].note[i] != '%')
						{
							if (lastPer)
							{
								lastPer = false;
								realNote += '%';
							}
						}
						else
							lastPer = true;

						if (realNote.empty() && (Parent->details[detailIndex].note[i] == ' ' || Parent->details[detailIndex].note[i] == '\n'))
							continue;

						realNote += Parent->details[detailIndex].note[i];
					}

					Parent->details[detailIndex].note = realNote;

					detailsOpen = false;
					continue;
				}

				if (detailsOpen)
				{
					Parent->details[Parent->move.size() - 1].note += data[i];
					continue;
				}
			}

			//variant
			{
				if (data[i] == openVariation)
				{
					Parent->move.emplace_back(child, child);					
					Parent = &Parent->children.emplace_back(CldMovesPath(Parent));

					Parent->details.reserve(20);
					Parent->move.reserve(30);
					continue;
				}

				if (data[i] == closeVariation)
				{
					Parent = Parent->parent;
					continue;
				}
			}

			if(data[i] < regulaMove)
			{
				//start part of move
				startMovePart = data[i];
			}
			else
			{
				//table move
				Parent->move.emplace_back(data[i], 0ui8);
			}

		}

		if (!onlyRead)
			GetData(m_DataRead);

		m_MovesPath.ReloadChildren();
	}

	void CldGame::WriteMoves(std::vector<uint8_t>& data, CldMovesPath movePath) const
	{
		const uint8_t child = 255;
		const uint8_t regulaMove = 64;
		const uint8_t openVariation = 253;
		const uint8_t closeVariation = 254;
		const uint8_t detail = 255;

		int index = 0;

		if (movePath.details.contains(-1) && (movePath.details.at(-1).note != "" || !movePath.details.at(-1).cmds.empty()))
		{
			auto detailData = movePath.details.at(-1);

			data.emplace_back(detail);

			if (!detailData.note.empty())
			{
				size_t pos = data.size();
				data.resize(pos + detailData.note.size());
				std::memcpy(data.data() + pos, detailData.note.data(), detailData.note.size());
			}

			for (auto& [cmdname, cmdvalue] : detailData.cmds)
			{
				if (!detailData.note.empty())
					data.emplace_back(' ');
				
				std::string cmd = "[%" + cmdname + " " + cmdvalue + "] ";
				size_t pos = data.size();
				data.resize(pos + cmd.size());
				std::memcpy(data.data() + pos, cmd.data(), cmd.size());
			}

			data.emplace_back(detail);
		}

		for (int i = 0; i < movePath.move.size(); i++)
		{
			if (movePath.move[i].first == child)
			{
				data.emplace_back(openVariation);
				WriteMoves(data, movePath.children[index]); 
				data.emplace_back(closeVariation);

				index += 1;
			}
			else
			{
				data.emplace_back(movePath.move[i].first);

				if (movePath.move[i].first < regulaMove)
					data.emplace_back(movePath.move[i].second);

				if (movePath.details.contains(i) && (movePath.details.at(i).note != "" || !movePath.details.at(i).cmds.empty()))
				{
					auto detailData = movePath.details.at(i);

					data.emplace_back(detail);

					if (!detailData.note.empty())
					{
						size_t pos = data.size();
						data.resize(pos + detailData.note.size());
						std::memcpy(data.data() + pos, detailData.note.data(), detailData.note.size());
					}

					for (auto& [cmdname, cmdvalue] : detailData.cmds)
					{
						if (!detailData.note.empty())
							data.emplace_back(' ');

						std::string cmd = "[%" + cmdname + " " + cmdvalue + "] ";
						size_t pos = data.size();
						data.resize(pos + cmd.size());
						std::memcpy(data.data() + pos, cmd.data(), cmd.size());
					}

					data.emplace_back(detail);
				}
			}
		}
	}
}