// Winter 2019

#include "JointNode.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "cs488-framework/MathUtils.hpp"
#include <iostream>
using namespace std;
using namespace glm;
//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
	: SceneNode(name)
{
	m_nodeType = NodeType::JointNode;
	cur_angleX = 0.0f;
	cur_angleY = 0.0f;
}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {

}
//---------------------------------------------------------------------------------------
void JointNode::set_joint_x(double min, double init, double max) {
	m_joint_x.min = min;
	m_joint_x.init = init;
	m_joint_x.max = max;
	rotate('x', m_joint_x.init);
	// cout << "set_joint_x " << m_joint_x.init << endl;
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
	m_joint_y.min = min;
	m_joint_y.init = init;
	m_joint_y.max = max;

	// cout << "set_joint_y " << m_joint_y.init << endl;
	rotate('y', m_joint_y.init);
}

float clampf(float a, float min, float max) {
  if (a > max) return max;
  if (a < min) return min;
  return a;
}

//---------------------------------------------------------------------------------------
void JointNode::rotate(char axis, float angle) {
	if (angle == 0.0f) return ;
	vec3 rot_axis;
	switch (axis)
	{
		case 'x':
			/* code */
			rot_axis = vec3(1,0,0);
			// cout << "before clamp node is " << m_name << " angle is " << angle << endl;
			// cout << "before clamp cur X " <<  cur_angleX << endl;
			angle = clampf(angle, m_joint_x.min - cur_angleX, m_joint_x.max - cur_angleX);
			cur_angleX += angle;
			// cout << "around x, node is " << m_name << " angle is " << cur_angleX << endl;
			break;
		case 'y':
			rot_axis = vec3(0,1,0);
			angle = clampf(angle, m_joint_y.min - cur_angleY, m_joint_y.max - cur_angleY);
			cur_angleY += angle;
			// cout << "around y, node is " << m_name << " angle is " << cur_angleY << endl;
			break;
		default:
			break;
	}
	
	mat4 rot_matrix = glm::rotate(degreesToRadians(angle), rot_axis);
	trans = rot_matrix * trans;
}