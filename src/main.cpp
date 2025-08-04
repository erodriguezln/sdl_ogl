#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_mouse.h>
#include <glad/glad.h>      // Loader para funciones OpenGL modernas
#include <glm.hpp>
#include <iostream>
#include <ostream>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Variables globales para ventana y contexto OpenGL
static SDL_Window *window = nullptr;
static SDL_GLContext context = nullptr;

uint64_t lastFrame = 0;
uint64_t currentFrame = 0;
float deltaTime = 0.0f;

// TEMPORAL Lo correcto seria usar una clase para el objeto e ir alterando su rotacion ahi, o algo.
float totalRotation = 0.0f;

unsigned int gProgramID = 0;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

typedef struct AppState {
    unsigned int VBO, VAO; // Vertex Buffer Object y Vertex Array Object
} AppState;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Configurar atributos del contexto OpenGL antes de crearlo
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8); // 8 bits para canal rojo
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8); // 8 bits para canal verde
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8); // 8 bits para canal azul
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); // 24 bits para depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // Habilitar double buffering

    // Crear ventana con soporte OpenGL
    window = SDL_CreateWindow("SDL OpenGL", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Crear contexto OpenGL asociado a la ventana
    context = SDL_GL_CreateContext(window);
    if (!context) {
        SDL_Log("Couldn't create OpenGL context: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Cargar funciones OpenGL modernas usando GLAD
    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        SDL_Log("Couldn't load GLAD function: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // SHADER
    const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main()
{
	gl_Position =projection * view * model * vec4(aPos, 1.0);
}
)";
    const char *fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;


void main()
{
	FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

    unsigned int vertex, fragment;

    // Crear programa OpenGL para shaders
    gProgramID = glCreateProgram();
    vertex = glCreateShader(GL_VERTEX_SHADER);
    fragment = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertex, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertex);

    glShaderSource(fragment, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragment);

    glAttachShader(gProgramID, vertex);
    glAttachShader(gProgramID, fragment);
    glLinkProgram(gProgramID);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // Inicializar estado de la aplicación
    AppState *state = new AppState;
    state->VAO = 0;

    // Habilitar test de profundidad para 3D correcto
    glEnable(GL_DEPTH_TEST);

    // Definir vértices del cubo (posición XYZ + coordenadas UV)
    // Cada cara está formada por 2 triángulos (6 vértices)
    constexpr float vertices[] = {
        // Cara frontal
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

        // Cara trasera
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        // Cara izquierda
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        // Cara derecha
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        // Cara inferior
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        // Cara superior
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
    };

    glm::vec3 cubePosition = glm::vec3(0.0f, 0.0f, 0.0f); // Posición del cubo

    // CONFIGURACIÓN DE BUFFERS OPENGL:
    glGenVertexArrays(1, &state->VAO); // Generar VAO (guarda configuración de vértices)
    glGenBuffers(1, &state->VBO); // Generar VBO (buffer para datos de vértices)

    glBindVertexArray(state->VAO); // Activar VAO para configurar

    glBindBuffer(GL_ARRAY_BUFFER, state->VBO); // Activar VBO como buffer de vértices
    // Copiar datos de vértices al buffer en GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Configurar layout de vértices: posición (3 floats) en índice 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0); // Habilitar atributo de posición

    glUseProgram(gProgramID);


    *appstate = state; // Pasar estado a SDL
    SDL_GL_SetSwapInterval(1);

    lastFrame = SDL_GetTicksNS();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    AppState *state = static_cast<AppState *>(appstate);

    // Manejar cierre de ventana
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS; // Terminar aplicación limpiamente
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *state = static_cast<AppState *>(appstate);

    // Delta Time
    currentFrame = SDL_GetTicksNS();
    deltaTime = static_cast<float>(currentFrame - lastFrame) / 1000000000.0f;
    lastFrame = currentFrame;

    // RENDERIZADO:
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Color de fondo (gris-azulado)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpiar buffers

    glUseProgram(gProgramID);


    glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(WINDOW_WIDTH / WINDOW_HEIGHT), 0.1f,
                                            100.0f);
    glUniformMatrix4fv(glGetUniformLocation(gProgramID, "projection"), 1, GL_FALSE, &projection[0][0]);

    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glUniformMatrix4fv(glGetUniformLocation(gProgramID, "view"), 1, GL_FALSE, &view[0][0]);

    glBindVertexArray(state->VAO); // Activar configuración de vértices

    glm::mat4 model = glm::mat4(1.0f);
    float speed = 20.0f;
    totalRotation += deltaTime * speed;
    model = glm::rotate(model, glm::radians(totalRotation), glm::vec3(1.0f, 0.3f, 0.5f));

    glUniformMatrix4fv(glGetUniformLocation(gProgramID, "model"), 1, GL_FALSE, &model[0][0]);

    glDrawArrays(GL_TRIANGLES, 0, 36); // Dibujar 36 vértices como triángulos (12 triángulos = 6 caras)

    SDL_GL_SwapWindow(window); // Intercambiar buffers (mostrar frame renderizado)

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    AppState *state = static_cast<AppState *>(appstate);

    if (state) {
        if (state->VAO != 0) {
            glDeleteVertexArrays(1, &state->VAO);
        }
        if (state->VBO != 0) {
            glDeleteBuffers(1, &state->VBO);
        }
        delete state;
    }

    if (context) {
        SDL_GL_DestroyContext(context);
    }

    if (window) {
        SDL_DestroyWindow(window);
    }

    SDL_Quit();
}