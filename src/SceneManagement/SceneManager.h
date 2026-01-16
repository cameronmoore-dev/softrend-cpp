#pragma once

#include <memory>
#include <stack>

#include "jawl/base_window.h"
#include "Scene.h"

class SceneManager
{
public:
	template<typename T>
	void addScene(jawl::Window& window)
	{
		if (!m_scenes.empty())
		{
			m_scenes.pop();
		}
		m_scenes.push(std::make_unique<T>(window));
		m_scenes.top()->start();
	}

	std::unique_ptr<Scene>& getActiveScene() { return m_scenes.top(); }

private:
	std::stack<std::unique_ptr<Scene>> m_scenes;
};