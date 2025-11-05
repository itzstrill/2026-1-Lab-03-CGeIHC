// Práctica 2: índices, mesh, proyecciones, transformaciones geométricas
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <glew.h>
#include <glfw3.h>

// glm
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// clases
#include "Mesh.h"
#include "Shader.h"
#include "Window.h"

// util
const float toRadians = 3.14159265f / 180.0f;
Window mainWindow;

std::vector<Mesh*>      meshList;        // 0: pirámide, 1: cubo
std::vector<MeshColor*> meshColorList;   // 0: letras
std::vector<Shader>     shaderList;

// rutas existentes
static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
static const char* vShaderColor = "shaders/shadercolor.vert";
static const char* fShaderColor = "shaders/shadercolor.frag";

void agregarCuadradoRGB(GLfloat*& vertices, int& numFloats, float x, float y, float r, float g, float b, float tam) {
    const int floatsPorVert = 6;   // x y z r g b
    const int nuevosVerts = 6;   // 2 triángulos
    const int totalNuevos = nuevosVerts * floatsPorVert;

    GLfloat* temp = new GLfloat[numFloats + totalNuevos];
    for (int i = 0; i < numFloats; i++) temp[i] = vertices ? vertices[i] : 0.0f;

    float m = tam / 2.0f;
    float x0 = x - m, x1 = x + m;
    float y0 = y - m, y1 = y + m;
    float z = 0.0f;
    int idx = numFloats;

    // tri 1
    temp[idx++] = x0; temp[idx++] = y0; temp[idx++] = z; temp[idx++] = r; temp[idx++] = g; temp[idx++] = b;
    temp[idx++] = x1; temp[idx++] = y0; temp[idx++] = z; temp[idx++] = r; temp[idx++] = g; temp[idx++] = b;
    temp[idx++] = x1; temp[idx++] = y1; temp[idx++] = z; temp[idx++] = r; temp[idx++] = g; temp[idx++] = b;
    // tri 2
    temp[idx++] = x0; temp[idx++] = y0; temp[idx++] = z; temp[idx++] = r; temp[idx++] = g; temp[idx++] = b;
    temp[idx++] = x1; temp[idx++] = y1; temp[idx++] = z; temp[idx++] = r; temp[idx++] = g; temp[idx++] = b;
    temp[idx++] = x0; temp[idx++] = y1; temp[idx++] = z; temp[idx++] = r; temp[idx++] = g; temp[idx++] = b;

    if (vertices) delete[] vertices;
    vertices = temp;
    numFloats += totalNuevos;
}
inline void addBlock(GLfloat*& v, int& n, float ox, float oy, float tam, int ix, int iy, float r, float g, float b) {
    agregarCuadradoRGB(v, n, ox + ix * tam, oy + iy * tam, r, g, b, tam);
}

// ---------- tus geometrías (Mesh) ----------
void CreaPiramide() {
    unsigned int ind[] = { 0,1,2, 1,3,2, 3,0,2, 1,0,3 };
    GLfloat v[] = { -0.5f,-0.5f,0.0f,  0.5f,-0.5f,0.0f,  0.0f,0.5f,-0.25f,  0.0f,-0.5f,-0.5f };
    Mesh* p = new Mesh(); p->CreateMesh(v, ind, 12, 12); meshList.push_back(p);
}
void CrearCubo() {
    unsigned int idx[] = { 0,1,2,2,3,0, 1,5,6,6,2,1, 7,6,5,5,4,7, 4,0,3,3,7,4, 4,5,1,1,0,4, 3,2,6,6,7,3 };
    GLfloat v[] = { -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,
                   -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  -0.5f, 0.5f,-0.5f };
    Mesh* c = new Mesh(); c->CreateMesh(v, idx, 24, 36); meshList.push_back(c);
}

