// Winter 2019

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"
#include "JointNode.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <set>
#include <stack>
#include <unordered_map>
#include <vector>

struct LightSource {
	glm::vec3 position;
	glm::vec3 rgbIntensity;
};

// data structure to implement undo and redo
struct JointsInfo {
	JointNode* jointNode;
	float cur_angleX;
	float cur_angleY;
	bool isSelected;
    glm::mat4 trans;

	JointsInfo(JointNode* node) {
		jointNode = node;
		cur_angleX = node->cur_angleX;
		cur_angleY = node->cur_angleY;
		isSelected = node->isSelected;
		trans = node->trans;
	}

	void apply() {
		jointNode->cur_angleX = cur_angleX;
		jointNode->cur_angleY = cur_angleY;
		jointNode->isSelected = isSelected;
		jointNode->trans = trans;
	}
};



class A3 : public CS488Window {
public:
	A3(const std::string & luaSceneFile);
	virtual ~A3();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	//-- Virtual callback methods
	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	//-- One time initialization methods:
	void processLuaSceneFile(const std::string & filename);
	void createShaderProgram();
	void enableVertexShaderInputSlots();
	void uploadVertexDataToVbos(const MeshConsolidator & meshConsolidator);
	void mapVboDataToVertexShaderInputLocations();
	void initViewMatrix();
	void initLightSources();

	void initPerspectiveMatrix();
	void uploadCommonSceneUniforms();
	void renderSceneGraph(const SceneNode &node);
	void renderArcCircle();
	void renderNode(const SceneNode * root, glm::mat4 view, glm::mat4 model);
	void applyPositionChange(double xPos, double yPos);
	void applyJointsChange(double xPos, double yPos);
	void pickingSetup();
	JointNode * findJoint(unsigned int objID, SceneNode * root);
	void resetPosition();
	void resetRotation();
	void resetJoints(SceneNode * node);
	void resetAll();
	void undoOperation();
	void redoOperation();
	void saveState();

	glm::mat4 m_perpsective;
	glm::mat4 m_view;

	glm::mat4 translateTrans;
  	glm::mat4 rotateTrans;

	LightSource m_light;

	//-- GL resources for mesh geometry data:
	GLuint m_vao_meshData;
	GLuint m_vbo_vertexPositions;
	GLuint m_vbo_vertexNormals;
	GLint m_positionAttribLocation;
	GLint m_normalAttribLocation;
	ShaderProgram m_shader;

	//-- GL resources for trackball circle geometry:
	GLuint m_vbo_arcCircle;
	GLuint m_vao_arcCircle;
	GLint m_arc_positionAttribLocation;
	ShaderProgram m_shader_arcCircle;

	// BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
	// object. Each BatchInfo object contains an index offset and the number of indices
	// required to render the mesh with identifier MeshId.
	BatchInfoMap m_batchInfoMap;

	std::string m_luaSceneFile;

	std::shared_ptr<SceneNode> m_rootNode;

	// Options 
	bool trackBall;
	bool zBuffer;
	bool backCulling;
	bool frontCulling;

	// Mode
	enum Mode {Position, Joints};
	int mode;

	// Mouse properties
	bool mouse_leftdown;
	bool mouse_middledown;
	bool mouse_rightdown;

	double mouseLastX;
	double mouseLastY;

	// trackballinformation
	glm::vec3 lastTrackball;

	bool picking;

	std::set<JointNode*> selectedJoints;

	// undo redo
	std::vector<JointsInfo> oldInfo;
	std::stack<std::vector<JointsInfo>> undo;
	std::stack<std::vector<JointsInfo>> redo;
};
