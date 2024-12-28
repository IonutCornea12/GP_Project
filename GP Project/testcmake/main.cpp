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
#include "Rain.hpp"
#include "RainParticle.hpp"
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
glm::mat4 sunmodel;
GLuint sunmodelLoc;

GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;
glm::mat4 lightProjection;
glm::mat4 lightView;
glm::mat4 lightSpaceMatrix;
// Directional light
glm::vec3 lightDir;      // base direction
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
GLuint shadowMapFBO;
GLuint depthMapTexture;

// Light Position (Global)
glm::vec3 lightPosition(400.0f, 400.0f, 1.0f);
// Camera
gps::Camera myCamera(
    glm::vec3(-42.7423f, 3.57602f, 57.6629f),//sta
    glm::vec3(-68.0686f, 2.66757f, 39.8092f),
    glm::vec3(0.0f, 1.0f, 0.0f)
);

float cameraSpeed = 0.5f;

// Position/orbit radius for the "sun" cube
float cubeHeight = 130.0f;
float orbitRadius = 130.0f;
// Input states
bool pressedKeys[1024];
float angleY = 0.0f;

GLfloat lightAngle = 0.0f; // Initialize lightAngle here
bool nightMode = false;
bool shadowsEnabled = true;
bool fogEnabled = false;
bool cameraAnimationActive = false;
bool rainEnabled = false;
bool flashActive = false;

// At the top of main.cpp, among other global variables
Rain* rainSystem = nullptr;
// Models

// Flash Effect Timing Variables
float rainElapsedTime = 0.0f;
float rainElapsedTime2 = 0.0f;
float flashDuration = 0.0f;
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
gps::Model3D screenQuad;  // The "sun" model
gps::Model3D rainQuad;  // The "sun" model

// Shaders
gps::Shader shaderStart;
gps::Shader lightShader;
gps::Shader skyboxShader;
gps::Shader depthShader;
gps::Shader screenQuadShader; // Shadow Map Visualizer Shader
gps::Shader rainShader; // Shadow Map Visualizer Shader

// Fog + Depth
GLuint fogEnabledLoc;
GLuint rainEnabledLoc;
bool showDepthMap;

// Skybox
gps::SkyBox daySkyBox;
gps::SkyBox nightSkyBox;

GLuint daySkyID = daySkyBox.GetTextureId();   // for day skybox
GLuint nightSkyID = nightSkyBox.GetTextureId(); // for night skybox
enum class FlashState {
    IDLE,
    LIGHTING,
    FLASHING
};

FlashState currentFlashState = FlashState::IDLE;
float flashTimer = 0.0f;
float eventTimer = 0.0f; // Timer to accumulate time for events

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
// Define a simple quad (two triangles) for the raindrop

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
    if (key == GLFW_KEY_7 && action == GLFW_PRESS) {
        rainEnabled = !rainEnabled; // Toggle rain
        
        if (rainEnabled) {
            fogEnabled = true; // Ensure fog is enabled when rain starts
        } else {
            fogEnabled = false;
        }
        
        std::cout << "Rain: " << (rainEnabled ? "Enabled" : "Disabled") << std::endl;
    }
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
// initOpenGLState
// -----------------------------------------------------
void initOpenGLState() {
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glViewport(0, 0, retina_width, retina_height);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glDepthFunc(GL_LEQUAL);
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
    screenQuad.LoadModel("objects/quad/quad.obj");
    rainQuad.LoadModel("objects/ground/rain.obj");
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
    
    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    screenQuadShader.useShaderProgram();
            
    rainShader.loadShader("shaders/rainShader.vert", "shaders/rainShader.frag");
    rainShader.useShaderProgram();
}

