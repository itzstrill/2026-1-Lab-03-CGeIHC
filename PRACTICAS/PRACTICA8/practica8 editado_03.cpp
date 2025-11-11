/*
Práctica 7: Iluminación 1
*/
//para cargar imagen
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
//para probar el importer
//#include<assimp/Importer.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include"Model.h"
#include "Skybox.h"

//para iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture pisoTexture;
Texture AgaveTexture;

Model Kitt_M;
Model Llanta_M;
Model Blackhawk_M;
Model Bugatti_M;
Model Bugatti_cofre;
Model Reflector_M;

// --- NUEVO: control del cofre del Bugatti y faros/spotlights del coche ---
float hoodAngleDeg = 0.0f;        // ángulo actual del cofre
float targetHoodAngleDeg = 0.0f;  // objetivo (0° cerrado, 90° abierto)
const float hoodSpeedDeg = 1.0f; // velocidad de animación (grados/segundo, se siente lenta/suave)

glm::vec3 bugattiBasePos = glm::vec3(12.0f, -1.0f, 17.0f); // misma base que ya usas al dibujar el bugatti
// Pivote local del cofre (bisagra). Ajusta finamente si no coincide perfecto con tu malla:
glm::vec3 hoodPivotLocal = glm::vec3(0.0f, 0.58f, -0.80f);
// Punto “punta del cofre” (para poner la luz amarilla). También puedes afinarlo:
glm::vec3 hoodTipLocal = hoodPivotLocal + glm::vec3(0.0f, 0.0f, 2.0f);

// Para detectar avance/retroceso del coche (muevex):
float prevMueveX = 0.0f;
bool forwardBeam = false; // avanzar = X negativa
bool reverseBeam = false; // retroceder = X positiva

Skybox skybox;

//materiales
Material Material_brillante;
Material Material_opaco;


//Sphere cabeza = Sphere(0.5, 20, 20);
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// luz direccional
DirectionalLight mainLight;
//para declarar varias luces de tipo pointlight
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

// Vertex Shader
static const char* vShader = "shaders/shader_light.vert";

// Fragment Shader
static const char* fShader = "shaders/shader_light.frag";


//función de calculo de normales por promedio de vértices 
void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}


void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	unsigned int vegetacionIndices[] = {
	   0, 1, 2,
	   0, 2, 3,
	   4,5,6,
	   4,6,7
	};

	GLfloat vegetacionVertices[] = {            //Esta ultima columna es la posicion en como le va a dar la luz a ese objeto
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,


	};

	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh* obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh* obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);

	Mesh* obj4 = new Mesh();
	obj4->CreateMesh(vegetacionVertices, vegetacionIndices, 64, 12);
	meshList.push_back(obj4);

	calcAverageNormals(indices, 12, vertices, 32, 8, 5);

	calcAverageNormals(vegetacionIndices, 12, vegetacionVertices, 64, 8, 5);

}


void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}



