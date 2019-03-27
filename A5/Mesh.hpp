// Winter 2019

#pragma once

#include <vector>
#include <iosfwd>
#include <string>

#include <glm/glm.hpp>

#include "Primitive.hpp"
#include "options.hpp"


struct Triangle
{
	size_t v1;
	size_t v2;
	size_t v3;

	Triangle( size_t pv1 = 0, size_t pv2 = 0, size_t pv3 = 0)
		: v1( pv1 )
		, v2( pv2 )
		, v3( pv3 )
	{}

	size_t operator[] (int x) const {
			if ( x == 0 ) return v1;
			if ( x == 1 ) return v2;
			return v3;
	}
};

// A polygonal mesh.
class Mesh : public Primitive {
public:
  Mesh( const std::string& fname );
  Mesh( std::vector<glm::vec3>* all_vertices, std::vector<Triangle> &faces);
  virtual bool hit(Ray &ray, float t_min, float t_max, HitRecord &record) override;
	~Mesh();
private:
	std::vector<glm::vec3>* m_vertices;
	std::vector<Triangle> m_faces;
	std::vector<glm::vec3> m_real_vertices;

#ifdef ENABLE_BVH
    Triangle m_face;
		Mesh * left;
		Mesh * right;
#endif 

	glm::vec3 min_xyz;
	glm::vec3 max_xyz;

#ifdef ENABLE_BVH
	void BVH_Split();
#endif

    friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
};
