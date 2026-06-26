#include "PgnGame.h"

#include <array>
#include <fstream>

namespace Chess
{
	bool IsFileValidFormat(std::string filepath, std::string format)
	{
		size_t pointformatstart = filepath.find_last_of('.');
		std::string fs = filepath.substr(pointformatstart, filepath.size() - pointformatstart + 1);
		if (fs == format)
			return true;
		return false;
	}

	PgnGame::~PgnGame()
	{
		if (m_Count)
			__debugbreak();
	}

	const PgnGame::ChessMovesPath PgnGame::GetMovePathbyCopy() const
	{
		return m_chessmoves;
	}

	PgnGame::ChessMovesPath& PgnGame::GetMovePathbyRef()
	{
		return m_chessmoves;
	}

	std::string& PgnGame::GetResault()
	{
		return m_resualt;
	}

	std::vector<std::string> PgnGame::GetLabelNames() const
	{
		std::vector<std::string> output;
		output.reserve(m_labels.size());
		for (auto& [key, value] : m_labels)
			output.emplace_back(key);
		return output;
	}

	void PgnGame::AddReference()
	{
		m_Count++;
	}

	void PgnGame::RemoveReference()
	{
		m_Count--;
		if (m_Count < 0)
		{
			__debugbreak();
			m_Count = 0;
		}
	}

	bool PgnGame::IsFree() const
	{
		return m_Count == 0;
	}

	void PgnGame::Clear()
	{
		//m_Count = 0;
		m_DataRead = "";

		m_labels.clear();
		m_chessmoves.children.clear();
		m_chessmoves.details.clear();
		m_chessmoves.move.clear();
		m_resualt = "*";
	}

	bool PgnGame::IsLabelExist(const std::string& name) const
	{
		return m_labels.contains(name);
	}

	void PgnGame::RemoveLabel(const std::string& name)
	{
		//m_labels[name] = "";
		m_labels.erase(name);
	}

	std::string& PgnGame::operator[](const std::string& label)
	{		
		if (!m_labels.contains(label))
			m_labels[label] = "?";

		return m_labels[label];
	}

	std::string PgnGame::GetData() const
	{
		std::string output = "";
		bool resultExist = false;
		for (auto& [name, value] : m_labels)
		{
			output += "[" + name + " \"" + value + "\"]\n";
			if (name == "Result")
				resultExist = true;
		}
		output += '\n';

		WriteMoves(output, m_chessmoves);
		if (resultExist && m_labels.at("Result") != "?")
			output += m_labels.at("Result");
		else
			output += '*';

		output += "\n\n";

		return output;
	}

	std::string PgnGame::GetDataRead() const
	{
		return m_DataRead;
	}