int main()
{
	mainWindow = Window(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();
	prevMueveX = mainWindow.getmuevex();


	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.5f);

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();
	AgaveTexture = Texture("Textures/Agave.tga");
	AgaveTexture.LoadTextureA();

	Kitt_M = Model();
	Kitt_M.LoadModel("Models/kitt_optimizado.obj");
	Llanta_M = Model();
	Llanta_M.LoadModel("Models/llanta_optimizada.obj");
	Blackhawk_M = Model();
	Blackhawk_M.LoadModel("Models/uh60.obj");
	Bugatti_M = Model();
	Bugatti_M.LoadModel("Models/bugatti.obj");
	Bugatti_cofre = Model();
	Bugatti_cofre.LoadModel("Models/bugatti_cofre.obj");
	Reflector_M = Model();
	Reflector_M.LoadModel("Models/Reflector halo AdiXXioN.fbx");



	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	Material_brillante = Material(4.0f, 256);
	Material_opaco = Material(0.3f, 4);


	//luz direccional, sólo 1 y siempre debe de existir
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.5f, 0.3f, // El primero es la intensidad ambiental (Luz del sol) y el segundo la difusa (inensidad de colo)
		0.0f, 0.0f, -1.0f); // Posicion de la luz direccional (La luz que da el sol)

	//contador de luces puntuales
	unsigned int pointLightCount = 0;
	//Declaración de primer luz puntual
	pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, // Su radiacion esta baja pero el color esta en lo mas intenso
		-6.0f, 1.5f, 1.5f, //La posicion en la que da la luz roja
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	//Segunda luz puntual (Reflector)
	pointLights[1] = PointLight(0.0f, 0.0f, 1.0f,
		0.0f, 0.1f,
		0.0f, 0.0f, -7.0f,
		1.0f, 0.22f, 0.22f);
	pointLightCount++;

	unsigned int spotLightCount = 0;
	/*
	//linterna
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		5.0f);
	spotLightCount++;

	spotLights[3] = SpotLight(
		1.0f, 0.0f, 0.0f,
		1.0f, 2.0f,         // intensidades: ambient, diffuse
		-10.0f, 0.0f, 0.0f, // posición (un poco arriba y al frente)
		0.0f, -5.0f, 0.0f, // dirección (hacia el piso/escena)
		0.0f, 0.0f, 1.0f,   // atenuación: con=0, lin=0, exp=0 (prácticamente se elimina)
		15.0f);               // edge (apertura del cono, grados)
	spotLightCount++;


	//luz fija (Esta es la luz verde)
	spotLights[1] = SpotLight(0.0f, 1.0f, 0.0f,
		1.0f, 2.0f,   //Aunque se psse de 1.0 el valor maximo sigue siendo 1
		5.0f, 10.0f, 0.0f,
		0.0f, -5.0f, 0.0f,
		1.0f, 0.0f, 0.0f, //El 1,0,0 es una luz infinita y eso no existe en la vida real
		15.0f); //El radio en el que hace efecto la luz 
	spotLightCount++;

	// --- FARO DEL COCHE
	spotLights[2] = SpotLight(
		0.0f, 0.0f, 1.0f,      // color (R,G,B)
		5.0f, 1.0f,           // intensidades: ambiental, difusa
		0.0f, 0.0f, 0.0f,      // posición (placeholder, se actualiza cada frame)
		0.0f, -1.0f, 0.0f,     // dirección (placeholder)
		1.0f, 0.022f, 0.0019f, // atenuación (alcance realista)
		18.0f                  // ángulo del cono (edge, en grados)
	);
	spotLightCount++;
	*/
	/*spotLights[3] = SpotLight(
		1.0f, 1.0f, 0.0f,      // color amarillo (R,G,B)
		4.0f, 1.0f,            // intensidades: ambiental, difusa
		0.0f, 0.0f, 0.0f,      // posición (se actualizará cada frame)
		0.0f, -1.0f, 0.0f,     // dirección hacia el piso
		1.0f, 0.022f, 0.0019f, // atenuación (alcance realista)
		20.0f                  // ángulo del cono (edge en grados)
	);
	spotLightCount++;
	*/
	// [0] se queda como linterna ligada a la cámara (no tocar)

// [1] LUZ DEL COFRE (amarilla, NO blanca/azul)
	spotLights[1] = SpotLight(
		1.0f, 0.95f, 0.2f,     // color (R,G,B) -> amarillo cálido
		0.0f, 1.2f,            // intensidades: ambient, diffuse
		0.0f, 0.0f, 0.0f,      // pos placeholder (se actualiza cada frame)
		0.0f, 0.0f, 0.0f,     // dir placeholder (se actualiza cada frame)
		1.0f, 0.022f, 0.0019f, // atenuación realista
		18.0f                  // edge/apertura
	);
	spotLightCount++;

	// [2] SPOTLIGHT ADELANTE (se enciende al avanzar, X negativa)
	spotLights[2] = SpotLight(
		1.0f, 1.0f, 1.0f,      // blanco
		0.0f, 2.0f,            // intensidades
		0.0f, 0.0f, 0.0f,      // pos placeholder
		0.0f, -1.0f, 0.0f,     // dir placeholder
		1.0f, 0.022f, 0.0019f, // atenuación
		16.0f
	);
	spotLightCount++;

	// [3] SPOTLIGHT ATRÁS (se enciende al retroceder, X positiva)
	spotLights[3] = SpotLight(
		1.0f, 1.0f, 1.0f,      // blanco (puedes poner rojizo si quieres “luz de reversa”)
		8.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.022f, 0.0019f,
		16.0f
	);
	spotLightCount++;


	//se crean mas luces puntuales y spotlight 

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);
	////Loop mientras no se cierra la ventana
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;


		//Recibir eventos del usuario
		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// --- NUEVO: control por teclas del cofre ---
