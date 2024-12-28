//
//  Shader.hpp
//  Lab3
//
//  Created by CGIS on 05/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#ifndef Shader_hpp
#define Shader_hpp

#if defined (__APPLE__)
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif
#include <glm/glm.hpp>              // Include GLM core
#include <glm/gtc/matrix_transform.hpp> // Include GLM transformations if needed
#include <fstream>
#include <sstream>
#include <iostream>


namespace gps {
    
    class Shader {

    public:
        GLuint shaderProgram;
        void loadShader(std::string vertexShaderFileName, std::string fragmentShaderFileName);
        void useShaderProgram();
        void setMat4(const std::string &name, const glm::mat4 &mat) const;
        void setVec3(const std::string &name, const glm::vec3 &value) const;
    private:
        std::string readShaderFile(std::string fileName);
        void shaderCompileLog(GLuint shaderId);
        void shaderLinkLog(GLuint shaderProgramId);
    };
    
}

#endif /* Shader_hpp */
