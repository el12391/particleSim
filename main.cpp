#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include "include/Shaders/shader.h"
#include <cstddef>

const int MAX_PARTICLES = 10;
const float LOWER_BOUND = -1.0f;
const float UPPER_BOUND = 1.0f;
const float SIDE_SIZE = UPPER_BOUND - LOWER_BOUND;

float step(float boundry, float value) {return value > boundry ? 1.0f : 0.0f;}
float sign(float value) {return (value > 0.0f) - (value < 0.0f);}
float linearInterpolation(float x, float y, float a) {return x * (1.0f - a) + y * a;}
struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec4 color;
    float life;
};

void bounceParticle(Particle& particle, float deltaTime) {
    glm::vec2 futurePosition = particle.position + particle.velocity * deltaTime;

    float outOfBoundsX = step(1.0f, abs(futurePosition.x));
    float outOfBoundsY = step(1.0f, abs(futurePosition.y));

    glm::vec2 bounceVector = glm::vec2(-sign(futurePosition.x) * abs(particle.velocity.x),
                                        -sign(futurePosition.y) * abs(particle.velocity.y));
    glm::vec2 newVelocity = glm::vec2(
        linearInterpolation(particle.velocity.x, bounceVector.x, outOfBoundsX),
        linearInterpolation(particle.velocity.y, bounceVector.y, outOfBoundsY));
    particle.velocity = newVelocity;
    particle.position = glm::vec2(std::clamp(particle.position.x, LOWER_BOUND, UPPER_BOUND), 
                    std::clamp(particle.position.y, LOWER_BOUND, UPPER_BOUND));
}

class Renderer {
private:
public:
    Shader* shaderPointer;
    unsigned int instanceVBO;
    unsigned int VAO;

    Renderer (Shader& shader) {
        this->shaderPointer = &shader;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &instanceVBO);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        
        // 2. Allocate the memory (but don't put data in yet, pass nullptr)
        // GL_DYNAMIC_DRAW tells OpenGL we plan to change this data often
        glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(Particle), nullptr, GL_DYNAMIC_DRAW);

        // 3. Tell the VAO how to read the Particle struct!
        // Attribute 0: Position (vec2)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);

        // Attribute 1: Velocity (vec2) - starts after position (which is 2 floats = 8 bytes)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(sizeof(glm::vec2)));

        // Attribute 2: Color (vec4) - starts after pos + vel (4 floats = 16 bytes)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(2 * sizeof(glm::vec2)));
        

        // Unbind VBO and VAO
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void updatePoints (std::vector<Particle>& particles, float deltaTime) {
        
        for (auto& p : particles) {
            // this calculates the velocity and position of a bouncing particle
            bounceParticle(p, deltaTime);
            // if didnt bounce this is the same as before, but these are potentially bounced values
            p.position += p.velocity * deltaTime;

            //std::cout << "velocity: (" << p.velocity.x << ", " << p.velocity.y << ")" << std::endl;
            //std::cout << "position: (" << p.position.x << ", " << p.position.y << ")" << std::endl;
        }
        
        // 1. Bind the buffer we already created
        glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
        
        // 2. Overwrite the old memory with the actual, fresh vector data!
        glBufferSubData(GL_ARRAY_BUFFER, 0, particles.size() * sizeof(Particle), particles.data());
        
        // 3. Unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
};

// Callback function to resize the OpenGL viewport if the user resizes the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void SpawnParticleLine(std::vector<Particle>& pool, glm::vec2 start, glm::vec2 end, int amount) {
    // Find 'amount' dead particles in your pool and move them to these positions
    int spawned = 0;
    for (auto& p : pool) {
        float t = (float)spawned / (float)(amount - 1);
        p.position = start + t * (end - start);
        p.life = 1.0f;
        p.velocity = glm::vec2(0.0f, 0.5f);
        
        spawned++;

    }
}

void GiveRandomPositions(std::vector<Particle>& pool) {
    for (auto& p : pool) {
        p.position.x = SIDE_SIZE * rand()/RAND_MAX - UPPER_BOUND;
        p.position.y = SIDE_SIZE * rand()/RAND_MAX - UPPER_BOUND;
    }
}

void GiveRandomVelocities(std::vector<Particle>& pool) {
    for (auto& p : pool) {
        p.velocity.x = 2.0f * rand()/RAND_MAX - 1.0f;
        p.velocity.y = 2.0f * rand()/RAND_MAX - 1.0f;
    }
}

void GiveRandomColors(std::vector<Particle>& pool) {
    for (auto& p : pool) {
        p.color.r = 1.0f * rand()/RAND_MAX;
        p.color.g = 1.0f * rand()/RAND_MAX;
        p.color.b = 1.0f * rand()/RAND_MAX;
        p.color.a = 1.0f;
    }
}

int main() {
    srand(static_cast<unsigned int>(time(NULL)));

    // 1. Initialize GLFW
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW to use OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. Create the Window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Particle Simulator", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 3. Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_DEPTH_TEST);

    Shader shader("../include/Shaders/shader.vs", "../include/Shaders/shader.fs");
    Renderer renderer(shader);


    std::vector<Particle> particles(MAX_PARTICLES);
    //SpawnParticleLine(particles, glm::vec2(-1.0f, 1.0f), glm::vec2(1.0f, -1.0f), MAX_PARTICLES);
    GiveRandomPositions(particles);
    GiveRandomVelocities(particles);
    GiveRandomColors(particles);
    
    //std::cout << "got here" << std::endl;
    float lastFrame = 0.0f;
    // 4. The Render Loop!
    while (!glfwWindowShouldClose(window)) {
        // Input: close the window if escape is pressed
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        renderer.updatePoints(particles, deltaTime);
        // Rendering: Clear the screen to a dark blue color
        glClearColor(0.3f, 0.2f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        renderer.shaderPointer->use();

        glBindVertexArray(renderer.VAO);
        glDrawArrays(GL_POINTS, 0, particles.size());

        // Swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwPollEvents();
        glfwSwapBuffers(window);
        
    }

    // Clean up and exit
    glfwTerminate();
    return 0;
}