// Winter 2019

#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;

float static const ROTATE_FACTOR = 0.01f;
float static const TRANS_FACTOR = 0.01f;
float static const SCALE_FACTOR = 0.01f;
float static const FOV_LOW = 5.0f;
float static const FOV_HIGH = 160.0f;
float static const FOV_FACTOR = 0.5f;

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f)), left_down(false), middle_down(false), right_down(false)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();

	initCube();
	initCoordinatesFrame();
	reset();
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//---------------------------------------------------------------------------------------- Winter 2019
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & V0,   // Line Start (NDC coordinate)
		const glm::vec2 & V1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = V0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = V1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

//----------------------------------------------------------------------------------------
glm::mat4 A2::createView() {
	glm::vec3 lookat(0.0f,0.0f,-1.0f);
	glm::vec3 lookfrom(0.0f, 0.0f, 15.0f);
	glm::vec3 up(lookfrom.x, lookfrom.y + 1.0f, lookfrom.z);

	glm::vec3 z((lookat-lookfrom)/glm::distance(lookat,lookfrom));
	glm::vec3 x(glm::cross(z,up) / glm::distance(glm::cross(z,up), glm::vec3(0.0f)));
	glm::vec3 y(glm::cross(x,z));

	glm::mat4 R {
		vec4(x,0),
		vec4(y,0),
		vec4(z,0),
		vec4(0.0f, 0.0f, 0.0f, 1.0f),
	};
	R = glm::transpose(R);
	glm::mat4 T (1.0f);
	T[3].z = -lookfrom.z;
	return R * T;
}

//----------------------------------------------------------------------------------------
glm::mat4 A2::createProj() {
	float aspect = (float)(std::abs(viewportX1-viewportX2)) / (float)(std::abs(viewportY1 - viewportY2)); // viewport aspect
	float theta = glm::radians(fov);
	float cot = std::cos(theta/2) / std::sin(theta/2);
	glm::mat4 P(
		cot/aspect, 0, 0 ,0,
		0, cot, 0, 0,
		0, 0, (far+near)/(far-near), 1,
		0, 0, -2 * far * near / (far - near), 0
	);

	return P;
}

//----------------------------------------------------------------------------------------
void A2::reset() {
	mode = RotateModel;
	model = mat4(1.0f);
	mTransformation = mat4(1.0f);
	mScale = mat4(1.0f);
	viewportX1 = 0.05 * m_framebufferWidth;
	viewportY1 = 0.05 * m_framebufferHeight;
	viewportX2 = 0.95 * m_framebufferWidth;
	viewportY2 = 0.95 * m_framebufferHeight;
	
	view = createView();
	vTransformation = mat4(1.0f);
	near = 5.0f;
	far = 20.f;
	fov = 30.0f;
	proj = createProj();
}

//----------------------------------------------------------------------------------------
glm::vec4 A2::homogenize(glm::vec4& v) {
	return glm::vec4{
		v.x / v.w,
		v.y / v.w,
		v.z / v.w,
		1.0f
	};
}

//----------------------------------------------------------------------------------------
void A2::initCube() {

	// back bottom
	cube_vecs.push_back(vec4(-1.0f,-1.0f,-1.0f, 1.0f));
	cube_vecs.push_back(vec4(1.0f,-1.0f,-1.0f, 1.0f));

	// back left
	cube_vecs.push_back(vec4(-1.0f,-1.0f,-1.0f, 1.0f));
	cube_vecs.push_back(vec4(-1.0f,1.0f,-1.0f, 1.0f));

	// back top
	cube_vecs.push_back(vec4(-1.0f,1.0f,-1.0f, 1.0f));
	cube_vecs.push_back(vec4(1.0f,1.0f,-1.0f, 1.0f));

	// back right
	cube_vecs.push_back(vec4(1.0f,1.0f,-1.0f, 1.0f));
	cube_vecs.push_back(vec4(1.0f,-1.0f,-1.0f, 1.0f));

	// left bottom
	cube_vecs.push_back(vec4(-1.0f,-1.0f,-1.0f, 1.0f));
	cube_vecs.push_back(vec4(-1.0f,-1.0f,1.0f, 1.0f));
	
	// left top
	cube_vecs.push_back(vec4(-1.0f,1.0f,-1.0f, 1.0f));
	cube_vecs.push_back(vec4(-1.0f,1.0f,1.0f, 1.0f));

	// right top
	cube_vecs.push_back(vec4(1.0f,1.0f,-1.0f, 1.0f));
	cube_vecs.push_back(vec4(1.0f,1.0f,1.0f, 1.0f));

	// right bottom
	cube_vecs.push_back(vec4(1.0f,-1.0f,1.0f, 1.0f));
	cube_vecs.push_back(vec4(1.0f,-1.0f,-1.0f, 1.0f));

	// front right
	cube_vecs.push_back(vec4(1.0f,-1.0f,1.0f, 1.0f));
	cube_vecs.push_back(vec4(1.0f,1.0f,1.0f, 1.0f));

	// front bottom
	cube_vecs.push_back(vec4(1.0f,-1.0f,1.0f, 1.0f));
	cube_vecs.push_back(vec4(-1.0f,-1.0f,1.0f, 1.0f));

	// front top
	cube_vecs.push_back(vec4(-1.0f,1.0f,1.0f, 1.0f));
	cube_vecs.push_back(vec4(1.0f,1.0f,1.0f, 1.0f));

	// front left
	cube_vecs.push_back(vec4(-1.0f,1.0f,1.0f, 1.0f));
	cube_vecs.push_back(vec4(-1.0f,-1.0f,1.0f, 1.0f));
}

