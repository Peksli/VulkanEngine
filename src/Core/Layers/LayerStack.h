#pragma once

#include <vector>

#include "Core/Layers/Layer.h"


namespace VulkanEngine {

	class LayerStack
	{
	public:
		LayerStack() = default;
		virtual ~LayerStack() = default;

		void PushLayer(std::shared_ptr<Layer> layer);
		void PushOverlay(std::shared_ptr<Layer> overlay);
		void RemoveLayer(std::string layerName);
		void RemoveOverlay(std::string overlayName);

		auto rbegin()	const { return m_Layers.rbegin();	}
		auto rend()		const { return m_Layers.rend();		}
		auto begin()	const { return m_Layers.begin();	}
		auto end()		const { return m_Layers.end();		}

	private:
		bool IsPresent(std::shared_ptr<Layer> layer);
		void UpdateInsertIndexAfterRemoval(size_t removedIndex);

	private:
		std::vector<std::shared_ptr<Layer>>	m_Layers;
		uint32_t m_LayerInsertIndex = 0;
	};

}
