#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <SDL3/SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include "gif.hpp"
#include <chrono>

// Camera settings
struct Camera {
    glm::vec3 position = glm::vec3(0.0f, 5.0f, 10.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    float yaw = -90.0f;
    float pitch = 0.0f;
    float speed = 0.1f;
    float sensitivity = 0.1f;
} camera;

// Shader sources
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    out vec3 fragColor;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        fragColor = aColor;
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 fragColor;
    out vec4 FragColor;
    void main() {
        FragColor = vec4(fragColor, 1.0);
    }
)";

GLuint createShaderProgram() {
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) == false) {
        std::cout << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow(
        "Voxel World",
        800, 600,
        SDL_WINDOW_OPENGL
    );

    if (!window) {
        std::cout << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    
    // Initialize GLEW
    glewInit();

    // Create and use shader program
    GLuint shaderProgram = createShaderProgram();
    glUseProgram(shaderProgram);

    // Create a simple cube vertices (position and color)
    float vertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f
    };

    unsigned int indices[] = {
        // Front
        0, 1, 2,
        2, 3, 0,
        // Right
        1, 5, 6,
        6, 2, 1,
        // Back
        7, 6, 5,
        5, 4, 7,
        // Left
        4, 0, 3,
        3, 7, 4,
        // Bottom
        4, 5, 1,
        1, 0, 4,
        // Top
        3, 2, 6,
        6, 7, 3
    };

    // Create and bind VAO and VBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Get uniform locations
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    // Mouse movement variables
    int lastX = 400, lastY = 300;
    bool firstMouse = true;
    
    // Capture mouse
    SDL_SetWindowMouseGrab(window, true);
    SDL_SetWindowRelativeMouseMode(window, true);

    int windowWidth = 800;
    int windowHeight = 600;
    GifRecorder recorder(windowWidth, windowHeight, 30); // 30 FPS

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_ESCAPE) {
                        running = false;
                    }
                    else if (event.key.key == SDLK_F2) {
                        // Start/Stop recording
                        if (!recorder.isRecording()) {
                            recorder.startRecording();
                            std::cout << "Recording started..." << std::endl;
                            
                        } else {
                            std::cout << "Recording stopped..." << std::endl;
                            auto now = std::chrono::system_clock::now();
                            auto timestamp = std::chrono::system_clock::to_time_t(now);
                            std::string filename = "recording_" + std::to_string(timestamp) + ".gif";
                            recorder.stopRecording(filename);
                            std::cout << "Recording saved to: " << filename << std::endl;
                        }
                    }
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    float xoffset = event.motion.xrel * camera.sensitivity;
                    float yoffset = -event.motion.yrel * camera.sensitivity;

                    camera.yaw += xoffset;
                    camera.pitch += yoffset;

                    if (camera.pitch > 89.0f) camera.pitch = 89.0f;
                    if (camera.pitch < -89.0f) camera.pitch = -89.0f;

                    glm::vec3 direction;
                    direction.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
                    direction.y = sin(glm::radians(camera.pitch));
                    direction.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
                    camera.front = glm::normalize(direction);
                    break;
            }
        }

        // Handle keyboard input for movement
        const bool* keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_W])
            camera.position += camera.speed * camera.front;
        if (keystate[SDL_SCANCODE_S])
            camera.position -= camera.speed * camera.front;
        if (keystate[SDL_SCANCODE_A])
            camera.position -= glm::normalize(glm::cross(camera.front, camera.up)) * camera.speed;
        if (keystate[SDL_SCANCODE_D])
            camera.position += glm::normalize(glm::cross(camera.front, camera.up)) * camera.speed;

        // Clear the screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Create transformation matrices
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        // Set uniforms
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Draw the cube
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Add before SDL_GL_SwapWindow:
        if (recorder.isRecording()) {
            recorder.captureFrame();
        }

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
