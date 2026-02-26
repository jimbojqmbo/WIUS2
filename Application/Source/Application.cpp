#include "Application.h"

//Include GLEW
#include <GL/glew.h>

//Include GLFW
#include <GLFW/glfw3.h>

//Include the standard C++ headers
#include <stdio.h>
#include <stdlib.h>

#include "Scene1.h"
#include "Scene2.h"
#include "SceneLight.h"
#include "evochat.h"
#include "SceneTexture.h"
#include "SceneSkybox.h"
#include "SceneModel.h"

#include "KeyboardController.h"
#include "MouseController.h"
#include "SceneGUI.h"
#include "SceneText.h"

// WIU includes
#include "Scene01.h"
#include "Scene02.h"
#include "Scene03.h"
#include "Scene04.h"

GLFWwindow* m_window;
const unsigned char FPS = 144; // FPS of this game
const unsigned int frameTime = 1000 / FPS; // time for each frame

//Define an error callback
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
	_fgetchar();
}

//Define the key input callback
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int
	mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	KeyboardController::GetInstance()->Update(key, action);
}

//Define the mouse button callback
static void mousebtn_callback(GLFWwindow* window, int button, int action,
	int mods)
{
	// Send the callback to the mouse controller to handle
	if (action == GLFW_PRESS)
		MouseController::GetInstance()->UpdateMouseButtonPressed(button);
	else
		MouseController::GetInstance()->UpdateMouseButtonReleased(button);
}
//Define the mouse scroll callback
static void mousescroll_callback(GLFWwindow* window, double xoffset,
	double yoffset)
{
	MouseController::GetInstance()->UpdateMouseScroll(xoffset, yoffset);
}

void resize_callback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h); //update opengl the new window size
}

bool Application::IsKeyPressed(unsigned short key)
{
    return ((GetAsyncKeyState(key) & 0x8001) != 0);
}

Application::Application()
{
}

Application::~Application()
{
}

void Application::Init()
{
	//Set the error callback
	glfwSetErrorCallback(error_callback);

	//Initialize GLFW
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	//Set the GLFW window creation hints - these are optional
	glfwWindowHint(GLFW_SAMPLES, 4); //Request 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Request a specific OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //Request a specific OpenGL version
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

	//Create a window and create its OpenGL context
	m_window = glfwCreateWindow(1600, 900, "OPENGL FRAMEWORK", NULL, NULL);

	//If the window couldn't be created
	if (!m_window)
	{
		fprintf( stderr, "Failed to open GLFW window.\n" );
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//This function makes the context of the specified window current on the calling thread.
		glfwMakeContextCurrent(m_window);
	//Sets the key callback
	glfwSetKeyCallback(m_window, key_callback);
	//Sets the resize callback to handle window resizing
	glfwSetWindowSizeCallback(m_window, resize_callback);

	//Sets the key callback
	//glfwSetKeyCallback(m_window, key_callback);

	//Sets the mouse button callback
	glfwSetMouseButtonCallback(m_window, mousebtn_callback);
	//Sets the mouse scroll callback
	glfwSetScrollCallback(m_window, mousescroll_callback);
	// Hide and capture the cursor for FPS-style camera control
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Sets the resize callback to handle window resizing
	glfwSetWindowSizeCallback(m_window, resize_callback);

	glewExperimental = true; // Needed for core profile
	//Initialize GLEW
	GLenum err = glewInit();

	//If GLEW hasn't initialized
	if (err != GLEW_OK) 
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		//return -1;
	}

	sceneNum = SCENE_GUI;
}

