#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace Chess
{
	bool IsFileValidFormat(std::string filepath, std::string format);

	class PgnGame
	{
	public:
		struct Detail
		{
			std::unordered_map<std::string, std::string> cmds;
			std::string note;
		};

		struct ChessMovesPath
		{
			ChessMovesPath() = default;
			ChessMovesPath(ChessMovesPath* p) : parent(p) {};

			void ReloadChildren()
			{
				for (int i = 0; i < children.size(); i++)
				{
					children[i].parent = this;
					children[i].ReloadChildren();
				}
			}

			ChessMovesPath* parent = nullptr;
			std::vector<ChessMovesPath> children;
			std::unordered_map<size_t, Detail> details;
			std::vector<std::string> move;
		};

	public:
		PgnGame() = default;
		~PgnGame();

		const ChessMovesPath GetMovePathbyCopy() const;
		ChessMovesPath& GetMovePathbyRef();
		std::string& GetResault();
		std::vector<std::string> GetLabelNames() const;

		bool IsLabelExist(const std::string& name) const;
		void RemoveLabel(const std::string& name);

		void AddReference();
		void RemoveReference();

		bool IsFree() const;

		void Clear();
		std::string& operator[](const std::string& label);

		std::string GetData() const;

		std::string GetDataRead() const;

		void SetCurrentAsInitial();
		void SetDataRead(std::string_view data);

		void Parse(std::string_view data, bool onlyRead = false, bool readMoves = true);

	private:

		void WriteMoves(std::string& op, ChessMovesPath par) const;

		static bool func_delete_BC_stuff(ChessMovesPath* cur_Parent);

	private:

		std::unordered_map<std::string, std::string> m_labels;
		ChessMovesPath m_chessmoves;
		std::string m_resualt = "*";
		int m_Count = 0;

		std::string m_DataRead;
	};



}
