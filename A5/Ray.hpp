#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>

class Ray {
public:
	glm::vec3 origin;
	glm::vec3 direction;

    Ray(const glm::vec3& o,const glm::vec3& d);
    Ray();

    glm::vec3 Get_origin();
    glm::vec3 Get_direction();

    glm::vec3 At_t(float t);
};
