#pragma once

#include <memory>
#include <string>
#include "ChessCore/FileFormats/Clr/ClrFile.h"
#include "Walnut/Image.h"

namespace Panels {

	class ReferencePanel
	{
	public:
		void OnAttach();
		void OnImGuiRender();

		bool& IsPanelOpen();

	private:
		bool m_viewPanel = true;
		bool m_Running = true;

		std::shared_ptr<Chess::ClrFile> m_ClrFile;
		std::string m_clrFen;

		//str move, white wins, black wins, draws, count of games, average elo, last played
		std::vector<std::tuple<std::string, size_t, size_t, size_t, size_t, size_t, uint16_t>> m_ResultsOrdered;
		std::vector<std::pair<Chess::PgnGame*, size_t>> m_TopGames;

		std::vector<std::string> m_important_prop = {
			"White",    "Black",
			"WhiteElo", "BlackElo",
			"Date", "Result"
		};

		std::shared_ptr<Walnut::Image> m_IconPlay;
		std::shared_ptr<Walnut::Image> m_IconStop;
	};
}