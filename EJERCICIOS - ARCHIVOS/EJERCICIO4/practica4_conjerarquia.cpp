/*Práctica 4: Modelado Jerárquico.
Se implementa el uso de matrices adicionales para almacenar información de transformaciones geométricas que se quiere
heredar entre diversas instancias para que estén unidas
Teclas de la F a la K para rotaciones de articulaciones 
Teclas J y K para girar la canasta
*/
#include <stdio.h>
#include <string.h>
#include<cmath>
#include<vector>
#include <glew.h>
#include <glfw3.h>
//glm
#include<glm.hpp>
#include<gtc\matrix_transform.hpp>
#include<gtc\type_ptr.hpp>
#include <gtc\random.hpp>
//clases para dar orden y limpieza al còdigo
#include"Mesh.h"
#include"Shader.h"
#include"Sphere.h"
#include"Window.h"
#include"Camera.h"
//tecla E: Rotar sobre el eje X
//tecla R: Rotar sobre el eje Y
//tecla T: Rotar sobre el eje Z


using std::vector;

//Dimensiones de la ventana
const float toRadians = 3.14159265f / 180.0; //grados a radianes
const float PI = 3.14159265f;
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;
Camera camera;
Window mainWindow;
vector<Mesh*> meshList;
vector<Shader>shaderList;
//Vertex Shader
static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
Sphere sp = Sphere(1.0, 20, 20); //recibe radio, slices, stacks




void CrearCubo()
{
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};

	GLfloat cubo_vertices[] = {
		// front
		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		// back
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f
	};
	Mesh* cubo = new Mesh();
	cubo->CreateMesh(cubo_vertices, cubo_indices, 24, 36);
	meshList.push_back(cubo);
}

// Pirámide triangular regular
void CrearPiramideTriangular()
{
	unsigned int indices_piramide_triangular[] = {
			0,1,2,
			1,3,2,
			3,0,2,
			1,0,3

	};
	GLfloat vertices_piramide_triangular[] = {
		-0.5f, -0.5f,0.0f,	//0
		0.5f,-0.5f,0.0f,	//1
		0.0f,0.5f, -0.25f,	//2
		0.0f,-0.5f,-0.5f,	//3

	};
	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices_piramide_triangular, indices_piramide_triangular, 12, 12);
	meshList.push_back(obj1);

}
/*
Crear cilindro y cono con arreglos dinámicos vector creados en el Semestre 2023 - 1 : por Sánchez Pérez Omar Alejandro
*/
void CrearCilindro(int res, float R) {

	//constantes utilizadas en los ciclos for
	int n, i;
	//cálculo del paso interno en la circunferencia y variables que almacenarán cada coordenada de cada vértice
	GLfloat dt = 2 * PI / res, x, z, y = -0.5f;

	vector<GLfloat> vertices;
	vector<unsigned int> indices;

	//ciclo for para crear los vértices de las paredes del cilindro
	for (n = 0; n <= (res); n++) {
		if (n != res) {
			x = R * cos((n)*dt);
			z = R * sin((n)*dt);
		}
		//caso para terminar el círculo
		else {
			x = R * cos((0) * dt);
			z = R * sin((0) * dt);
		}
		for (i = 0; i < 6; i++) {
			switch (i) {
			case 0:
				vertices.push_back(x);
				break;
			case 1:
				vertices.push_back(y);
				break;
			case 2:
				vertices.push_back(z);
				break;
			case 3:
				vertices.push_back(x);
				break;
			case 4:
				vertices.push_back(0.5);
				break;
			case 5:
				vertices.push_back(z);
				break;
			}
		}
	}

	//ciclo for para crear la circunferencia inferior
	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0:
				vertices.push_back(x);
				break;
			case 1:
				vertices.push_back(-0.5f);
				break;
			case 2:
				vertices.push_back(z);
				break;
			}
		}
	}

	//ciclo for para crear la circunferencia superior
	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0:
				vertices.push_back(x);
				break;
			case 1:
				vertices.push_back(0.5);
				break;
			case 2:
				vertices.push_back(z);
				break;
			}
		}
	}

	//Se generan los indices de los vértices
	for (i = 0; i < vertices.size(); i++) indices.push_back(i);

	//se genera el mesh del cilindro
	Mesh* cilindro = new Mesh();
	cilindro->CreateMeshGeometry(vertices, indices, vertices.size(), indices.size());
	meshList.push_back(cilindro);
}

