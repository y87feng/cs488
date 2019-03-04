#ifndef Ray_hpp
#define Ray_hpp

#include <glm/ext.hpp>
#include "PhongMaterial.hpp"


class Ray {
public:
	glm::vec3 origin;
	glm::vec3 direction;

    Ray(glm::vec3& o, glm::vec3& d);

    glm::vec3 Get_origin() const;
    glm::vec3 Get_direction() const;

    glm::vec3 At_r(float r) const;
};

#endif