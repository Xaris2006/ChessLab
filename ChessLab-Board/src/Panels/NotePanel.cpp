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
		
		ChessAPI::SetNotePanelPointed(ImGui::IsWindowFocused());

		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.4f, 0.4f, 0.4f, 0.4f));

		ImGui::InputTextMultiline("##note", &ChessAPI::GetNote(ChessAPI::GetMoveIntFormat()).note, ImGui::GetContentRegionAvail());
		ImGui::PopStyleColor();

		ImGui::End();
		
	}

	bool& NotePanel::IsPanelOpen()
	{
		return m_viewPanel;
	}

}
