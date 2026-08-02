#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <span>

#include "../base/Encoding.h"

namespace Chess
{
	class CldGame
	{
	public:
		struct Detail
		{
			std::unordered_map<std::string, std::string> cmds;
			std::string note;
		};

		struct CldMovesPath
		{
			CldMovesPath() = default;
			CldMovesPath(CldMovesPath* p) : parent(p) {};

			void ReloadChildren()
			{
				for (int i = 0; i < children.size(); i++)
				{
					children[i].parent = this;
					children[i].ReloadChildren();
				}
			}

			CldMovesPath* parent = nullptr;
			std::vector<CldMovesPath> children;
			std::unordered_map<int, Detail> details;
			std::vector<std::pair<uint8_t, uint8_t>> move;
		};

	public:
		CldGame() = default;
		~CldGame();

		void SetEncoding(MoveEncoding encoding) { m_Encoding = encoding; }
		MoveEncoding GetEncoding() const { return m_Encoding; }
		void ConvertEncoding(MoveEncoding encoding, std::string startFen);

		const CldMovesPath GetMovePathbyCopy() const;
		CldMovesPath& GetMovePathbyRef();

		std::vector<size_t> GetLabelNames() const;
		bool IsLabelExist(size_t name) const;

		void RemoveLabel(size_t name);
		
		size_t& operator[](size_t label);
		size_t At(size_t label) const;

		void Clear();

		void GetData(std::vector<uint8_t>& data, uint8_t nameType = 1, uint8_t valueType = 4, bool useTable = true) const;
		void GetDataRead(std::vector<uint8_t>& data) const;

		void Parse(std::span<uint8_t> data, MoveEncoding encoding = MoveEncoding::CLD, uint8_t nameType = 1, uint8_t valueType = 4, bool onlyRead = false, bool readLabels = true, bool readMoves = true, bool readDetails = true);

	private:
		void WriteMoves(std::vector<uint8_t>& data, CldMovesPath movePath, bool useTable = true) const;

		static void ConvertCldMovePathCoreEToCldMovePathCldE(CldGame::CldMovesPath& cldMovePathCldE, const CldGame::CldMovesPath& cldMovePathCoreE, bool reset = true, std::string startFen = "");
		static void ConvertCldMovePathCldEToCldMovePathCoreE(CldGame::CldMovesPath& cldMovePathCoreE, const CldGame::CldMovesPath& cldMovePathCldE, bool reset = true, std::string startFen = "");

	private:
		std::vector<std::pair<size_t, size_t>> m_Labels;
		CldMovesPath m_MovesPath;
		MoveEncoding m_Encoding = MoveEncoding::CLD;

		std::vector<uint8_t> m_DataRead;
	};
}