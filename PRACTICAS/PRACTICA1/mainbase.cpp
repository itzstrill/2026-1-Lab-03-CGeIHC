#include <stdio.h>
#include <vector>    
#include <cstdlib>   //  rand, srand
#include <ctime>    
#include <glew.h>
#include <glfw3.h>

//Dimensiones de la ventana
const int WIDTH = 800, HEIGHT = 600;

// shaders para dibujar los triángulos 
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

// construir las letras con cuadritos de dos 
static void agregarCuadrado(std::vector<GLfloat>& verts, float x, float y, float tam) {
    float m = tam * 0.5f, x0 = x - m, x1 = x + m, y0 = y - m, y1 = y + m;
    // triángulo 1
    verts.insert(verts.end(), { x0,y0, x1,y0, x1,y1 });
    // triángulo 2
    verts.insert(verts.end(), { x0,y0, x1,y1, x0,y1 });
}
static void barraHorizontal(std::vector<GLfloat>& v, float xIni, float xFin, float y, float tam) {
    float paso = tam * 0.9f;
    for (float x = xIni; x <= xFin; x += paso) agregarCuadrado(v, x, y, tam);
}
static void barraVertical(std::vector<GLfloat>& v, float x, float yIni, float yFin, float tam) {
    float paso = tam * 0.9f;
    for (float y = yIni; y <= yFin; y += paso) agregarCuadrado(v, x, y, tam);
}
static void crearLetrasGEA(std::vector<GLfloat>& vertices) {
    const float t = 0.055f;   // grosor de la letra
    const float yTop = 0.60f;
    const float yBot = -0.60f;

    const float letterW = 0.45f;  // ancho por letra
    const float gap = 0.14f;  // separación entre letras
    const float left = -0.95f; // margen izquierdo

    float x0G = left, x1G = x0G + letterW;
    float x0E = x1G + gap, x1E = x0E + letterW;
    float x0A = x1E + gap, x1A = x0A + letterW;

    // G
    {
        float yMid = (yTop + yBot) * 0.5f;
        barraHorizontal(vertices, x0G, x1G, yTop, t);
        barraVertical(vertices, x0G, yBot, yTop, t);
        barraHorizontal(vertices, x0G, x1G, yBot, t);
        barraVertical(vertices, x1G, yBot, yMid - 0.02f, t);
        barraHorizontal(vertices, x0G + (x1G - x0G) * 0.45f, x1G, yMid, t);
    }
    // E
    {
        float yMid = (yTop + yBot) * 0.5f;
        barraVertical(vertices, x0E, yBot, yTop, t);
        barraHorizontal(vertices, x0E, x1E, yTop, t);
        barraHorizontal(vertices, x0E, x0E + (x1E - x0E) * 0.65f, yMid, t);
        barraHorizontal(vertices, x0E, x1E, yBot, t);
    }
    // A
    {
        float yMid = (yTop + yBot) * 0.5f;
        barraVertical(vertices, x0A, yBot, yTop, t);
        barraVertical(vertices, x1A, yBot, yTop, t);
        barraHorizontal(vertices, x0A, x1A, yTop, t);
        barraHorizontal(vertices,
            x0A + (x1A - x0A) * 0.15f,
            x1A - (x1A - x0A) * 0.15f, yMid, t);
    }
}

int main()
{
    //Inicialización de GLFW
    if (!glfwInit())
    {
        printf("Falló inicializar GLFW");
        glfwTerminate();
        return 1;
    }
    //****  LAS SIGUIENTES 4 LÍNEAS SE COMENTAN EN DADO CASO DE QUE AL USUARIO NO LE FUNCIONE LA VENTANA Y PUEDA CONOCER LA VERSIÓN DE OPENGL QUE TIENE ****/

    //Asignando variables de GLFW y propiedades de ventana
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //para solo usar el core profile de OpenGL y no tener retrocompatibilidad
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    //CREAR VENTANA
    GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Primer ventana", NULL, NULL);

    if (!mainWindow)
    {
        printf("Fallo en crearse la ventana con GLFW");
        glfwTerminate();
        return 1;
    }
    //Obtener tamaño de Buffer
    int BufferWidth, BufferHeight;
    glfwGetFramebufferSize(mainWindow, &BufferWidth, &BufferHeight);

    //asignar el contexto
    glfwMakeContextCurrent(mainWindow);

    //permitir nuevas extensiones
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK)
    {
        printf("Falló inicialización de GLEW");
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return 1;
    }

    // Asignar valores de la ventana y coordenadas
    //Asignar Viewport
    glViewport(0, 0, BufferWidth, BufferHeight);
    printf("Version de Opengl: %s \n", glGetString(GL_VERSION));
    printf("Marca: %s \n", glGetString(GL_VENDOR));
    printf("Renderer: %s \n", glGetString(GL_RENDERER));
    printf("Shaders: %s \n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // preparar shaders, geometría y estado
    GLuint program = makeProgram(vtxSrc, fragSrc);
    if (!program) { glfwDestroyWindow(mainWindow); glfwTerminate(); return 1; }
    GLint uColor = glGetUniformLocation(program, "uColor");

    std::vector<GLfloat> vertices;
    crearLetrasGEA(vertices);                        // construir G-E-A
    GLsizei vertexCount = (GLsizei)(vertices.size() / 2);

    GLuint VAO = 0, VBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(GLfloat),
        vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // fondo aleatorio cada 2 s
    srand((unsigned)time(nullptr));
    float bgR = (float)rand() / RAND_MAX;
    float bgG = (float)rand() / RAND_MAX;
    float bgB = (float)rand() / RAND_MAX;
    double tLast = glfwGetTime();

    //Loop mientras no se cierra la ventana
    while (!glfwWindowShouldClose(mainWindow))
    {
        //Recibir eventos del usuario
        glfwPollEvents();

        // cambiar el color de fondo cada ~2 segundos
        double tNow = glfwGetTime();
        if (tNow - tLast >= 2.0) {
            bgR = (float)rand() / RAND_MAX;
            bgG = (float)rand() / RAND_MAX;
            bgB = (float)rand() / RAND_MAX;
            tLast = tNow;
        }

        //Limpiar la ventana
        glClearColor(bgR, bgG, bgB, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // dibujar las letras G-E-A con un solo color
        glUseProgram(program);
        glUniform3f(uColor, 1.0f, 1.0f, 1.0f); // blanco
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);

        glfwSwapBuffers(mainWindow);
    }

    // limpieza
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(program);

    return 0;
}
