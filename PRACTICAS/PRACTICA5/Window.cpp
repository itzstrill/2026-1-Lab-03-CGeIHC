#include "Window.h"
#include <iostream>

Window::Window()
{
	width = 800;
	height = 600;
	for (size_t i = 0; i < 1024; i++) { keys[i] = 0; }

	// Mouse
	lastX = 0.0f; lastY = 0.0f;
	xChange = 0.0f; yChange = 0.0f;
	mouseFirstMoved = true;

	muevex = 2.0f;

	// Movimiento helicóptero (no alterado)
	heliMoveZ = 0.0f;
	heliSpeed = 1.0f;

	// === NUEVO === Articulaciones (patas)
	articulacion1 = articulacion2 = articulacion3 = articulacion4 = 0.0f;
	artSpeed = 90.0f;   // grados/seg
	artMin = -45.0f;  // límites
	artMax = 45.0f;
}

Window::Window(GLint windowWidth, GLint windowHeight)
{
	width = windowWidth;
	height = windowHeight;
	for (size_t i = 0; i < 1024; i++) { keys[i] = 0; }

	// Mouse
	lastX = 0.0f; lastY = 0.0f;
	xChange = 0.0f; yChange = 0.0f;
	mouseFirstMoved = true;

	// Var de ejemplo existente en tu proyecto
	muevex = 2.0f;

	// Movimiento helicóptero (no alterado)
	heliMoveZ = 0.0f;
	heliSpeed = 1.0f;

	// === NUEVO === Articulaciones (patas)
	articulacion1 = articulacion2 = articulacion3 = articulacion4 = 0.0f;
	artSpeed = 90.0f;   // grados/seg
	artMin = -45.0f;  // límites
	artMax = 45.0f;
}

int Window::Initialise()
{
	// Init GLFW
	if (!glfwInit())
	{
		std::cout << "GLFW initialisation failed!" << std::endl;
		glfwTerminate();
		return 1;
	}

	// Setup GLFW Window properties
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	mainWindow = glfwCreateWindow(width, height, "Practica", NULL, NULL);
	if (!mainWindow)
	{
		std::cout << "GLFW window creation failed!" << std::endl;
		glfwTerminate();
		return 1;
	}

	// Get Buffer size info
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	// Set context for GLEW to use
	glfwMakeContextCurrent(mainWindow);

	// Handle Input
	createCallbacks();
	glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Allow modern extension features
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "GLEW initialisation failed!" << std::endl;
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glViewport(0, 0, bufferWidth, bufferHeight);

	glfwSetWindowUserPointer(mainWindow, this);

	return 0;
}

void Window::createCallbacks()
{
	glfwSetKeyCallback(mainWindow, ManejaTeclado);
	glfwSetCursorPosCallback(mainWindow, ManejaMouse);
}

GLfloat Window::getXChange()
{
	GLfloat theChange = xChange;
	xChange = 0.0f;
	return theChange;
}

GLfloat Window::getYChange()
{
	GLfloat theChange = yChange;
	yChange = 0.0f;
	return theChange;
}

void Window::ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!theWindow) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			theWindow->keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			theWindow->keys[key] = false;
		}
	}
}

void Window::ManejaMouse(GLFWwindow* window, double xPos, double yPos)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
	if (!theWindow) return;

	if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = (GLfloat)xPos;
		theWindow->lastY = (GLfloat)yPos;
		theWindow->mouseFirstMoved = false;
	}

	theWindow->xChange = (GLfloat)xPos - theWindow->lastX;
	theWindow->yChange = theWindow->lastY - (GLfloat)yPos;

	theWindow->lastX = (GLfloat)xPos;
	theWindow->lastY = (GLfloat)yPos;
}

void Window::updateHeliMovement(float dt)
{
	// Conserva tu comportamiento: mover en Z con J/H (ejemplo)
	if (keys[GLFW_KEY_J]) { heliMoveZ -= heliSpeed * dt; }
	if (keys[GLFW_KEY_H]) { heliMoveZ += heliSpeed * dt; }
}

void Window::updateArticulaciones(float dt)
{
	// SHIFT invierte sentido
	int dir = (keys[GLFW_KEY_LEFT_SHIFT] || keys[GLFW_KEY_RIGHT_SHIFT]) ? -1 : 1;

	auto avanzar = [&](float& ang, bool held) {
		if (!held) return;
		ang += dir * artSpeed * dt;
		if (ang > artMax) ang = artMax;
		if (ang < artMin) ang = artMin;
		};

	// Teclas 1..4 controlan cada pata
	avanzar(articulacion1, keys[GLFW_KEY_1]);
	avanzar(articulacion2, keys[GLFW_KEY_2]);
	avanzar(articulacion3, keys[GLFW_KEY_3]);
	avanzar(articulacion4, keys[GLFW_KEY_4]);

	// 0 resetea
	if (keys[GLFW_KEY_0]) {
		articulacion1 = articulacion2 = articulacion3 = articulacion4 = 0.0f;
	}
}

Window::~Window()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
}
