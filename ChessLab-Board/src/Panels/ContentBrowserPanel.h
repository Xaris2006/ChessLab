#pragma once

#include "../ChessAPI/ChessAPI.h"

#include "Walnut/Image.h"

#include <filesystem>
#include <memory>

namespace Panels
{
	class ContentBrowserPanel
	{
	public:
		void OnAttach();

		void OnImGuiRender();

		bool& IsPanelOpen();
	private:
		bool m_viewPanel = true;

		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;

		std::shared_ptr<Walnut::Image> m_DirectoryIcon;
		std::shared_ptr<Walnut::Image> m_FileIcon;
		std::shared_ptr<Walnut::Image> m_FileIconPGN;
		std::shared_ptr<Walnut::Image> m_FileIconCOB;
	};

}
