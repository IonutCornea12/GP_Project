//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright © 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>
#include <vector>

// -----------------------------------------------------
// Global Variables
// -----------------------------------------------------
int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

// For camera mouse movement
float lastX = 1000, lastY = 600;
bool firstMouse = true;

// Matrices and uniforms
glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
GLuint enableShadowsLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

// Directional light
glm::vec3 lightDir;      // base direction
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

// Light Position (Global)
glm::vec3 lightPosition;

// Camera
gps::Camera myCamera(
    glm::vec3(-42.7423f, 3.57602f, 57.6629f),//sta
    glm::vec3(-68.0686f, 2.66757f, 39.8092f),
    glm::vec3(0.0f, 1.0f, 0.0f)
);

float cameraSpeed = 0.5f;

// Position/orbit radius for the "sun" cube
float cubeHeight = 400.0f;

// We'll store a base sun position as a 4D vector (for easy transforms).
glm::vec4 baseSunPos(400.0f, 400.0f, 0.0f, 1.0f);

// Input states
bool pressedKeys[1024];
float angleY = 0.0f;

GLfloat lightAngle = 0.0f; // Initialize lightAngle here
bool nightMode = false;
bool shadowsEnabled = false;
bool fogEnabled = false;
bool cameraAnimationActive = false;

// Models
gps::Model3D APA;
gps::Model3D NISIP;
gps::Model3D ZAPADA;
gps::Model3D FLORI;
gps::Model3D PAMANT;
gps::Model3D IARBA;
gps::Model3D PAMANTSAT;

gps::Model3D COPACI;
gps::Model3D CASE;
gps::Model3D FANTANA;
gps::Model3D MARKET;
gps::Model3D ZID;
gps::Model3D LAMP;
gps::Model3D HAY;

gps::Model3D lightCube;  // The "sun" model

// Shaders
gps::Shader shaderStart;
gps::Shader lightShader;
gps::Shader skyboxShader;
gps::Shader depthShader;
gps::Shader shadowMapVisualizerShader; // Shadow Map Visualizer Shader

// Shadow mapping
GLuint shadowMapFBO;
GLuint depthMapTexture;

// Fog + Depth
GLuint fogEnabledLoc;
bool showDepthMap = false;

// Skybox
gps::SkyBox daySkyBox;
gps::SkyBox nightSkyBox;

GLuint daySkyID = daySkyBox.GetTextureId();   // for day skybox
GLuint nightSkyID = nightSkyBox.GetTextureId(); // for night skybox


// Multiple lamp positions for point lights
std::vector<glm::vec3> lampPositions = {
    glm::vec3(-131.7f,   2.5f,  9.1f),
    glm::vec3(-126.039f, 2.5f, 42.502f),
    glm::vec3(-81.9977f, 2.5f, 29.3346f),
    glm::vec3(-61.4288f, 2.5f, 51.1717f),
    glm::vec3(-70.899f,  2.5f, 42.5096f),
    glm::vec3(-4.70084f, 2.5f, 70.5314f)
};
glm::vec3 pointLightColor = glm::vec3(1.0f, 1.0f, 0.8f);

std::vector<glm::vec3> cameraWaypoints = {
    glm::vec3(11.9557, 3.49008, 76.0564),
    glm::vec3(-23.0059, 3.49008, 70.0662),
    glm::vec3(-36.8901, 3.49008, 59.082),
    glm::vec3(-58.5219, 3.49008, 42.9351),
    glm::vec3(-84.6402, 3.49008, 35.1376),
    glm::vec3(-103.507, 3.49008, 37.3233),
    glm::vec3(-121.651, 3.49008, 24.6471),
    glm::vec3(-112.39, 3.49008, 4.68153),
    glm::vec3(-94.075, 3.49008, -0.77422)
};

int currentSegment = 0;
float segmentProgress = 0.0f;

// Speed in "units per second" (approx)
float cameraAnimSpeed = 2.0f;

