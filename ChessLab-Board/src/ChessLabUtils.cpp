#include "ChessLabUtils.h"

#include "ChessAPI/ChessAPI.h"
#include "AppManagerChild.h"
#include "Windows/WindowsUtils.h"

bool g_AlreadyOpenedModalOpen = false;
bool g_LoadingModalOpen = false;
float g_LoadingPersentage = 0;

namespace ChessLab::Utils
{
	void New()
	{
		ChessAPI::OpenChessFile("");

		AppManagerChild::OwnChessFile("");
	}

	void Open()
	{
		std::string filepath = Windows::Utils::OpenFile(L"PGN Database (*.pgn)\0*.pgn\0Chess Lab Database (*.cld)\0*.cld\0\0");
		if (!filepath.empty())
		{
			bool anwser = AppManagerChild::IsChessFileAvail(filepath);

			if (anwser)
			{
				g_LoadingModalOpen = true;

				static std::thread* openerThread = nullptr;

				if (openerThread != nullptr && openerThread->joinable())
				{
					openerThread->join();
					delete openerThread;
				}


				openerThread = new std::thread(
					[filepath]()
					{
						g_LoadingPersentage = 0;
						ChessAPI::OpenChessFile(filepath, &g_LoadingPersentage);
						AppManagerChild::OwnChessFile(ChessAPI::GetPgnFilePath());
						g_LoadingPersentage = 1;
					});
			}
			else
			{
				g_AlreadyOpenedModalOpen = true;
			}
		}
	}

	void SaveAs()
	{
		std::string filepath = Windows::Utils::SaveFile(L"PGN Database (*.pgn)\0*.pgn\0Chess Lab Database (*.cld)\0*.cld\0\0");
		if (!filepath.empty())
		{
			g_LoadingModalOpen = true;

			std::thread(
				[filepath]()
				{
					g_LoadingPersentage = 0;
					ChessAPI::OverWriteChessFile(filepath, &g_LoadingPersentage);
					AppManagerChild::OwnChessFile(ChessAPI::GetPgnFilePath());
					g_LoadingPersentage = 1;
				}
			).detach();
		}
	}

	void Save()
	{
		if (ChessAPI::GetPgnFileName() == "New Game")
		{
			SaveAs();
		}
		else
		{
			g_LoadingModalOpen = true;

			std::thread(
				[]()
				{
					g_LoadingPersentage = 0;
					ChessAPI::OverWriteChessFile("", &g_LoadingPersentage);
					g_LoadingPersentage = 1;
				}
			).detach();
		}
	}
}