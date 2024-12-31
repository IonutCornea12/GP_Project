#ifndef RAIN_HPP
#define RAIN_HPP

#include <vector>
#include <random>
#include <glm/glm.hpp>
#include "RainParticle.hpp"
#include "Shader.hpp"
#include "Model3D.hpp"

class Rain {
public:
    // Updated constructor to include isSnow
    Rain(unsigned int maxParticles,
         gps::Shader& shader,
         gps::Model3D& quadModel,
         glm::vec3 areaSize,
         const glm::vec3& initialCameraPos,
         bool isSnow = false); 

    ~Rain();

    void Update(float deltaTime, const glm::vec3& cameraPos);
    void Render(const glm::mat4& projection,
                const glm::mat4& view,
                const glm::vec3& cameraPos);

private:
    std::vector<RainParticle> particles;
    unsigned int maxParticles;
    gps::Shader& shader;
    gps::Model3D& quadModel;
    glm::vec3 areaSize;

    bool isSnow; // <-- Indicate if we do rain or snow

    // Random number generation
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distributionPosX;
    std::uniform_real_distribution<float> distributionPosY;
    std::uniform_real_distribution<float> distributionPosZ;
    std::uniform_real_distribution<float> distributionLife;

    // Optional: for sideways drift (common for snow)
    std::uniform_real_distribution<float> distributionWindX;

    // Respawn particles relative to camera position
    void RespawnParticle(RainParticle& particle,
                         const glm::vec3& cameraPos);
};

#endif // RAIN_HPP
