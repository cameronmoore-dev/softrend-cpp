#pragma once

#include <memory>
#include <stack>

#include "Scene.h"
#include "../Window.h"

class SceneManager
{
public:
	template<typename T>
	void addScene(Window& window)
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