//función para crear un cono
void CrearCono(int res, float R) {

	//constantes utilizadas en los ciclos for
	int n, i;
	//cálculo del paso interno en la circunferencia y variables que almacenarán cada coordenada de cada vértice
	GLfloat dt = 2 * PI / res, x, z, y = -0.5f;

	vector<GLfloat> vertices;
	vector<unsigned int> indices;

	//caso inicial para crear el cono
	vertices.push_back(0.0);
	vertices.push_back(0.5);
	vertices.push_back(0.0);

	//ciclo for para crear los vértices de la circunferencia del cono
	for (n = 0; n <= (res); n++) {
		x = R * cos((n)*dt);
		z = R * sin((n)*dt);
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0:
				vertices.push_back(x);
				break;
			case 1:
				vertices.push_back(y);
				break;
			case 2:
				vertices.push_back(z);
				break;
			}
		}
	}
	vertices.push_back(R * cos(0) * dt);
	vertices.push_back(-0.5);
	vertices.push_back(R * sin(0) * dt);


	for (i = 0; i < res + 2; i++) indices.push_back(i);

	//se genera el mesh del cono
	Mesh* cono = new Mesh();
	cono->CreateMeshGeometry(vertices, indices, vertices.size(), res + 2);
	meshList.push_back(cono);
}

//función para crear pirámide cuadrangular unitaria
void CrearPiramideCuadrangular()
{
	vector<unsigned int> piramidecuadrangular_indices = {
		0,3,4,
		3,2,4,
		2,1,4,
		1,0,4,
		0,1,2,
		0,2,4

	};
	vector<GLfloat> piramidecuadrangular_vertices = {
		0.5f,-0.5f,0.5f,
		0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f,0.5f,
		0.0f,0.5f,0.0f,
	};
	Mesh* piramide = new Mesh();
	piramide->CreateMeshGeometry(piramidecuadrangular_vertices, piramidecuadrangular_indices, 15, 18);
	meshList.push_back(piramide);
}



void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

}


