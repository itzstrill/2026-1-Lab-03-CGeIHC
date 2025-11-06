#include <stdio.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <glew.h>
#include <glfw3.h>

// Dimensiones de la ventana
const int WIDTH = 800, HEIGHT = 600;

// ===== Shaders =====
static const char* vtxSrc = R"(
#version 430 core
layout (location = 0) in vec2 aPos;
void main(){
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

static const char* fragSrc = R"(
#version 430 core
out vec4 FragColor;
uniform vec3 uColor;
void main(){
    FragColor = vec4(uColor, 1.0);
}
)";

// ===== Utilidades de shader/programa =====
static GLuint compile(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0; glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len);
        glGetShaderInfoLog(s, len, nullptr, log.data());
        printf("%s\n", log.data());
        glDeleteShader(s); return 0;
    }
    return s;
}
static GLuint makeProgram(const char* vs, const char* fs) {
    GLuint v = compile(GL_VERTEX_SHADER, vs);
    GLuint f = compile(GL_FRAGMENT_SHADER, fs);
    if (!v || !f) return 0;
    GLuint p = glCreateProgram();
    glAttachShader(p, v); glAttachShader(p, f);
    glLinkProgram(p);
    glDeleteShader(v); glDeleteShader(f);
    GLint ok = 0; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0; glGetProgramiv(p, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len);
        glGetProgramInfoLog(p, len, nullptr, log.data());
        printf("%s\n", log.data());
        glDeleteProgram(p); return 0;
    }
    return p;
}

// ===== Constructores de formas 2D (llenadas con triángulos) =====
static void agregarCuadrado(std::vector<GLfloat>& verts, float cx, float cy, float tam) {
    // cuadrado axis-aligned centrado en (cx,cy) con lado = tam
    float m = tam * 0.5f, x0 = cx - m, x1 = cx + m, y0 = cy - m, y1 = cy + m;
    // triángulo 1
    verts.insert(verts.end(), { x0,y0, x1,y0, x1,y1 });
    // triángulo 2
    verts.insert(verts.end(), { x0,y0, x1,y1, x0,y1 });
}

static void agregarRombo(std::vector<GLfloat>& verts, float cx, float cy, float r) {
    // "rombo" (diamante): un cuadrado rotado 45°, definido por sus 4 vértices cardinales
    // top, right, bottom, left
    float xt = cx, yt = cy + r;
    float xr = cx + r, yr = cy;
    float xb = cx, yb = cy - r;
    float xl = cx - r, yl = cy;
    // Dos triángulos: (top, left, bottom) y (top, right, bottom)
    verts.insert(verts.end(), { xt,yt,  xl,yl,  xb,yb });
    verts.insert(verts.end(), { xt,yt,  xr,yr,  xb,yb });
}

int main()
{
    // Inicialización de GLFW
    if (!glfwInit()) {
        printf("Falló inicializar GLFW");
        glfwTerminate();
        return 1;
    }

    // Pide contexto 4.3 core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Crear ventana
    GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Fondo ciclico + cuadrado y rombo", NULL, NULL);
    if (!mainWindow) {
        printf("Fallo en crearse la ventana con GLFW");
        glfwTerminate();
        return 1;
    }

    int BufferWidth, BufferHeight;
    glfwGetFramebufferSize(mainWindow, &BufferWidth, &BufferHeight);
    glfwMakeContextCurrent(mainWindow);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        printf("Falló inicialización de GLEW");
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return 1;
    }

    //glViewport(0, 0, BufferWidth, BufferHeight);
    int side = (BufferWidth < BufferHeight) ? BufferWidth : BufferHeight;
    int x = (BufferWidth - side) / 2;
    int y = (BufferHeight - side) / 2;
    glViewport(x, y, side, side);

    printf("Version de Opengl: %s \n", glGetString(GL_VERSION));
    printf("Marca: %s \n", glGetString(GL_VENDOR));
    printf("Renderer: %s \n", glGetString(GL_RENDERER));
    printf("Shaders: %s \n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Shaders
    GLuint program = makeProgram(vtxSrc, fragSrc);
    if (!program) { glfwDestroyWindow(mainWindow); glfwTerminate(); return 1; }
    GLint uColor = glGetUniformLocation(program, "uColor");

    // ===== Geometrías: un cuadrado (izq) y un rombo (der) =====
    std::vector<GLfloat> vSquare;
    std::vector<GLfloat> vDiamond;

    // cuadrado centrado en x=-0.5, tamaño 0.6
    agregarCuadrado(vSquare, -0.5f, 0.0f, 0.6f);

    // rombo centrado en x=+0.5, "radio" (distancia al vértice) 0.35
    agregarRombo(vDiamond, 0.5f, 0.0f, 0.35f);

    GLuint vaoSquare = 0, vboSquare = 0;
    glGenVertexArrays(1, &vaoSquare);
    glGenBuffers(1, &vboSquare);
    glBindVertexArray(vaoSquare);
    glBindBuffer(GL_ARRAY_BUFFER, vboSquare);
    glBufferData(GL_ARRAY_BUFFER, vSquare.size() * sizeof(GLfloat), vSquare.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    GLuint vaoDiamond = 0, vboDiamond = 0;
    glGenVertexArrays(1, &vaoDiamond);
    glGenBuffers(1, &vboDiamond);
    glBindVertexArray(vaoDiamond);
    glBindBuffer(GL_ARRAY_BUFFER, vboDiamond);
    glBufferData(GL_ARRAY_BUFFER, vDiamond.size() * sizeof(GLfloat), vDiamond.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // ===== Fondo cíclico R->G->B =====
    const double COLOR_PERIOD = 1.2; // segundos por color (ajústalo a gusto)
    int bgIndex = 0;                 // 0=R, 1=G, 2=B
    double tLast = glfwGetTime();

    // Loop principal
    while (!glfwWindowShouldClose(mainWindow)) {
        glfwPollEvents();

        // Cambiar fondo cada COLOR_PERIOD
        double tNow = glfwGetTime();
        if (tNow - tLast >= COLOR_PERIOD) {
            bgIndex = (bgIndex + 1) % 3;
            tLast = tNow;
        }

        float r = (bgIndex == 0) ? 1.0f : 0.0f;
        float g = (bgIndex == 1) ? 1.0f : 0.0f;
        float b = (bgIndex == 2) ? 1.0f : 0.0f;

        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Dibujar formas
        glUseProgram(program);

        // Cuadrado (blanco)
        glUniform3f(uColor, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(vaoSquare);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(vSquare.size() / 2));
        glBindVertexArray(0);

        // Rombo (amarillo)
        glUniform3f(uColor, 1.0f, 1.0f, 0.0f);
        glBindVertexArray(vaoDiamond);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(vDiamond.size() / 2));
        glBindVertexArray(0);

        glfwSwapBuffers(mainWindow);
    }

    // Limpieza
    glDeleteBuffers(1, &vboSquare);
    glDeleteVertexArrays(1, &vaoSquare);
    glDeleteBuffers(1, &vboDiamond);
    glDeleteVertexArrays(1, &vaoDiamond);
    glDeleteProgram(program);

    glfwDestroyWindow(mainWindow);
    glfwTerminate();
    return 0;
}