glm::mat4 computeLightSpaceMatrix() {
    // Light projection matrix (orthographic for directional light)
    float near_plane = 1.0f;
    float far_plane = 1000.0f;
    float ortho_width = 50.0f;
    float ortho_height = 50.0f;
    glm::mat4 lightProjection = glm::ortho(-ortho_width, ortho_width, -ortho_height, ortho_height, near_plane, far_plane);

    // Light view matrix (light position and direction)
    glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f);  // Assume light points towards the origin
    glm::vec3 lightUp = glm::vec3(0.0f, 1.0f, 0.0f);      // Up direction
    glm::mat4 lightView = glm::lookAt(lightPosition, lightTarget, lightUp);

    // Combine the matrices
    return lightProjection * lightView;
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
// initUniforms
// -----------------------------------------------------
void initUniforms() {
    shaderStart.useShaderProgram();
    lightAngle = 0.0f; // Initialize lightAngle here

    // Model, View, Projection matrices
    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(shaderStart.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(shaderStart.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    normalMatrixLoc = glGetUniformLocation(shaderStart.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    
    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(shaderStart.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(400.0f, 400.0f, 0.0f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    lightDirLoc = glGetUniformLocation(shaderStart.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(shaderStart.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
     // Point lights
    GLint locPointLights = glGetUniformLocation(shaderStart.shaderProgram, "pointLightPositions");
    glUniform3fv(locPointLights, lampPositions.size(), glm::value_ptr(lampPositions[0]));
    GLint pointLightColorLoc = glGetUniformLocation(shaderStart.shaderProgram, "pointLightColor");
    glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));
    // Night Mode
    GLint nightModeLoc = glGetUniformLocation(shaderStart.shaderProgram, "nightMode");
    glUniform1i(nightModeLoc, nightMode);
    glUniform1f(glGetUniformLocation(shaderStart.shaderProgram, "flashMultiplier"), 1.0f);

    // Initialize Rain System
    unsigned int maxRainParticles = 100000;
     glm::vec3 rainAreaSize(100.0f, 20.0f, 100.0f); // Define the area where rain occurs
     glm::vec3 initialCameraPos = myCamera.getPosition(); // Get actual camera position
     rainSystem = new Rain(maxRainParticles, rainShader, rainQuad, rainAreaSize, initialCameraPos);
    
    // Initialize Flash Multiplier to 1.0 (no flash)
}

// -----------------------------------------------------
// renderScene
// -----------------------------------------------------
void renderScene(float deltaTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // ------------------------------------------------
    // 1) RENDER SUN (Light Cube)
    // ------------------------------------------------
    lightShader.useShaderProgram();

    // Set view and projection matrices
    glm::mat4 currentView = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(currentView));
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Calculate model matrix with updated height and orbit
    glm::mat4 lightModel = glm::mat4(1.0f);
    lightModel = glm::rotate(lightModel, glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around y-axis
    lightModel = glm::translate(lightModel, glm::vec3(orbitRadius, cubeHeight, 0.0f)); // Translate to orbit position with height
    lightModel = glm::scale(lightModel, glm::vec3(10.0f, 10.0f, 10.0f)); // Scale by 10x

    // Pass the model matrix to the shader
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));

    // Update lightPosition based on model matrix
    glm::vec4 lightPosWorld = lightModel * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Assuming lightCube is centered at origin
    lightPosition = glm::vec3(lightPosWorld); // Update global light position

    // Compute lightSpaceMatrix based on updated lightPosition
    lightSpaceMatrix = computeLightSpaceMatrix();

    // Pass lightSpaceMatrix to depthShader
    glUseProgram(depthShader.shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(depthShader.shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    // Adjust directional light color based on night mode
    glUseProgram(shaderStart.shaderProgram);
    glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "enableFog"), fogEnabled);
    glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "nightMode"), nightMode);
    glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "rainEnabled"), rainEnabled);

    // Update light color based on night mode
    lightColor = nightMode ? glm::vec3(0.2f, 0.2f, 0.4f) : glm::vec3(1.0f, 1.0f, 1.0f);
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    // ------------------------------------------------
    // 2) RENDER DEPTH MAP (FBO)
    // ------------------------------------------------
    depthShader.useShaderProgram();

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    drawObjects(depthShader, true); // Render scene for depth map
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ------------------------------------------------
    // 3) RENDER SCENE TO SCREEN (Final pass)
    // ------------------------------------------------
    if (showDepthMap) {
        glViewport(0, 0, retina_width, retina_height);
        glClear(GL_COLOR_BUFFER_BIT);

        screenQuadShader.useShaderProgram();

        // Bind the depth map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 1);

        glDisable(GL_DEPTH_TEST);
        screenQuad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glViewport(0, 0, retina_width, retina_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderStart.useShaderProgram();
        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Update light direction based on rotation
        lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

        // Bind the shadow map
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "shadowMap"), 3);


        // Pass the updated lightSpaceMatrix to shaderStart
        glUniformMatrix4fv(glGetUniformLocation(shaderStart.shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        // Render all objects with shadows
        drawObjects(shaderStart, false);

        // Update View Matrix
        glm::mat4 currentView = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(currentView));

        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // ------------------------------------------------
        // 4) RENDER RAIN && Flashed
        // ------------------------------------------------
        if (rainEnabled) {
            eventTimer += deltaTime;
            
            switch (currentFlashState) {
                case FlashState::IDLE:
                    if (eventTimer >= 7.0f) { // Every 5 seconds
                        currentFlashState = FlashState::LIGHTING;
                        flashTimer = 1.0f; // Light objects for 1 second
                        eventTimer = 0.0f;
                        std::cout << "Lighting Activated!" << std::endl;
                    }
                    break;
                case FlashState::LIGHTING:
                    flashTimer -= deltaTime;
                    if (flashTimer <= 0.0f) {
                        currentFlashState = FlashState::FLASHING;
                        flashTimer = 0.5f; // Flash for 2 seconds
                        std::cout << "Flashing Activated!" << std::endl;
                    }
                    break;
                case FlashState::FLASHING:
                    flashTimer -= deltaTime;
                    if (flashTimer <= 0.0f) {
                        currentFlashState = FlashState::IDLE;
                        flashTimer = 0.0f;
                        std::cout << "Flash Deactivated!" << std::endl;
                    }
                    break;
            }
        }
            shaderStart.useShaderProgram();
                       if (currentFlashState == FlashState::LIGHTING) {
                           glUniform1f(glGetUniformLocation(shaderStart.shaderProgram, "flashMultiplier"), 1.5f); // Increase lighting
                           glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "flashActive"), 0);
                       }
                       else if (currentFlashState == FlashState::FLASHING) {
                           glUniform1f(glGetUniformLocation(shaderStart.shaderProgram, "flashMultiplier"), 1.0f); // Normal lighting
                           glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "flashActive"), 1); // Flash to white
                       }
                       else { // IDLE
                           glUniform1f(glGetUniformLocation(shaderStart.shaderProgram, "flashMultiplier"), 1.0f); // Normal lighting
                           glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "flashActive"), 0);
                       }
        if (rainEnabled && rainSystem != nullptr) {
              glm::vec3 cameraPosition = myCamera.getPosition(); // Get camera position
              rainSystem->Update(deltaTime, cameraPosition);     // Update rain system
              rainSystem->Render(projection, view, cameraPosition); // Render rain
          }
        // ------------------------------------------------
        // 5) RENDER SKYBOX (If Fog Is Disabled)
        // ------------------------------------------------
        if (!fogEnabled && !rainEnabled) {
            if (nightMode) {
                nightSkyBox.Draw(skyboxShader, currentView, projection);
            } else {
                daySkyBox.Draw(skyboxShader, currentView, projection);
            }
        }
    }
    // ------------------------------------------------
    // 6) RENDER SUN (Light Cube)
    // ------------------------------------------------
    lightShader.useShaderProgram();

    // Ensure the correct uniforms are set again (optional redundancy)
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
    // Draw lightCube only if fog is disabled, night mode is off, and rain is not active
    if(!fogEnabled && !nightMode && !rainEnabled){
        lightCube.Draw(lightShader);
    }
    
}
// main.cpp - initFBO
void initFBO() {
    glGenFramebuffers(1, &shadowMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, depthMapTexture, 0
    );
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;

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
    delete rainSystem; // Add this line

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

    double lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(glWindow)) {
        double currentFrameTime = glfwGetTime();
        float deltaTime = (float)(currentFrameTime - lastFrameTime);
        lastFrameTime = currentFrameTime;

        processMovement();

        updateCameraAnimation(deltaTime);

        renderScene(deltaTime);


        glfwPollEvents();
        glfwSwapBuffers(glWindow);
    }

    cleanup();
    return 0;
}