int main()
{
	mainWindow = Window(800, 600);
	mainWindow.Initialise();

	CrearCubo();                   
	CrearPiramideTriangular();     
	CrearCilindro(5, 1.0f);        
	CrearCono(25, 2.0f);           
	CrearPiramideCuadrangular();   
	CreateShaders();

	camera = Camera(
		glm::vec3(0.0f, 0.0f, 0.0f),   
		glm::vec3(0.0f, 1.0f, 0.0f),     
		-60.0f,                         
		0.0f,                          
		0.3f, 0.3f                       
	);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformColor = 0;

	glm::mat4 projection = glm::perspective(
		glm::radians(60.0f),
		(GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(),
		0.1f, 100.0f
	);

	// Esfera para articulaciones
	sp.init();
	sp.load();

	// Matrices de modelo
	glm::mat4 model(1.0f), modelaux(1.0f);

	// Color inicial
	glm::vec3 color(0.0f);

	// ÁNGULO DE LA CANASTA 
	float anguloCanastaDeg = 0.0f;          // grados
	const float giroSpeedDeg = 10.0f;       // grados/seg 

	while (!mainWindow.getShouldClose())
	{
		// Tiempo
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		// Entrada de usuario (teclado/mouse)
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Control de CANASTA: J (izq) / K (der) 
		{
			const bool pressJ = mainWindow.getsKeys()[GLFW_KEY_J];
			const bool pressK = mainWindow.getsKeys()[GLFW_KEY_K];
			if (pressJ) anguloCanastaDeg -= giroSpeedDeg * deltaTime;
			if (pressK) anguloCanastaDeg += giroSpeedDeg * deltaTime;
		}

		// Limpieza de buffers
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Activar shader y ubicar uniforms
		shaderList[0].useShader();
		uniformModel = shaderList[0].getModelLocation();
		uniformProjection = shaderList[0].getProjectLocation();
		uniformView = shaderList[0].getViewLocation();
		uniformColor = shaderList[0].getColorLocation();

		// Proyección y vista (una vez por frame)
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

		// BASE (caja de la máquina) 
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 5.5f, -4.0f));     
		modelaux = model;                                             
		model = glm::scale(model, glm::vec3(5.0f, 3.0f, 3.0f));     
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 0.85f, 0.0f);                            
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		meshList[0]->RenderMesh();

		// ARTICULACIÓN 1 
		model = modelaux;
		model = glm::rotate(model, glm::radians(mainWindow.getarticulacion1()), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 0.0f, 0.0f);                             
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// PRIMER BRAZO 
		model = glm::rotate(model, glm::radians(135.0f), glm::vec3(0.0f, 0.0f, 1.0f)); 
		model = glm::translate(model, glm::vec3(2.5f, 0.0f, 0.0f));               
		modelaux = model;                                                        
		{
			glm::mat4 m = model;
			m = glm::scale(m, glm::vec3(5.0f, 1.0f, 1.0f));                       
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
			color = glm::vec3(1.0f, 0.85f, 0.0f);                                   
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			meshList[0]->RenderMesh();
		}

		// ARTICULACIÓN 2
		model = modelaux;
		model = glm::translate(model, glm::vec3(2.5f, 0.0f, 0.0f));                    
		model = glm::rotate(model, glm::radians(mainWindow.getarticulacion2()), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 0.0f, 0.0f);                                          
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// SEGUNDO BRAZO
		model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));               
		modelaux = model;                                                           
		{
			glm::mat4 m = model;
			m = glm::scale(m, glm::vec3(1.0f, 5.0f, 1.0f));                         
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
			color = glm::vec3(1.0f, 0.85f, 0.0f);                            
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			meshList[0]->RenderMesh();
		}

		// ARTICULACIÓN 3
		model = modelaux;
		model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));                
		model = glm::rotate(model, glm::radians(mainWindow.getarticulacion3()), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		color = glm::vec3(1.0f, 0.0f, 0.0f);                                         
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// TERCER BRAZO 
		model = glm::rotate(model, glm::radians(135.0f), glm::vec3(0.0f, 0.0f, 1.0f));  
		model = glm::translate(model, glm::vec3(2.5f, 0.0f, 0.0f));                 
		glm::mat4 model_brazo3 = model;                                            
		{
			glm::mat4 m = model_brazo3;
			m = glm::scale(m, glm::vec3(5.0f, 1.0f, 1.0f));                         
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
			color = glm::vec3(1.0f, 0.85f, 0.0f);                                   
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			meshList[0]->RenderMesh();
		}

		// ARTICULACIÓN 4
		glm::mat4 model_art4 = model_brazo3;
		model_art4 = glm::translate(model_art4, glm::vec3(2.5f, 0.0f, 0.0f));          
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model_art4));
		color = glm::vec3(1.0f, 0.0f, 0.0f);                                       
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		sp.render();

		// CANASTA
		{
			glm::mat4 m = model_art4;
			m = glm::translate(m, glm::vec3(1.7f, 0.0f, 0.0f));          // pequeño voladizo delante de la ultima articulación

			// Giro “a izquierda/derecha” sobre el eje vertical Y
			m = glm::rotate(m, glm::radians(anguloCanastaDeg), glm::vec3(0.0f, 1.0f, 0.0f));

			// Tamaño de la canasta
			m = glm::scale(m, glm::vec3(3.8f, 2.6f, 2.6f));

			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
			color = glm::vec3(0.92f, 0.92f, 0.92f);                      
			glUniform3fv(uniformColor, 1, glm::value_ptr(color));
			meshList[0]->RenderMesh();              
		}
		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}//FIN FIN FIN :)
