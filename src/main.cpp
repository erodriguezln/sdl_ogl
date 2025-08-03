#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_mouse.h>
#include <glad/glad.h>      // Loader para funciones OpenGL modernas
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Variables globales para ventana y contexto OpenGL
static SDL_Window *window = nullptr;
static SDL_GLContext context = nullptr;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

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

    const char *version = (const char *) glGetString(GL_VERSION);
    const char *vendor = (const char *) glGetString(GL_VENDOR);
    const char *renderer = (const char *) glGetString(GL_RENDERER);
    const char *glsl_version = (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION);

    SDL_Log("OpenGL Version: %s", version);
    SDL_Log("Vendor: %s", vendor);
    SDL_Log("Renderer: %s", renderer);
    SDL_Log("GLSL Version: %s", glsl_version);

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

    *appstate = state; // Pasar estado a SDL

    return SDL_APP_CONTINUE;
}

// EVENTOS: Se ejecuta cuando hay eventos (teclado, mouse, etc.)
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    AppState *state = static_cast<AppState*> (appstate);

    // Manejar cierre de ventana
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS; // Terminar aplicación limpiamente
    }

    return SDL_APP_CONTINUE;
}

// BUCLE PRINCIPAL: Se ejecuta cada frame para renderizar
SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *state = static_cast<AppState *>(appstate);

    // RENDERIZADO:
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Color de fondo (gris-azulado)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpiar buffers

    glBindVertexArray(state->VAO); // Activar configuración de vértices

    glDrawArrays(GL_TRIANGLES, 0, 36); // Dibujar 36 vértices como triángulos (12 triángulos = 6 caras)

    SDL_GL_SwapWindow(window); // Intercambiar buffers (mostrar frame renderizado)

    return SDL_APP_CONTINUE;
}

// LIMPIEZA: Se ejecuta al cerrar la aplicación
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