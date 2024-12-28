// Rain.cpp
#include "Rain.hpp"

Rain::Rain(unsigned int maxParticles, gps::Shader& shader, gps::Model3D& quadModel, glm::vec3 areaSize)
    : maxParticles(maxParticles), shader(shader), quadModel(quadModel), areaSize(areaSize),
      distributionPosX(-areaSize.x / 2.0f, areaSize.x / 2.0f),
      distributionPosY(10.0f, 20.0f), // Height above the ground
      distributionPosZ(-areaSize.z / 2.0f, areaSize.z / 2.0f),
      distributionLife(5.0f, 10.0f) // Lifetime in seconds
{
    particles.resize(maxParticles);
    for(auto &particle : particles) {
        RespawnParticle(particle);
    }
}

Rain::~Rain() {
    // No dynamic memory to clean up in this implementation
}

void Rain::RespawnParticle(RainParticle& particle)
{
    particle.Position = glm::vec3(distributionPosX(generator),
                                  distributionPosY(generator),
                                  distributionPosZ(generator));
    particle.Velocity = glm::vec3(0.0f, -50.0f, 0.0f); // Fast downward velocity
    particle.Life = distributionLife(generator);
}

void Rain::Update(float deltaTime)
{
    for(auto &particle : particles)
    {
        particle.Life -= deltaTime;
        if(particle.Life > 0.0f)
        {
            // Update position based on velocity
            particle.Position += particle.Velocity * deltaTime;

            // Optionally, add some variation to velocity or direction
            // For simplicity, we're keeping it constant here
        }
        else
        {
            // Respawn particle
            RespawnParticle(particle);
        }
    }
}

void Rain::Render(const glm::mat4& projection, const glm::mat4& view)
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

    // Render each raindrop
    for(auto &particle : particles)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, particle.Position);
        model = glm::scale(model, glm::vec3(0.1f)); // Adjust size as needed
        shader.setMat4("model", model);
        shader.setVec3("color", glm::vec3(0.5f, 0.5f, 0.5f)); // Set color uniform
        quadModel.Draw(shader);
    }

    // Re-enable depth mask and disable blending
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
