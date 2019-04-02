// Fall 2018

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "HitRecord.hpp"
#include "Ray.hpp"


class Mesh;
class Primitive {
public:
  virtual ~Primitive();
  virtual bool hit( Ray &ray, float t_min, float t_max, HitRecord &record);
};

class Sphere : public Primitive {
  Primitive * m_nonhier_sphere;
public:
  Sphere();
  virtual ~Sphere();
  virtual bool hit( Ray &ray, float t_min, float t_max, HitRecord &record) override;
};

class Cube : public Primitive {
  Primitive * m_nonhier_cube;
public:
  Cube();
  virtual ~Cube();
  virtual bool hit(Ray &ray, float t_min, float t_max, HitRecord &record) override;
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius);
  NonhierSphere(const glm::vec3& pos, double radius, const glm::vec3 v);
  virtual ~NonhierSphere();
  virtual bool hit( Ray &ray, float t_min, float t_max, HitRecord &record) override;

private:
  glm::vec3 m_pos;
  double m_radius;
  glm::vec3 velocity;

  glm::vec3 vertices[8];
};


class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size);

  virtual ~NonhierBox();
  virtual bool hit( Ray &ray, float t_min, float t_max, HitRecord &record) override;
private:
  glm::vec3 m_pos;
  double m_size;
  std::vector<glm::vec3> vertices;

  Primitive * m_mesh;
};

class NonhierTriPrism : public Primitive {
public:
  NonhierTriPrism(const glm::vec3& pos, double side_len, double height);

  virtual ~NonhierTriPrism();
  virtual bool hit( Ray &ray, float t_min, float t_max, HitRecord &record) override;
private:
  glm::vec3 m_pos;
  double m_side_len;
  double m_height;
  std::vector<glm::vec3> vertices;

  Primitive * m_mesh;
};

class NonhierTriPyramid : public Primitive {
public:
  NonhierTriPyramid(const glm::vec3& pos, double size);

  virtual ~NonhierTriPyramid();
  virtual bool hit( Ray &ray, float t_min, float t_max, HitRecord &record) override;
private:
  glm::vec3 m_pos;
  double m_size;
  std::vector<glm::vec3> vertices;

  Primitive * m_mesh;
};