// U = abrir (hasta 90°), O = cerrar (hasta 0°)
		if (mainWindow.getsKeys()[GLFW_KEY_O]) targetHoodAngleDeg = 90.0f;
		if (mainWindow.getsKeys()[GLFW_KEY_I]) targetHoodAngleDeg = 0.0f;

		// Interpolación suave (clamp) hacia el objetivo
		if (fabs(targetHoodAngleDeg - hoodAngleDeg) > 0.01f) {
			float step = hoodSpeedDeg * deltaTime;
			if (hoodAngleDeg < targetHoodAngleDeg) hoodAngleDeg = fminf(hoodAngleDeg + step, targetHoodAngleDeg);
			else                                   hoodAngleDeg = fmaxf(hoodAngleDeg - step, targetHoodAngleDeg);
		}

		// --- NUEVO: detectar dirección de movimiento en el eje X ---
		float curX = mainWindow.getmuevex();
		float dx = curX - prevMueveX;
		// “avanzar” = moverse hacia X negativa
		forwardBeam = (dx < -0.0005f);
		// “retroceder” = moverse hacia X positiva
		reverseBeam = (dx > 0.0005f);
		prevMueveX = curX;


		mainWindow.updateHeliMovement(deltaTime);

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();

		//información en el shader de intensidad especular y brillo
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		// luz ligada a la cámara de tipo flash
		//sirve para que en tiempo de ejecución (dentro del while) se cambien propiedades de la luz
		glm::vec3 lowerLight = camera.getCameraPosition();
		lowerLight.y -= 0.3f;
		spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

		//información al shader de fuentes de iluminación
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);

		/*
		// Seguimiento de la luz del faro del coche
		glm::mat4 M_car = glm::mat4(1.0f);
		M_car = glm::translate(M_car, glm::vec3(0.0f + mainWindow.getmuevex(), 0.5f, -3.0f));
		M_car = glm::scale(M_car, glm::vec3(0.5f));
		M_car = glm::rotate(M_car, -90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::vec3 headLocal = glm::vec3(47.0f, 0.0f, -13.0f); // faro delantero izquierdo
		glm::vec3 headPos = glm::vec3(M_car * glm::vec4(headLocal, 1.0f));

		// Dirección "hacia delante" del coche en espacio mundo.
		glm::vec3 headDir = glm::normalize(glm::vec3(M_car * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));

		// Actualiza posición + dirección del spotlight del faro
		spotLights[2].SetFlash(headPos, headDir);
		*/
		// Seguimiento de spotlights del coche (adelante/atrás)
		glm::mat4 M_car = glm::mat4(1.0f);
		M_car = glm::translate(M_car, glm::vec3(0.0f + mainWindow.getmuevex(), 0.5f, -3.0f));
		M_car = glm::scale(M_car, glm::vec3(0.5f));
		M_car = glm::rotate(M_car, -90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));

		// Posiciones locales de faro delantero y trasero (ajusta si tu malla no coincide)
		glm::vec3 headLocal = glm::vec3(40.0f, 0.0f, -13.0f);   // delantero
		glm::vec3 tailLocal = glm::vec3(-47.0f, 0.0f, -13.0f);  // trasero (estimado)

		// A mundo
		glm::vec3 headPos = glm::vec3(M_car * glm::vec4(headLocal, 1.0f));
		glm::vec3 tailPos = glm::vec3(M_car * glm::vec4(tailLocal, 1.0f));

		// Direcciones a mundo
		glm::vec3 forwardDir = glm::normalize(glm::vec3(M_car * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
		glm::vec3 backwardDir = -forwardDir;

		// Encendido según banderas
		if (forwardBeam) {
			// Enciende el de adelante
			// Si tu clase SpotLight tiene setters de intensidades:
			// spotLights[2].SetAmbientIntensity(0.1f);
			// spotLights[2].SetDiffuseIntensity(2.0f);
			spotLights[2].SetFlash(headPos, forwardDir);
		}
		else {
			// Apaga (si no tienes setters, deja un cono pequeño sin afectar)
			// spotLights[2].SetAmbientIntensity(0.0f);
			// spotLights[2].SetDiffuseIntensity(0.0f);
			spotLights[2].SetFlash(headPos, forwardDir);
		}

		if (reverseBeam) {
			// Enciende el de atrás
			// spotLights[3].SetAmbientIntensity(0.1f);
			// spotLights[3].SetDiffuseIntensity(2.0f);
			spotLights[3].SetFlash(tailPos, backwardDir);
		}
		else {
			// Apaga
			// spotLights[3].SetAmbientIntensity(0.0f);
			// spotLights[3].SetDiffuseIntensity(0.0f);
			spotLights[3].SetFlash(tailPos, backwardDir);
		}


		glm::mat4 model(1.0);
		glm::mat4 modelaux(1.0);
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);


		glm::mat4 M_heli = glm::mat4(1.0f);

		//Luz del helicóptero
		M_heli = glm::translate(M_heli, glm::vec3(0.0f + mainWindow.getHeliMoveZ(), 5.0f, 6.0f));
		M_heli = glm::scale(M_heli, glm::vec3(0.5f));
		M_heli = glm::rotate(M_heli, -90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		M_heli = glm::rotate(M_heli, 90.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));

		glm::vec3 heliCenter = glm::vec3(M_heli * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		glm::vec3 heliLightPos = heliCenter + glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 heliLightDir = glm::vec3(0.0f, -1.0f, 0.0f);
		spotLights[3].SetFlash(heliLightPos, heliLightDir);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		pisoTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

		meshList[2]->RenderMesh();

		//Instancia del coche 
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.5f, -3.0f));
		modelaux = model;
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Kitt_M.RenderModel();

		//Llanta delantera izquierda
		model = modelaux;
		model = glm::translate(model, glm::vec3(7.0f, -0.5f, 8.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		color = glm::vec3(0.5f, 0.5f, 0.5f);//llanta con color gris
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		//Llanta trasera izquierda
		model = modelaux;
		model = glm::translate(model, glm::vec3(15.5f, -0.5f, 8.0f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		//Llanta delantera derecha
		model = modelaux;
		model = glm::translate(model, glm::vec3(7.0f, -0.5f, 1.5f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();

		//Llanta trasera derecha
		model = modelaux;
		model = glm::translate(model, glm::vec3(15.5f, -0.5f, 1.5f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Llanta_M.RenderModel();


		model = glm::mat4(1.0);
		//model = glm::translate(model, glm::vec3(0.0f, 5.0f, 6.0));
		model = glm::translate(model, glm::vec3(0.0f + mainWindow.getHeliMoveZ(), 5.0f, 6.0f));
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		model = glm::rotate(model, -90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Blackhawk_M.RenderModel();

		//Reflector
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-4.5f, 4.5f, -7.0f));
		model = glm::scale(model, glm::vec3(7.0f, 7.0f, 7.0f));
		model = glm::rotate(model, -165 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, -45 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Reflector_M.RenderModel();
		/*
		//Bugatti
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(12.0f + mainWindow.getmuevex(), -1.0f, 17.0f));
		modelaux = model;
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Bugatti_M.RenderModel();

		//Bugatti_cofre
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(12.0f + mainWindow.getmuevex(), -1.0f, 17.0f));
		modelaux = model;
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		model = glm::rotate(model, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Bugatti_cofre.RenderModel();
		*/

		// --- BUGATTI (carrocería) tal cual lo tienes ---
		glm::mat4 M_bug = glm::mat4(1.0f);
		M_bug = glm::translate(M_bug, glm::vec3(bugattiBasePos.x + mainWindow.getmuevex(), bugattiBasePos.y, bugattiBasePos.z));
		glm::mat4 M_bug_aux = M_bug;
		M_bug = glm::scale(M_bug, glm::vec3(4.0f, 4.0f, 4.0f));
		M_bug = glm::rotate(M_bug, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(M_bug));
		Bugatti_M.RenderModel();

		// --- COFRE (rota sobre pivote y actualiza spotlight amarillo) ---
		glm::mat4 M_cofre = M_bug_aux;
		M_cofre = glm::scale(M_cofre, glm::vec3(4.0f, 4.0f, 4.0f));
		M_cofre = glm::rotate(M_cofre, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));

		// rotación local alrededor de la bisagra (eje Y, como pediste)
		M_cofre = glm::translate(M_cofre, hoodPivotLocal);
		M_cofre = glm::rotate(M_cofre, hoodAngleDeg * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		M_cofre = glm::translate(M_cofre, -hoodPivotLocal);

		// dibuja cofre
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(M_cofre));
		Bugatti_cofre.RenderModel();

		// spotlight del cofre: posición en la “punta” y dirección siguiendo la cara del cofre
		glm::vec3 hoodPos = glm::vec3(M_cofre * glm::vec4(1.0f, hoodTipLocal));
		glm::vec3 hoodDir = glm::normalize(glm::vec3(M_cofre * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f))); // “normal” hacia +Z local
		spotLights[1].SetFlash(hoodPos, hoodDir);


		//Agave ¿qué sucede si lo renderizan antes del coche y el helicóptero?
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 1.0f, -4.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

		//blending: transparencia o traslucidez
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		AgaveTexture.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[3]->RenderMesh();
		glDisable(GL_BLEND);

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
