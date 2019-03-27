// Winter 2019

#include "PhongMaterial.hpp"

PhongMaterial::PhongMaterial(
	const glm::vec3& kd, const glm::vec3& ks, double shininess )
	: m_kd(kd)
	, m_ks(ks)
	, m_shininess(shininess)
{
	m_reflectiveness = 0.3f;
	m_refractiveness = 0.7f;
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