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

const unsigned int SHADOW_WIDTH = 1920;
const unsigned int SHADOW_HEIGHT = 1080;

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
float cubeHeight = 200.0f;

// Input states
bool pressedKeys[1024];
float angleY = 0.0f;

GLfloat lightAngle = 0.0f; // Initialize lightAngle here
bool nightMode = false;
bool shadowsEnabled = true;
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
gps::Model3D screenQuad;  // The "sun" model

// Shaders
gps::Shader shaderStart;
gps::Shader lightShader;
gps::Shader skyboxShader;
gps::Shader depthShader;
gps::Shader screenQuadShader; // Shadow Map Visualizer Shader

// Fog + Depth
GLuint fogEnabledLoc;
bool showDepthMap;

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
    
//    if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
//        shadowsEnabled = !shadowsEnabled;
//        std::cout << "Shadows " << (shadowsEnabled ? "Enabled" : "Disabled") << std::endl;
//        
//        // Update the shader uniform
//        shaderStart.useShaderProgram();
//        glUniform1i(enableShadowsLoc, shadowsEnabled ? 1 : 0);
//    }
   // shaderStart.useShaderProgram();
    
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

void initQuad() {
    float quadVertices[] = {
        // Positions    // TexCoords
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f,
        -1.0f,  1.0f,   0.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}


// -----------------------------------------------------
// initUniforms
// -----------------------------------------------------
void initUniforms() {
    shaderStart.useShaderProgram();
    
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
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    lightDirLoc = glGetUniformLocation(shaderStart.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(shaderStart.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    // Fog
    fogEnabledLoc = glGetUniformLocation(shaderStart.shaderProgram, "enableFog");
    glUniform1i(fogEnabledLoc, fogEnabled);
    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    // Point lights
    GLint locPointLights = glGetUniformLocation(shaderStart.shaderProgram, "pointLightPositions");
    glUniform3fv(locPointLights, lampPositions.size(), glm::value_ptr(lampPositions[0]));
    GLint pointLightColorLoc = glGetUniformLocation(shaderStart.shaderProgram, "pointLightColor");
    glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));

    // Night Mode
    GLint nightModeLoc = glGetUniformLocation(shaderStart.shaderProgram, "nightMode");
    glUniform1i(nightModeLoc, nightMode);
}

//void renderDepthMap() {
//    // 1. Render scene to depth map
//    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
//    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
//    glClear(GL_DEPTH_BUFFER_BIT);
//    depthShader.useShaderProgram();
//    glUniformMatrix4fv(glGetUniformLocation(depthShader.shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
//    drawObjects(depthShader, true);
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//    
//    // 2. Render depth map to screen
//    glViewport(0, 0, retina_width, retina_height);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    
//    screenQuad.useShaderProgram();
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
//    glUniform1i(glGetUniformLocation(screenQuad.shaderProgram, "shadowMap"), 0);
//    
//    glBindVertexArray(quadVAO);
//    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//    glBindVertexArray(0);
//}
// -----------------------------------------------------
// renderScene
// -----------------------------------------------------
void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);
    glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    // Pass this matrix to the depth shader
    glUniformMatrix4fv(glGetUniformLocation(depthShader.shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    
    // Adjust directional light color based on night mode
    shaderStart.useShaderProgram();
    glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "enableFog"), fogEnabled);
    glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "nightMode"), nightMode);
    // Update light color based on night mode
    lightColor = nightMode ? glm::vec3(0.2f, 0.2f, 0.4f) : glm::vec3(1.0f, 1.0f, 1.0f);
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    
    // ------------------------------------------------
    // 1) RENDER DEPTH MAP (FBO)
    // ------------------------------------------------
    depthShader.useShaderProgram();
   
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    drawObjects(depthShader, true); // Render scene for depth map
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // ------------------------------------------------
    // 2) RENDER SCENE TO SCREEN (Final pass)
    // ------------------------------------------------
    if (showDepthMap) {
        glViewport(0, 0, retina_width, retina_height);

        glClear(GL_COLOR_BUFFER_BIT);

        screenQuadShader.useShaderProgram();

        //bind the depth map
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
        
        lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
        
        // Bind the shadow map before rendering the main scene
        
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "shadowMap"), 3);
        
        glUniform1i(glGetUniformLocation(shaderStart.shaderProgram, "shadowsEnabled"), shadowsEnabled);
        
        glUniformMatrix4fv(glGetUniformLocation(depthShader.shaderProgram, "lightSpaceMatrix"),
                           1,
                           GL_FALSE,
                           glm::value_ptr(computeLightSpaceMatrix()));
        
        drawObjects(shaderStart, false); // Render scene with shadows
        
        
        // Update View Matrix
        glm::mat4 currentView = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(currentView));
        
        
        // Update Directional Light Direction based on lightPosition
//        glm::vec3 lightDirWorld = glm::normalize(lightPosition); // Normalize to get direction
//        glm::vec3 lightDirTransformed = glm::normalize(glm::inverseTranspose(glm::mat3(currentView)) * lightDirWorld);
//        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTransformed));
        
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        // ------------------------------------------------
        // 3) RENDER SUN (Light Cube)
        // ------------------------------------------------
        lightShader.useShaderProgram();
        
        // Set view and projection matrices
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(currentView));
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE,glm::value_ptr(projection));

        float orbitRadius = 100.0f;

        // Update the model matrix
        model = glm::mat4(1.0f); // Start with an identity matrix
        model = glm::rotate(model, glm::radians(lightAngle), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around the y-axis
        model = glm::translate(model, glm::vec3(orbitRadius, cubeHeight, 0.0f)); // Translate to the orbit position
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f)); // Scale if necessary
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        
        lightCube.Draw(lightShader);

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
    }
}
// -----------------------------------------------------
// initFBO
// -----------------------------------------------------

void initFBO() {
    // Use the global shadowMapFBO and depthMapTexture
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

    // Initialize last frame time
    double lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(glWindow)) {
        // 1) Compute delta time
        double currentFrameTime = glfwGetTime();
        float deltaTime = (float)(currentFrameTime - lastFrameTime);
        lastFrameTime = currentFrameTime;

        // 2) Process input
        processMovement();

        // 3) Update camera animation
        updateCameraAnimation(deltaTime);

        // 4) Render depth map

        // 5) Render the main scene with shadows
        renderScene(); // Uses the depth map to render shadows

    
        glfwPollEvents();
        glfwSwapBuffers(glWindow);
    }

    cleanup();
    return 0;
}
