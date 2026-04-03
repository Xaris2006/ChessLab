#pragma once

#include <array>
#include <vector>
#include <map>
#include <string>

#include "../ChessAPI/ChessAPI.h"
#include "ChessCore/FileFormats/SearchWork.h"

#include "Walnut/Image.h"

namespace Panels {

	class DatabasePanel
	{
	public:
		DatabasePanel();
		~DatabasePanel() = default;

		void Reset();

		void OnImGuiRender();

	private:
		std::filesystem::path m_filePath;

		//int m_activeTable = 0;

		bool m_AdvancedOptions = false;

		bool m_name_white = true;
		bool m_name_black = true;
		std::string m_name_to_search;
		std::string m_eco_to_search;
		std::string m_date_to_search;

		std::string m_name_to_searchW;
		std::string m_elo_to_searchW;
		std::string m_title_to_searchW;
		std::string m_fideId_to_searchW;

		std::string m_name_to_searchB;
		std::string m_elo_to_searchB;
		std::string m_title_to_searchB;
		std::string m_fideId_to_searchB;
		
		std::string m_result_to_search;
		std::string m_event_to_search;
		std::string m_round_to_search;
		std::string m_site_to_search;
		std::string m_source_to_search;

		std::vector<std::string> m_ecoItems;

		std::vector<bool*> m_IsOpened;

		std::vector<std::string> m_important_prop = {
			"White",    "Black",
			"WhiteElo", "BlackElo",
			"Date", "ECO", "Result",
			"Event", "Round", "Site"
		};

		std::vector<std::shared_ptr<std::pair<Chess::SearchOptions, Chess::SearchResult>>> m_searchTables;

		std::shared_ptr<Walnut::Image> m_IconPlay = std::make_shared<Walnut::Image>("Resources/Icons/PlayButton.png");
		std::shared_ptr<Walnut::Image> m_IconStop = std::make_shared<Walnut::Image>("Resources/Icons/StopButton.png");
	};

	

}
