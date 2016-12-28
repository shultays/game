#include <string>
#include <iostream>

#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <SDL.h>

#include "cGlobals.h"
#include "Bloodworks.h"

std::string programName = "Bloodworks";

SDL_Window *mainWindow;
SDL_GLContext mainContext;

bool SetOpenGLAttributes();
void CheckSDLError(int line);
void RunGame();
void Cleanup();

Coral coral;

bool Init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "Failed to init SDL\n";
		return false;
	}

	mainWindow = SDL_CreateWindow(programName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (!mainWindow)
	{
		std::cout << "Unable to create window\n";
		CheckSDLError(__LINE__);
		return false;
	}

	lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::jit);

	mainContext = SDL_GL_CreateContext(mainWindow);
	SetOpenGLAttributes();
	glewInit();
	return true;
}

bool SetOpenGLAttributes()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	return true;
}


void InitGame()
{
	coral.init();
	game = new Bloodworks();
	game->initInternal();
}

int main(int argc, char *argv[])
{
	if (!Init())
		return -1;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	SDL_GL_SwapWindow(mainWindow);
	SDL_GL_SetSwapInterval(0);
	debugRenderer.init();
	InitGame();
	RunGame();

	Cleanup();

	return 0;
}

void RunGame()
{
	while (coral.gameRunning)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_MOUSEMOTION)
			{
				input.setMousePos(Vec2((float)event.motion.x, (float)(coral.windowHeight - event.motion.y)), Vec2((float)event.motion.xrel, (float)-event.motion.yrel));
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				input.pressKey(event.button.button + mouse_start);
			}
			else if (event.type == SDL_MOUSEBUTTONUP)
			{
				input.releaseKey(event.button.button + mouse_start);
			}
			else if (event.type == SDL_KEYDOWN)
			{
				input.pressKey(event.key.keysym.scancode);
			}
			else if (event.type == SDL_KEYUP)
			{
				input.releaseKey(event.key.keysym.scancode);
			}
			else if (event.type == SDL_WINDOWEVENT)
			{

				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_RESIZED:
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					coral.windowResized(event.window.data1, event.window.data2);
					game->windowResized(event.window.data1, event.window.data2);
					break;
				}
			}
			else if(event.type == SDL_QUIT)
			{
				coral.quitGame();
			}
		}
		coral.tick();
	}
}

void Cleanup()
{
	SAFE_DELETE(game);
	SDL_GL_DeleteContext(mainContext);
	SDL_DestroyWindow(mainWindow);
	SDL_Quit();
	debugRenderer.freeAll();
	resources.freeAll();
}

void CheckSDLError(int line = -1)
{
	std::string error = SDL_GetError();

	if (error != "")
	{
		std::cout << "SLD Error : " << error << std::endl;

		if (line != -1)
			std::cout << "\nLine : " << line << std::endl;

		SDL_ClearError();
	}
}