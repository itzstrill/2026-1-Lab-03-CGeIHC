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
//MODIFICACION PERSONAL PARA UN NUEVO TIPO DE RUEDA
void CrearCilindroSolido(int res, float R) {
	if (res < 3) res = 3;

	const float h = 1.0f;     // altura total
	const float y0 = -0.5f;    // base
	const float y1 = 0.5f;    // tapa
	const float dt = 2.0f * PI / float(res);

	std::vector<GLfloat> vertices;   // posiciones (x,y,z)
	std::vector<unsigned int> idx;   // indices para GL_TRIANGLES

	// 0 = centro inferior, 1 = centro superior
	vertices.push_back(0.0f); vertices.push_back(y0); vertices.push_back(0.0f); // 0
	vertices.push_back(0.0f); vertices.push_back(y1); vertices.push_back(0.0f); // 1

	// anillo inferior (res+1 para cerrar costura)
	int baseStart = 2;
	for (int i = 0; i <= res; ++i) {
		float a = i * dt;
		float x = R * cosf(a);
		float z = R * sinf(a);
		vertices.push_back(x); vertices.push_back(y0); vertices.push_back(z);
	}

	// anillo superior (res+1)
	int topStart = baseStart + (res + 1);
	for (int i = 0; i <= res; ++i) {
		float a = i * dt;
		float x = R * cosf(a);
		float z = R * sinf(a);
		vertices.push_back(x); vertices.push_back(y1); vertices.push_back(z);
	}

	// pared lateral (dos triangulos por segmento)
	for (int i = 0; i < res; ++i) {
		unsigned int b0 = baseStart + i;
		unsigned int b1 = baseStart + i + 1;
		unsigned int t0 = topStart + i;
		unsigned int t1 = topStart + i + 1;

		// b0 t0 b1
		idx.push_back(b0); idx.push_back(t0); idx.push_back(b1);
		// b1 t0 t1
		idx.push_back(b1); idx.push_back(t0); idx.push_back(t1);
	}

	// tapa inferior (fan de triangulos)
	for (int i = 0; i < res; ++i) {
		unsigned int b0 = baseStart + i;
		unsigned int b1 = baseStart + i + 1;
		idx.push_back(0);   // centro inferior
		idx.push_back(b1);
		idx.push_back(b0);
	}

	// tapa superior (fan de triangulos)
	for (int i = 0; i < res; ++i) {
		unsigned int t0 = topStart + i;
		unsigned int t1 = topStart + i + 1;
		idx.push_back(1);   // centro superior
		idx.push_back(t0);
		idx.push_back(t1);
	}

	Mesh* m = new Mesh();
	// Importante: RenderMesh debe dibujar con GL_TRIANGLES
	m->CreateMeshGeometry(vertices, idx, (unsigned)vertices.size(), (unsigned)idx.size());
	meshList.push_back(m);
}


