#pragma once

namespace Panels {

	class NotePanel
	{
	public:
		void OnImGuiRender();

		bool& IsPanelOpen();

	private:
		bool m_viewPanel = false;
	};
}
