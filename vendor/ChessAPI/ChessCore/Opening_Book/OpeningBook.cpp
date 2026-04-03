#include "OpeningBook.h"

#include "../FileFormats/pgn/PgnFile.h"
#include "../GameManager.h"

#include <fstream>

namespace Chess
{
	void OpeningBook::CreateCOBByPGN(const std::string& pgnfilepath, int& status)
	{
		if (!IsFileValidFormat(pgnfilepath, ".pgn")) { return; }
		
		PgnFile pgnfile;
		pgnfile.OpenFile(pgnfilepath);
				
		if (!pgnfile.GetSize()) { return; }

		status = 0;

		std::string format_version = "COB_FORMAT_VERSION_0.1";


		std::map<PositionID, std::vector<MoveOB>> PositionMoveMap;

		PositionID pos;
		int moveIndex = -1;
		int strindex;

		std::vector<uint8_t> buffer;
		buffer.reserve(PositionMoveMap.size() * 40);
		uint8_t* intptr = nullptr;

		for (int x = 0; x < format_version.size(); x++)
			buffer.push_back(format_version[x]);

		GameManager game;
		game.InitPgnGame(pgnfile[0]);

		for (int i = 0; i < pgnfile.GetSize(); i++)
		{
			float s = (float)i / (float)pgnfile.GetSize();
			status = s * 100.0f - 1;
			if (status < 1)
				status = 1;

			auto& pgnGame = pgnfile[i];

			if (pgnGame.GetResault() == "*")
				continue;

			if (pgnGame["FEN"] != "?" && pgnGame["FEN"] != "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
				continue;

			game.InitPgnGame(pgnGame);
			std::string move = "";
			auto& movestr = game.GetPgnGame().GetMovePathbyRef();

			const int maxMoves = 44;

			for (int j = 0; j < movestr.move.size() && j < maxMoves; j++)
			{
				game.GoNextMove();
			}
			for (int j = movestr.move.size() - 1 < maxMoves -1 ? movestr.move.size() - 1 : maxMoves-1; j >= 0; j--)
			{
				if (movestr.move[j] == "child")
					continue;
				pos = game.GetFormatedFEN();
				if (move != "")
				{
					auto& mapelement = PositionMoveMap[pos];
					for (int x = 0; x < mapelement.size(); x++)
					{
						if (mapelement[x].strmove == move)
						{
							moveIndex = x;
							break;
						}
					}
					if (moveIndex == -1)
					{ 
						mapelement.push_back({ move });
						moveIndex = mapelement.size() - 1;
					}

					auto& cur_move = mapelement[moveIndex];
					cur_move.played += 1;
					if (pgnGame.GetResault() == "1-0")
						cur_move.whitewins += 1;
					else if (pgnGame.GetResault() == "0-1")
						cur_move.blackwins += 1;
					moveIndex = -1;
				}
				strindex = movestr.move[j].find(' ');
				move = movestr.move[j].substr(strindex + 1, movestr.move[j].size() - strindex + 1);
				game.GoPreviusMove();
			}
			pos = game.GetFormatedFEN();
			if (move != "")
			{
				auto& mapelement = PositionMoveMap[pos];
				for (int x = 0; x < mapelement.size(); x++)
				{
					if (mapelement[x].strmove == move)
					{
						moveIndex = x;
						break;
					}
				}
				if (moveIndex == -1) { mapelement.push_back({ move }); moveIndex = mapelement.size() - 1; }

				auto& cur_move = mapelement[moveIndex];
				cur_move.played += 1;
				if (pgnGame.GetResault() == "1-0")
					cur_move.whitewins += 1;
				else if (pgnGame.GetResault() == "0-1")
					cur_move.blackwins += 1;
				moveIndex = -1;
			}
		}

		size_t pointformatstart = pgnfilepath.find_last_of('.');
		std::string name = pgnfilepath.substr(0, pointformatstart);
		name += ".cob";
		
		for (std::map<PositionID, std::vector<MoveOB>>::iterator it = PositionMoveMap.begin(); it != PositionMoveMap.end(); ++it)
		{
			for (int j = 0; j < it->first.size(); j++)
			{
				buffer.emplace_back(it->first[j]);
			}
			for (int j = 0; j < it->second.size(); j++)
			{
				auto& cur_move = it->second[j];
				for (int x = 0; x < cur_move.strmove.size(); x++)
					buffer.emplace_back(cur_move.strmove[x]);
				buffer.emplace_back(0b00000000);

				intptr = (uint8_t*)(void*)(&cur_move.whitewins);
				buffer.emplace_back(intptr[0]);
				buffer.emplace_back(intptr[1]);
				buffer.emplace_back(intptr[2]);
				buffer.emplace_back(intptr[3]);

				intptr = (uint8_t*)(void*)(&cur_move.blackwins);
				buffer.emplace_back(intptr[0]);
				buffer.emplace_back(intptr[1]);
				buffer.emplace_back(intptr[2]);
				buffer.emplace_back(intptr[3]);

				intptr = (uint8_t*)(void*)(&cur_move.played);
				buffer.emplace_back(intptr[0]);
				buffer.emplace_back(intptr[1]);
				buffer.emplace_back(intptr[2]);
				buffer.emplace_back(intptr[3]);
			}
			buffer.emplace_back(0b00000000);
		}
		
		std::ofstream outfile(name, std::ios_base::binary | std::ios_base::out);
		outfile.write((const char*)buffer.data(), buffer.size());
		outfile.close();

		status = 100;
	}

	OpeningBook::OpeningBook(const std::string& cobfilepath)
	{
		if (!IsFileValidFormat(cobfilepath, ".cob")) { return; }
		
		std::streampos maxSize;

		std::ifstream infile(cobfilepath, std::ios_base::binary | std::ios_base::in | std::ios_base::ate);
		if (infile.is_open())
		{
			maxSize = infile.tellg();
			m_data.resize(maxSize);
			m_size = maxSize;
			infile.seekg(0, std::ios_base::beg);
			infile.read((char*)m_data.data(), maxSize);
			infile.close();
		}

		for (int i = 0; i < 22 && i < maxSize; i++)
		{
			m_version += (char)m_data[i];
		}
		
	}
	OpeningBook::OpeningBook()
	{

	}

	std::vector<OpeningBook::MoveOB>& OpeningBook::GetMovesByPosition(const PositionID& posID, const SortingType& stype)
	{
		if (posID == m_lasePosID && m_lastSortingType == stype)
			return m_SortedlastMoveOB;
		m_lasePosID = posID;
		m_lastSortingType = stype;

		m_lastMoveOB.clear();
		m_SortedlastMoveOB.clear();

		if (posID.empty())
			return m_SortedlastMoveOB;

		bool finded = false;

		for (int i = 22; i < m_size; i++)
		{
			if (m_data[i] == posID[0])
			{
				for (int j = 1; j < posID.size(); j++)
				{
					if (m_data[i + j] == posID[j])
						finded = true;
					else
					{
						finded = false;
						break;
					}
				}
				i += posID.size();
				if (finded)
				{
					std::string move = "";
					while (true)
					{
						if (m_data[i] == 0b00000000)
							break;

						while (m_data[i] != 0b00000000)
						{
							move += (char)m_data[i];
							i += 1;
						}
						i += 1;

						m_lastMoveOB.push_back({ move });

						auto& curMove = m_lastMoveOB[m_lastMoveOB.size() - 1];
						curMove.strmove = move;

						int* intptr = (int*)(void*)&m_data[i];
						curMove.whitewins = *intptr;
						i += 4;

						intptr = (int*)(void*)&m_data[i];
						curMove.blackwins = *intptr;
						i += 4;

						intptr = (int*)(void*)&m_data[i];
						curMove.played = *intptr;
						i += 4;

						move = "";
					}
				}					
			}
		}

		switch (stype)
		{
		case MOSTPLAYED:
		{
			bool smaller = true;

			for (int i = 0; i < m_lastMoveOB.size(); i++)
			{
				for (int j = 0; j < m_SortedlastMoveOB.size(); j++)
				{
					if (m_lastMoveOB[i].played >= m_SortedlastMoveOB[j].played) { m_SortedlastMoveOB.insert(m_SortedlastMoveOB.begin() + j, m_lastMoveOB[i]); smaller = false; break; }
				}
				if (smaller)
					m_SortedlastMoveOB.emplace_back(m_lastMoveOB[i]);
				smaller = true;
			}
			break;
		}
		case BESTWHITE:
		{
			bool smaller = true;

			for (int i = 0; i < m_lastMoveOB.size(); i++)
			{
				for (int j = 0; j < m_SortedlastMoveOB.size(); j++)
				{
					if ((float)m_lastMoveOB[i].whitewins / (float)m_lastMoveOB[i].played >= (float)m_SortedlastMoveOB[j].whitewins / (float)m_SortedlastMoveOB[j].played) { m_SortedlastMoveOB.insert(m_SortedlastMoveOB.begin() + j, m_lastMoveOB[i]); smaller = false; break; }
				}
				if (smaller)
					m_SortedlastMoveOB.emplace_back(m_lastMoveOB[i]);
				smaller = true;
			}
			break;
		}
		case BESTBLACK:
		{
			bool smaller = true;

			for (int i = 0; i < m_lastMoveOB.size(); i++)
			{
				for (int j = 0; j < m_SortedlastMoveOB.size(); j++)
				{
					if ((float)m_lastMoveOB[i].blackwins / (float)m_lastMoveOB[i].played >= (float)m_SortedlastMoveOB[j].blackwins / (float)m_SortedlastMoveOB[j].played) { m_SortedlastMoveOB.insert(m_SortedlastMoveOB.begin() + j, m_lastMoveOB[i]); smaller = false; break; }
				}
				if (smaller)
					m_SortedlastMoveOB.emplace_back(m_lastMoveOB[i]);
				smaller = true;
			}
			break;
		}
		case BESTDRAW:
		{
			bool smaller = true;

			for (int i = 0; i < m_lastMoveOB.size(); i++)
			{
				for (int j = 0; j < m_SortedlastMoveOB.size(); j++)
				{
					if ((float)(m_lastMoveOB[i].played - m_lastMoveOB[i].whitewins - m_lastMoveOB[i].blackwins)/ (float)m_lastMoveOB[i].played >= (float)(m_SortedlastMoveOB[j].played - m_SortedlastMoveOB[j].whitewins - m_SortedlastMoveOB[j].blackwins)/ (float)m_SortedlastMoveOB[j].played) { m_SortedlastMoveOB.insert(m_SortedlastMoveOB.begin() + j, m_lastMoveOB[i]); smaller = false; break; }
				}
				if (smaller)
					m_SortedlastMoveOB.emplace_back(m_lastMoveOB[i]);
				smaller = true;
			}
			break;
		}
		case RAW:
		{
			m_SortedlastMoveOB = m_lastMoveOB;
			break;
		}
		}

		

		return m_SortedlastMoveOB;
	}

}
