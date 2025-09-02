#pragma once

class Scene
{
public:
	Scene() = default;
	virtual ~Scene() = default;

	virtual void start() = 0;
	
protected:
	virtual void processInput() = 0;
	virtual void update() = 0;
	virtual void render() = 0;
	virtual void shutdown() = 0;
};