// Fullscreen Quad for Shadow Map Visualization
GLuint quadVAO = 0;
GLuint quadVBO;

// -----------------------------------------------------
// Error Checking
// -----------------------------------------------------
GLenum glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
        case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

// -----------------------------------------------------
// Callbacks
// -----------------------------------------------------
void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized to width: %d , and height: %d\n", width, height);
    glViewport(0, 0, width, height);
    glWindowWidth = width;
    glWindowHeight = height;
}

void updateCameraAnimation(float deltaTime)
{
    // If the animation is done or we have no more segments, do nothing
    if (!cameraAnimationActive) return;
    if (currentSegment >= (int)cameraWaypoints.size() - 1) {
        // Reached final waypoint, or out of range
        return;
    }

    // Increase our progress along the current segment
    segmentProgress += cameraAnimSpeed * deltaTime * 0.1f;
    // You can tweak the '0.1f' if you want a slower or faster effective speed

    if (segmentProgress >= 1.0f) {
        // Move to the next segment
        segmentProgress = 0.0f;
        currentSegment++;
        // If we went beyond the last segment, clamp or loop
        if (currentSegment >= (int)cameraWaypoints.size() - 1) {
            currentSegment = (int)cameraWaypoints.size() - 1;
            cameraAnimationActive = false; // or loop back to 0
            return;
        }
    }

    // Calculate the interpolated position between the two waypoints
    glm::vec3 startPos = cameraWaypoints[currentSegment];
    glm::vec3 endPos   = cameraWaypoints[currentSegment + 1];
    // Linear interpolation (LERP)
    glm::vec3 newCamPos = glm::mix(startPos, endPos, segmentProgress);

    // Update the camera position
    myCamera.setPosition(newCamPos);

    // (Optional) Make camera look toward the *next* waypoint
    glm::vec3 dir = glm::normalize(endPos - newCamPos);
    myCamera.setLookDirection(dir);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            pressedKeys[key] = true;
        else if (action == GLFW_RELEASE)
            pressedKeys[key] = false;
    }
    if (key == GLFW_KEY_0 && action == GLFW_PRESS) {
        glm::vec3 cameraPosition = myCamera.getPosition();
        std::cout << "Camera Position: ("
                  << cameraPosition.x << ", "
                  << cameraPosition.y << ", "
                  << cameraPosition.z << ")" << std::endl;
    }
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        nightMode = !nightMode;  // Toggle night mode
        std::cout << "Night Mode: " << (nightMode ? "Enabled" : "Disabled") << std::endl;
    }

    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        fogEnabled = !fogEnabled; // Toggle fog
        std::cout << "Fog Mode: " << (fogEnabled ? "Enabled" : "Disabled") << std::endl;
    }


    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        // Toggle camera animation on/off
        cameraAnimationActive = !cameraAnimationActive;
        // If we turn it on, reset to start
        if (cameraAnimationActive) {
            currentSegment = 0;
            segmentProgress = 0.0f;
        }
    }
    
    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        static bool wireframeEnabled = false;
        wireframeEnabled = !wireframeEnabled;

        if (wireframeEnabled) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Enable wireframe
            std::cout << "Wireframe Mode: Enabled" << std::endl;
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Disable wireframe
            std::cout << "Wireframe Mode: Disabled" << std::endl;
        }
    }
    if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
        showDepthMap = !showDepthMap;
        std::cout << "Show Depth Map: " << (showDepthMap ? "Enabled" : "Disabled") << std::endl;
    }
    
    if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
            shadowsEnabled = !shadowsEnabled;
            std::cout << "Shadows " << (shadowsEnabled ? "Enabled" : "Disabled") << std::endl;
        }
    shaderStart.useShaderProgram();
     glUniform1i(enableShadowsLoc, shadowsEnabled ? 1 : 0);
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.3f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    myCamera.rotate(yOffset, xOffset);
}

