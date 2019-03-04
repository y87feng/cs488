// Winter 2019

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;


// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
	VertexData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;
	GLuint index;
	GLsizei numVertices;
};


class A2 : public CS488Window {
public:
	A2();
	virtual ~A2();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	void createShaderProgram();
	void enableVertexAttribIndices();
	void generateVertexBuffers();
	void mapVboDataToVertexAttributeLocation();
	void uploadVertexDataToVbos();
	void initCube();
	glm::mat4 createView();
	glm::mat4 createProj();
	glm::vec4 homogenize(glm::vec4& v);
	void reset();
	void rotateModel(double xPos, double yPos);
	void translateModel(double xPos, double yPos);
	void scaleModel(double xPos, double yPos);
	void rotateView(double xPos, double yPos);
	void translateView(double xPos, double yPos);
	bool clipping(glm::vec4& A, glm::vec4 & B);
	void perspective(double xPos, double yPos);
	void viewport(double xPos,double yPos);

	void initLineData();
	void initCoordinatesFrame();

	void setLineColour(const glm::vec3 & colour);

	void drawLine (
			const glm::vec2 & v0,
			const glm::vec2 & v1
	);

	enum Mode {
		RotateView,
		TranslateView,
		Perspective,
		RotateModel,
		TranslateModel,
		ScaleModel,
		Viewport
	};

	ShaderProgram m_shader;

	GLuint m_vao;            // Vertex Array Object
	GLuint m_vbo_positions;  // Vertex Buffer Object
	GLuint m_vbo_colours;    // Vertex Buffer Object

	VertexData m_vertexData;

	glm::vec3 m_currentLineColour;

	std::vector<glm::vec4> cube_vecs;
	std::vector<glm::vec4> c_frame;

	glm::mat4 model;
	glm::mat4 mTransformation;
	glm::mat4 mScale;
	glm::mat4 view;
	glm::mat4 vTransformation;
	glm::mat4 proj;
	Mode mode;

	bool left_down;
	bool middle_down;
	bool right_down;

	double prev_mouse_x;

	float near;
	float far;
	float fov;

	float viewportX1;
	float viewportY1;
	float viewportX2;
	float viewportY2;
};
