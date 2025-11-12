#pragma once
#include <stdio.h>
#include <glew.h>
#include <glfw3.h>

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise();

	// Buffers y estado de ventana
	GLfloat getBufferWidth() { return bufferWidth; }
	GLfloat getBufferHeight() { return bufferHeight; }
	bool    getShouldClose() { return glfwWindowShouldClose(mainWindow); }
	void    swapBuffers() { return glfwSwapBuffers(mainWindow); }

	// Entrada de mouse/teclado
	GLfloat getXChange();
	GLfloat getYChange();
	GLfloat getmuevex() { return muevex; }
	bool* getsKeys() { return keys; }

	// Movimiento existente (no alterado): helicóptero J/H
	float getHeliMoveZ() const { return heliMoveZ; }
	void  updateHeliMovement(float dt);   // integra J/H con deltaTime

	// === NUEVO === Control de articulaciones para 4 patas (1-4, SHIFT invierte, 0 resetea)
	float getarticulacion1() const { return articulacion1; }
	float getarticulacion2() const { return articulacion2; }
	float getarticulacion3() const { return articulacion3; }
	float getarticulacion4() const { return articulacion4; }
	void  updateArticulaciones(float dt);

	~Window();

private:
	GLFWwindow* mainWindow;
	GLint width, height;
	bool  keys[1024];
	GLint bufferWidth, bufferHeight;

	void   createCallbacks();
	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	GLfloat muevex;
	bool   mouseFirstMoved;

	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);

	// Estado previo existente
	float heliMoveZ;   // acumulador
	float heliSpeed;   // velocidad (unidades/seg)

	// === NUEVO === Estado de articulaciones y parámetros
	float articulacion1;
	float articulacion2;
	float articulacion3;
	float articulacion4;
	float artSpeed;  // grados/seg
	float artMin;    // límite inferior
	float artMax;    // límite superior

};
