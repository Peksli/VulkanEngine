#include "LifetimeManager.h"

namespace VulkanEngine {

	void LifetimeManager::PushFunction(std::function<void()>&& function)
	{
		m_Deletors.push_back(std::move(function));
	}

	void LifetimeManager::Flush()
	{
		for (auto it = m_Deletors.rbegin(); it != m_Deletors.rend(); ++it) 
		{
			(*it)();
		}

		m_Deletors.clear();
	}

}