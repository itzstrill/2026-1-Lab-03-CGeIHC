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
const float toRadians = 3.14159265f/180.0; //grados a radianes
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


// ====== ROBOT CAT: estados ======
struct Joint1D { float ang = 0.0f, minDeg, maxDeg, speedDeg; };
struct Leg { Joint1D hip, knee; };

static Leg legs[4];               // 0=FL,1=FR,2=BL,3=BR
static Joint1D tail[3];           // cola: base, seg2, seg3

static float headYaw = 0.0f, headPitch = 0.0f;

static inline float clampf(float v, float a, float b) { return glm::min(glm::max(v, a), b); }

// Inicializa rangos/velocidades
static void initRobotCat()
{
	auto setJ = [](Joint1D& j, float minD, float maxD, float speed) { j.minDeg = minD; j.maxDeg = maxD; j.speedDeg = speed; j.ang = 0.0f; };

	for (int i = 0; i < 4; ++i) {
		setJ(legs[i].hip, -40.0f, +30.0f, 45.0f);   // cadera
		setJ(legs[i].knee, 0.0f, +70.0f, 60.0f);  // rodilla (solo flexiona hacia adelante)
	}
	setJ(tail[0], -35.0f, +35.0f, 50.0f);
	setJ(tail[1], -35.0f, +35.0f, 50.0f);
	setJ(tail[2], -35.0f, +35.0f, 50.0f);
}



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
			x = R * cos((0)*dt);
			z = R * sin((0)*dt);
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
	Mesh *cilindro = new Mesh();
	cilindro->CreateMeshGeometry(vertices, indices, vertices.size(), indices.size());
	meshList.push_back(cilindro);
}

//función para crear un cono
void CrearCono(int res,float R) {

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


	for (i = 0; i < res+2; i++) indices.push_back(i);

	//se genera el mesh del cono
	Mesh *cono = new Mesh();
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
	Mesh *piramide = new Mesh();
	piramide->CreateMeshGeometry(piramidecuadrangular_vertices, piramidecuadrangular_indices, 15, 18);
	meshList.push_back(piramide);
}



void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

}

