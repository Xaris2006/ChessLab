#include "MoveTables.h"

#include <unordered_map>
#include <fstream>

static std::unordered_map<uint16_t, uint8_t> s_3STCLDE_E;
static std::unordered_map<uint16_t, uint8_t> s_3STCLDE_M;
static std::unordered_map<uint16_t, uint8_t> s_3STCLDE_L;
static std::vector<uint16_t> s_3STCLDE_I;

static std::unordered_map<uint16_t, uint8_t> s_3STCOREE_E;
static std::unordered_map<uint16_t, uint8_t> s_3STCOREE_M;
static std::unordered_map<uint16_t, uint8_t> s_3STCOREE_L;
static std::vector<uint16_t> s_3STCOREE_I;

namespace Chess
{
	void InitializeMoveTables()
	{
		{
			std::ifstream infileTable("table.clt", std::ios::binary);

			std::vector<uint8_t> data;

			infileTable.seekg(0, std::ios_base::end);
			std::streampos maxIndex = infileTable.tellg();
			infileTable.seekg(0, std::ios::beg);

			data.resize(maxIndex);
			infileTable.read(reinterpret_cast<char*>(data.data()), data.size());

			infileTable.close();

			s_3STCLDE_I.resize(189 * 3);

			for (int i = 0; i < data.size(); i += 2)
			{
				uint16_t move = (((uint16_t)data[i] << 8) | data[i + 1]);

				if (i < 189 * 2)
					s_3STCLDE_E[move] = (i / 2) % 189;
				else if (i < 189 * 4)
					s_3STCLDE_M[move] = (i / 2) % 189;
				else
					s_3STCLDE_L[move] = (i / 2) % 189;

				s_3STCLDE_I[i / 2] = move;
			}
		}

		{
			std::ifstream infileTable("tableCore.clt", std::ios::binary);

			std::vector<uint8_t> data;

			infileTable.seekg(0, std::ios_base::end);
			std::streampos maxIndex = infileTable.tellg();
			infileTable.seekg(0, std::ios::beg);

			data.resize(maxIndex);
			infileTable.read(reinterpret_cast<char*>(data.data()), data.size());

			infileTable.close();

			s_3STCOREE_I.resize(189 * 3);

			for (int i = 0; i < data.size(); i += 2)
			{
				uint16_t move = (((uint16_t)data[i] << 8) | data[i + 1]);

				if (i < 189 * 2)
					s_3STCOREE_E[move] = (i / 2) % 189;
				else if (i < 189 * 4)
					s_3STCOREE_M[move] = (i / 2) % 189;
				else
					s_3STCOREE_L[move] = (i / 2) % 189;

				s_3STCOREE_I[i / 2] = move;
			}
		}
	}

	void GetMoveTablesNames(std::vector<std::string>& names)
	{
		names = { "3STCLDE", "3STCOREE" };
	}

	uint8_t GetMoveIntex(std::string_view name, uint16_t move, size_t count)
	{
		if (name == "3STCLDE")
		{
			if (count < 45)
			{
				if (s_3STCLDE_E.contains(move))
					return s_3STCLDE_E[move] + 64;
				else
					return 0;
			}
			else if (count < 70)
			{
				if (s_3STCLDE_M.contains(move))
					return s_3STCLDE_M[move] + 64;
				else
					return 0;
			}
			else if (s_3STCLDE_L.contains(move))
				return s_3STCLDE_L[move] + 64;
			else
				return 0;
		}
		else if (name == "3STCOREE")
		{
			if (count < 45)
			{
				if (s_3STCOREE_E.contains(move))
					return s_3STCOREE_E[move] + 64;
				else
					return 0;
			}
			else if (count < 70)
			{
				if (s_3STCOREE_M.contains(move))
					return s_3STCOREE_M[move] + 64;
				else
					return 0;
			}
			else if (s_3STCOREE_L.contains(move))
				return s_3STCOREE_L[move] + 64;
			else
				return 0;
		}

		return 0;
	}

	uint16_t GetMoveByIndex(std::string_view name, uint8_t index, size_t count)
	{
		if (name == "3STCLDE")
		{
			if (index < 64)
				return 0;
			
			size_t indexOfMove = index - 64;

			if (count >= 70)
				indexOfMove += (189 * 2);
			else if (count >= 45)
				indexOfMove += 189;

			return s_3STCLDE_I.at(indexOfMove);			
		}
		else if (name == "3STCOREE")
		{
			if (index < 64)
				return 0;

			size_t indexOfMove = index - 64;

			if (count >= 70)
				indexOfMove += (189 * 2);
			else if (count >= 45)
				indexOfMove += 189;

			return s_3STCOREE_I.at(indexOfMove);
		}

		return 0;
	}
}