//----------------------------------------------------------------------------------------
void A2::initCoordinatesFrame() {

	// x axis
	c_frame.push_back(vec4(0.0f,0.0f,0.0f, 1.0f));
	c_frame.push_back(vec4(1.0f,0.0f,0.0f, 1.0f));

	// y axis
	c_frame.push_back(vec4(0.0f,0.0f,0.0f, 1.0f));
	c_frame.push_back(vec4(0.0f,1.0f,0.0f, 1.0f));

	// z axis
	c_frame.push_back(vec4(0.0f,0.0f,0.0f, 1.0f));
	c_frame.push_back(vec4(0.0f,0.0f,1.0f, 1.0f));
}

//----------------------------------------------------------------------------------------

void A2::rotateModel(double xPos, double yPos) {
	float xOffset = xPos - prev_mouse_x;
	
	if (left_down) {
		// rotate about x
		mat4 R(1.0f);
		R[1][1] = cos(xOffset * ROTATE_FACTOR);
		R[1][2] = sin(xOffset * ROTATE_FACTOR);
		R[2][1] = -sin(xOffset * ROTATE_FACTOR);
		R[2][2] = cos(xOffset * ROTATE_FACTOR);
		mTransformation = mTransformation * R;
	} 
	if (middle_down) {
		// rotate about y
		mat4 R(1.0f);
		R[0][0] = cos(xOffset * ROTATE_FACTOR);
		R[2][0] = sin(xOffset * ROTATE_FACTOR);
		R[0][2] = -sin(xOffset * ROTATE_FACTOR);
		R[2][2] = cos(xOffset * ROTATE_FACTOR);
		mTransformation = mTransformation * R;
	}
	if (right_down) {
		// rotate about z
		mat4 R(1.0f);
		R[0][0] = cos(xOffset * ROTATE_FACTOR);
		R[0][1] = sin(xOffset * ROTATE_FACTOR);
		R[1][0] = -sin(xOffset * ROTATE_FACTOR);
		R[1][1] = cos(xOffset * ROTATE_FACTOR);
		mTransformation = mTransformation * R;
	}
	// cout << xOffset * ROTATE_FACTOR << endl;
}

//----------------------------------------------------------------------------------------
void A2::translateModel(double xPos, double yPos) {
	float xOffset = xPos - prev_mouse_x;
	if (left_down) {
		// translate on x
		mat4 T(1.0f);
		T[3].x = xOffset * TRANS_FACTOR;
		mTransformation = mTransformation * T;
	} 
	if (middle_down) {
		// translate on y
		mat4 T(1.0f);
		T[3].y = xOffset * TRANS_FACTOR;
		mTransformation = mTransformation * T;
	}
	if (right_down) {
		// translate on z
		mat4 T(1.0f);
		T[3].z = xOffset * TRANS_FACTOR;
		mTransformation = mTransformation * T;
	}
}

