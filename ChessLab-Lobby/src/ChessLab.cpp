#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/UI/UI.h"

#include "Manager/AppManager.h"
#include "Manager/ToolManager.h"
#include "ChessCore/FileFormats/ChessFileManager.h"
#include "ChessCore/FileFormats/FileManager.h"

#include "ChessLabUtils.h"
#include "Panels.h"

bool g_AlreadyOpenedModalOpen = false;

class LobbyLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
	{
		ChessLab::Utils::CreateSingularity();

		Process* startProcess;

		if (!ChessLab::Utils::IsSecondInstance())
			startProcess = new Process(L"Start.exe", L"");
		
		Manager::AppManager::Init();
		Manager::ToolManager::Init();
		Chess::FileManager::Init();
		Chess::ChessFileManager::Init();
		Panels::Init();

		Panels::OnAttach();

		if (!ChessLab::Utils::IsSecondInstance())
		{
			using namespace std::chrono_literals;
			//std::this_thread::sleep_for(1s);
			startProcess->EndProcess();
			delete startProcess;
		}
	}

	virtual void OnDetach() override
	{
		Panels::OnDetach();

		ChessLab::Utils::DestroySingularity();
		
		Chess::ChessFileManager::Shutdown();
		Chess::FileManager::Shutdown();
		Manager::AppManager::Shutdown();
		Manager::ToolManager::Shutdown();
	}

	virtual void OnUIRender() override
	{
		if (ChessLab::Utils::IsSecondInstance())
		{
			Walnut::Application::Get().Close();
			return;
		}		

		ChessLab::Utils::ReadRequestFile();

		Panels::OnKeyEvent();
		Panels::OnImGuiRender();
	}

};

Walnut::Application* Walnut::CreateApplication(int argc, wchar_t** wargv)
{
	ChessLab::Utils::InitializeAppSpecification();
	ChessLab::Utils::InitializeArguments(argc, wargv);
	ChessLab::Utils::InitializeAppDirectory();
	ChessLab::Utils::InitializeCacheDirectory();

	Walnut::Application* app = new Walnut::Application(ChessLab::Utils::GetAppSpecification(), 117 - 50);
	
	app->SetMinImGuiWindowSize(370.0f);
	app->SetDockNodeFlags(ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoTabBar);

	std::shared_ptr<LobbyLayer> chessLayer = std::make_shared<LobbyLayer>();
	app->PushLayer(chessLayer);
	app->SetMenubarCallback(ChessLab::Utils::GetMenuBar);

	return app;
}