#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_mouse.h>
#include <glad/glad.h>      // Loader para funciones OpenGL modernas
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Shader.h"
#include "Texture.h"
#include "Camera.h"

// Variables globales para ventana y contexto OpenGL
static SDL_Window *window = nullptr;
static SDL_GLContext context = nullptr;

uint64_t lastFrame = 0;
uint64_t currentFrame = 0;
float deltaTime = 0.0f;

// TEMPORAL Lo correcto sería usar una clase para el objeto e ir alterando su rotation ahi, o algo.
float totalRotation = 0.0f;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

typedef struct AppState {
    unsigned int VBO, VAO; // Vertex Buffer Object y Vertex Array Object
    Shader shader;
    Camera *camera;
} AppState;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    // SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    // SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

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

    SDL_SetWindowRelativeMouseMode(window, true);
    SDL_SetWindowMouseGrab(window, true);

    int major, minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    SDL_Log("OpenGL Context Created: %d.%d", major, minor);

    int flags;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &flags);
    if (flags & SDL_GL_CONTEXT_DEBUG_FLAG) {
        SDL_Log("Debug context enabled");
    } else {
        SDL_Log("Debug context NOT enabled");
    }

    // Cargar funciones OpenGL modernas usando GLAD
    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        SDL_Log("Couldn't load GLAD function: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // TODO Necesito crear con new para que no se destruya una vez salga del contexto actual
    Texture wood("assets/container.jpg");
    Texture awesomeface("assets/awesomeface.png");

    // activa texture0 y bindea el id de textura wood a esa textura
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, wood.getID());

    // activa texture1 y bindea el id de textura awesomeface a esa textura
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, awesomeface.getID());

    // Constructor se llama por defecto por lo que se debe asignar
    // Shader ahora, ya que es un objeto no puntero.
    AppState *state = new AppState{
        0, 0,
        Shader(
            "assets/shaders/shader.vert",
            "assets/shaders/shader.frag"
        )
    };
    state->VAO = 0;


    // Habilitar test de profundidad para 3D correcto
    glEnable(GL_DEPTH_TEST);

    // Definir vértices del cubo (posición XYZ + coordenadas UV)
    // Cada cara está formada por 2 triángulos (6 vértices)
    constexpr float vertices[] = {
        // Cara frontal
        // X, Y, Z, U, V
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

    // glVertexAttribPointer(location, num_componentes, tipo, normalizar, stride, offset)
    // location: índice del atributo en el shader (layout location)
    // num_componentes: cuántos valores leer (3 para XYZ, 2 para UV)
    // stride: bytes entre vértices consecutivos
    // offset: bytes desde el inicio del vértice hasta este atributo
    // Formato del buffer: [X Y Z U V] [X Y Z U V] [X Y Z U V]...
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0); // Habilitar atributo de posición

    // Layout = 1
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    state->shader.use();
    state->shader.setInt("texture1", 0);
    state->shader.setInt("texture2", 1);


    *appstate = state; // Pasar estado a SDL
    SDL_GL_SetSwapInterval(1);

    lastFrame = SDL_GetTicksNS();

    state->camera = new Camera();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    AppState *state = static_cast<AppState *>(appstate);

    if (event->type == SDL_EVENT_MOUSE_MOTION) {
        if (SDL_GetWindowRelativeMouseMode(window)) {
            const float xOffset = event->motion.xrel;
            const float yOffset = event->motion.yrel;

            state->camera->processMouse(xOffset, yOffset);
        }
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        switch (event->key.scancode) {
            case SDL_SCANCODE_ESCAPE:
                return SDL_APP_SUCCESS;
        }
    }

    // Manejar cierre de ventana
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS; // Terminar aplicación limpiamente
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *state = static_cast<AppState *>(appstate);

    const bool *keys = SDL_GetKeyboardState(nullptr);

    // Delta Time
    currentFrame = SDL_GetTicksNS();
    deltaTime = static_cast<float>(currentFrame - lastFrame) / 1000000000.0f;
    lastFrame = currentFrame;

    // RENDERIZADO:
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Color de fondo (gris-azulado)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpiar buffers

    state->shader.use();

    if (keys[SDL_SCANCODE_W]) {
        state->camera->processKeyboard(FORWARD, deltaTime);
    }
    if (keys[SDL_SCANCODE_S]) {
        state->camera->processKeyboard(BACKWARD, deltaTime);
    }
    if (keys[SDL_SCANCODE_A]) {
        state->camera->processKeyboard(LEFT, deltaTime);
    }
    if (keys[SDL_SCANCODE_D]) {
        state->camera->processKeyboard(RIGHT, deltaTime);
    }

    glm::mat4 projection = glm::perspective(glm::radians(state->camera->fov),
                                            static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT), 0.1f,
                                            100.0f);
    state->shader.setMat4("projection", projection);

    glm::mat4 view = state->camera->getViewMatrix();
    state->shader.setMat4("view", view);

    glBindVertexArray(state->VAO); // Activar configuración de vértices

    glm::mat4 model = glm::mat4(1.0f);
    float speed = 20.0f;
    totalRotation += deltaTime * speed;

    model = glm::rotate(model, glm::radians(totalRotation), glm::vec3(1.0f, 0.3f, 0.5f));
    state->shader.setMat4("model", model);

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