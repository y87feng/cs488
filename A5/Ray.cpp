#include <iostream>
#include "Ray.hpp"

using namespace std;
using namespace glm;

Ray::Ray() {
    origin = vec3();
    direction =  vec3();
    time = 0.0f;
}

Ray::Ray(const vec3& o,const vec3& d) {
    origin = o;
    direction = d;
    time = 0.0f;
}

Ray::Ray(const glm::vec3& o,const glm::vec3& d, float t) {
    origin = o;
    direction = d;
    time = t;
    // cout << "here ray time is " << time << endl;
}

vec3 Ray::Get_origin() {
    return origin;
}

vec3 Ray::Get_direction() {
    return direction;
}

vec3 Ray::At_t(float t) {
    return origin + t * direction;
}

float Ray::get_time() {
    return time;
}