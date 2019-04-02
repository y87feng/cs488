// Winter 2019
#include <glm/glm.hpp>
#include <vector>

#include <iostream>
#include "Ray.hpp"
#include "Primitive.hpp"
#include "polyroots.hpp"
#include "Mesh.hpp"


using namespace glm;
using namespace std;


Primitive::~Primitive()
{
}

bool Primitive::hit( Ray &ray, float t_min, float t_max, HitRecord &record) {
    return false;
}

Sphere::Sphere() {
    m_nonhier_sphere = new NonhierSphere(vec3(0.0, 0.0, 0.0), 1.0);
}

Sphere::~Sphere()
{
    delete m_nonhier_sphere;
}

bool Sphere::hit( Ray &ray, float t_min, float t_max, HitRecord &record) {
    return m_nonhier_sphere->hit(ray, t_min, t_max, record);
}

Cube::Cube() {
    m_nonhier_cube = new NonhierBox(vec3(0.0, 0.0, 0.0), 1.0);
}
Cube::~Cube()
{
    delete m_nonhier_cube;
}

bool Cube::hit(Ray &ray, float t_min, float t_max, HitRecord &record) {
    return m_nonhier_cube->hit(ray, t_min, t_max, record);
}

NonhierSphere::NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius), velocity(0.0f)
  {
  }

NonhierSphere::NonhierSphere(const glm::vec3& pos, double radius, const glm::vec3 v)
 : m_pos(pos), m_radius(radius), velocity(v) {
}

NonhierSphere::~NonhierSphere()
{
}

bool NonhierSphere::hit(Ray &ray, float t_min, float t_max, HitRecord &record) {
	
    /*
        [(origin + t * direction) - center]^2 = radius^2
        d^2*t^2 + 2 * (o-c) * d * t + (o-c)^2 - r^2 = 0
    */
   vec3 pos = m_pos + ray.get_time() * velocity;
//    cout << "ray time is " << ray.get_time() << endl;
//    cout << "pos is " << to_string(pos) << endl;

   vec3 dis_oc = ray.Get_origin() - pos;

   double A = dot(ray.Get_direction(), ray.Get_direction());
   double B = 2 * dot(ray.Get_direction(), dis_oc);
   double C = dot(dis_oc, dis_oc) - m_radius * m_radius;

    double roots[2];
    size_t n_roots =  quadraticRoots(A, B, C, roots);

    float t = 0;
    if (n_roots == 0) {
        return false;
    } else if (n_roots == 1) {
        t = roots[0];
    } else {
        t = glm::min(roots[0], roots[1]);
    }

    // cout << "Nsphere current root is " << n_roots  <<" " << roots[0] << " " << roots[1] << endl;

    if ( t <= t_min || t >= t_max ) return false;

    // cout << "pass interval check " << t_min  <<" " <<t_max << endl;
    
    // record
    record.t = t;
    record.hit_point = ray.At_t(t);
    record.normal = record.hit_point - m_pos;

    return true;
}

NonhierBox::NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
  {
    vertices.resize(8);
    vertices[0] = m_pos + glm::vec3(0.0, 0.0, 0.0);
    vertices[1] = m_pos + glm::vec3(m_size, 0.0, 0.0);
    vertices[2] = m_pos + glm::vec3(m_size, 0.0, m_size);
    vertices[3] = m_pos + glm::vec3(0.0, 0.0, m_size);
    vertices[4] = m_pos + glm::vec3(0.0, m_size, 0.0);
    vertices[5] = m_pos + glm::vec3(m_size, m_size, 0.0);
    vertices[6] = m_pos + glm::vec3(m_size, m_size, m_size);
    vertices[7] = m_pos + glm::vec3(0.0, m_size, m_size);
    
    std::vector<Triangle> triangle_inx = {
      Triangle(0, 1, 2),
      Triangle(0, 2, 3),
      Triangle(0, 7, 4),
      Triangle(0, 3, 7),
      Triangle(0, 4, 5),
      Triangle(0, 5, 1),
      Triangle(6, 2, 1),
      Triangle(6, 1, 5),
      Triangle(6, 5, 4),
      Triangle(6, 4, 7),
      Triangle(6, 7, 3),
      Triangle(6, 3, 2)
    };

    m_mesh = new Mesh(&vertices, triangle_inx);
  }

NonhierBox::~NonhierBox()
{
    delete m_mesh;
}

bool NonhierBox::hit(Ray &ray, float t_min, float t_max, HitRecord &record) {
   return m_mesh->hit(ray, t_min, t_max, record);
}

NonhierTriPrism::NonhierTriPrism(const glm::vec3& pos, double side_len, double height) {
    m_pos = pos;
    m_side_len = side_len;
    m_height = height;

    vertices.resize(6);
    vertices[0] = m_pos + glm::vec3(0.0, 0.0, 0.0);
    vertices[1] = m_pos + glm::vec3(side_len, 0.0, 0.0);
    vertices[2] = m_pos + glm::vec3(side_len/2, 0.0, side_len * sqrt(3) / 2);
    vertices[3] = m_pos + glm::vec3(0.0, height, 0);
    vertices[4] = m_pos + glm::vec3(side_len, height, 0.0);
    vertices[5] = m_pos + glm::vec3(side_len/2, height, side_len * sqrt(3) / 2);
    
    std::vector<Triangle> triangle_inx = {
      Triangle(0, 1, 2),
      Triangle(0, 1, 3),
      Triangle(1, 3, 4),
      Triangle(0, 2, 3),
      Triangle(2, 3, 5),
      Triangle(3, 4, 5),
      Triangle(1, 2, 4),
      Triangle(2, 4, 5)
    };

    m_mesh = new Mesh(&vertices, triangle_inx);
}

NonhierTriPrism::~NonhierTriPrism()
{
    delete m_mesh;
}

bool NonhierTriPrism::hit(Ray &ray, float t_min, float t_max, HitRecord &record) {
   return m_mesh->hit(ray, t_min, t_max, record);
}

NonhierTriPyramid::NonhierTriPyramid(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size) {
    vertices.resize(4);
    vertices[0] = m_pos + glm::vec3(0.0, 0.0, 0.0);
    vertices[1] = m_pos + glm::vec3(size, 0.0, 0.0);
    vertices[2] = m_pos + glm::vec3(size/2, 0.0, size * sqrt(3) / 2);
    vertices[3] = m_pos + glm::vec3(size/2, size, size/2);
    
    std::vector<Triangle> triangle_inx = {
      Triangle(0, 1, 2),
      Triangle(0, 1, 3),
      Triangle(1, 2, 3),
      Triangle(0, 2, 3)
    };

    m_mesh = new Mesh(&vertices, triangle_inx);
}

NonhierTriPyramid::~NonhierTriPyramid()
{
    delete m_mesh;
}

bool NonhierTriPyramid::hit(Ray &ray, float t_min, float t_max, HitRecord &record) {
   return m_mesh->hit(ray, t_min, t_max, record);
}