#pragma once

#include "Web.h"

#include <memory>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <thread>

#include "Walnut/Image.h"

namespace Panels
{
	class ToolsPanel
	{
	public:
		void OnAttach();

		void OnImGuiRender();

	private:
		struct DownloadableTool
		{
			std::string name;
			std::string id;
			std::string at;

			std::string iconId;
			std::string iconAt;

			std::string detailsId;
			std::string detailsAt;

			Web::DownLoadStatus status = Web::Nothing;
			std::thread* thread = nullptr;
		};

	private:
		void FindAvailableTools();
		void FindDownloadableTools(); 

	private:

		std::vector<std::filesystem::path> m_AvailableTools;
		std::vector<DownloadableTool> m_DownloadableTools;

		int m_TargetedToolIndex = -1;

		std::unordered_map<std::string, std::string> m_ToolLabelNameToValue;

		std::vector<std::shared_ptr<Walnut::Image>> m_ToolIcons;
		
		std::vector<bool> m_DownloadableToolExists;
		std::vector<std::filesystem::path> m_DownloadableToolIconsToLoad;
		std::vector<std::shared_ptr<Walnut::Image>> m_DownloadableToolIcons;
		std::unordered_map<std::string, std::string> m_DownloadableToolLabelNameToValue;

		std::shared_ptr<Walnut::Image> m_RefreshIcon;
	};

}