// -----------------------------------------------------
// processMovement
// -----------------------------------------------------
void processMovement() {
    // If you want angleY usage, you can do that here
    if (pressedKeys[GLFW_KEY_Q]) {
        angleY -= 5.0f;
    }
    if (pressedKeys[GLFW_KEY_E]) {
        angleY += 5.0f;
    }

    if (pressedKeys[GLFW_KEY_J]) {
        lightAngle -= 2.0f; // Adjust rotation speed as needed
        if(lightAngle < 0.0f) lightAngle += 360.0f;
    }
    if (pressedKeys[GLFW_KEY_L]) {
        lightAngle += 2.0f; // Adjust rotation speed as needed
        if(lightAngle > 360.0f) lightAngle -= 360.0f;
    }
    // Extra adjustments for your "cubeHeight"
    if (pressedKeys[GLFW_KEY_UP]) {
        cubeHeight += 0.1f;
    }
    if (pressedKeys[GLFW_KEY_DOWN]) {
        cubeHeight -= 0.1f;
    }

    // Camera movement
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
    }
    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
    }
    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
    }
    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
    }
}

// -----------------------------------------------------
// initOpenGLWindow
// -----------------------------------------------------
bool initOpenGLWindow() {
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Window scaling for HiDPI
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    // sRBG
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    // Antialiasing
    glfwWindowHint(GLFW_SAMPLES, 4);

    glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
    if (!glWindow) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return false;
    }

    glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwSetKeyCallback(glWindow, keyboardCallback);
    glfwSetCursorPosCallback(glWindow, mouseCallback);
    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(glWindow);
    glfwSwapInterval(1);

#if not defined (__APPLE__)
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version  = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

    return true;
}

// -----------------------------------------------------
// computeLightSpaceTrMatrix
// -----------------------------------------------------
glm::mat4 computeLightSpaceTrMatrix(GLfloat angle) {
    // Rotate around the Y-axis for natural sun movement
    glm::mat4 revolve = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec4 rotatedLightPos = revolve * baseSunPos;
   // glm::vec3 rotatedLightDir = revolve * lightDir;

    lightPosition = glm::vec3(rotatedLightPos); // Update global light position
    
    // Orthographic projection matrix for directional light
    glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 1.0f, 100.0f);
    // Light view matrix
    glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 upDirection = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 lightView = glm::lookAt(lightPosition, lightTarget, upDirection);
    
    return lightProjection * lightView;
}

// -----------------------------------------------------
// initOpenGLState
// -----------------------------------------------------
void initOpenGLState() {
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glViewport(0, 0, retina_width, retina_height);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_FRAMEBUFFER_SRGB);
}

// -----------------------------------------------------
// initObjects
// -----------------------------------------------------
void initObjects() {
    // Load your ground
    APA.LoadModel("objects/ground/APA.obj");
    NISIP.LoadModel("objects/ground/NISIP.obj");
    ZAPADA.LoadModel("objects/ground/ZAPADA.obj");
    FLORI.LoadModel("objects/ground/FLORI.obj");
    PAMANT.LoadModel("objects/ground/PAMANT.obj");
    PAMANTSAT.LoadModel("objects/ground/PAMANTSAT.obj");
    IARBA.LoadModel("objects/ground/IARBA.obj");

    // Trees
    COPACI.LoadModel("objects/tree/COPACI.obj");

    // Decorations
    CASE.LoadModel("objects/decoration/CASE.obj");
    FANTANA.LoadModel("objects/decoration/FANTANA.obj");
    MARKET.LoadModel("objects/decoration/MARKET.obj");
    ZID.LoadModel("objects/decoration/ZID.obj");
    LAMP.LoadModel("objects/decoration/LAMP.obj");
    HAY.LoadModel("objects/decoration/HAY.obj");


    // Light cube for the "sun"
    lightCube.LoadModel("objects/cube/SOARE.obj");
}

// -----------------------------------------------------
// renderQuad
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // Positions        // TexCoords
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, // Top-left
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // Bottom-left
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // Bottom-right

            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, // Top-left
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // Bottom-right
             1.0f,  1.0f, 0.0f,  1.0f, 1.0f  // Top-right
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        
        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        
        // Texture coordinate attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
