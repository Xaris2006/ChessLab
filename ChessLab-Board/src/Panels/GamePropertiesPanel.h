#pragma once

#include <vector>
#include <string>

#include "../ChessAPI/ChessAPI.h"

namespace Panels {

	class GamePropertiesPanel
	{
	public:
		GamePropertiesPanel() = default;

		void OnImGuiRender();
		void Reset();

		bool& IsPanelOpen();

	private:
		bool m_viewPanel = true;

		bool m_addinglabel = false;
		std::string m_nlabelname = "";
		std::vector<std::string> m_important_prop = {
			"White",    "Black",
			"WhiteElo", "BlackElo",
			"Date", "ECO", "Result",
			"Event", "Round", "Site"
		};
	};
}
