// Winter 2019

#pragma once

#include "SceneNode.hpp"

class JointNode : public SceneNode {
public:
	JointNode(const std::string & name);
	virtual ~JointNode();

	void set_joint_x(double min, double init, double max);
	void set_joint_y(double min, double init, double max);

	struct JointRange {
		double min, init, max;
	};

	void rotate(char axis, float angle) override;

	float cur_angleX;
	float cur_angleY;

	JointRange m_joint_x, m_joint_y;
};