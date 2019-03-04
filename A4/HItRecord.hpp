#ifndef HitRecord_hpp
#define HitRecord_hpp

#include "Material.hpp"
#include <glm/glm.hpp>

class HitRecord {
    float t;
    glm::vec3 hit_point;
    glm::vec3 normal;
    Material *material;
};

#endif