void Application::Run()
{
	//Main Loop
	//Scene *scene = new SceneText();
	//scene->Init();

	Scene* scene1 = new SceneGUI();
	Scene* scene2 = new Scene01();
	Scene* scene3 = new Scene02();
	Scene* scene4 = new Scene03();
	Scene* scene5 = new Scene04();

	Scene* scene = scene1; //uncomment this after done editing DO NOT COMMIT THIS CHANGE
	//Scene* scene = scene4; //remove this after done editing DO NOT COMMIT THIS CHANGE
	scene->Init();

	sceneNum = SCENE_GUI;
	bool isEnterUp = false;

	m_timer.startTimer();    // Start timer to calculate how long it takes to render this frame

	while (!glfwWindowShouldClose(m_window) && !IsKeyPressed(VK_ESCAPE))
	{
		scene->Update(m_timer.getElapsedTime());
		scene->Render();

		static bool isLeftUp = false;
		static bool isRightUp = false;
		int mouseX = MouseController::GetInstance()->GetMousePositionX();
		int mouseY = MouseController::GetInstance()->GetMousePositionY();
		//std::cout << mouseX << ", " << mouseY << std::endl;
		if (!isLeftUp && MouseController::GetInstance()->IsButtonDown(GLFW_MOUSE_BUTTON_LEFT))
		{
			isLeftUp = true;

			//ominous runners
			if (mouseX > 0 && mouseX < 1600 && mouseY > 0 && mouseY < 900) {
				std::cout << "enter game" << std::endl;
				scene1->Exit();
				scene2->Init();
				PlaySound(NULL, 0, 0);
				scene = scene2;
				sceneNum = SCENE_01;
				PlaySound(TEXT("Sounds//granny.wav"), NULL, SND_FILENAME | SND_ASYNC);
			}
		}

		/*
		// === FROM MAIN MENU TO SCENE01 ===
		if (!isEnterUp && KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_1)) {
			if (sceneNum == SCENE_GUI) {
				scene1->Exit();
				scene2->Init();
				PlaySound(NULL, 0, 0);
				scene = scene2;
				sceneNum = SCENE_01;
				//PlaySound(TEXT("Sounds//fah.wav"), NULL, SND_FILENAME | SND_ASYNC);
			}
			isEnterUp = true;
		}
		else if (isEnterUp && KeyboardController::GetInstance()->IsKeyUp(GLFW_KEY_1)) {
			isEnterUp = false;
		}

		// === FROM MAIN MENU TO SCENE02 ===
		if (!isEnterUp && KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_2)) {
			if (sceneNum == SCENE_GUI) {
				scene1->Exit();
				scene3->Init();
				PlaySound(NULL, 0, 0);
				scene = scene3;
				sceneNum = SCENE_02;
				//PlaySound(TEXT("Sounds//fah.wav"), NULL, SND_FILENAME | SND_ASYNC);
			}
			isEnterUp = true;
		}
		else if (isEnterUp && KeyboardController::GetInstance()->IsKeyUp(GLFW_KEY_2)) {
			isEnterUp = false;
		}

		// === FROM MAIN MENU TO SCENE03 ===
		if (!isEnterUp && KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_3)) {
			if (sceneNum == SCENE_GUI) {
				scene1->Exit();
				scene4->Init();
				PlaySound(NULL, 0, 0);
				scene = scene4;
				sceneNum = SCENE_03;
				//PlaySound(TEXT("Sounds//fah.wav"), NULL, SND_FILENAME | SND_ASYNC);
			}
			isEnterUp = true;
		}
		else if (isEnterUp && KeyboardController::GetInstance()->IsKeyUp(GLFW_KEY_3)) {
			isEnterUp = false;
		}

		// === FROM MAIN MENU TO SCENE04 ===
		if (!isEnterUp && KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_4)) {
			if (sceneNum == SCENE_GUI) {
				scene1->Exit();
				scene5->Init();
				PlaySound(NULL, 0, 0);
				scene = scene5;
				sceneNum = SCENE_04;
				//PlaySound(TEXT("Sounds//fah.wav"), NULL, SND_FILENAME | SND_ASYNC);
			}
			isEnterUp = true;
		}
		else if (isEnterUp && KeyboardController::GetInstance()->IsKeyUp(GLFW_KEY_4)) {
			isEnterUp = false;
		}*/

		// === FROM SCENE01 to MAIN MENU ===
		if (!isEnterUp && KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_BACKSPACE)) {
			if (sceneNum == SCENE_01) {
				scene2->Exit();
				scene1->Init();
				PlaySound(NULL, 0, 0);
				scene = scene1;
				sceneNum = SCENE_GUI;

				glViewport(0, 0, 1600, 900);
			}
			isEnterUp = true;
		}
		else if (isEnterUp && KeyboardController::GetInstance()->IsKeyUp(GLFW_KEY_BACKSPACE)) {
			isEnterUp = false;
		}

		{
			// THE FOLLOWING SCENE SWITCHING MECHANICS ARE FOR SCENE01 ONLY
			{
				// === FROM SCENE01 TO SCENE02 (duck shooting) ===
				if (sceneNum == SCENE_01)
				{
					Scene01* s02 = dynamic_cast<Scene01*>(scene);
					if (s02 && s02->scene02request)
					{
						s02->scene02request = false;

						scene2->Exit();      // exit Scene01
						scene3->Init();
						//PlaySound(NULL, 0, 0);
						PlaySound(TEXT("Sounds//granny.wav"), NULL, SND_FILENAME | SND_ASYNC);

						scene = scene3;
						sceneNum = SCENE_02;
					}
				}

				// === FROM SCENE01 TO SCENE03 (Basketball) ===
				if (sceneNum == SCENE_01)
				{
					Scene01* s03 = dynamic_cast<Scene01*>(scene);
					if (s03 && s03->scene03request)
					{
						s03->scene03request = false;

						scene2->Exit();      // exit Scene01
						scene4->Init();      // Scene03 is scene4 in your setup
						//PlaySound(NULL, 0, 0);
						PlaySound(TEXT("Sounds//granny.wav"), NULL, SND_FILENAME | SND_ASYNC);

						scene = scene4;
						sceneNum = SCENE_03;
					}
				}

				// === FROM SCENE01 TO SCENE04 (Ball bouncer) ===
				if (sceneNum == SCENE_01)
				{
					Scene01* s04 = dynamic_cast<Scene01*>(scene);
					if (s04 && s04->scene04request)
					{
						s04->scene04request = false;

						scene2->Exit();      // exit Scene01
						scene5->Init();
						//PlaySound(NULL, 0, 0);
						PlaySound(TEXT("Sounds//granny.wav"), NULL, SND_FILENAME | SND_ASYNC);

						scene = scene5;
						sceneNum = SCENE_04;
					}
				}
			}

			// SCENE SWITCHING MECHANIC FOR SCENE02
			if (sceneNum == SCENE_02)
			{
				Scene02* s03 = dynamic_cast<Scene02*>(scene);
				if (s03 && s03->startingSceneRequest)
				{
					s03->startingSceneRequest = false;

					scene3->Exit();      // exit Scene02
					scene2->Init();      // init Scene01
					//PlaySound(NULL, 0, 0);
					PlaySound(TEXT("Sounds//granny.wav"), NULL, SND_FILENAME | SND_ASYNC);

					scene = scene2;
					sceneNum = SCENE_01;
				}
			}
		}

		{
			// THE FOLLOWING SCENE SWITCHING MECHANICS ARE FOR SCENE03 ONLY


			// === FROM SCENE03 TO SCENE01 (bumper car) ===
			if (sceneNum == SCENE_03)
			{
				Scene03* s01 = dynamic_cast<Scene03*>(scene);
				if (s01 && s01->scene01request)
				{
					s01->scene01request = false;

					scene4->Exit();      // exit Scene03
					scene2->Init();
					//PlaySound(NULL, 0, 0);
					PlaySound(TEXT("Sounds//granny.wav"), NULL, SND_FILENAME | SND_ASYNC);

					scene = scene2;
					sceneNum = SCENE_01;
				}
			}

			// === FROM SCENE03 TO SCENE02 (duck shootout) ===
			if (sceneNum == SCENE_03)
			{
				Scene03* s02 = dynamic_cast<Scene03*>(scene);
				if (s02 && s02->scene02request)
				{
					s02->scene02request = false;

					scene4->Exit();      // exit Scene03
					scene3->Init();
					//PlaySound(NULL, 0, 0);
					PlaySound(TEXT("Sounds//granny.wav"), NULL, SND_FILENAME | SND_ASYNC);

					scene = scene3;
					sceneNum = SCENE_02;
				}
			}

			// === FROM SCENE03 TO SCENE04 (Ball bouncer) ===
			if (sceneNum == SCENE_03)
			{
				Scene03* s04 = dynamic_cast<Scene03*>(scene);
				if (s04 && s04->scene04request)
				{
					s04->scene04request = false;

					scene4->Exit();      // exit Scene03
					scene5->Init();
					//PlaySound(NULL, 0, 0);
					PlaySound(TEXT("Sounds//granny.wav"), NULL, SND_FILENAME | SND_ASYNC);

					scene = scene5;
					sceneNum = SCENE_04;
				}
			}
		}

		if (KeyboardController::GetInstance()->IsKeyPressed('N'))
		{
			enablePointer = false;
			std::cout << "Pointer disabled \n";
		}
		if (KeyboardController::GetInstance()->IsKeyPressed('M'))
		{
			enablePointer = true;
			std::cout << "Pointer enabled \n";
		}

		if (enablePointer == false) {
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else if (enablePointer == true) {
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		//Swap buffers
		glfwSwapBuffers(m_window);
		KeyboardController::GetInstance()->PostUpdate();

		KeyboardController::GetInstance()->PostUpdate();

		MouseController::GetInstance()->PostUpdate();
		double mouse_x, mouse_y;
		glfwGetCursorPos(m_window, &mouse_x, &mouse_y);
		MouseController::GetInstance()->UpdateMousePosition(mouse_x, mouse_y);

		//Get and organize events, like keyboard and mouse input, window resizing, etc...
		glfwPollEvents();
        m_timer.waitUntil(frameTime);       // Frame rate limiter. Limits each frame to a specified time in ms.   

	} //Check if the ESC key had been pressed or if the window had been closed
	

	scene->Update(m_timer.getElapsedTime());
	scene->Render();

	scene->Exit();

	delete scene1;
	delete scene2;
}

void Application::Exit()
{
	KeyboardController::DestroyInstance();

	//Close OpenGL window and terminate GLFW
	glfwDestroyWindow(m_window);
	//Finalize and clean up GLFW
	glfwTerminate();

}