// -----------------------------------------------------
// drawObjects
// -----------------------------------------------------
void drawObjects(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();
   
    //NISIP
    
    glm::mat4 NISIPMODEL = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    NISIPMODEL = glm::scale(NISIPMODEL, glm::vec3(8.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(NISIPMODEL));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * NISIPMODEL));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    NISIP.Draw(shader);
    
    //IARBA
    
    glm::mat4 IARBAMODEL = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    IARBAMODEL = glm::scale(IARBAMODEL, glm::vec3(8.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(IARBAMODEL));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * IARBAMODEL));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    IARBA.Draw(shader);
    
    
    //ZAPADA
    
    glm::mat4 ZAPADAMODEL = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    ZAPADAMODEL = glm::scale(ZAPADAMODEL, glm::vec3(8.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(ZAPADAMODEL));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * ZAPADAMODEL));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    ZAPADA.Draw(shader);
    
    //FLORI
    
    glm::mat4 FLORIMODEL = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    FLORIMODEL = glm::scale(FLORIMODEL, glm::vec3(8.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(FLORIMODEL));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * FLORIMODEL));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    FLORI.Draw(shader);
    
    //PAMANT
    
    glm::mat4 PAMANTMODEL = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    PAMANTMODEL = glm::scale(PAMANTMODEL, glm::vec3(8.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(PAMANTMODEL));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * PAMANTMODEL));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    PAMANT.Draw(shader);
    
    //PAMANTSAT
    
    glm::mat4 PAMANTSATMODEL = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    PAMANTSATMODEL = glm::scale(PAMANTSATMODEL, glm::vec3(8.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(PAMANTSATMODEL));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * PAMANTSATMODEL));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    PAMANTSAT.Draw(shader);
    
    //COPACI
    
    glm::mat4 COPACIMODEL = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    COPACIMODEL = glm::scale(COPACIMODEL, glm::vec3(8.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(COPACIMODEL));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * COPACIMODEL));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    COPACI.Draw(shader);
    
    //CASE
    
    glm::mat4 CASEMODEL = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    CASEMODEL = glm::scale(CASEMODEL, glm::vec3(8.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(CASEMODEL));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * CASEMODEL));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    CASE.Draw(shader);
    
    //FANTANA
    
    glm::mat4 FANTANAMODEL = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    FANTANAMODEL = glm::scale(FANTANAMODEL, glm::vec3(8.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(FANTANAMODEL));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * FANTANAMODEL));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    
    FANTANA.Draw(shader);
    //LAMP
    
    glm::mat4 LAMPMODEL = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    LAMPMODEL = glm::scale(LAMPMODEL, glm::vec3(8.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(LAMPMODEL));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * LAMPMODEL));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    
    
    LAMP.Draw(shader);
    
    //MARKET
    
    glm::mat4 MARKETMODEL = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    MARKETMODEL = glm::scale(MARKETMODEL, glm::vec3(8.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(MARKETMODEL));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * MARKETMODEL));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    MARKET.Draw(shader);
    
    //ZID
    

    glm::mat4 ZIDMODEL = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    ZIDMODEL = glm::scale(ZIDMODEL, glm::vec3(8.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(ZIDMODEL));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * ZIDMODEL));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    ZID.Draw(shader);
    
    //HAY
    
    glm::mat4 HAYMODEL = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    HAYMODEL = glm::scale(HAYMODEL, glm::vec3(8.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(HAYMODEL));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * HAYMODEL));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    HAY.Draw(shader);
    
    //APA
    
    glm::mat4 APAMODEL = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    APAMODEL = glm::scale(APAMODEL, glm::vec3(8.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(APAMODEL));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * APAMODEL));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    APA.Draw(shader);
    
    
}

