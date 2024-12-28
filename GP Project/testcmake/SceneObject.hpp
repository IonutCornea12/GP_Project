// SceneObject.hpp
#ifndef SCENEOBJECT_HPP
#define SCENEOBJECT_HPP

#include "Model3D.hpp"
#include "BoundingSphere.hpp"
#include <glm/glm.hpp>

class SceneObject {
public:
    gps::Model3D* model;      // Pointer to the Model3D instance
    glm::mat4 modelMatrix;    // Transformation matrix (position, rotation, scale)
    BoundingSphere boundingSphere; // Bounding sphere in world space

    // Constructor
    SceneObject(gps::Model3D* mdl, const glm::mat4& mtx, const BoundingSphere& bs)
        : model(mdl), modelMatrix(mtx), boundingSphere(bs) {}
};

#endif // SCENEOBJECT_HPP
