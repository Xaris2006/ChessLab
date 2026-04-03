#pragma once

#include <vector>
#include <string>

#include "../ChessAPI/ChessAPI.h"

namespace Panels {

	class NotePanel
	{
	public:
		NotePanel() = default;

		void OnImGuiRender();

		bool& IsPanelOpen();

	private:
		bool m_viewPanel = false;
	};
}