// -----------------------------------------------------
// initShaders
// -----------------------------------------------------
void initShaders() {
    shaderStart.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
    shaderStart.useShaderProgram();
    
    depthShader.loadShader("shaders/depthShader.vert", "shaders/depthShader.frag");
    depthShader.useShaderProgram();
    
    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    lightShader.useShaderProgram();
    
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
    
    shadowMapVisualizerShader.loadShader("shaders/shadowMapVisualizer.vert", "shaders/shadowMapVisualizer.frag");
    shadowMapVisualizerShader.useShaderProgram();
    glUniform1i(glGetUniformLocation(shadowMapVisualizerShader.shaderProgram, "shadowMap"), 0); // Texture unit 0
}

// -----------------------------------------------------
// initUniforms
// -----------------------------------------------------
void initUniforms() {
    shaderStart.useShaderProgram();
    lightAngle = 0.0f; // Initialize lightAngle here

    // Model, View, Normal, Projection
    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(shaderStart.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(shaderStart.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(shaderStart.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    projection = glm::perspective(glm::radians(45.0f),
                     (float)retina_width / (float)retina_height,
                      0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(shaderStart.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    enableShadowsLoc = glGetUniformLocation(shaderStart.shaderProgram, "enableShadows");
    if (enableShadowsLoc == -1) {
        std::cerr << "Warning: 'enableShadows' uniform not found or not active!" << std::endl;
    } else {
        // Initialize 'enableShadows' to false
        glUniform1i(enableShadowsLoc, shadowsEnabled ? 1 : 0);
    }
    lightDir = glm::vec3(400.0f, 400.0f, 0.0f);
    lightDirLoc = glGetUniformLocation(shaderStart.shaderProgram, "lightDir");

    // Define a base color for the directional light
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    lightColorLoc = glGetUniformLocation(shaderStart.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    // Fog
    fogEnabledLoc = glGetUniformLocation(shaderStart.shaderProgram, "enableFog");
    glUniform1i(fogEnabledLoc, fogEnabled);

    // Point lights
    GLint locPointLights = glGetUniformLocation(shaderStart.shaderProgram, "pointLightPositions");
    glUniform3fv(locPointLights, lampPositions.size(), glm::value_ptr(lampPositions[0]));
    GLint pointLightColorLoc = glGetUniformLocation(shaderStart.shaderProgram, "pointLightColor");
    glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));
    
    // Night Mode
    GLint nightModeLoc = glGetUniformLocation(shaderStart.shaderProgram, "nightMode");
    glUniform1i(nightModeLoc, nightMode);
}

// -----------------------------------------------------
// renderScene
// -----------------------------------------------------
void renderScene() {
    // Adjust directional light color based on night mode
    shaderStart.useShaderProgram();
    glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "enableFog"), fogEnabled);
    glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "nightMode"), nightMode);
    glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "showDepthMap"), showDepthMap);
    
    // Update light color based on night mode
    lightColor = nightMode ? glm::vec3(0.2f, 0.2f, 0.4f) : glm::vec3(1.0f, 1.0f, 1.0f);
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    
    // Compute light space matrix using lightAngle
    glm::mat4 lightSpaceMatrix = computeLightSpaceTrMatrix(lightAngle);
    
    // ------------------------------------------------
    // 1) RENDER DEPTH MAP (FBO)
    // ------------------------------------------------
   
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glClear(GL_DEPTH_BUFFER_BIT);
   
    depthShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
   
    drawObjects(depthShader, true); // Depth pass
   
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
   
    // ------------------------------------------------
    // 2) RENDER SCENE TO SCREEN (Final pass)
    // ------------------------------------------------
    glViewport(0, 0, retina_width, retina_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    shaderStart.useShaderProgram();
    glUniform1i(enableShadowsLoc, shadowsEnabled ? 1 : 0);
    
    // Update View Matrix
    glm::mat4 currentView = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(currentView));

    // Update Light Space Matrix
    glUniformMatrix4fv(glGetUniformLocation(shaderStart.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
   
    // Bind Shadow Map Texture to texture unit 3
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "shadowMap"), 0);
   
    // Update Directional Light Direction based on lightPosition
    glm::vec3 lightDirWorld = glm::normalize(lightPosition); // Normalize to get direction
    glm::vec3 lightDirTransformed = glm::normalize(glm::inverseTranspose(glm::mat3(currentView)) * lightDirWorld);
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTransformed));
   
    // Draw Objects
    drawObjects(shaderStart, false);
   
    // ------------------------------------------------
    // 3) RENDER SUN (Light Cube)
    // ------------------------------------------------
    lightShader.useShaderProgram();
        
        // Set view and projection matrices
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(currentView));
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        // Create model matrix for the sun
        glm::mat4 modelForSun = glm::translate(glm::mat4(1.0f), lightPosition);
        modelForSun = glm::scale(modelForSun, glm::vec3(10.0f)); // Adjust scale as needed
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelForSun));
    if(!nightMode){
        // Draw the sun model
        lightCube.Draw(lightShader);
    }
    // ------------------------------------------------
    // 4) RENDER SKYBOX (If Fog Is Disabled)
    // ------------------------------------------------
    if (!fogEnabled) {
        if (nightMode) {
            nightSkyBox.Draw(skyboxShader, currentView, projection);
        } else {
            daySkyBox.Draw(skyboxShader, currentView, projection);
        }
    }
   
    // ------------------------------------------------
    // 5) RENDER SHADOW MAP VISUALIZATION (Optional)
    // ------------------------------------------------
    if (showDepthMap) {
        shadowMapVisualizerShader.useShaderProgram();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        renderQuad();
    }
}
// -----------------------------------------------------
// initFBO
// -----------------------------------------------------
void initFBO() {
    glGenFramebuffers(1, &shadowMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

    // Create depth texture
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Attach depth texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    // No color attachment
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
// -----------------------------------------------------
// initSkybox
// -----------------------------------------------------
void initSkybox() {
    std::vector<const GLchar*> dayFaces;
    dayFaces.push_back("skybox/day/day_middler.png");
    dayFaces.push_back("skybox/day/day_left.png");
    dayFaces.push_back("skybox/day/day_top.png");
    dayFaces.push_back("skybox/day/day_down.png");
    dayFaces.push_back("skybox/day/day_middle.png");
    dayFaces.push_back("skybox/day/day_right.png");

    
    std::vector<const GLchar*> nightFaces;

    nightFaces.push_back("skybox/night/night_middler.png");
    nightFaces.push_back("skybox/night/night_left.png");
    nightFaces.push_back("skybox/night/night_top.png");
    nightFaces.push_back("skybox/night/night_down.png");
    nightFaces.push_back("skybox/night/night_middle.png");
    nightFaces.push_back("skybox/night/night_right.png");
 
    daySkyBox.Load(dayFaces);
    
    
    nightSkyBox.Load(nightFaces);

}

// -----------------------------------------------------
// cleanup
// -----------------------------------------------------
void cleanup() {
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    glfwDestroyWindow(glWindow);

    glfwTerminate();
}

// -----------------------------------------------------
// main
// -----------------------------------------------------
int main(int argc, const char * argv[]) {

    if (!initOpenGLWindow()) {
        glfwTerminate();
        return 1;
    }

    initOpenGLState();
    initObjects();
    initShaders();
    initUniforms();
    initFBO();
    initSkybox();

    static double lastFrameTime = 0.0;
    double currentFrameTime = glfwGetTime();
    float deltaTime = (float)(currentFrameTime - lastFrameTime);
    lastFrameTime = currentFrameTime;
    
    // (Inside your main loop)
    while (!glfwWindowShouldClose(glWindow)) {
        // 1) Compute delta time
        double currentFrameTime = glfwGetTime();
        float deltaTime = (float)(currentFrameTime - lastFrameTime);
        lastFrameTime = currentFrameTime;

        // 2) Process input
        processMovement();

        // 3) Update camera animation (added)
        updateCameraAnimation(deltaTime);

        // 4) Render
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(glWindow);
    }

    cleanup();
    return 0;
}
