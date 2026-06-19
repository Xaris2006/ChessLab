#pragma once

#include "Panels/ProfilePanel.h"
#include "Panels/ToolsPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/HelpPanel.h"

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

	ProfilePanel& GetProfilePanel();
	ToolsPanel& GetToolsPanel();
	HelpPanel& GetHelpPanel();
	ContentBrowserPanel& GetContentBrowserPanel();
}
