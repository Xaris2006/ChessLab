#include "CldGame.h"

#include "../MoveTables.h"

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
		for (int i = 0; i < m_Labels.size(); i++)
		{
			if (m_Labels[i].first == name)
				return true;
		}

		return 0;

		//return m_Labels.contains(name);
	}


	void CldGame::RemoveLabel(size_t name)
	{
		//if (m_Labels.contains(name))
		//	m_Labels.erase(name);

		for (int i = 0; i < m_Labels.size(); i++)
		{
			if (m_Labels[i].first == name)
			{
				m_Labels.erase(m_Labels.begin() + i);
				return;
			}
		}
	}

	size_t& CldGame::operator[](size_t label)
	{
		//if (!m_Labels.contains(label))
		//	m_Labels[label] = 0;

		//return m_Labels[label];

		for (int i = 0; i < m_Labels.size(); i++)
		{
			if (m_Labels[i].first == label)
				return m_Labels[i].second;
		}

		return m_Labels.emplace_back(label, 0).second;
	}

	size_t CldGame::At(size_t label) const
	{
		//return m_Labels.at(label);

		for (int i = 0; i < m_Labels.size(); i++)
		{
			if (m_Labels[i].first == label)
				return m_Labels[i].second;
		}

		return 0;
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

	void CldGame::Parse(std::span<uint8_t> data, MoveEncoding encoding, uint8_t nameType, uint8_t valueType, bool onlyRead, bool readLabels, bool readMoves, bool readDetails)
	{
		Clear();

		m_Encoding = encoding;

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
		
			if (!readLabels)
				continue;
		
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
			
			//m_Labels.try_emplace(labelNameToAdd, labelValueToAdd);
			m_Labels.emplace_back(labelNameToAdd, labelValueToAdd);
		}
		
		//switch ((nameType << 4) | valueType)
		//{
		//	// Common combinations (Compile distinct blazing fast loops for each)
		//case (1 << 4) | 1: ParseLabelsContainer<uint8_t, uint8_t>(data, movesStartIndex, readLabels, m_Labels); break;
		//case (1 << 4) | 2: ParseLabelsContainer<uint8_t, uint16_t>(data, movesStartIndex, readLabels, m_Labels); break;
		//case (1 << 4) | 4: ParseLabelsContainer<uint8_t, uint32_t>(data, movesStartIndex, readLabels, m_Labels); break;
		//
		//case (2 << 4) | 1: ParseLabelsContainer<uint16_t, uint8_t>(data, movesStartIndex, readLabels, m_Labels); break;
		//case (2 << 4) | 2: ParseLabelsContainer<uint16_t, uint16_t>(data, movesStartIndex, readLabels, m_Labels); break;
		//case (2 << 4) | 4: ParseLabelsContainer<uint16_t, uint32_t>(data, movesStartIndex, readLabels, m_Labels); break;
		//
		//case (4 << 4) | 1: ParseLabelsContainer<uint32_t, uint8_t>(data, movesStartIndex, readLabels, m_Labels); break;
		//case (4 << 4) | 2: ParseLabelsContainer<uint32_t, uint16_t>(data, movesStartIndex, readLabels, m_Labels); break;
		//case (4 << 4) | 4: ParseLabelsContainer<uint32_t, uint32_t>(data, movesStartIndex, readLabels, m_Labels); break;
		//
		//default: break; // Handle invalid types if necessary
		//}

		if (!readMoves)
			return;
		
		const uint8_t child = 255;
		const uint8_t regulaMove	 = 64;
		const uint8_t openVariation  = 253;
		const uint8_t closeVariation = 254;
		const uint8_t detail         = 255;
		//everything else is table moves

		CldMovesPath* Parent = &m_MovesPath;
		auto* moves = &Parent->move;
		//Parent->details.reserve(20);
		//Parent->move.reserve(80);
		
		bool detailsOpen = false;
		uint8_t startMovePart = 64;

		for (size_t i = movesStartIndex; i < data.size(); i++)
		{
			//regular move started
			if (startMovePart < regulaMove)
			{
				moves->emplace_back(startMovePart, data[i]);
				startMovePart = regulaMove;
				continue;
			}

#define curData data[i]

			//details
			{
				if (!detailsOpen && curData == detail)
				{
					detailsOpen = true;
					continue;
				}

				if (detailsOpen && curData == detail)
				{
					if (!readDetails)
					{
						detailsOpen = false;
						continue;
					}
				
					size_t detailIndex = moves->size() - 1;

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
							if (Parent->details[detailIndex].note[i] == '\n')
								continue;

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

					int lastIndex = 0;
					for (int indexN = 0; indexN < realNote.size(); indexN++)
					{
						if (realNote[indexN] != ' ')
							lastIndex = indexN;
					}

					Parent->details[detailIndex].note = std::string(realNote.begin(), realNote.begin() + lastIndex + 1);

					detailsOpen = false;
					continue;
				}

				if (detailsOpen)
				{
					if (!readDetails)
						continue;

					Parent->details[moves->size() - 1].note += curData;
					continue;
				}
			}

			//variant
			{
				if (curData == openVariation)
				{
					Parent->move.emplace_back(child, child);					
					Parent = &Parent->children.emplace_back(CldMovesPath(Parent));
					moves = &Parent->move;

					Parent->details.reserve(5);
					Parent->move.reserve(10);
					continue;
				}

				if (curData == closeVariation)
				{
					Parent = Parent->parent;
					moves = &Parent->move;
					continue;
				}
			}

			if(curData < regulaMove)
			{
				//start part of move
				startMovePart = curData;
			}
			else
			{
				//table move
				uint16_t convertedMove = 0;
				
				if (encoding == MoveEncoding::CLD)
					convertedMove = GetMoveByIndex("3STCLDE", curData, moves->size() - Parent->children.size());
				else if (encoding == MoveEncoding::CORE)
					convertedMove = GetMoveByIndex("3STCOREE", curData, moves->size() - Parent->children.size());
				
				moves->emplace_back(convertedMove & 0x00FF, convertedMove >> 8);
			}

#undef curData
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
				uint8_t convertedMove = 0;

				if (m_Encoding == MoveEncoding::CLD)
					convertedMove = GetMoveIntex("3STCLDE", *(uint16_t*)&movePath.move[i], i - index);
				else if (m_Encoding == MoveEncoding::CORE)
					convertedMove = GetMoveIntex("3STCOREE", *(uint16_t*)&movePath.move[i], i - index);

				if (convertedMove == 0)
				{
					data.emplace_back(movePath.move[i].first);
					data.emplace_back(movePath.move[i].second);
				}
				else
					data.emplace_back(convertedMove);

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