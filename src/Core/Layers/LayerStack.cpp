#include "Core/Layers/LayerStack.h"
#include "Core/LogSystem.h"


namespace VulkanEngine {

	void LayerStack::PushLayer(std::shared_ptr<Layer> layer)
	{
		if (IsPresent(layer))
		{
			VulkanEngine_ERROR("Failed to push layer cz already present with the same name");
			return;
		}

		layer->OnAttach();

		m_Layers.insert(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
	}

	void LayerStack::PushOverlay(std::shared_ptr<Layer> overlay)
	{
		if (IsPresent(overlay))
		{
			VulkanEngine_ERROR("Failed to push layer cz already present with the same name");
			return;
		}

		overlay->OnAttach();

		m_Layers.push_back(overlay);
	}

	void LayerStack::RemoveLayer(std::string layerName)
	{
		auto it = std::find_if(m_Layers.begin(), m_Layers.end(),
			[&layerName](std::shared_ptr<Layer> layer)
			{
				return layer->GetName() == layerName;
			});

		if (it != m_Layers.end())
		{
			size_t distance = std::distance(m_Layers.begin(), it);
			UpdateInsertIndexAfterRemoval(distance);

			(*it)->OnDetach();

			m_Layers.erase(it);
		}
	}

	void LayerStack::RemoveOverlay(std::string overlayName)
	{
		for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it)
		{
			if ((*it)->GetName() == overlayName)
			{
				size_t distance = std::distance(m_Layers.begin(), it.base() - 1);
				UpdateInsertIndexAfterRemoval(distance);

				(*it)->OnDetach();

				m_Layers.erase(it.base() - 1);
				return;
			}
		}
	}

	bool LayerStack::IsPresent(std::shared_ptr<Layer> layer)
	{
		return std::find_if(m_Layers.begin(), m_Layers.end(),
			[&layer](std::shared_ptr<Layer> existLayer)
			{
				return existLayer->GetName() == layer->GetName();
			}) != m_Layers.end();
	}

	void LayerStack::UpdateInsertIndexAfterRemoval(size_t removedIndex)
	{
		if (removedIndex < m_LayerInsertIndex)
		{
			m_LayerInsertIndex--;
		}
	}

}
