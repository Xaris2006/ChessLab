#include "AppManager.h"

#include <mutex>
#include <chrono>
#include <unordered_map>
#include <filesystem>

#include "ChessCore/pgn/Pgn.h"

static std::mutex addMutex;
extern bool g_AlreadyOpenedModalOpen;

static Manager::AppManager* s_AppManager = nullptr;

namespace Manager
{
	void AppManager::Init()
	{
		s_AppManager = new AppManager();

		s_AppManager->m_CheckingThread = new std::thread(
			[]()
			{
				Chess::PgnFile::PgnPath_Hash hasher;

				while (true)
				{
					if (s_AppManager->m_EndThread)
					{
						for (int i = 0; i < s_AppManager->m_Apps.size(); i++)
						{
							s_AppManager->m_Apps[i].EndProcess();
						}
						return;
					}

					s_AppManager->m_Commands.clear();
					//m_OpenedPaths.clear();
					std::vector<int> endApps;
					std::unordered_map<int, size_t> AskForNewFilePath;

					s_AppManager->m_Commands.resize(s_AppManager->m_Apps.size());

					for (int i = 0; i < s_AppManager->m_Apps.size(); i++)
					{
						if(!s_AppManager->m_Apps[i].IsProcessActive())
						{
							endApps.emplace_back(i);
							continue;
						}

						std::string Strcmd(s_AppManager->m_Apps[i].Read());
						//std::cout << Strcmd << std::endl;

						if (Strcmd.find("*End") != std::string::npos)
							endApps.emplace_back(i);

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
									s_AppManager->m_Commands[i].File = std::string(rcmd.begin() + FileIndex, rcmd.end());
									continue;
								}
							}
							{
								size_t OpenIndex = rcmd.find("*Open");
								if (OpenIndex != std::string::npos)
								{
									s_AppManager->m_Commands[i].Open = std::string(rcmd.begin() + OpenIndex, rcmd.end());
									continue;
								}
							}
							{
								size_t AskIndex = rcmd.find("*Ask");
								if (AskIndex != std::string::npos)
								{
									s_AppManager->m_Commands[i].Ask = std::string(rcmd.begin() + AskIndex, rcmd.end());
									continue;
								}
							}
						}
					}

					for (int i = 0; i < s_AppManager->m_Commands.size(); i++)
					{
						if (s_AppManager->m_Commands[i].File.size())
						{
							std::string path = "";
							size_t IndexPath = s_AppManager->m_Commands[i].File.find("Path:");
							if (IndexPath != std::string::npos)
								path = std::string(s_AppManager->m_Commands[i].File.begin() + IndexPath + 5, s_AppManager->m_Commands[i].File.begin() + s_AppManager->m_Commands[i].File.find(":Path"));

							if (!path.empty())
								s_AppManager->m_OpenedPaths[i] = std::stoull(path);
						}

						if (s_AppManager->m_Commands[i].Open.size())
						{
							std::string path = "";
							size_t IndexPath = s_AppManager->m_Commands[i].Open.find("Path:");
							if (IndexPath != std::string::npos)
								path = std::string(s_AppManager->m_Commands[i].Open.begin() + IndexPath + 5, s_AppManager->m_Commands[i].Open.begin() + s_AppManager->m_Commands[i].Open.find(":Path"));
							//here
							s_AppManager->CreateApp(path);
						}

						if (s_AppManager->m_Commands[i].Ask.size())
						{
							std::string path = "";
							size_t IndexPath = s_AppManager->m_Commands[i].Ask.find("Path:");
							if (IndexPath != std::string::npos)
								path = std::string(s_AppManager->m_Commands[i].Ask.begin() + IndexPath + 5, s_AppManager->m_Commands[i].Ask.begin() + s_AppManager->m_Commands[i].Ask.find(":Path"));

							if (path.empty())
								s_AppManager->m_Apps[i].Write("Accept\n");
							else
								AskForNewFilePath[i] = std::stoull(path);
						}
					}
					
					for (auto& [key, value] : AskForNewFilePath)
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
							s_AppManager->m_Apps[key].Write("Decline\n");
						else
							s_AppManager->m_Apps[key].Write("Accept\n");
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
									g_AlreadyOpenedModalOpen = true;
									break;
								}
							}
						}

						if (!alreadyOpened)
						{
							auto pathToAdd = std::filesystem::canonical(npath).wstring();
							s_AppManager->m_Apps.emplace_back(L"ChessLabApp\\ChessLab-Board.exe", pathToAdd);
							s_AppManager->m_Apps[s_AppManager->m_Apps.size() - 1].Write("Ok");
						}
					}

					addMutex.unlock();

					using namespace std::chrono_literals;
					std::this_thread::sleep_for(300ms);

					for (int i = 0; i < endApps.size(); i++)
					{
						s_AppManager->m_Apps[endApps[i] - i].EndProcess();
						s_AppManager->m_Apps.erase(s_AppManager->m_Apps.begin() + endApps[i] - i);
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
		Chess::PgnFile::PgnPath_Hash hasher;


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