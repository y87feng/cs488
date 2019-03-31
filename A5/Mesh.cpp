// Winter 2019

#include <iostream>
#include <fstream>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <algorithm>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"
#include "options.hpp"
#include "polyroots.hpp"
#include "random.hpp"

using namespace glm;
using namespace std;

static const float Epsilon = 1e-2;

// retrieve https://github.com/stharlan/oglwalker/blob/768c3fb65ebf831f20dae0747b72d9f959f1f121/OglWalker/GeometryOperations.cpp
bool triangleIntersection(Ray &ray, vec3 vertex0, vec3 vertex1, vec3 vertex2, float &res) {
	const float EPSILON = 0.0000001;
	glm::vec3 edge1, edge2, h, s, q;
	float a, f, u, v;
	edge1 = vertex1 - vertex0;
	edge2 = vertex2 - vertex0; 

	h = glm::cross(ray.Get_direction(), edge2);
	a = glm::dot(edge1, h);
	if (a > -EPSILON && a < EPSILON)
		return false;
	f = 1 / a;
	s = ray.Get_origin() - vertex0;
	u = f * (glm::dot(s, h));
	if (u < 0.0 || u > 1.0)
		return false;
	q = glm::cross(s, edge1);
	v = f * glm::dot(ray.Get_direction(), q);
	if (v < 0.0 || u + v > 1.0)
		return false;
	// At this stage we can compute t to find out where the intersection point is on the line.

	float t = f * glm::dot(edge2, q);
	if (t > EPSILON) // ray intersection
	{
		res = t;
		return true;
	}
	else // This means that there is a line intersection but not a ray intersection.
    return false;
}

#ifdef ENABLE_BVH
void Mesh::BVH_Split() {

	if ( m_faces.size() == 1 ) {
			m_face = m_faces[0];
	} else {
			int axis = int( 3 * rand_float() );

			// sort by random direction
			// cout << "got " << m_faces.size() << " faces " << endl;
			// cout << "split by " << axis << ", ";

			std::sort( m_faces.begin(), m_faces.end(), [&](const Triangle t1, const Triangle t2) {
							double x1 = (*m_vertices)[t1.v1][axis];
							double x2 = (*m_vertices)[t2.v1][axis];
							return x1 < x2;
							});
			vector<Triangle> left_vec = vector<Triangle>( m_faces.begin(), m_faces.begin() + m_faces.size() / 2 );
			vector<Triangle> right_vec = vector<Triangle>( m_faces.begin() + m_faces.size() / 2, m_faces.end() );
			left = new Mesh( m_vertices, left_vec);
			right = new Mesh( m_vertices, right_vec);
	}
	m_faces.clear();
}
#endif


Mesh::Mesh( const std::string& fname )
{
	// cout << fname << " Mesh constructor" << endl;
#ifdef ENABLE_BVH
    left = NULL;
    right = NULL;
#endif
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	// vector<vec3> tmp_vertices;

	std::ifstream ifs( fname.c_str() );
	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			m_real_vertices.push_back( glm::vec3( vx, vy, vz ) );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}

	m_vertices = &m_real_vertices;
	// cout << "mesh constructor " << endl; 

	// find min_xyz and max_xyz, this is the root bounding box
	if (m_real_vertices.size() != 0) {
		min_xyz = m_real_vertices[0] - vec3(Epsilon);
		max_xyz = m_real_vertices[0] + vec3(Epsilon);

		for ( vec3& point : m_real_vertices ) {
			min_xyz = glm::vec3( glm::min(min_xyz.x, point.x - Epsilon),
			 glm::min(min_xyz.y, point.y - Epsilon),
			  glm::min(min_xyz.z, point.z - Epsilon) );
			max_xyz = glm::vec3( glm::max(max_xyz.x, point.x + Epsilon),
			 glm::max(max_xyz.y, point.y + Epsilon),
			  glm::max(max_xyz.z, point.z + Epsilon) );
        }
	}
	// cout << "min_xyz " << to_string(min_xyz) << endl;
	// cout << "max_xyz " << to_string(max_xyz) << endl;

	// implement BVH
#ifdef ENABLE_BVH
	// cout << "BVH\n";
	BVH_Split();
#endif
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*
  
  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	if( mesh.m_have_norm ) {
  	  out << " / " << glm::to_string( v.m_normal );
	}
	if( mesh.m_have_uv ) {
  	  out << " / " << glm::to_string( v.m_uv );
	}
  }

*/
  out << "}";
  return out;
}

