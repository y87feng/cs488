#include "Ray.hpp"

using namespace std;
using namespace glm;

Ray::Ray(vec3& o, vec3& d) {
    origin = o;
    direction d;
}

vec3 Ray::Get_origin() const {
    return origin;
}

vec3 Ray::Get_direction() const {
    return direction;
}

vec3 Ray::at_t(float t) const {
    return origin + t * direction;
}
