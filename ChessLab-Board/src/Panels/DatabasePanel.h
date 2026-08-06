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
		void OnAttach();
		void OnImGuiRender();

		void Reset();
		void OpenInfoPopup() { m_OpenInfoPopup = true; }
		bool IsInfoPopupOpen() const { return m_OpenInfoPopup; }

	private:
		void GamePopup();

		void DrawTools(int tableIndex = -1);
		void InfoPopup();

	private:
		std::filesystem::path m_filePath;

		std::unordered_set<size_t> m_pressedIndexes;

		bool m_AdvancedOptions = false;

		bool m_name_white = true;
		bool m_name_black = true;
		std::string m_name_to_search;
		std::string m_eco_to_search;
		std::string m_date_to_search;

		std::string m_name_to_searchW;
		std::string m_title_to_searchW;
		std::string m_fideId_to_searchW;
		bool m_elo_to_searchW = false;
		int m_eloMax_to_searchW = 4000;
		int m_eloMin_to_searchW = 1000;

		std::string m_name_to_searchB;
		std::string m_title_to_searchB;
		std::string m_fideId_to_searchB;
		bool m_elo_to_searchB = false;
		int m_eloMax_to_searchB = 4000;
		int m_eloMin_to_searchB = 1000;
		
		std::string m_fen_to_search;
		bool m_WhiteWin_Result = false;
		bool m_BlackWin_Result = false;
		bool m_Draw_Result = false;
		std::string m_result_to_search;
		std::string m_event_to_search;
		std::string m_round_to_search;
		std::string m_site_to_search;
		std::string m_source_to_search;

		std::vector<std::string> m_ecoItems;

		std::vector<std::string> m_important_prop = {
			"White", "WhiteElo", 
			"Black", "BlackElo",
			"Date", "ECO", "Result",
			"Event", "Round", "Site"
		};

		bool m_GoToLinePressed = false;
		size_t m_GoToLine = 1;

		std::vector<std::shared_ptr<std::pair<Chess::SearchOptions, Chess::SearchResult>>> m_searchTables;

		std::shared_ptr<Walnut::Image> m_IconPlay;
		std::shared_ptr<Walnut::Image> m_IconStop;
		std::shared_ptr<Walnut::Image> m_IconDelete;
		std::shared_ptr<Walnut::Image> m_IconDeleteAll;
		std::shared_ptr<Walnut::Image> m_IconRestore;
		std::shared_ptr<Walnut::Image> m_IconRestoreAll;
		std::shared_ptr<Walnut::Image> m_IconAdd;
		std::shared_ptr<Walnut::Image> m_IconSave;
		std::shared_ptr<Walnut::Image> m_IconSaveAs;
		std::shared_ptr<Walnut::Image> m_IconCopyGame;
		std::shared_ptr<Walnut::Image> m_IconPasteGame;
		std::shared_ptr<Walnut::Image> m_IconEditor;
		std::shared_ptr<Walnut::Image> m_IconSearch;
		std::shared_ptr<Walnut::Image> m_IconInfo;

		bool m_OpenInfoPopup = false;
	};
}
