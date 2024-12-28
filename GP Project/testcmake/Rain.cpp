// Rain.cpp
#include "Rain.hpp"
#include <glm/gtx/norm.hpp> // For glm::distance2

Rain::Rain(unsigned int maxParticles, gps::Shader& shader, gps::Model3D& quadModel, glm::vec3 areaSize, const glm::vec3& initialCameraPos)
    : maxParticles(maxParticles), shader(shader), quadModel(quadModel), areaSize(areaSize),
      distributionPosX(-areaSize.x / 2.0f, areaSize.x / 2.0f),
      distributionPosY(10.0f, 20.0f), // Height above the ground
      distributionPosZ(-areaSize.z / 2.0f, areaSize.z / 2.0f),
      distributionLife(5.0f, 10.0f) // Lifetime in seconds
{
    particles.resize(maxParticles);
    for(auto &particle : particles) {
        RespawnParticle(particle, initialCameraPos);
    }

    // If using instanced rendering, initialize instanceVBO here
    // (Omitted for brevity)
}

Rain::~Rain() {
    // Clean up the instance VBO if using instanced rendering
    // glDeleteBuffers(1, &instanceVBO);
}

void Rain::RespawnParticle(RainParticle& particle, const glm::vec3& cameraPos)
{
    particle.Position = cameraPos + glm::vec3(distributionPosX(generator),
                                            distributionPosY(generator),
                                            distributionPosZ(generator));
    particle.Velocity = glm::vec3(0.0f, -50.0f, 0.0f); // Fast downward velocity
    particle.Life = distributionLife(generator);
}

void Rain::Update(float deltaTime, const glm::vec3& cameraPos)
{
    for(auto &particle : particles)
    {
        particle.Life -= deltaTime;
        if(particle.Life > 0.0f)
        {
            // Update position based on velocity
            particle.Position += particle.Velocity * deltaTime;

            // Optional: Add slight variation to velocity for realism
            particle.Velocity += glm::vec3(0.0f, -1.0f, 0.0f) * deltaTime;
        }
        else
        {
            // Respawn particle relative to camera position
            RespawnParticle(particle, cameraPos);
        }
    }
}

void Rain::Render(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPos)
{
    shader.useShaderProgram();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setVec3("rainColor", glm::vec3(0.5f, 0.5f, 0.5f)); // Grey color for rain

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Disable depth mask to allow blending
    glDepthMask(GL_FALSE);

    // Define maximum render distance
    const float maxDistanceSquared = 50.0f * 50.0f; // 50 units radius

    // Render each raindrop within the range
    for(auto &particle : particles)
    {
        // Compute squared distance to avoid sqrt for performance
        float distanceSquared = glm::distance2(particle.Position, cameraPos);
        if(distanceSquared <= maxDistanceSquared)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, particle.Position);
            model = glm::scale(model, glm::vec3(0.1f)); // Adjust size as needed
            shader.setMat4("model", model);
            quadModel.Draw(shader);
        }
    }

    // Re-enable depth mask and disable blending
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