	void PgnGame::Parse(std::string_view data, bool onlyRead, bool readMoves)
	{
		Clear();

		if (data.empty())
			return;

		bool labelstart = false;
		bool labelvalue = false;

		bool labelArea = true;

		bool dollaropen = false;
		bool detailsopen = false;


		std::string labelnamestr = "";
		std::string labelvaluestr = "";

		ChessMovesPath* Parent = &m_chessmoves;

		if (readMoves)
		{
			Parent->details.reserve(20);
			Parent->move.reserve(80);
			Parent->move.emplace_back("");
		}
		
		for (size_t i = 0; i < data.size(); i++)
		{
			if ((unsigned char)data[i] < 32 && data[i] != '\n')
				continue;

			if (labelArea)
			{
				if (data[i] == '\n')
					continue;

				if (data[i] == '\\')
				{
					i++;
					continue;
				}

				if (!labelvalue)
				{
					if (!labelstart && data[i] != '[')
					{
						i -= 1;
						labelArea = false;
						continue;
					}

					if (data[i] == '[' && !labelstart)
					{
						labelstart = true;
						labelnamestr.clear();
						labelvaluestr.clear();
						continue;
					}

					if (data[i] == ']' && labelstart)
					{
						labelstart = false;
						m_labels[labelnamestr] = labelvaluestr;
						continue;
					}

					if (data[i] == '"')
					{
						labelvalue = true;
						continue;
					}
				}

				if (data[i] == '"' && labelvalue)
				{
					labelvalue = false;
					continue;
				}

				if (labelvalue)
				{
					labelvaluestr += data[i];
					continue;
				}

				if (labelstart)//!labelvalue
				{
					if (data.size() > i + 1 && data[i + 1] == '"')
						continue;
					labelnamestr += data[i];
					continue;
				}
			}

			if (!readMoves)
				break;

			//--moveArea--

			//dollar???
			{
				if (data[i] == '$')
				{
					dollaropen = true;
					continue;
				}

				if (dollaropen)
				{
					if (data[i] < '0' || data[i] > '9')
					{
						dollaropen = false;
						i--;
					}
					continue;
				}
			}

			//notes
			{
				if (data[i] == '{' && !detailsopen)
				{
					detailsopen = true;
					continue;
				}

				if (data[i] == '}')
				{
					if (Parent->move.empty())
						continue;

					int detailIndex = Parent->move.size() - 2;
					
					Parent->details[detailIndex].note += ' ';

					std::string realNote;
					bool lastPer = false;
					bool cmdSection = false, cmdSectionValue = false;

					std::string cmdName, cmdValue;

					for(int i = 0; i < Parent->details[detailIndex].note.size() - 1; i++)
					{
						if(Parent->details[detailIndex].note[i] == '[' && Parent->details[detailIndex].note[i + 1] == '%')
						{
							cmdSection = true;
							i++;
							continue;
						}

						if(cmdSection && !cmdSectionValue && (Parent->details[detailIndex].note[i] == ' ' || Parent->details[detailIndex].note[i] == '\n'))
						{
							cmdSectionValue = true;
							continue;
						}

						if(cmdSectionValue && Parent->details[detailIndex].note[i] == ']')
						{
							cmdSection = false;
							cmdSectionValue = false;
							
							Parent->details[detailIndex].cmds[cmdName] = cmdValue;
							
							cmdName.clear();
							cmdValue.clear();
							continue;
						}

						if(cmdSection)
						{
							if (Parent->details[detailIndex].note[i] == '\n')
								continue;

							if(cmdSectionValue)
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

						if(realNote.empty() && (Parent->details[detailIndex].note[i] == ' ' || Parent->details[detailIndex].note[i] == '\n'))
							continue;

						realNote += Parent->details[detailIndex].note[i];
					}

					Parent->details[detailIndex].note = realNote;

					if (Parent->details[detailIndex].note.empty() && Parent->details[detailIndex].cmds.empty())
						Parent->details.erase(detailIndex);

					detailsopen = false;
					continue;
				}

				if (detailsopen)
				{
					if (Parent->move.empty())
						continue;

					Parent->details[Parent->move.size() - 2].note += data[i];
					continue;
				}
			}

			//variant
			{
				if (data[i] == '(')
				{
					if (Parent->move.back().empty())
						Parent->move.resize(Parent->move.size() - 1);

					Parent->children.emplace_back(ChessMovesPath(Parent));
					Parent->move.emplace_back("child");
					Parent = &Parent->children.back();

					Parent->details.reserve(20);
					Parent->move.reserve(30);
					Parent->move.emplace_back("");
					continue;
				}

				if (data[i] == ')')
				{
					if (Parent->move.back().empty())
						Parent->move.pop_back();

					if (Parent->move.back().find('*') != std::string::npos)
						Parent->move.pop_back();

					if (Parent->move.back().find('-') != std::string::npos)
					{
						size_t zero = Parent->move.back().find('0');
						size_t one = Parent->move.back().find('1');
						size_t slash = Parent->move.back().find('/');
						if ((std::abs((double)one - zero) > 1 && std::abs((double)one - zero) < 10)
							|| (slash != std::string::npos && one != std::string::npos))
						{
							Parent->move.pop_back();
						}
					}

					Parent = Parent->parent;
					Parent->move.emplace_back("");

					continue;
				}
			}

			//moves
			{
				if (data[i] == '\n' && data[i - 1] == ' ' && data[i - 2] == '.')
					continue;

				if ((data[i] == ' ' || data[i] == '\n') && (data[i - 1] != '.' && data[i - 1] != ' ' && data[i - 1] != '\r' && data[i - 1] != '\n'))
				{
					if (!Parent->move.empty() && Parent->move.back().empty())
					{
						continue;
					}

					if (!Parent->move.empty())
					{
						if (Parent->move.back().back() == '.')
							__debugbreak();
					}

					Parent->move.emplace_back("");
					continue;
				}

				if (data[i] == '.' && data[i - 1] == '.')
				{
					Parent->move.back() = "";
					i += 2;
					//1... move
					continue;
				}

				if (data[i] == '\n' || data[i] == ' ')
				{
					if (!Parent->move.back().empty() && Parent->move.back().back() == '.')
						Parent->move.back() += ' ';
					
					continue;
				}

				Parent->move[Parent->move.size() - 1] += data[i];
			}

		}

		if (Parent->move.empty())
			return;

		if (Parent->move.back().empty())
			Parent->move.pop_back();

		if (Parent->move.empty())
			return;

		if (Parent->move.back().find('*') != std::string::npos)
			Parent->move.pop_back();

		if (Parent->move.empty())
			return;

		if (Parent->move.back().find('-') != std::string::npos)
		{
			size_t zero = Parent->move.back().find('0');
			size_t one = Parent->move.back().find('1');
			size_t slash = Parent->move.back().find('/');
			if ((std::abs((double)one - zero) > 1 && std::abs((double)one - zero) < 10)
				|| (slash != std::string::npos && one != std::string::npos))
			{
				m_resualt = Parent->move.back();
				Parent->move.pop_back();
			}
		}

		func_delete_BC_stuff(&m_chessmoves);

		if (m_labels["Result"] == "?")
			m_resualt = '*';
		else
			m_resualt = m_labels["Result"];

		if(!onlyRead)
			m_DataRead = GetData();

		//m_DataRead = data;

		m_chessmoves.ReloadChildren();
	}

	void PgnGame::WriteMoves(std::string& op, ChessMovesPath par) const
	{
		int index = 0;

		if (par.details.contains(-1) && (par.details[-1].note != "" || !par.details[-1].cmds.empty()))
		{
			op += "{ ";

			if (!par.details[-1].note.empty())
				op += (par.details[-1].note + " ");

			for (auto& [cmdname, cmdvalue] : par.details[-1].cmds)
			{
				op += ("[%" + cmdname + " " + cmdvalue + "] ");
			}

			op += "} ";
		}

		for (int i = 0; i < par.move.size(); i++)
		{
			if (par.move[i] == "child") { op += "( "; WriteMoves(op, par.children[index]); op += ") "; index += 1; continue; }
			else
			{
				op += par.move[i] + " ";

				if (par.details.contains(i) && (par.details[i].note != "" || !par.details[i].cmds.empty()))
				{
					op += "{ ";
					
					if (!par.details[i].note.empty())
						op += (par.details[i].note + " ");

					for(auto& [cmdname, cmdvalue] : par.details[i].cmds)
					{
						op += ("[%" + cmdname + " " + cmdvalue + "] ");
					}

					op += "} ";
				}
			}
		}
	}

	bool PgnGame::func_delete_BC_stuff(ChessMovesPath* cur_Parent)
	{
		const std::array<std::string, 1> CB_weird_moves = { "Z0" };
		int index_of_children = 0;
		for (int i = 0; i < cur_Parent->move.size(); i++)
		{
			if (cur_Parent->move[i].find("...") != std::string::npos)
			{
				cur_Parent->move[i] = cur_Parent->move[i].substr(cur_Parent->move[i].find("...") + 4);
			}
			else if (cur_Parent->move[i] == "child")
			{
				if (!func_delete_BC_stuff(&cur_Parent->children[index_of_children]))
				{
					cur_Parent->children.erase(cur_Parent->children.begin() + index_of_children);
					cur_Parent->move.erase(cur_Parent->move.begin() + i);
					i -= 1;
				}
				else
					index_of_children += 1;
			}

			for (int j = 0; j < CB_weird_moves.size(); j++)
			{
				if (cur_Parent->move[i].find(CB_weird_moves[j]) != std::string::npos)
				{
					cur_Parent->move.resize(i);
					cur_Parent->children.resize(index_of_children);
					if (i == 0)
						return false;
					return true;
				}
			}
		}
		return true;
	}

	void PgnGame::SetCurrentAsInitial()
	{
		m_DataRead = GetData();
	}
}