bool Mesh::hit(Ray &ray, float t_min, float t_max, HitRecord &record) {

	// check if the ray hit the bounding box
	{
		vec3 center = (min_xyz + max_xyz) / 2;
		double m_radius = glm::distance(center, min_xyz);
		vec3 dis_oc = ray.Get_origin() - center;

		double A = dot(ray.Get_direction(), ray.Get_direction());
		double B = 2 * dot(ray.Get_direction(), dis_oc);
		double C = dot(dis_oc, dis_oc) - m_radius * m_radius;

		double roots[2];
		size_t n_roots = quadraticRoots(A, B, C, roots);

		// float t = 0;
		if (n_roots == 0) {
			// cout << "reject\n";
			return false;
		} 
		// else if (n_roots == 1) {
		// 	t = roots[0];
		// } else {
		// 	t = glm::min(roots[0], roots[1]);
		// }
	}

	// {
	// 	float tmin = 0;
	// 	float tmax = std::numeric_limits<float>::max();
		
	// 	for (int a = 0; a < 3; ++a) {
	// 		float invD = 1.0f / ray.Get_direction()[a];
	// 		float t0 = (min_xyz[a] - ray.Get_origin()[a]) * invD;
	// 		float t1 = (max_xyz[a] - ray.Get_origin()[a]) * invD;
	// 		if (invD < 0.0f) std::swap(t0, t1);
	// 		tmin = t0 > tmin ? t0 : tmin;
	// 		tmax = t1 < tmax ? t1 : tmax;
			
    //         // static int reject_count = 0;
	// 		if (tmin >= tmax) {
	// 			cout << "a is " << a << endl;
	// 			cout << "direction is " << ray.Get_direction()[a] << endl;
	// 			cout << "invD is " << invD << endl;
	// 			cout << "t0 is " << t0 << " t1 " << t1 << endl;
	// 			cout << "tmin is " << tmin << " tmax is " <<tmax <<endl;
    //             // if ( ++reject_count % 1000 == 0)
    //             //     cout << "rejected by bounding box test: " << ++reject_count << endl;
    //             return false;
    //         }
	// 	}
	// }

	// cout << "hit test\n";
	float cur_min_maxt = t_max;
	vec3 normal = vec3();
    bool hit = false;

#ifdef ENABLE_BVH
	if (left) { // nodes that have children
		HitRecord temp;

		if (left->hit(ray, t_min, cur_min_maxt, temp)) {
			hit = true;
			cur_min_maxt = temp.t;
			normal = temp.normal;
		}
		if (right->hit(ray, t_min, cur_min_maxt, temp)) {
			hit = true;
			cur_min_maxt = temp.t;
			normal = temp.normal;
		}
	}
	else { // leaf
		Triangle triangle = m_face;
		float tmp_t;
        if ( triangleIntersection(ray, (*m_vertices)[triangle.v1], (*m_vertices)[triangle.v2], (*m_vertices)[triangle.v3], tmp_t) ) {
			// cout << "hit\n";
            if ( tmp_t < cur_min_maxt && tmp_t > t_min ) {
                hit = true;
                cur_min_maxt = tmp_t;
                normal = cross((*m_vertices)[triangle.v1] - (*m_vertices)[triangle.v2], (*m_vertices)[triangle.v2] - (*m_vertices)[triangle.v3]);
            }
        }
	}
#else
	// cout << "mesh " << m_faces.size() << endl;
	for ( auto triangle : m_faces ) {
        float tmp_t;
		cout << "normal before hit\n";
        if ( triangleIntersection(ray, (*m_vertices)[triangle.v1], (*m_vertices)[triangle.v2], (*m_vertices)[triangle.v3], tmp_t) ) {
			cout << "hit\n";
            if ( tmp_t < cur_min_maxt && tmp_t > t_min ) {
                hit = true;
                cur_min_maxt = tmp_t;
                normal = cross((*m_vertices)[triangle.v1] - (*m_vertices)[triangle.v2], (*m_vertices)[triangle.v2] - (*m_vertices)[triangle.v3]);
            }
        }
    }
#endif

	if ( !hit ) return false;

	if ( dot( ray.Get_direction(), normal ) > 0 ) normal = -normal;

	record.t = cur_min_maxt;
	record.normal = normal;
	record.hit_point = ray.At_t(record.t);
	record.material = nullptr;

	// cout << t_min << " current t is " << record.t << endl;

	return hit;
}

Mesh::Mesh( std::vector<glm::vec3>* all_vertices, std::vector<Triangle> &faces) {
#ifdef ENABLE_BVH
    left = NULL;
    right = NULL;
#endif
	// static int level = 0;
	// level ++;
	// cout << "level is " << level << endl; 
	m_vertices = all_vertices;
	m_faces = faces;

	// find min_xyz and max_xyz
	if (faces.size() != 0) {
		min_xyz = (*m_vertices)[faces[0][0]] - glm::vec3(Epsilon, Epsilon, Epsilon);
		max_xyz = (*m_vertices)[faces[0][0]] + glm::vec3(Epsilon, Epsilon, Epsilon);
		for ( const Triangle &face : faces ) {
            for ( int i = 0; i < 3; ++i ) {
                vec3 point = (*m_vertices)[face[i]];
				min_xyz = glm::vec3( glm::min(min_xyz.x, point.x - Epsilon),
				 glm::min(min_xyz.y, point.y - Epsilon), glm::min(min_xyz.z, point.z - Epsilon) );
				max_xyz = glm::vec3( glm::max(max_xyz.x, point.x + Epsilon),
				 glm::max(max_xyz.y, point.y + Epsilon), glm::max(max_xyz.z, point.z + Epsilon) );
            }
        }
	}
	// cout << "min_xyz " << to_string(min_xyz) << endl;
	// cout << "max_xyz " << to_string(max_xyz) << endl;

#ifdef ENABLE_BVH
	// cout << "BVH\n";
	BVH_Split();
#endif
	//
	// m_faces.resize(faces.size());
	// for (size_t i = 0; i < faces.size(); i++) {
	// 	Triangle tri((size_t)faces[i].x, (size_t)faces[i].y, (size_t)faces[i].z);

	// 	m_faces[i] = tri;
	// }
}

Mesh::~Mesh() {
#ifdef ENABLE_BVH
    if (left) delete left;
    if (right) delete right;
#endif
}