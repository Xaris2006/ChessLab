#pragma once

#include <vector>
#include <string>
#include <thread>
#include <atomic>

#include "ChessCore/Opening_Book/OpeningBook.h"


namespace Panels {

	class OpeningBookPanel
	{
	public:
		void OnDetach();

		void OnImGuiRender();
		
		void OpenCOBfile(const std::string& filepath);
		bool CloseCOBfile();
		std::vector<Chess::OpeningBook::MoveOB>& GetOpeningBookMoves(const Chess::OpeningBook::PositionID& posID);
		
		bool& IsPanelOpen();

	private:
		bool m_viewPanel = true;

		Chess::OpeningBook* m_OpeningBook = nullptr;
		Chess::OpeningBook::PositionID m_CurPosition;
		std::vector<Chess::OpeningBook::MoveOB> m_Moves;

		std::string m_cobPath = "";
		std::string m_cobFilename = "";

		std::thread* m_PlayThread = nullptr;
		std::atomic<bool> m_EndPlayThreadJob = true;
	};
}
