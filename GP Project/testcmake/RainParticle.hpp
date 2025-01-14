// RainParticle.hpp
#ifndef RAINDROPPARTICLE_HPP
#define RAINDROPPARTICLE_HPP

#include <glm/glm.hpp>

struct RainParticle {
    glm::vec3 Position;  // Position of the particle in 3D space
    glm::vec3 Velocity;  // Velocity vector indicating the direction and speed
    float Life;          // Remaining lifespan of the particle

    RainParticle()
        : Position(0.0f), Velocity(0.0f), Life(1.0f) {}
};

#endif
