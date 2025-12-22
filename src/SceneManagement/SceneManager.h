#pragma once

#include <memory>
#include <stack>

#include "Scene.h"
#include "Window.h"
#include "Renderer.h"

class SceneManager
{
public:
	template<typename T>
	void addScene(Window& window, Renderer &renderer)
	{
		if (!m_scenes.empty())
		{
			m_scenes.pop();
		}
		m_scenes.push(std::make_unique<T>(window, renderer));
		m_scenes.top()->start();
	}

	std::unique_ptr<Scene>& getActiveScene() { return m_scenes.top(); }

private:
	std::stack<std::unique_ptr<Scene>> m_scenes;
};