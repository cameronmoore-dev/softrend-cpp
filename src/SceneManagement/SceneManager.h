#pragma once

#include <memory>
#include <stack>

#include "AppContext.h"
#include "Scene.h"

namespace scene_manager
{
	inline std::stack<std::unique_ptr<Scene>> m_scenes;

	template<typename T>
	inline void addScene(AppContext &app)
	{
		m_scenes.push(std::make_unique<T>(app));
		m_scenes.top()->start();
	}

	inline void removeScene()
	{
		m_scenes.pop();
		m_scenes.top()->start();
	}

	inline std::unique_ptr<Scene>& getActiveScene() { return m_scenes.top(); }
}