//----------------------------------------------------------------------------------------
void A2::scaleModel(double xPos, double yPos) {
	float xOffset = xPos - prev_mouse_x;
	
	if (left_down) {
		// scale on x
		mat4 S(1.0f);
		if (S[0].x + xOffset * SCALE_FACTOR > 0) S[0].x += xOffset * SCALE_FACTOR;
		mScale = mScale * S;
	} 
	if (middle_down) {
		// scale on y
		mat4 S(1.0f);
		if (S[1].y + xOffset * SCALE_FACTOR > 0) S[1].y += xOffset * SCALE_FACTOR;
		mScale = mScale * S;
	}
	if (right_down) {
		// scale on z
		mat4 S(1.0f);
		if (S[2].z + xOffset * SCALE_FACTOR > 0) S[2].z += xOffset * SCALE_FACTOR;
		mScale = mScale * S;
	}
}

//----------------------------------------------------------------------------------------
void A2::rotateView(double xPos, double yPos) {
	float xOffset = xPos - prev_mouse_x;
	
	if (left_down) {
		// rotate about x
		mat4 R(1.0f);
		R[1][1] = cos(xOffset * ROTATE_FACTOR);
		R[1][2] = sin(xOffset * ROTATE_FACTOR);
		R[2][1] = -sin(xOffset * ROTATE_FACTOR);
		R[2][2] = cos(xOffset * ROTATE_FACTOR);
		glm::mat4 inverse_R = glm::mat4(1.0f) / R;
		vTransformation = inverse_R * vTransformation;
	} 
	if (middle_down) {
		// rotate about y
		mat4 R(1.0f);
		R[0][0] = cos(xOffset * ROTATE_FACTOR);
		R[2][0] = sin(xOffset * ROTATE_FACTOR);
		R[0][2] = -sin(xOffset * ROTATE_FACTOR);
		R[2][2] = cos(xOffset * ROTATE_FACTOR);
		glm::mat4 inverse_R = glm::mat4(1.0f) / R;
		vTransformation = inverse_R * vTransformation;
	}
	if (right_down) {
		// rotate about z
		mat4 R(1.0f);
		R[0][0] = cos(xOffset * ROTATE_FACTOR);
		R[0][1] = sin(xOffset * ROTATE_FACTOR);
		R[1][0] = -sin(xOffset * ROTATE_FACTOR);
		R[1][1] = cos(xOffset * ROTATE_FACTOR);
		glm::mat4 inverse_R = glm::mat4(1.0f) / R;
		vTransformation = inverse_R * vTransformation;
	}
}

//----------------------------------------------------------------------------------------
void A2::translateView(double xPos, double yPos) {
	float xOffset = xPos - prev_mouse_x;
	
	if (left_down) {
		// translate on x
		mat4 T(1.0f);
		T[3].x = xOffset * TRANS_FACTOR;
		glm::mat4 inverse_T = glm::mat4(1.0f) / T;
		vTransformation = inverse_T * vTransformation;
	} 
	if (middle_down) {
		// translate on y
		mat4 T(1.0f);
		T[3].y = xOffset * TRANS_FACTOR;
		glm::mat4 inverse_T = glm::mat4(1.0f) / T;
		vTransformation = inverse_T * vTransformation;
	}
	if (right_down) {
		// translate on z
		mat4 T(1.0f);
		T[3].z = xOffset * TRANS_FACTOR;
		glm::mat4 inverse_T = glm::mat4(1.0f) / T;
		vTransformation = inverse_T * vTransformation;
	}
}

//----------------------------------------------------------------------------------------
void A2::perspective(double xPos, double yPos) {
	float xOffset = xPos - prev_mouse_x;
	if (left_down) {
		// adjust FOV
		fov += xOffset * FOV_FACTOR;
		fov = std::max(fov, FOV_LOW);
		fov = std::min(fov, FOV_HIGH);
		proj = createProj();
	} 
	if (middle_down) {
		// adjust near
		near = std::min(far, near + xOffset * TRANS_FACTOR);
		proj = createProj();
	}
	if (right_down) {
		// adjust far
		far = std::max(near, far + xOffset * TRANS_FACTOR);
		proj = createProj();
	}
}

//----------------------------------------------------------------------------------------
void A2::viewport(double xPos,double yPos) {
	if (left_down) {
		viewportX2 = glm::clamp((float)xPos, 0.0f, (float)m_framebufferWidth);
		viewportY2 = glm::clamp((float)yPos, 0.0f, (float)m_framebufferHeight);
	}
}

