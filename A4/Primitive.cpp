// Winter 2019

#include "Primitive.hpp"

Primitive::~Primitive()
{
}

bool Primitive::hit(const Ray &ray, float t_min, float t_max, HitRecord &record) {
    return false;
}

Sphere::~Sphere()
{
}

Cube::~Cube()
{
}

NonhierSphere::~NonhierSphere()
{
}

NonhierBox::~NonhierBox()
{
}
