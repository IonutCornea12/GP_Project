// BoundingSphere.hpp
#ifndef BOUNDINGSPHERE_HPP
#define BOUNDINGSPHERE_HPP

#include <glm/glm.hpp>

struct BoundingSphere {
    glm::vec3 center; // Center of the sphere in world coordinates
    float radius;     // Radius of the sphere

    // Constructor
    BoundingSphere() : center(glm::vec3(0.0f)), radius(1.0f) {}
    BoundingSphere(const glm::vec3& c, float r) : center(c), radius(r) {}
};

#endif // BOUNDINGSPHERE_HPP
