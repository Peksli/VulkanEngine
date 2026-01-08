#pragma once

#include <vector>
#include <functional>
#include <algorithm>


namespace VulkanEngine {

	class LifetimeManager
	{
	public:
		LifetimeManager() = default;
		LifetimeManager(const LifetimeManager&)				= delete;
		LifetimeManager& operator=(const LifetimeManager&)	= delete;
		virtual ~LifetimeManager() = default;

		void PushFunction(std::function<void()>&& function);
		void Flush();

		template<typename F, typename... Args>
		void Push(F&& function, Args&&... args) 
		{
			PushFunction([=]() 
				{
					function(args...);
				});
		}

	private:
		std::vector<std::function<void()>> m_Deletors;
	};

}