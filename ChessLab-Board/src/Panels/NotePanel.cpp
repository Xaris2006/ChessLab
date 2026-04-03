#include "NotePanel.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

namespace Panels
{
	void NotePanel::OnImGuiRender()
	{
		if (!m_viewPanel)
			return;

		ImGui::Begin("Notes", &m_viewPanel);
		
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.4f, 0.4f, 0.4f, 0.4f));

		ImGui::InputTextMultiline("##note", &ChessAPI::GetActiveGame().GetNote(ChessAPI::GetActiveGame().GetLastMoveKey()).note, ImGui::GetContentRegionAvail());
		ImGui::PopStyleColor();

		ImGui::End();
		
	}

	bool& NotePanel::IsPanelOpen()
	{
		return m_viewPanel;
	}

}