int main()
{
    // ====== Inicialización ======
    mainWindow = Window(800, 600);
    mainWindow.Initialise();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // ====== Geometrías ======
    CrearCubo();                       // [0] cubo
    CrearPiramideTriangular();         // [1] orejas
    CrearCilindro(96, 1.0f);           // [2] cuello
    CreateShaders();

    // ====== Cámara ======
    camera = Camera(
        glm::vec3(6.5f, 4.5f, 6.5f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -135.0f, -15.0f, 0.3f, 0.3f
    );

    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformColor = 0;
    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(),
        0.1f, 100.0f
    );

    // ====== Esfera (articulaciones) 
    sp.init(); sp.load();

    // ====== Estados (declarados arriba) 
    initRobotCat();

    // Patas: mismas restricciones pero velocidades MÁS LENTAS
    for (int i = 0; i < 4; ++i) {
        legs[i].hip.minDeg = -45.0f;
        legs[i].hip.maxDeg = +45.0f;
        legs[i].hip.speedDeg = 25.0f;   

        legs[i].knee.maxDeg = 85.0f;
        legs[i].knee.speedDeg = 30.0f;  
    }

    // Cola: rango amplio 
    for (int i = 0; i < 3; ++i) {
        tail[i].minDeg = -150.0f;
        tail[i].maxDeg = +150.0f;
        tail[i].speedDeg = 45.0f;      
    }

    // Orejas y cabeza (velocidades reducidas)
    float earTiltDeg = 0.0f;
    const float earMin = -25.0f, earMax = +25.0f, earSpeed = 20.0f; // antes 40

    const float headYawSpeed = 25.0f; //
    const float headPitchSpeed = 18.0f; // 
    const float headYawMin = -35.0f, headYawMax = +35.0f;
    const float headPitMin = -15.0f, headPitMax = +20.0f;

    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        // -------- Controles (SOLO LETRAS) --------
        {
            auto keys = mainWindow.getsKeys();

            // Orejas: E (-), R (+)
            if (keys[GLFW_KEY_E]) earTiltDeg = clampf(earTiltDeg - earSpeed * deltaTime, earMin, earMax);
            if (keys[GLFW_KEY_R]) earTiltDeg = clampf(earTiltDeg + earSpeed * deltaTime, earMin, earMax);

            // Cabeza: Yaw (T -, Y +)  |  Pitch (U -, I +)
            if (keys[GLFW_KEY_T]) headYaw = clampf(headYaw - headYawSpeed * deltaTime, headYawMin, headYawMax);
            if (keys[GLFW_KEY_Y]) headYaw = clampf(headYaw + headYawSpeed * deltaTime, headYawMin, headYawMax);
            if (keys[GLFW_KEY_U]) headPitch = clampf(headPitch - headPitchSpeed * deltaTime, headPitMin, headPitMax);
            if (keys[GLFW_KEY_I]) headPitch = clampf(headPitch + headPitchSpeed * deltaTime, headPitMin, headPitMax);

            // PATAS: {Z,X}{C,V}{B,N}{M,L} => FL, FR, BL, BR
            auto moveLegPair = [&](int i, bool negKey, bool posKey)
                {
                    if (negKey) {
                        legs[i].hip.ang = clampf(legs[i].hip.ang - legs[i].hip.speedDeg * deltaTime, legs[i].hip.minDeg, legs[i].hip.maxDeg);
                        legs[i].knee.ang = clampf(legs[i].knee.ang - legs[i].knee.speedDeg * deltaTime, legs[i].knee.minDeg, legs[i].knee.maxDeg);
                    }
                    if (posKey) {
                        legs[i].hip.ang = clampf(legs[i].hip.ang + legs[i].hip.speedDeg * deltaTime, legs[i].hip.minDeg, legs[i].hip.maxDeg);
                        legs[i].knee.ang = clampf(legs[i].knee.ang + legs[i].knee.speedDeg * deltaTime, legs[i].knee.minDeg, legs[i].knee.maxDeg);
                    }
                };
            moveLegPair(0, keys[GLFW_KEY_Z], keys[GLFW_KEY_X]); // FL
            moveLegPair(1, keys[GLFW_KEY_C], keys[GLFW_KEY_V]); // FR
            moveLegPair(2, keys[GLFW_KEY_B], keys[GLFW_KEY_N]); // BL
            moveLegPair(3, keys[GLFW_KEY_M], keys[GLFW_KEY_L]); // BR

            // Cola 3 ejes "implícitos" (1,1,1) con control bidireccional: F/G, H/J, K/P
            if (keys[GLFW_KEY_F]) tail[0].ang = clampf(tail[0].ang - tail[0].speedDeg * deltaTime, tail[0].minDeg, tail[0].maxDeg);
            if (keys[GLFW_KEY_G]) tail[0].ang = clampf(tail[0].ang + tail[0].speedDeg * deltaTime, tail[0].minDeg, tail[0].maxDeg);
            if (keys[GLFW_KEY_H]) tail[1].ang = clampf(tail[1].ang - tail[1].speedDeg * deltaTime, tail[1].minDeg, tail[1].maxDeg);
            if (keys[GLFW_KEY_J]) tail[1].ang = clampf(tail[1].ang + tail[1].speedDeg * deltaTime, tail[1].minDeg, tail[1].maxDeg);
            if (keys[GLFW_KEY_K]) tail[2].ang = clampf(tail[2].ang - tail[2].speedDeg * deltaTime, tail[2].minDeg, tail[2].maxDeg);
            if (keys[GLFW_KEY_P]) tail[2].ang = clampf(tail[2].ang + tail[2].speedDeg * deltaTime, tail[2].minDeg, tail[2].maxDeg);
        }

        // -------- Limpieza y shader --------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderList[0].useShader();
        uniformModel = shaderList[0].getModelLocation();
        uniformProjection = shaderList[0].getProjectLocation();
        uniformView = shaderList[0].getViewLocation();
        uniformColor = shaderList[0].getColorLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

        glm::vec3 color(0.0f);

        // ======== ESCENA: GATO ROBOT 
        glm::mat4 root = glm::mat4(1.0f);
        root = glm::translate(root, glm::vec3(0.0f, 0.0f, -4.0f));

        // ---- Torso alargado (gris) ----
        glm::mat4 torso = root;
        {
            glm::mat4 m = torso;
            m = glm::translate(m, glm::vec3(0.0f, 0.60f, 0.0f));
            m = glm::scale(m, glm::vec3(0.9f, 0.7f, 1.6f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
            color = glm::vec3(0.62f, 0.62f, 0.65f);  // gris
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));
            meshList[0]->RenderMesh();
        }
        torso = glm::translate(torso, glm::vec3(0.0f, 0.60f, 0.0f));      // eje local

        // ---- Cuello (gris) ----
        glm::mat4 neck = torso;
        neck = glm::translate(neck, glm::vec3(0.0f, +0.42f, +0.75f));
        {
            glm::mat4 m = neck;
            m = glm::scale(m, glm::vec3(0.18f, 0.25f, 0.18f));           
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
            color = glm::vec3(0.60f, 0.60f, 0.63f);  // gris
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));
            meshList[2]->RenderMesh();
        }

        // ---- Cabeza (gris) ----
        glm::mat4 headM = neck;
        headM = glm::translate(headM, glm::vec3(0.0f, +0.20f, +0.06f));
        headM = glm::rotate(headM, glm::radians(headYaw), glm::vec3(0, 1, 0));
        headM = glm::rotate(headM, glm::radians(headPitch), glm::vec3(1, 0, 0)); 
        {
            glm::mat4 m = headM;
            m = glm::scale(m, glm::vec3(0.35f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
            color = glm::vec3(0.70f, 0.70f, 0.73f);  // gris claro
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));
            sp.render();
        }

        
        for (int sgn : { -1, +1 })
        {
            glm::mat4 e = headM;
            e = glm::translate(e, glm::vec3(0.18f * sgn, +0.30f, -0.02f));
            e = glm::rotate(e, glm::radians(sgn * earTiltDeg), glm::vec3(0, 0, 1));
            e = glm::scale(e, glm::vec3(0.18f, 0.25f, 0.18f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(e));
            color = glm::vec3(0.62f, 0.62f, 0.66f);  // gris
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));
            meshList[1]->RenderMesh();
        }

        // ---- Helper para dibujar una pata (gris) y articulaciones rojas ----
        auto drawLeg = [&](const glm::vec3& anchor, const Leg& L)
            {
                const float rJoint = 0.09f;
                const float thighY = 0.45f, thighXZ = 0.16f; 
                const float shinY = 0.42f, shinXZ = 0.14f;

                glm::mat4 hip = torso;
                hip = glm::translate(hip, anchor);

                // cadera (ROJO)
                {
                    glm::mat4 m = hip;
                    m = glm::scale(m, glm::vec3(rJoint));
                    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
                    glm::vec3 red(1.0f, 0.0f, 0.0f);
                    glUniform3fv(uniformColor, 1, glm::value_ptr(red));
                    sp.render();
                }
                // rotación en X
                hip = glm::rotate(hip, glm::radians(L.hip.ang), glm::vec3(1, 0, 0));

                // muslo (GRIS)
                glm::mat4 thigh = hip;
                thigh = glm::translate(thigh, glm::vec3(0.0f, -thighY * 0.5f, 0.0f));
                {
                    glm::mat4 m = thigh;
                    m = glm::scale(m, glm::vec3(thighXZ, thighY, thighXZ));
                    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
                    glm::vec3 gray(0.62f, 0.62f, 0.65f);
                    glUniform3fv(uniformColor, 1, glm::value_ptr(gray));
                    meshList[0]->RenderMesh();
                }

                // rodilla (ROJO)
                glm::mat4 knee = hip;
                knee = glm::translate(knee, glm::vec3(0.0f, -thighY, 0.0f));
                {
                    glm::mat4 m = knee;
                    m = glm::scale(m, glm::vec3(rJoint));
                    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
                    glm::vec3 red(1.0f, 0.0f, 0.0f);
                    glUniform3fv(uniformColor, 1, glm::value_ptr(red));
                    sp.render();
                }
                // rotación rodilla (X)
                knee = glm::rotate(knee, glm::radians(L.knee.ang), glm::vec3(1, 0, 0));

                // pantorrilla (GRIS)
                glm::mat4 shin = knee;
                shin = glm::translate(shin, glm::vec3(0.0f, -shinY * 0.5f, 0.0f));
                {
                    glm::mat4 m = shin;
                    m = glm::scale(m, glm::vec3(shinXZ, shinY, shinXZ));
                    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
                    glm::vec3 gray(0.62f, 0.62f, 0.65f);
                    glUniform3fv(uniformColor, 1, glm::value_ptr(gray));
                    meshList[0]->RenderMesh();
                }
            };

        // Anclajes de patas
        const glm::vec3 FL(+0.45f, 0.10f, +0.80f);
        const glm::vec3 FR(-0.45f, 0.10f, +0.80f);
        const glm::vec3 BL(+0.45f, 0.10f, -0.80f);
        const glm::vec3 BR(-0.45f, 0.10f, -0.80f);

        drawLeg(FL, legs[0]);
        drawLeg(FR, legs[1]);
        drawLeg(BL, legs[2]);
        drawLeg(BR, legs[3]);

        auto drawTailSeg = [&](glm::mat4 parent, float segLen, float segThick, float angDeg)->glm::mat4
            {
                // articulación (ROJO)
                {
                    glm::mat4 m = parent;
                    m = glm::scale(m, glm::vec3(0.08f));
                    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
                    glm::vec3 red(1.0f, 0.0f, 0.0f);
                    glUniform3fv(uniformColor, 1, glm::value_ptr(red));
                    sp.render();
                }
                // rotación en (1,1,1)
                parent = glm::rotate(parent, glm::radians(angDeg), glm::normalize(glm::vec3(1, 1, 1)));

                // segmento (GRIS)
                glm::mat4 seg = parent;
                seg = glm::translate(seg, glm::vec3(0.0f, 0.0f, -segLen * 0.5f));
                {
                    glm::mat4 m = seg;
                    m = glm::scale(m, glm::vec3(segThick, segThick, segLen));
                    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
                    glm::vec3 gray(0.62f, 0.62f, 0.65f);
                    glUniform3fv(uniformColor, 1, glm::value_ptr(gray));
                    meshList[0]->RenderMesh();
                }
                glm::mat4 tip = parent;
                tip = glm::translate(tip, glm::vec3(0.0f, 0.0f, -segLen));
                return tip;
            };

        glm::mat4 tailBase = torso;
        tailBase = glm::translate(tailBase, glm::vec3(0.0f, 0.25f, -0.85f));
        glm::mat4 t1 = drawTailSeg(tailBase, 0.38f, 0.12f, tail[0].ang);
        glm::mat4 t2 = drawTailSeg(t1, 0.36f, 0.12f, tail[1].ang);
        drawTailSeg(t2, 0.34f, 0.12f, tail[2].ang);


        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}
