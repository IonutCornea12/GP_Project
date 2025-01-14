#include "Rain.hpp"
#include <glm/gtx/norm.hpp> // for glm::distance2

Rain::Rain(unsigned int maxParticles,
           gps::Shader& shader,
           gps::Model3D& quadModel,
           glm::vec3 areaSize,
           const glm::vec3& initialCameraPos,
           bool isSnow)
    : maxParticles(maxParticles),
      shader(shader),
      quadModel(quadModel),
      areaSize(areaSize),
      isSnow(isSnow)
{
 

    if (!isSnow) {
        // RAIN distributions
        distributionPosX = std::uniform_real_distribution<float>(-areaSize.x/2.0f, areaSize.x/2.0f);
        distributionPosY = std::uniform_real_distribution<float>(10.0f, 20.0f); // spawn above camera
        distributionPosZ = std::uniform_real_distribution<float>(-areaSize.z/2.0f, areaSize.z/2.0f);
        distributionLife = std::uniform_real_distribution<float>(5.0f, 10.0f);
        distributionWindX = std::uniform_real_distribution<float>(-0.1f, 0.1f);
    }
    else {
        // SNOW distributions
        distributionPosX = std::uniform_real_distribution<float>(-areaSize.x/2.0f, areaSize.x/2.0f);
        distributionPosY = std::uniform_real_distribution<float>(15.0f, 30.0f); // spawn higher
        distributionPosZ = std::uniform_real_distribution<float>(-areaSize.z/2.0f, areaSize.z/2.0f);
        distributionLife = std::uniform_real_distribution<float>(6.0f, 12.0f);
        //more horizontal drift
        distributionWindX = std::uniform_real_distribution<float>(-1.0f, 1.0f);
    }

    // Resize vector and respawn all
    particles.resize(maxParticles);//change size of vector to mactch size of maxParticles
    for (auto &particle : particles) {
        RespawnParticle(particle, initialCameraPos);
    }
}

Rain::~Rain() {}

void Rain::RespawnParticle(RainParticle& particle, const glm::vec3& cameraPos)
{
    particle.Position = cameraPos + glm::vec3(
        distributionPosX(generator),
        distributionPosY(generator),
        distributionPosZ(generator)
    );

    particle.Life = distributionLife(generator);

    if (!isSnow) {
        // RAIN velocity
        // Big downward velocity, minimal horizontal
        particle.Velocity = glm::vec3(0.0f, -50.0f, 0.0f);
    }
    else {
        // SNOW velocity
        float windX = distributionWindX(generator);
        // slower downward, some horizontal
        particle.Velocity = glm::vec3(windX, -10.0f, 0.0f);
    }
}

void Rain::Update(float deltaTime, const glm::vec3& cameraPos)
{
    for (auto &particle : particles) {
        particle.Life -= deltaTime;//lifespan decrease
        if (particle.Life > 0.0f) {
            // Move the particle
            particle.Position += particle.Velocity * deltaTime;

            if (!isSnow) {
                // RAIN: accelerates downward faster
                particle.Velocity.y -= 0.9f * deltaTime;
            }
            else {
                // SNOW: gently accelerate downward, also random drift
                float driftVariation = 20.0f * deltaTime;
                particle.Velocity.x += driftVariation * ((float)rand() / RAND_MAX - 0.5f);

                // slow gravity
                particle.Velocity.y -= 0.1f * deltaTime;
            }
        }
        else {
            // Respawn again above the camera
            RespawnParticle(particle, cameraPos);
        }
    }
}

void Rain::Render(const glm::mat4& projection,
                  const glm::mat4& view,
                  const glm::vec3& cameraPos)
{
    shader.useShaderProgram();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    if (!isSnow) {
        // RAIN color = gray
        shader.setVec3("rainColor", glm::vec3(0.4f, 0.4f, 0.5f));
    }
    else {
        // SNOW color - white
        shader.setVec3("rainColor", glm::vec3(1.0f, 1.0f, 1.0f));
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_FALSE);

    float maxDistanceSquared = (isSnow ? 80.0f : 50.0f);
    maxDistanceSquared *= maxDistanceSquared;

    for (auto &particle : particles) {
        float dist2 = glm::distance2(particle.Position, cameraPos);
        if (dist2 <= maxDistanceSquared) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, particle.Position);
            if (!isSnow) {
                //size
                model = glm::scale(model, glm::vec3(0.15f));
            } else {
                // snowflake is bigger
                model = glm::scale(model, glm::vec3(0.2f));
            }

            shader.setMat4("model", model);
            quadModel.Draw(shader);
        }
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
