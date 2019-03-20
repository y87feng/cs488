// Winter 2019

#include "PhongMaterial.hpp"

PhongMaterial::PhongMaterial(
	const glm::vec3& kd, const glm::vec3& ks, double shininess )
	: m_kd(kd)
	, m_ks(ks)
	, m_shininess(shininess)
{}

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