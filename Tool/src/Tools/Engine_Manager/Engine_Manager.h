#pragma once

#include "Walnut/Application.h"

#include "Web.h"

namespace Tools::EngineManager
{
	class Layer : public Walnut::Layer
	{
	public:
		virtual void OnAttach() override;

		virtual void OnDetach() override;

		virtual void OnUIRender() override;

		void ShowAboutModal();

		void AddEngine();

	private:
		struct DownloadableEngine
		{
			std::string name;
			//std::string id;
			//std::string at;

			std::string url;

			Web::DownLoadStatus status = Web::Nothing;
			std::thread* thread = nullptr;
		};

	private:
		void FindAvailEngines();
		void FindAvailOnWebEngines();

		void GetEngineSettings();
		void SetEngineSettings() const;
		
		void UI_DrawAboutModal();
		void FilePopup();
		void RenamePopup();
		void EmptyPopup();
		void ErrorOnDeletingPopup();

	private:
		bool m_AboutModalOpen = false;

		std::vector<std::filesystem::path> m_AvailableEngines;
		std::vector<DownloadableEngine> m_DownloadableEngines;

		int m_TargetedEngineIndex = -1;

		std::shared_ptr<Walnut::Image> m_EngineIcon;
		std::shared_ptr<Walnut::Image> m_RefreshIcon;

		int m_threadCount = 2;
		int m_hashMb = 256;
		int m_lines = 1; //1->5
		int m_SkillLevel = 20; //0->20

		std::string m_SyzygyPath = "";
		bool m_Syzygy50MoveRule = true;

		bool m_LimitStrength = false;
		int m_Elo = 1400;

		std::string m_DefaultEngine = "";

	};

	Walnut::Application* CreateApplication(int argc, wchar_t** wargv);
}