// Overlay de "tread": rectangulitos muy delgados alrededor de la pared lateral.
// Se dibuja con color distinto encima del cilindro solido para ver el giro.
// parametros:
//   res: segmentos del cilindro (igual que el solido, p.ej. 64..128)
//   R:   radio base del cilindro
//   every: cada cuantos segmentos pongo una barra (1 = en todos, 2 = uno si/uno no, etc.)
//   width: ancho angular relativo de cada barra (0..1, siendo 1 el ancho de un segmento)
//   extrude: factor radial extra para que no se "pegue" con el solido (p.ej. 0.02 = 2%)
void CrearCilindroTreadOverlay(int res, float R, int every = 2, float width = 0.6f, float extrude = 0.02f) {
	if (res < 3) res = 3;
	if (every < 1) every = 1;
	if (width < 0.05f) width = 0.05f;
	if (width > 1.0f)  width = 1.0f;

	const float y0 = -0.5f, y1 = 0.5f;
	const float dt = 2.0f * PI / float(res);
	const float R2 = R * (1.0f + extrude);     // un poco mas grande que el solido
	const float halfFrac = 0.5f * width;       

	std::vector<GLfloat> vertices;   // posiciones
	std::vector<unsigned int> idx;   // triangulos

	// Recorremos los segmentos y, para los que seleccionemos, generamos una "barra" fina:
	// Es un rectangulo sobre la pared (dos triangulos), extruido radialmente (R2),
	// con altura completa (y de -0.5 a 0.5) y ancho angular reducido.
	for (int i = 0; i < res; ++i) {
		if ((i % every) != 0) continue; 

		float aCenter = (i + 0.5f) * dt;              
		float a0 = aCenter - halfFrac * dt;           
		float a1 = aCenter + halfFrac * dt;         

		// 4 vertices (dos en y0, dos en y1)
		unsigned int base = (unsigned int)(vertices.size() / 3);

		// v0: (a0, y0)
		vertices.push_back(R2 * cosf(a0)); vertices.push_back(y0); vertices.push_back(R2 * sinf(a0));
		// v1: (a1, y0)
		vertices.push_back(R2 * cosf(a1)); vertices.push_back(y0); vertices.push_back(R2 * sinf(a1));
		// v2: (a0, y1)
		vertices.push_back(R2 * cosf(a0)); vertices.push_back(y1); vertices.push_back(R2 * sinf(a0));
		// v3: (a1, y1)
		vertices.push_back(R2 * cosf(a1)); vertices.push_back(y1); vertices.push_back(R2 * sinf(a1));

		// dos triangulos: v0 v2 v1  y  v1 v2 v3
		idx.push_back(base + 0); idx.push_back(base + 2); idx.push_back(base + 1);
		idx.push_back(base + 1); idx.push_back(base + 2); idx.push_back(base + 3);
	}

	Mesh* tread = new Mesh();
	tread->CreateMeshGeometry(vertices, idx, (unsigned)vertices.size(), (unsigned)idx.size());
	meshList.push_back(tread);
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

    // Raster
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Meshes (indices en meshList)
    CrearCubo();                         // [0] cubo
    CrearPiramideTriangular();           // [1] no usada
    CrearCilindroSolido(96, 1.0f);       // [2] cilindro solido (ruedas)
    CrearCilindroTreadOverlay(96, 1.0f, 2, 0.6f, 0.02f); // [3] overlay de barras
    CrearCono(25, 2.0f);                 // [4]
    CrearPiramideCuadrangular();         // [5] piramide (chasis)
    CreateShaders();

    // Camara
    camera = Camera(
        glm::vec3(10.0f, 8.0f, -4.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -90.0f, -10.0f, 0.3f, 0.3f
    );

    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformColor = 0;
    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(),
        0.1f, 100.0f
    );

    // esfera (articulaciones)
    sp.init(); sp.load();

    glm::mat4 model(1.0f), modelaux(1.0f);
    glm::vec3 color(0.0f);

    // Estados y velocidades
    float anguloCanastaDeg = 0.0f;     // canasta (al final de los brazos)
    const float canastaSpeedDeg = 15.0f;  // mas lento

    float ruedaDeg[4] = { 0,0,0,0 };     // Z X C V
    const float ruedaSpeedDeg = 60.0f;

    float gruaXDeg = 0.0f;             // NUEVO: giro de la grua sobre X en la articulacion 1
    const float gruaXSpeedDeg = 20.0f; // lento

    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        // Controles
        {
            auto keys = mainWindow.getsKeys();

            // canasta mas lenta
            if (keys[GLFW_KEY_J]) anguloCanastaDeg -= canastaSpeedDeg * deltaTime;
            if (keys[GLFW_KEY_K]) anguloCanastaDeg += canastaSpeedDeg * deltaTime;

            // ruedas (todas sobre eje Y)
            if (keys[GLFW_KEY_Z]) ruedaDeg[0] -= ruedaSpeedDeg * deltaTime;
            if (keys[GLFW_KEY_X]) ruedaDeg[1] -= ruedaSpeedDeg * deltaTime;
            if (keys[GLFW_KEY_C]) ruedaDeg[2] -= ruedaSpeedDeg * deltaTime;
            if (keys[GLFW_KEY_V]) ruedaDeg[3] -= ruedaSpeedDeg * deltaTime;

            // NUEVO: giro de la grua completa (excepto chasis/ruedas) sobre X con N/M
            if (keys[GLFW_KEY_N]) gruaXDeg -= gruaXSpeedDeg * deltaTime; // izquierda
            if (keys[GLFW_KEY_M]) gruaXDeg += gruaXSpeedDeg * deltaTime; // derecha
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderList[0].useShader();
        uniformModel = shaderList[0].getModelLocation();
        uniformProjection = shaderList[0].getProjectLocation();
        uniformView = shaderList[0].getViewLocation();
        uniformColor = shaderList[0].getColorLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

        // Root comun (posicion del vehiculo)
        glm::mat4 root = glm::mat4(1.0f);
        root = glm::translate(root, glm::vec3(0.0f, 5.5f, -4.0f));

        // Transform para la grua que SI gira: base/cabina + articulaciones + brazos + canasta
        glm::mat4 grua = root;
        grua = glm::rotate(grua, glm::radians(gruaXDeg), glm::vec3(0, 1, 0)); // pivot en articulacion 1 (centro de base)

        // ===== BASE (cabina) - gira con la grua =====
        {
            glm::mat4 m = grua;
            m = glm::scale(m, glm::vec3(5.0f, 3.0f, 3.0f)); // 5x3x3
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
            color = glm::vec3(1.0f, 0.85f, 0.0f);
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));
            meshList[0]->RenderMesh();
        }

        // ===== CHASIS (piramide) 
        {
            const float chasisY = -1.8f;                 // antes -2.2f
            const glm::vec3 chasisScale(5.0f, 3.2f, 3.0f);

            glm::mat4 m = root;                          // usar root (sin rotacion de grua)
            m = glm::translate(m, glm::vec3(0.0f, chasisY, 0.0f));
            m = glm::scale(m, chasisScale);

            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
            color = glm::vec3(0.80f, 0.80f, 0.85f);
            glUniform3fv(uniformColor, 1, glm::value_ptr(color));
            meshList[5]->RenderMesh();
        }

       
        {
     
            const float chasisY = -1.8f;   // centro del chasis (piramide)
            const float chasisScaleY = 3.2f;   // altura de la piramide

            // base inferior de la piramide
            const float yApex = chasisY + 0.5f * chasisScaleY;   // punta superior
            const float yBase = chasisY - 0.5f * chasisScaleY;   // base inferior

            
            const float yW = yBase;                

            // radio y grosor (a lo largo del eje X tras la rotacion -90Z)
            const float r = 1.0f;
            const float t = 0.6f;

            // separacion minima para que no se “meta” en el chasis
            const float gapX = 0.06f;              // empuje extra hacia afuera en ±X

            // medio ancho/profundidad de la base del chasis
            const float baseHalfX = 2.5f;          // 5.0 / 2
            const float baseHalfZ = 1.5f;          // 3.0 / 2

            // como estamos en yW = yBase, la seccion del chasis ahi es la base completa
            const float hxCur = baseHalfX;
            const float hzCur = baseHalfZ;

            struct Wheel {
                glm::vec3 baseCenter;  // punto de contacto con el chasis (sin empuje)
                float     signX;       // +1 derecha, -1 izquierda
                int       idx;         
            } wheels[4] = {
                { glm::vec3(-hxCur, yW, -hzCur), -1.0f, 0 },  // frente izq
                { glm::vec3(hxCur, yW, -hzCur), +1.0f, 1 },  // frente der
                { glm::vec3(-hxCur, yW,  hzCur), -1.0f, 2 },  // trasera izq
                { glm::vec3(hxCur, yW,  hzCur), +1.0f, 3 }   // trasera der
            };

            for (const auto& wdef : wheels)
            {
                glm::mat4 w = root; // sin rotacion de grua

                // empuje t/2 para alinear la tapa interna + un gap extra
                glm::vec3 center = wdef.baseCenter + glm::vec3(wdef.signX * (t * 0.5f + gapX), 0.0f, 0.0f);
                w = glm::translate(w, center);

                // eje del cilindro -> X
                w = glm::rotate(w, glm::radians(-90.0f), glm::vec3(0, 0, 1));

                // rodado (todas sobre Y)
                w = glm::rotate(w, glm::radians(ruedaDeg[wdef.idx]), glm::vec3(0, 1, 0));

                // escala: radio y grosor
                glm::mat4 wScale = glm::scale(w, glm::vec3(r, t, r));

                // cilindro solido
                glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(wScale));
                color = glm::vec3(0.12f, 0.12f, 0.12f);
                glUniform3fv(uniformColor, 1, glm::value_ptr(color));
                meshList[2]->RenderMesh();

                // overlay de barras
                glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(wScale));
                color = glm::vec3(0.35f, 0.35f, 0.35f);
                glUniform3fv(uniformColor, 1, glm::value_ptr(color));
                meshList[3]->RenderMesh();
            }
        }

        // ===== ARTICULACION 1 =====
        // parte de 'grua' (que ya incluye el giro global gruaXDeg)
        // y AQUI sumamos el giro local de la articulacion 1 (tecla F)
        glm::mat4 model_art1 = grua;
        model_art1 = glm::rotate(model_art1,
            glm::radians(mainWindow.getarticulacion1()),
            glm::vec3(0, 0, 1));   // mismo eje que usabas antes (Z)
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model_art1));
        color = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        sp.render();


        // ===== PRIMER BRAZO =====
        model = model_art1;  // <--- importante: usar el transform de la articulacion 1
        model = glm::rotate(model, glm::radians(135.0f), glm::vec3(0, 0, 1));
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

        // ===== ARTICULACION 2 =====
        model = modelaux;
        model = glm::translate(model, glm::vec3(2.5f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion2()), glm::vec3(0, 0, 1));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        sp.render();

        // ===== SEGUNDO BRAZO =====
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

        // ===== ARTICULACION 3 =====
        model = modelaux;
        model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));
        model = glm::rotate(model, glm::radians(mainWindow.getarticulacion3()), glm::vec3(0, 0, 1));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        sp.render();

        // ===== TERCER BRAZO =====
        model = glm::rotate(model, glm::radians(135.0f), glm::vec3(0, 0, 1));
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

        // ===== ARTICULACION 4 =====
        glm::mat4 model_art4 = model_brazo3;
        model_art4 = glm::translate(model_art4, glm::vec3(2.5f, 0.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model_art4));
        color = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        sp.render();

        // ===== CANASTA (mas lenta) =====
        {
            glm::mat4 m = model_art4;
            m = glm::translate(m, glm::vec3(1.7f, 0.0f, 0.0f));
            m = glm::rotate(m, glm::radians(anguloCanastaDeg), glm::vec3(0, 1, 0));
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
}
