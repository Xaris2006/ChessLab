#include "ChessLabUtils.h"
#include "Panels.h"
#include "AppManagerChild.h"

#include "ChessAPI/ChessAPI.h"
#include "ChessCore/FileFormats/FileManager.h"
#include "ChessCore/FileFormats/ChessFileManager.h"

#include "Walnut/EntryPoint.h"

class ChessLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
	{
		AppManagerChild::Init();
		Chess::FileManager::Init();
		Chess::ChessFileManager::Init();
		ChessAPI::Init();
		Panels::Init();
		
		Panels::OnAttach();

		ChessLab::Utils::LoadViewStyle();
	}

	virtual void OnDetach() override
	{
		Panels::OnDetach();

		Chess::ChessFileManager::Shutdown();
		Chess::FileManager::Shutdown();
		AppManagerChild::ShutDown();
	}

	virtual void OnUIRender() override
	{	
		AppManagerChild::OnUpdate();
		Panels::OnKeyEvent();
		Panels::OnImGuiRender();
	}
};

Walnut::Application* Walnut::CreateApplication(int argc, wchar_t** wargv)
{	
	ChessLab::Utils::InitializeArguments(argc, wargv);
	ChessLab::Utils::InitializeAppDirectory();
	ChessLab::Utils::InitializeCacheDirectory();

	ChessLab::Utils::InitializeAppSpecification();
	Walnut::ApplicationSpecification& spec = ChessLab::Utils::GetAppSpecification();

	Walnut::Application* app = new Walnut::Application(spec, 237 + 60);
	std::shared_ptr<ChessLayer> chessLayer = std::make_shared<ChessLayer>();

	app->PushLayer(chessLayer);
	app->SetMenubarCallback(ChessLab::Utils::GetMenuBar);
	
	return app;
}