#pragma once

#include "Panels/DatabasePanel.h"
#include "Panels/GamePropertiesPanel.h"
#include "Panels/EnginePanel.h"
#include "Panels/MovePanel.h"
#include "Panels/NotePanel.h"
#include "Panels/BoardPanel.h"
#include "Panels/ReferencePanel.h"

namespace Panels
{
	void Init();
	void OnAttach();
	void OnDetach();
	void OnImGuiRender();
	void OnKeyEvent();

	void OpenAlreadyOpenedPopup();
	bool IsAlreadyOpenedPopupOpen();

	void OpenLoadingPopup();
	bool IsLoadingPopupOpen();
	float GetLoadingPersentage();
	float& GetLoadingPersentageRef();
	void SetLoadingPersentage(float persentage);

	void OpenAboutPopup();
	bool IsAboutPopupOpen();

	BoardPanel& GetBoardPanel();
	DatabasePanel& GetDatabasePanel();
	GamePropertiesPanel& GetGamePropertiesPanel();
	EnginePanel& GetEnginePanel();
	MovePanel& GetMovePanel();
	NotePanel& GetNotePanel();
	ReferencePanel& GetReferencePanel();
}