// práctica 3: Modelado Geométrico y Cámara Sintética.
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
#include <gtc\random.hpp>

#include "Mesh.h"
#include "Shader.h"
#include "Sphere.h"
#include "Window.h"
#include "Camera.h"

// Dimensiones / tiempo
using std::vector;
const float PI = 3.14159265f;
GLfloat deltaTime = 0.0f, lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// Globales existentes
Camera camera;
Window mainWindow;
vector<Mesh*> meshList;
vector<Shader> shaderList;

// Shaders
static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
static const char* vShaderColor = "shaders/shadercolor.vert";

// Esfera (ventana circular trasera) / Es esfera o circulo?
Sphere sp = Sphere(1.0, 20, 20); // radio, slices, stacks

// ---------- PRIMITIVAS ---------- PRIMITIVAS ---------- PRIMITIVAS ---------- 
void CrearCubo()
{
    unsigned int cubo_indices[] = {
        // front
        0, 1, 2,  2, 3, 0,
        // right
        1, 5, 6,  6, 2, 1,
        // back
        7, 6, 5,  5, 4, 7,
        // left
        4, 0, 3,  3, 7, 4,
        // bottom
        4, 5, 1,  1, 0, 4,
        // top
        3, 2, 6,  6, 7, 3
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

void CrearPiramideTriangular()
{
    unsigned int indices_p[] = { 0,1,2,  1,3,2,  3,0,2,  1,0,3 };
    GLfloat vertices_p[] = {
        -0.5f,-0.5f, 0.0f,   //0
         0.5f,-0.5f, 0.0f,   //1
         0.0f, 0.5f,-0.25f,  //2
         0.0f,-0.5f,-0.5f    //3
    };
    Mesh* obj1 = new Mesh();
    obj1->CreateMesh(vertices_p, indices_p, 12, 12);
    meshList.push_back(obj1);
}

// Cilindro
void CrearCilindro(int res, float R) {
    int n, i;
    GLfloat dt = 2 * PI / res, x, z, y = -0.5f;
    vector<GLfloat> vertices; vector<unsigned int> indices;

    for (n = 0; n <= res; n++) {
        if (n != res) { x = R * cos(n * dt); z = R * sin(n * dt); }
        else { x = R * cos(0);      z = R * sin(0); }
        for (i = 0; i < 6; i++) {
            switch (i) {
            case 0: vertices.push_back(x); break;
            case 1: vertices.push_back(y); break;
            case 2: vertices.push_back(z); break;
            case 3: vertices.push_back(x); break;
            case 4: vertices.push_back(0.5f); break;
            case 5: vertices.push_back(z); break;
            }
        }
    }
    for (n = 0; n <= res; n++) {
        x = R * cos(n * dt); z = R * sin(n * dt);
        vertices.push_back(x); vertices.push_back(-0.5f); vertices.push_back(z);
    }
    for (n = 0; n <= res; n++) {
        x = R * cos(n * dt); z = R * sin(n * dt);
        vertices.push_back(x); vertices.push_back(0.5f); vertices.push_back(z);
    }
    for (i = 0; i < (int)vertices.size(); i++) indices.push_back(i);

    Mesh* cilindro = new Mesh();
    cilindro->CreateMeshGeometry(vertices, indices, vertices.size(), indices.size());
    meshList.push_back(cilindro);
}

// Cono
void CrearCono(int res, float R) {
    int n, i; GLfloat dt = 2 * PI / res, x, z, y = -0.5f;
    vector<GLfloat> vertices; vector<unsigned int> indices;
    vertices.push_back(0.0f); vertices.push_back(0.5f); vertices.push_back(0.0f);
    for (n = 0; n <= res; n++) {
        x = R * cos(n * dt); z = R * sin(n * dt);
        vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
    }
    vertices.push_back(R * cos(0) * dt); vertices.push_back(-0.5f); vertices.push_back(R * sin(0) * dt);
    for (i = 0; i < res + 2; i++) indices.push_back(i);

    Mesh* cono = new Mesh();
    cono->CreateMeshGeometry(vertices, indices, vertices.size(), res + 2);
    meshList.push_back(cono);
}

// Pirámide cuadranda
void CrearPiramideCuadrangular()
{
    vector<unsigned int> idx = { 0,3,4, 3,2,4, 2,1,4, 1,0,4, 0,1,2, 0,2,4 };
    vector<GLfloat> v = {
         0.5f,-0.5f, 0.5f,
         0.5f,-0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f,
        -0.5f,-0.5f, 0.5f,
         0.0f, 0.5f, 0.0f
    };
    Mesh* p = new Mesh();
    p->CreateMeshGeometry(v, idx, 15, 18);
    meshList.push_back(p);
}

void CreateShaders()
{
    Shader* s1 = new Shader(); s1->CreateFromFiles(vShader, fShader);       shaderList.push_back(*s1);
    Shader* s2 = new Shader(); s2->CreateFromFiles(vShaderColor, fShader);  shaderList.push_back(*s2);
}

// -------------------- ESCENA CASA -------------------- ESCENA CASA -------------------- ESCENA CASA --------------------
int main()
{
    mainWindow = Window(800, 600);
    mainWindow.Initialise();

    CrearCubo();                  // idx 0
    CrearPiramideTriangular();    // idx 1 (no se usa, pero lo conservo)
    CrearCilindro(24, 0.5f);      // idx 2
    CrearCono(24, 1.0f);          // idx 3
    CrearPiramideCuadrangular();  // idx 4 (esta si se usa) XD
    CreateShaders();

    // Ajustes de camara
    camera = Camera(
        glm::vec3(0.0f, 4.0f, 10.0f),   // posición 
        glm::vec3(0.0f, 1.0f, 0.0f),   
        -60.0f, 0.0f,                 
        2.0f, 2.0f                      // moveSpeed, turnSpeed
    );


    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (float)mainWindow.getBufferWidth() / (float)mainWindow.getBufferHeight(), 0.1f, 100.0f);


    // Esfera (ventana trasera)
    sp.init();  // crea buffers
    sp.load();  // manda a GPU (usa el mismo shader de color)

    // Uniforms
    GLuint uModel = 0, uProj = 0, uView = 0, uColor = 0;

    // Colores
    const glm::vec3 COLOR_RED(0.92f, 0.20f, 0.17f); // casa
    const glm::vec3 COLOR_BLUE(0.05f, 0.20f, 0.95f); // techo y la ventana circular
    const glm::vec3 COLOR_GREEN(0.45f, 0.92f, 0.45f); // puertas/ventanas/arbol
    const glm::vec3 COLOR_BROWN(0.40f, 0.25f, 0.10f); // troncos
    const glm::vec3 COLOR_GRASS(0.20f, 0.55f, 0.20f); // piso

    float houseW = 6.0f, houseD = 6.0f, houseH = 4.0f, roofH = 3.0f;

    while (!mainWindow.getShouldClose())
    {
        // Timing + eventos
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime; deltaTime += (now - lastTime) / limitFPS; lastTime = now;
        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        // Clear
        glClearColor(0.85f, 0.90f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Shader
        shaderList[0].useShader();
        uModel = shaderList[0].getModelLocation();
        uProj = shaderList[0].getProjectLocation();
        uView = shaderList[0].getViewLocation();
        //uColor = shaderList[0].getColorLocation();

        // Proyección y vista (una vez por frame)
        glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));
        glm::mat4 view = camera.calculateViewMatrix();
        glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 model(1.0f);

        // PISO 
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.51f, -4.0f));
        model = glm::scale(model, glm::vec3(30.0f, 0.02f, 30.0f));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uColor, 1, glm::value_ptr(COLOR_GRASS));
        meshList[0]->RenderMesh();

        // CUERPO DE LA CASA
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, houseH / 2.0f, -4.0f));
        model = glm::scale(model, glm::vec3(houseW, houseH, houseD));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uColor, 1, glm::value_ptr(COLOR_RED));
        meshList[0]->RenderMesh();

        // TECHO
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, houseH + roofH / 2.0f, -4.0f));
        model = glm::scale(model, glm::vec3(houseW * 1.1f, roofH, houseD * 1.1f));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uColor, 1, glm::value_ptr(COLOR_BLUE));
        meshList[4]->RenderMeshGeometry();

        // PUERTA
        float doorW = 1.5f, doorH = 2.2f, doorT = 0.2f;
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, doorH / 2.0f, -4.0f + houseD / 2.0f + doorT / 2.0f + 0.01f));
        model = glm::scale(model, glm::vec3(doorW, doorH, doorT));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uColor, 1, glm::value_ptr(COLOR_GREEN));
        meshList[0]->RenderMesh();

        // VENTANAS
        float winW = 1.4f, winH = 1.4f, winT = 0.2f, winY = 2.8f, winX = 1.7f;
        for (int s : {-1, +1}) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(s * winX, winY, -4.0f + houseD / 2.0f + winT / 2.0f + 0.01f));
            model = glm::scale(model, glm::vec3(winW, winH, winT));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(uColor, 1, glm::value_ptr(COLOR_GREEN));
            meshList[0]->RenderMesh();
        }

        // VENTANAS 
        float sideZ = 1.7f;
        // Lado izquierdo
        for (int sZ : {-1, +1}) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-houseW / 2.0f - 0.01f - winT / 2.0f, winY, -4.0f + sZ * sideZ));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0)); // pegar al muro lateral
            model = glm::scale(model, glm::vec3(winW, winH, winT));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(uColor, 1, glm::value_ptr(COLOR_GREEN));
            meshList[0]->RenderMesh();
        }
        // Lado derecho
        for (int sZ : {-1, +1}) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(+houseW / 2.0f + 0.01f + winT / 2.0f, winY, -4.0f + sZ * sideZ));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
            model = glm::scale(model, glm::vec3(winW, winH, winT));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(uColor, 1, glm::value_ptr(COLOR_GREEN));
            meshList[0]->RenderMesh();
        }

        // VENTANA CIRCULAR TRASERA 
        float circR = 1.2f;
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.2f, -4.0f - houseD / 2.0f - 0.02f));
        model = glm::scale(model, glm::vec3(circR, circR, circR));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uColor, 1, glm::value_ptr(COLOR_BLUE));
        sp.render(); // usa el mismo shader activo

        // ÁRBOLES
        struct Tree { glm::vec3 p; };
        std::vector<Tree> trees = {
            { glm::vec3(-houseW / 2.0f - 2.2f, 0.0f, -4.0f + 2.0f) },
            { glm::vec3(+houseW / 2.0f + 2.2f, 0.0f, -4.0f + 2.0f) }
        };
        for (auto& t : trees) {
            // Tronco
            float trunkH = 1.2f, trunkR = 0.25f;
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(t.p.x, trunkH / 2.0f, t.p.z));
            model = glm::scale(model, glm::vec3(trunkR, trunkH, trunkR));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(uColor, 1, glm::value_ptr(COLOR_BROWN));
            meshList[2]->RenderMeshGeometry();

            // Copa
            float coneH = 1.6f, coneR = 0.9f;
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(t.p.x, trunkH + coneH / 2.0f, t.p.z));
            model = glm::scale(model, glm::vec3(coneR, coneH, coneR));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(uColor, 1, glm::value_ptr(COLOR_GREEN));
            meshList[3]->RenderMeshGeometry();
        }

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
} // Y FIN

		