//----------------------------------------------------------------------------------------
// clipping after projection and before normalization
bool A2::clipping(glm::vec4& A, glm::vec4 & B) {
	std::function<float (glm::vec4&)> BL = [](glm::vec4& p) { return p.x + p.w; };
	std::function<float (glm::vec4&)> BR = [](glm::vec4& p) { return p.w - p.x; };
	std::function<float (glm::vec4&)> BB = [](glm::vec4& p) { return p.w + p.y; };
	std::function<float (glm::vec4&)> BT = [](glm::vec4& p) { return p.w - p.y; };
	std::function<float (glm::vec4&)> BN = [](glm::vec4& p) { return p.z + p.w; };
	std::function<float (glm::vec4&)> BF = [](glm::vec4& p) { return p.w - p.z; };

	std::vector<std::function<float (glm::vec4&)>> clippingBoundries {
		BL, BR, BB, BT, BN, BF
	};

	for (auto clippingBoundary : clippingBoundries) {
		
		float boundary1 = clippingBoundary(A);
		float boundary2 = clippingBoundary(B);

		
		if (boundary1 < 0 && boundary2 < 0) {
			return false;
		}
		if (boundary1 >= 0 && boundary2 >= 0) {
			continue;
		}

		float a = boundary1 / (boundary1 - boundary2);

		if (boundary1 < 0) { // A is outside
			A = (1-a)*A + a * B;
		} else { // B is outside
			B = (1-a)*A + a * B;
		}

	}

	return true;
}