// ---------- letras A G E con MeshColor ----------
void CrearInicialesAGE() {
    GLfloat* V = nullptr; int N = 0;
    const float tam = 0.07f;     // más grande
    const float oy = -0.15f;
    const float oxA = -0.80f, oxG = -0.20f, oxE = 0.40f;
    const float Ar = 1.0f, Ag = 0.2f, Ab = 0.2f;
    const float Gr = 0.2f, Gg = 1.0f, Gb = 0.2f;
    const float Er = 0.2f, Eg = 0.5f, Eb = 1.0f;

    // A: columnas + barra superior + barra media (rejilla 7x9)
    for (int y = 0; y <= 8; ++y) { addBlock(V, N, oxA, oy, tam, 0, y, Ar, Ag, Ab); addBlock(V, N, oxA, oy, tam, 6, y, Ar, Ag, Ab); }
    for (int x = 0; x <= 6; ++x) { addBlock(V, N, oxA, oy, tam, x, 8, Ar, Ag, Ab); }
    for (int x = 1; x <= 5; ++x) { addBlock(V, N, oxA, oy, tam, x, 4, Ar, Ag, Ab); }

    // G: marco C + diente
    for (int x = 0; x <= 6; ++x) { addBlock(V, N, oxG, oy, tam, x, 8, Gr, Gg, Gb); addBlock(V, N, oxG, oy, tam, x, 0, Gr, Gg, Gb); }
    for (int y = 0; y <= 8; ++y) { addBlock(V, N, oxG, oy, tam, 0, y, Gr, Gg, Gb); }
    for (int y = 0; y <= 4; ++y) { addBlock(V, N, oxG, oy, tam, 6, y, Gr, Gg, Gb); }
    for (int x = 3; x <= 6; ++x) { addBlock(V, N, oxG, oy, tam, x, 4, Gr, Gg, Gb); }

    // E: columna izq + barras
    for (int y = 0; y <= 8; ++y) { addBlock(V, N, oxE, oy, tam, 0, y, Er, Eg, Eb); }
    for (int x = 0; x <= 6; ++x) { addBlock(V, N, oxE, oy, tam, x, 8, Er, Eg, Eb); addBlock(V, N, oxE, oy, tam, x, 0, Er, Eg, Eb); }
    for (int x = 0; x <= 4; ++x) { addBlock(V, N, oxE, oy, tam, x, 4, Er, Eg, Eb); }

    MeshColor* letras = new MeshColor();
    letras->CreateMeshColor(V, N);   // pos+color intercalados (6 floats/vert)  :contentReference[oaicite:2]{index=2}
    meshColorList.push_back(letras);
    if (V) { delete[] V; V = nullptr; }
}

// ---------- cargar shaders (solo 2: letras y casa) ----------
enum { SH_GEOM = 0, SH_LETTERS = 1 };

void CreateShaders() {
    Shader* sGeom = new Shader();    // casa (color por uniform)
    sGeom->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*sGeom);

    Shader* sLetters = new Shader(); // letras (color por vértice)
    sLetters->CreateFromFiles(vShaderColor, fShaderColor);
    shaderList.push_back(*sLetters);
}

