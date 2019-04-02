// Winter 2019
#include <iostream>
#include "PhongMaterial.hpp"
#include "random.hpp"

using namespace std;

PhongMaterial::PhongMaterial(
	const glm::vec3& kd, const glm::vec3& ks, double shininess)
	: m_kd(kd)
	, m_ks(ks)
	, m_shininess(shininess)
	, m_reflectiveness(0.0f)
	, m_refractiveness(0.0f)
	, m_refraction_ratio(1.0f)
{
	// float rd_num = rand_float();

	// cout << "reflective is " << m_reflectiveness << " refractive is" << m_refractiveness << endl;
	// cout << "refraction ratio is " << m_refraction_ratio << endl;
}

PhongMaterial::PhongMaterial(
	const glm::vec3& kd, const glm::vec3& ks, double shininess,
	 double reflect, double refract , double refract_ratio)
	: m_kd(kd)
	, m_ks(ks)
	, m_shininess(shininess)
	, m_reflectiveness(reflect)
	, m_refractiveness(refract)
	, m_refraction_ratio(refract_ratio)
{
	// float rd_num = rand_float();

	// cout << "reflective is " << m_reflectiveness << " refractive is" << m_refractiveness << endl;
	// cout << "refraction ratio is " << m_refraction_ratio << endl;
}

PhongMaterial::~PhongMaterial()
{}

glm::vec3 PhongMaterial::diffuse() {
	return m_kd;
}

glm::vec3 PhongMaterial::specular() {
	return m_ks;
}

double PhongMaterial::shininess() {
	return m_shininess;
}

double PhongMaterial::reflectiveness() {
	return m_reflectiveness;
}

double PhongMaterial::refractiveness() {
	return m_refractiveness;
}

double PhongMaterial::refraction_ratio() {
	return m_refraction_ratio;
}