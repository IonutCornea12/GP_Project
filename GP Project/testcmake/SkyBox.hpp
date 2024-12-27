//
//  SkyBox.hpp
//  Example Cubemap / Skybox
//
//  Created by CGIS on 16/12/2016.
//  Modified to include GetTextureId().
//

#ifndef SkyBox_hpp
#define SkyBox_hpp

#include "Shader.hpp"
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <stdio.h>

namespace gps {

    class SkyBox
    {
    public:
        // Constructor
        SkyBox();
        
        // Load a skybox from 6 texture paths
        // 'cubeMapFaces' = {right, left, top, bottom, back, front}
        void Load(std::vector<const GLchar*> cubeMapFaces);
        
        // Draw the skybox using a given shader and the provided view/projection
        void Draw(gps::Shader shader, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
        
        // Returns the raw OpenGL texture ID for the cubemap
        GLuint GetTextureId();
        
    private:
        // Vertex Array Object and Vertex Buffer Object for the cube geometry
        GLuint skyboxVAO;
        GLuint skyboxVBO;
        
        // This holds the cubemap texture ID once loaded
        GLuint cubemapTexture;
        
        // Helper: load the 6 faces of the cubemap
        GLuint LoadSkyBoxTextures(std::vector<const GLchar*> cubeMapFaces);
        
        // Helper: set up the cube geometry (VAO, VBO)
        void InitSkyBox();
    };
}

#endif /* SkyBox_hpp */
