#include "SearchWork.h"

namespace Chess
{
	SearchWork::SearchWork()
	{
		m_Data = std::make_shared<PgnManager::SearchWorkData>();
		m_ID = PgnManager::Get().AddWork(m_Data);
	}

	SearchWork::~SearchWork()
	{
		PgnManager::Get().RemoveWork(m_ID);
	}

	void SearchWork::SearchLabel(PgnManager::FileID id, const std::map<std::string, std::string>& settings)
	{
		std::unique_lock<std::shared_mutex> ul(m_Data->WorkMutex);

		size_t added = 0;
		auto file = PgnManager::Get().GetFileData(id);

		PgnManager::TargetID nID;
		m_Data->Targets[nID] = std::make_shared<PgnManager::Target>();

		m_Data->Targets[nID]->FDataID = id;
		m_Data->Targets[nID]->Settings = settings;

		for (size_t i = 0; i < file->DataPointers.size(); i += PgnManager::SearchWorkData::s_countPerJob)
		{
			PgnManager::SearchWorkData::Job job;
			job.Index = i;
			job.tID = nID;
			m_Data->JobsToDo.emplace_back(job);
			added++;
		}

		m_Data->Jobs += added;
	}

	void SearchWork::SearchMoves(PgnManager::FileID id, const std::vector<std::string>& moves)
	{

	}

	void SearchWork::Clear()
	{
		PgnManager::Get().RemoveWork(m_ID);

		m_Data = std::make_shared<PgnManager::SearchWorkData>();
		m_ID = PgnManager::Get().AddWork(m_Data);
	}

	std::vector<size_t>& SearchWork::GetResult() const
	{
		//for (auto& [id, data] : m_Data->Targets)
		//{
		//	return data->PossitiveIndexes;
		//}
		return m_Data->AllPossitiveIndexes;
	}

	float SearchWork::GetPercentage() const
	{
		if (m_Data->Jobs == 0)
			return 0.0f;
		return 100.0f - (float)m_Data->JobsToDo.size() / m_Data->Jobs * 100.0f;
	}
}