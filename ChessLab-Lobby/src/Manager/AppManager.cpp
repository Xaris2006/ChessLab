#include "AppManager.h"

#include <mutex>
#include <chrono>
#include <unordered_map>
#include <filesystem>

#include "../Panels.h"
#include "ChessCore/FileFormats/FileManager.h"

static std::mutex addMutex;

static Manager::AppManager* s_AppManager = nullptr;

namespace Manager
{
	void AppManager::Init()
	{
		s_AppManager = new AppManager();

		s_AppManager->m_CheckingThread = new std::thread(
			[]()
			{
				Chess::FileManager::PathHash hasher;

				while (true)
				{
					if (s_AppManager->m_EndThread)
					{
						for (auto& [handle, process] : s_AppManager->m_Apps)
						{
							process->EndProcess();
							delete process;
						}
						return;
					}

					s_AppManager->m_Commands.clear();
					std::vector<HANDLE> endApps;
					std::unordered_map<HANDLE, size_t> AskForNewFilePath;

					for (auto& [handle, process] : s_AppManager->m_Apps)
					{
						if(!process->IsProcessActive())
						{
							endApps.emplace_back(handle);
							continue;
						}

						std::string Strcmd(process->Read());
						//std::cout << Strcmd << std::endl;

						if (Strcmd.find("*End") != std::string::npos)
							endApps.emplace_back(handle);

						size_t index = 0;
						std::vector<std::string> rawCommand;
						while (index < Strcmd.size())
						{
							size_t StartIndex = Strcmd.find("*", index);
							if (StartIndex != std::string::npos)
							{
								index = Strcmd.find('\n', StartIndex);
								if (index == std::string::npos)
									break;
								rawCommand.emplace_back(Strcmd.begin() + StartIndex, Strcmd.begin() + index);
								continue;
							}
							index = Strcmd.size();
						}
						for (auto& rcmd : rawCommand)
						{
							{
								size_t FileIndex = rcmd.find("*File");
								if (FileIndex != std::string::npos)
								{
									s_AppManager->m_Commands[handle].File = std::string(rcmd.begin() + FileIndex, rcmd.end());
									continue;
								}
							}
							{
								size_t OpenIndex = rcmd.find("*Open");
								if (OpenIndex != std::string::npos)
								{
									s_AppManager->m_Commands[handle].Open = std::string(rcmd.begin() + OpenIndex, rcmd.end());
									continue;
								}
							}
							{
								size_t AskIndex = rcmd.find("*Ask");
								if (AskIndex != std::string::npos)
								{
									s_AppManager->m_Commands[handle].Ask = std::string(rcmd.begin() + AskIndex, rcmd.end());
									continue;
								}
							}
						}
					}

					for (auto& [handle, cmd] : s_AppManager->m_Commands)
					{
						if (cmd.File.size())
						{
							std::string path = "";
							size_t IndexPath = cmd.File.find("Path:");
							if (IndexPath != std::string::npos)
								path = std::string(cmd.File.begin() + IndexPath + 5, cmd.File.begin() + cmd.File.find(":Path"));

							if (!path.empty())
								s_AppManager->m_OpenedPaths[handle] = std::stoull(path);
						}

						if (cmd.Open.size())
						{
							std::string path = "";
							size_t IndexPath = cmd.Open.find("Path:");
							if (IndexPath != std::string::npos)
								path = std::string(cmd.Open.begin() + IndexPath + 5, cmd.Open.begin() + cmd.Open.find(":Path"));
							//here
							s_AppManager->CreateApp(std::filesystem::u8path(path));
						}

						if (cmd.Ask.size())
						{
							std::string path = "";
							size_t IndexPath = cmd.Ask.find("Path:");
							if (IndexPath != std::string::npos)
								path = std::string(cmd.Ask.begin() + IndexPath + 5, cmd.Ask.begin() + cmd.Ask.find(":Path"));

							if (path.empty())
								s_AppManager->m_Apps[handle]->Write("Accept\n");
							else
								AskForNewFilePath[handle] = std::stoull(path);
						}
					}
					
					for (auto& [handle, value] : AskForNewFilePath)
					{
						bool alreadyOpened = false;
						for (auto& [otherKey, otherValue] : s_AppManager->m_OpenedPaths)
						{
							if (otherValue == value)
							{
								alreadyOpened = true;
								break;
							}
						}

						if (alreadyOpened)
							s_AppManager->m_Apps[handle]->Write("Decline\n");
						else
							s_AppManager->m_Apps[handle]->Write("Accept\n");
					}

					addMutex.lock();
					
					if (s_AppManager->m_AddApp)
					{
						s_AppManager->m_AddApp = false;
						bool alreadyOpened = false;

						std::filesystem::path npath = s_AppManager->m_NewPath;
						auto npathHash = hasher(npath);

						if (npathHash != 0)
						{
							for (auto& [key, other] : s_AppManager->m_OpenedPaths)
							{
								if (other == npathHash)
								{
									alreadyOpened = true;
									Panels::OpenAlreadyOpenedPopup();
									break;
								}
							}
						}

						if (!alreadyOpened)
						{
							std::error_code ec;
							auto pathToAdd = std::filesystem::canonical(npath, ec).wstring();

							if (!ec)
							{
								Process* nApp = new Process(L"ChessLabApp\\ChessLab-Board.exe", pathToAdd);
								s_AppManager->m_Apps[nApp->GetHandle()] = nApp;
								nApp->Write("Ok");
							}
						}
					}

					addMutex.unlock();

					using namespace std::chrono_literals;
					std::this_thread::sleep_for(300ms);

					for (int i = 0; i < endApps.size(); i++)
					{
						s_AppManager->m_Apps[endApps[i]]->EndProcess();
						delete s_AppManager->m_Apps[endApps[i]];
						s_AppManager->m_Apps.erase(endApps[i]);
						s_AppManager->m_OpenedPaths.erase(endApps[i]);
					}
				}
			}
		);
	}

	void AppManager::Shutdown()
	{
		s_AppManager->m_EndThread = true;
		s_AppManager->m_CheckingThread->join();
		delete s_AppManager->m_CheckingThread;
	}

	AppManager& AppManager::Get()
	{
		return *s_AppManager;
	}

	void AppManager::CreateApp(const std::filesystem::path& path)
	{
		addMutex.lock();

		m_AddApp = true;
		m_NewPath = path;

		addMutex.unlock();
	}

	bool AppManager::IsAppOpen(const std::filesystem::path& path) const
	{
		bool founded = false;
		Chess::FileManager::PathHash hasher;

		for (auto& [key, value] : m_OpenedPaths)
		{
			if (value == hasher(path))
			{
				founded = true;
				break;
			}
		}

		return founded;
	}
}