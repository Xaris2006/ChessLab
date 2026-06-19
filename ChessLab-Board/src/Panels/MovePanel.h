#pragma once

#include <vector>

#include "../ChessAPI/ChessAPI.h"

namespace Panels {

	class MovePanel
	{
	public:
		void OnImGuiRender();

		bool& IsPanelOpen();

	private:
		void WriteMove(const Chess::PgnGame::ChessMovesPath& par, Chess::GameManager::MoveKey& pathmove, Chess::GameManager::MoveKey prev_pathmove, float extrain = 0);


	private:
		bool m_viewPanel = true;

		Chess::PgnGame::ChessMovesPath m_moves;
		std::vector<int> m_HoveredMove;
	};
}
