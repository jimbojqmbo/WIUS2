
#ifndef APPLICATION_H
#define APPLICATION_H

#include "timer.h"

class Application
{
public:
	Application();
	~Application();
	void Init();
	void Run();
	void Exit();
	static bool IsKeyPressed(unsigned short key);

	enum SCENE_NUM
	{
		SCENE_TEXT = 0,
		SCENE_GUI,
		TOTAL_SCENE
	};

private:

	//Declare a window object
	StopWatch m_timer;

	bool enablePointer = true;
	bool showPointer = true;

	SCENE_NUM sceneNum;
	bool isEnterUp = false;
};

#endif