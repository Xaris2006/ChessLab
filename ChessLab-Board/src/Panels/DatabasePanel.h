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
		//std::string m_elo_to_searchW;
		std::string m_title_to_searchW;
		std::string m_fideId_to_searchW;
		int m_eloMax_to_searchW = 4000;
		int m_eloMin_to_searchW = 1000;

		std::string m_name_to_searchB;
		std::string m_title_to_searchB;
		std::string m_fideId_to_searchB;
		int m_eloMax_to_searchB = 4000;
		int m_eloMin_to_searchB = 1000;
		
		bool m_WhiteWin_Result = false;
		bool m_BlackWin_Result = false;
		bool m_Draw_Result = false;
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
		std::shared_ptr<Walnut::Image> m_IconDelete = std::make_shared<Walnut::Image>("Resources/Icons/bin.png");
		std::shared_ptr<Walnut::Image> m_IconDeleteAll = std::make_shared<Walnut::Image>("Resources/Icons/delete.png");
		std::shared_ptr<Walnut::Image> m_IconRestore = std::make_shared<Walnut::Image>("Resources/Icons/restore.png");
		std::shared_ptr<Walnut::Image> m_IconRestoreAll = std::make_shared<Walnut::Image>("Resources/Icons/refresh.png");

		std::shared_ptr<Walnut::Image> m_IconAdd = std::make_shared<Walnut::Image>("Resources/Icons/plus.png");
		std::shared_ptr<Walnut::Image> m_IconSave = std::make_shared<Walnut::Image>("Resources/Icons/save.png");
		std::shared_ptr<Walnut::Image> m_IconSaveAs = std::make_shared<Walnut::Image>("Resources/Icons/save-as.png");
		std::shared_ptr<Walnut::Image> m_IconCopyGame = std::make_shared<Walnut::Image>("Resources/Icons/copy.png");
		std::shared_ptr<Walnut::Image> m_IconPasteGame = std::make_shared<Walnut::Image>("Resources/Icons/paste.png");
		std::shared_ptr<Walnut::Image> m_IconEditor = std::make_shared<Walnut::Image>("Resources/Icons/editor.png");
	};

	

}