//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here ...

	// Call at the beginning of frame, before drawing lines:
	initLineData();
	
	// viewports
	float viewportLeft = viewportX1 < viewportX2 ? viewportX1 : viewportX2;
	float viewportRight = viewportX1 < viewportX2 ? viewportX2 : viewportX1;
	float viewportTop = viewportY1 < viewportY2 ? viewportY1 : viewportY2;
	float viewportBottom = viewportY1 < viewportY2 ? viewportY2 : viewportY1;

	float xscale = (viewportRight - viewportLeft) / m_framebufferWidth;
	float yscale = (viewportBottom - viewportTop) / m_framebufferHeight;
	float left_in_norm = (viewportLeft - m_framebufferWidth / 2) / (m_framebufferWidth / 2);
	float right_in_norm = (viewportRight - m_framebufferWidth / 2) / (m_framebufferWidth / 2);
	float top_in_norm = (m_framebufferHeight / 2 - viewportTop) / (m_framebufferHeight / 2);
	float bottom_in_norm = (m_framebufferHeight / 2 - viewportBottom) / (m_framebufferHeight / 2);
	float middle_x = (left_in_norm + right_in_norm) / 2;
	float middle_y = (top_in_norm + bottom_in_norm) / 2;

	// draw cube
	for (size_t i = 0; i < cube_vecs.size(); i += 2) {
		if ( i < 2 * 4) setLineColour(vec3(1.0f, 0.7f, 0.8f));
		else if (i < 2 * 8) setLineColour(vec3(1.0f, 0.0f, 1.0f));
		else setLineColour(vec3(0.2f, 1.0f, 1.0f));
		glm::vec4 A = proj * vTransformation * view * model * mTransformation * mScale * cube_vecs[i];
		glm::vec4 B = proj * vTransformation * view * model * mTransformation * mScale * cube_vecs[i+1];

		// clip here
		if (clipping(A, B)) {
			A = homogenize(A);
			B = homogenize(B);

			drawLine(glm::vec2(A.x * xscale + middle_x, A.y * yscale + middle_y),
			 glm::vec2(B.x * xscale + middle_x, B.y * yscale + middle_y));
		}
	}

	// draw modelling frame
	for (size_t i = 0; i < c_frame.size(); i += 2) {
		if ( i < 2 * 1) setLineColour(vec3(1.0f, 0.0f, 0.0f));
		else if (i < 2 * 2) setLineColour(vec3(0.0f, 1.0f, 0.0f));
		else setLineColour(vec3(0.0f, 0.0f, 1.0f));
		glm::vec4 A = proj * vTransformation * view * model * mTransformation * c_frame[i];
		glm::vec4 B = proj * vTransformation * view * model * mTransformation * c_frame[i+1];

		// clip here
		if (clipping(A, B)) {
			A = homogenize(A);
			B = homogenize(B);

			drawLine(glm::vec2(A.x * xscale + middle_x, A.y * yscale + middle_y),
			 glm::vec2(B.x * xscale + middle_x, B.y * yscale + middle_y));
		}
	}

	// draw world frame
	for (size_t i = 0; i < c_frame.size(); i += 2) {
		if ( i < 2 * 1) setLineColour(vec3(1.0f, 0.0f, 0.0f));
		else if (i < 2 * 2) setLineColour(vec3(0.0f, 1.0f, 0.0f));
		else setLineColour(vec3(0.0f, 0.0f, 1.0f));
		glm::vec4 A = proj * vTransformation * view * model * c_frame[i];
		glm::vec4 B = proj * vTransformation * view * model * c_frame[i+1];

		// clip here
		if (clipping(A, B)) {
			A = homogenize(A);
			B = homogenize(B);

			drawLine(glm::vec2(A.x * xscale + middle_x, A.y * yscale + middle_y),
			 glm::vec2(B.x * xscale + middle_x, B.y * yscale + middle_y));
		}
	}

	// draw viewport boundaries
	setLineColour(vec3(0.0f, 0.0f, 0.0f));
	drawLine(glm::vec2(left_in_norm, top_in_norm), glm::vec2(left_in_norm, bottom_in_norm));
	drawLine(glm::vec2(left_in_norm, top_in_norm), glm::vec2(right_in_norm, top_in_norm));
	drawLine(glm::vec2(right_in_norm, bottom_in_norm), glm::vec2(left_in_norm, bottom_in_norm));
	drawLine(glm::vec2(right_in_norm, bottom_in_norm), glm::vec2(right_in_norm, top_in_norm));
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...
		ImGui::RadioButton("Rotate View     (o)", (int*)&mode, RotateView);
        ImGui::RadioButton("Translate View  (e)", (int*)&mode, TranslateView);
        ImGui::RadioButton("Perspective     (p)", (int*)&mode, Perspective);
		ImGui::RadioButton("Rotate Model    (r)", (int*)&mode, RotateModel);
        ImGui::RadioButton("Translate Model (t)", (int*)&mode, TranslateModel);
        ImGui::RadioButton("Scale Model     (s)", (int*)&mode, ScaleModel);
		ImGui::RadioButton("Viewport        (v)", (int*)&mode, Viewport);
 
		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Reset             (a)" ) ) {
			reset();
		}
		if( ImGui::Button( "Quit Application  (q)" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );
		ImGui::Text( "Near: %.1f, Far: %.1f", near, far);

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	
	if (!ImGui::IsMouseHoveringAnyWindow()) {
		switch(mode) {
			case RotateModel:
				rotateModel(xPos, yPos);
				break;
			case ScaleModel:
				scaleModel(xPos, yPos);
				break;
			case TranslateModel:
				translateModel(xPos, yPos);
				break;
			case RotateView:
				rotateView(xPos, yPos);
				break;
			case TranslateView:
				translateView(xPos, yPos);
				break;
			case Perspective:
				perspective(xPos, yPos);
				break;
			case Viewport:
				viewport(xPos, yPos);
				break;
			default:
				break;
		}
	}

	prev_mouse_x = xPos;

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (actions == GLFW_PRESS) {
				left_down = true;

				// viewport case
				if (mode == Viewport) {
					viewportX1 = glm::clamp(ImGui::GetMousePos().x, 0.0f, (float)m_framebufferWidth);
					viewportY1 = glm::clamp(ImGui::GetMousePos().y, 0.0f, (float)m_framebufferHeight);
					viewportX2 = viewportX1;
					viewportY2 = viewportY1;
				}
			} else {
				left_down = false;
			}
		} 

		if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			if (actions == GLFW_PRESS) {
				middle_down = true;
			} else {
				middle_down = false;
			}
		}

		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (actions == GLFW_PRESS) {
				right_down = true;
			} else {
				right_down = false;
			}
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_A) {
			reset();
			eventHandled = true;
		}

		if (key == GLFW_KEY_E) {
			mode = TranslateView;
			eventHandled = true;
		}

		if (key == GLFW_KEY_O) {
			mode = RotateView;
			eventHandled = true;
		}

		if (key == GLFW_KEY_P) {
			mode = Perspective;
			eventHandled = true;
		}

		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		}

		if (key == GLFW_KEY_R) {
			mode = RotateModel;
			eventHandled = true;
		}

		if (key == GLFW_KEY_S) {
			mode = ScaleModel;
			eventHandled = true;
		}

		if (key == GLFW_KEY_T) {
			mode = TranslateModel;
			eventHandled = true;
		}

		if (key == GLFW_KEY_V) {
			mode = Viewport;
			eventHandled = true;
		}
	}

	return eventHandled;
}
