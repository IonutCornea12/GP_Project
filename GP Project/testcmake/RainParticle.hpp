// RainParticle.hpp
#ifndef RAINDROPPARTICLE_HPP
#define RAINDROPPARTICLE_HPP

#include <glm/glm.hpp>

struct RainParticle {
    glm::vec3 Position;
    glm::vec3 Velocity;
    float Life; // Remaining life of the particle. If <= 0, it will be respawned.

    RainParticle()
        : Position(0.0f), Velocity(0.0f), Life(1.0f) {}
};

#endif // RAINDROPPARTICLE_HPP