int main() {
    mainWindow = Window(800, 600);
    mainWindow.Initialise();
    glEnable(GL_DEPTH_TEST);

    CreaPiramide();      // meshList[0]
    CrearCubo();         // meshList[1]
    CrearInicialesAGE(); // meshColorList[0]
    CreateShaders();

    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 100.0f);

    while (!mainWindow.getShouldClose()) {
        glfwPollEvents();
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        /*
        // ---- LETRAS ----
        shaderList[SH_LETTERS].useShader();
        {
            GLuint uModel = shaderList[SH_LETTERS].getModelLocation();
            GLuint uProj = shaderList[SH_LETTERS].getProjectLocation();
            glm::mat4 M(1.0f);
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));
            meshColorList[0]->RenderMeshColor(); // usa aPos(0) y aColor(1)  :contentReference[oaicite:3]{index=3}
        }
        */
       
        // ---- CASA (mismo shader, color uniforme con 'objectColor') ----
        shaderList[SH_GEOM].useShader();
        {
            GLuint uModel = shaderList[SH_GEOM].getModelLocation();
            GLuint uProj = shaderList[SH_GEOM].getProjectLocation();
            GLuint uCol = shaderList[SH_GEOM].getUniformLocation(); // "objectColor" en shader.frag
            glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));

            // Paleta similar a tu referencia
            const glm::vec3 COLOR_RED_BODY = { 0.93f, 0.18f, 0.18f }; // muro
            const glm::vec3 COLOR_BLUE_ROOF = { 0.00f, 0.25f, 1.00f }; // techo azul intenso
            const glm::vec3 COLOR_LIME = { 0.61f, 0.98f, 0.28f }; // ventanas y puerta (verde-lima)
            const glm::vec3 COLOR_DARK_GREEN = { 0.10f, 0.45f, 0.10f }; // copas
            const glm::vec3 COLOR_BROWN = { 0.35f, 0.27f, 0.17f }; // troncos
            const glm::vec3 COLOR_GRASS = { 0.00f, 0.70f, 0.15f }; // césped

            glm::mat4 M;

            // CÉSPED (fino y ancho, con margen)
            M = glm::mat4(1.0f);
            M = glm::translate(M, glm::vec3(0.0f, -0.92f, -3.1f));
            M = glm::scale(M, glm::vec3(2.0f, 0.06f, 0.6f));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            glUniform3fv(uCol, 1, &COLOR_GRASS[0]);
            meshList[1]->RenderMesh();

            // MURO (más compacto)
            // centro y=-0.20, alto=0.90 ? top ? 0.25 (deja espacio al techo)
            M = glm::mat4(1.0f);
            M = glm::translate(M, glm::vec3(0.0f, -0.20f, -3.0f));
            M = glm::scale(M, glm::vec3(1.10f, 0.90f, 0.12f));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            glUniform3fv(uCol, 1, &COLOR_RED_BODY[0]);
            meshList[1]->RenderMesh();

            // TECHO (pirámide) ajustado
            // base = top del muro (? 0.25); scale_y=0.60 ? apex ? 0.25 + 0.30 = 0.55
            M = glm::mat4(1.0f);
            M = glm::translate(M, glm::vec3(0.0f, 0.55f, -2.98f)); // un pelín al frente
            M = glm::scale(M, glm::vec3(1.40f, 0.60f, 0.20f));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            glUniform3fv(uCol, 1, &COLOR_BLUE_ROOF[0]);
            meshList[0]->RenderMesh();

            // PUERTA (centrada y más pequeña)
            M = glm::mat4(1.0f);
            M = glm::translate(M, glm::vec3(0.0f, -0.55f, -2.95f));
            M = glm::scale(M, glm::vec3(0.28f, 0.32f, 0.06f));
            glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(M));
            glUniform3fv(uCol, 1, &COLOR_LIME[0]);
            meshList[1]->RenderMesh();

            // VENTANAS (simétricas)
            auto drawWindow = [&](float x) {
                glm::mat4 W(1.0f);
                W = glm::translate(W, glm::vec3(x, 0.05f, -2.95f));
                W = glm::scale(W, glm::vec3(0.22f, 0.22f, 0.06f));
                glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(W));
                glUniform3fv(uCol, 1, &COLOR_LIME[0]);
                meshList[1]->RenderMesh();
                };
            drawWindow(-0.35f);
            drawWindow(0.35f);

            // ÁRBOLES (compactos y dentro del marco)
            auto drawTree = [&](float side) {
                // Tronco
                glm::mat4 T(1.0f);
                T = glm::translate(T, glm::vec3(0.90f * side, -0.78f, -2.95f));
                T = glm::scale(T, glm::vec3(0.12f, 0.18f, 0.06f));
                glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(T));
                glUniform3fv(uCol, 1, &COLOR_BROWN[0]);
                meshList[1]->RenderMesh();

                // Copa (base justo encima del tronco)
                glm::mat4 C(1.0f);
                C = glm::translate(C, glm::vec3(0.90f * side, -0.42f, -3.0f));
                C = glm::scale(C, glm::vec3(0.45f, 0.48f, 0.20f));
                glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(C));
                glUniform3fv(uCol, 1, &COLOR_DARK_GREEN[0]);
                meshList[0]->RenderMesh();
                };
            drawTree(-1.0f); // izq
            drawTree(1.0f); // der
        }
        

        glUseProgram(0);
        mainWindow.swapBuffers();
    }
    return 0;
}
