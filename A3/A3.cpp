// Winter 2019

#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <queue>

using namespace glm;

static bool show_gui = true;
static float TRANSLATE_FACTOR = 0.01f;

const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0),
	  mode(Position),
	  trackBall(false),
	  zBuffer(true),
	  backCulling(false),
	  frontCulling(false),
	  mouse_leftdown(false),
	  mouse_middledown(false),
	  mouse_rightdown(false),
	  translateTrans(1.0f),
	  rotateTrans(1.0f),
	  picking(false)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	glClearColor(0.85, 0.85, 0.85, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();


	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could Not Open " << filename << std::endl;
	}
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(-0.5f, 0.0f, -1.0f);
	m_light.rgbIntensity = vec3(0.5f); // light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("picking");
		glUniform1i( location, picking ? 1 : 0 );

		if (!picking) {
			//-- Set LightSource uniform for the scene:
			{
				location = m_shader.getUniformLocation("light.position");
				glUniform3fv(location, 1, value_ptr(m_light.position));
				location = m_shader.getUniformLocation("light.rgbIntensity");
				glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
				CHECK_GL_ERRORS;
			}

			//-- Set background light ambient intensity
			{
				location = m_shader.getUniformLocation("ambientIntensity");
				vec3 ambientIntensity(0.05f);
				glUniform3fv(location, 1, value_ptr(ambientIntensity));
				CHECK_GL_ERRORS;
			}
		}
	}
	m_shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

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
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Application")) {
				if (ImGui::MenuItem("Reset Position (I)")) {
					// reset Position
					resetPosition();
				}
				if (ImGui::MenuItem("Reset Orientation (O)")) {
					// reset Orientation
					resetRotation();
				}
				if (ImGui::MenuItem("Reset Joint (S)")) {
					// reset Joints
					resetJoints(m_rootNode.get());
				}
				if (ImGui::MenuItem("Reset All (A)")) {
					// reset all
					resetAll();
				}
				if( ImGui::MenuItem( "Quit (Q)" ) ) {
					glfwSetWindowShouldClose(m_window, GL_TRUE);
				}
				 ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit")) {
				if (ImGui::MenuItem("Undo (U)")) {
					// undo
					undoOperation();
				}

				if (ImGui::MenuItem("Redo (R)")) {
					// redo
					redoOperation();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Options")) {
				ImGui::Checkbox("Circle (C)", &trackBall);
				ImGui::Checkbox("Z-buffer (Z)", &zBuffer);
				ImGui::Checkbox("Backface culling (B)", &backCulling);
				ImGui::Checkbox("Frontface culling (F)", &frontCulling);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		// Create Button, and check if it was clicked:
		ImGui::RadioButton( "Position/Orientation (P)", (int*)&mode, Position);
		ImGui::RadioButton( "Joints (J)", (int*)&mode, Joints);

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix,
		const glm::mat4 & modelMatrix,
		bool picking
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * modelMatrix;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		if (picking) {
			unsigned int idx = node.m_nodeId;
			float r = float(idx&0xff) / 255.0f;
			float g = float((idx>>8)&0xff) / 255.0f;
			float b = float((idx>>16)&0xff) / 255.0f;

			location = shader.getUniformLocation("material.kd");
			glUniform3f( location, r, g, b );
			CHECK_GL_ERRORS;
		}
		else {
			//-- Set NormMatrix:
			location = shader.getUniformLocation("NormalMatrix");
			mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
			glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
			CHECK_GL_ERRORS;


			//-- Set Material values:
			location = shader.getUniformLocation("material.kd");
			vec3 kd = node.material.kd;
			if (node.isSelected) {
				kd = {0, 0.5, 0.5};
			}
			glUniform3fv(location, 1, value_ptr(kd));
			CHECK_GL_ERRORS;
			location = shader.getUniformLocation("material.ks");
			vec3 ks = node.material.ks;
			glUniform3fv(location, 1, value_ptr(ks));
			CHECK_GL_ERRORS;
			location = shader.getUniformLocation("material.shininess");
			glUniform1f(location, node.material.shininess);
			CHECK_GL_ERRORS;
		}

	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {
	if (zBuffer) {
		glEnable( GL_DEPTH_TEST );
	}
	if (frontCulling && backCulling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT_AND_BACK);
	} else if (frontCulling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	} else if (backCulling) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	renderSceneGraph(*m_rootNode);

	if (zBuffer) {
		glDisable( GL_DEPTH_TEST );
	}

	if (frontCulling || backCulling) {
		glDisable( GL_CULL_FACE );
	}
	if (trackBall) {
		renderArcCircle();
	}
}

//----------------------------------------------------------------------------------------
void A3::renderNode(const SceneNode * root, mat4 view, mat4 model) {
	if (root == nullptr) return;
	
	mat4 tmp_model = model * root->get_transform();
	if (root->m_nodeType == NodeType::GeometryNode) {
		// if (root->m_name == "leftThigh") {
		// 	cout << "leftThigh " << root->trans << endl;
		// 	// cout << "old model " << model << endl;
		// 	cout << "new model " << tmp_model << endl;
		// }
		// cout << root->m_name << " model is " << trans << endl;

		const GeometryNode * geometryNode = static_cast<const GeometryNode *>(root);

		updateShaderUniforms(m_shader, *geometryNode, view, tmp_model , picking);

		// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
		BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

		//-- Now render the mesh:
		m_shader.enable();
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
		m_shader.disable();
	} 
	// else if (root->m_nodeType == NodeType::JointNode) {
	// 	if (root->m_name == "leftThighJoint") cout << "leftThighJoint " << root->trans << endl;
	// }

	
	for (const SceneNode * node : root->children) {
		renderNode(node, view, tmp_model); // recursion
	}
}
//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	// This is emphatically *not* how you should be drawing the scene graph in
	// your final implementation.  This is a non-hierarchical demonstration
	// in which we assume that there is a list of GeometryNodes living directly
	// underneath the root node, and that we can draw them in a loop.  It's
	// just enough to demonstrate how to get geometry and materials out of
	// a GeometryNode and onto the screen.

	// You'll want to turn this into recursive code that walks over the tree.
	// You can do that by putting a method in SceneNode, overridden in its
	// subclasses, that renders the subtree rooted at every node.  Or you
	// could put a set of mutually recursive functions in this class, which
	// walk down the tree from nodes of different types.
	mat4 torso_pos = root.get_transform();
	if (root.children.size() != 0) {
		torso_pos = root.children.front()->get_transform() * torso_pos;
		// cout << torso_pos << endl;
	}
	renderNode(&root, m_view, translateTrans * torso_pos * rotateTrans * inverse(torso_pos));


	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::resetPosition() {
	translateTrans = mat4(1.0f);
}

void A3::resetRotation() {
	rotateTrans = mat4(1.0f);
}

void A3::resetJoints(SceneNode * node) {
	undo = stack<std::vector<JointsInfo>>();
	redo = stack<std::vector<JointsInfo>>();
	if (node->m_nodeType == NodeType::JointNode) {
		JointNode* joint = (JointNode*)node;
		joint->cur_angleX = 0;
		joint->cur_angleY = 0;
		joint->set_transform(mat4(1.0f));
		joint->rotate('x', joint->m_joint_x.init);
		joint->rotate('y', joint->m_joint_y.init);
	}
	for (SceneNode* child : node->children) {
		resetJoints(child);
	}
}

void A3::resetAll() {
	resetPosition();
	resetRotation();
	resetJoints(m_rootNode.get());
}

//----------------------------------------------------------------------------------------
JointNode * A3::findJoint(unsigned int objID, SceneNode * root) {

	std::queue<SceneNode*> q;
	q.push(root);

	while (!q.empty()) {
		SceneNode * node = q.front();
		q.pop();
		// cout << node->m_name << " " << node->m_nodeId << endl;

		for (SceneNode * child : node->children) {
			if (child->m_nodeId == objID) {
				if (node->m_nodeType == NodeType::JointNode) {
					child->isSelected = !child->isSelected;
					return (JointNode*)node;
				}
			} else {
				q.push(child);
			}
		}
	}

	return nullptr;
}
//----------------------------------------------------------------------------------------
void A3::pickingSetup() {
	// cout << "try to do picking" << endl;
	double xpos, ypos;
	glfwGetCursorPos( m_window, &xpos, &ypos );

	picking = true;

	uploadCommonSceneUniforms();
	glClearColor(1.0, 1.0, 1.0, 1.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glClearColor(0.85, 0.85, 0.85, 1.0);

	draw();

	// I don't know if these are really necessary anymore.
	// glFlush();
	// glFinish();

	CHECK_GL_ERRORS;

	// Ugly -- FB coordinates might be different than Window coordinates
	// (e.g., on a retina display).  Must compensate.
	xpos *= double(m_framebufferWidth) / double(m_windowWidth);
	// WTF, don't know why I have to measure y relative to the bottom of
	// the window in this case.
	ypos = m_windowHeight - ypos;
	ypos *= double(m_framebufferHeight) / double(m_windowHeight);

	GLubyte buffer[ 4 ] = { 0, 0, 0, 0 };
	// A bit ugly -- don't want to swap the just-drawn false colours
	// to the screen, so read from the back buffer.
	glReadBuffer( GL_BACK );
	// Actually read the pixel at the mouse location.
	glReadPixels( int(xpos), int(ypos), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
	CHECK_GL_ERRORS;

	// Reassemble the object ID.
	unsigned int objID = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16);
	// cout << objID << endl;
	JointNode* joint = findJoint(objID, m_rootNode.get());
	if (joint != nullptr) {
		// cout << joint->m_name << endl;
		if (selectedJoints.find(joint) != selectedJoints.end()) {
			selectedJoints.erase(joint);
		} else {
			selectedJoints.insert(joint);
		}
	}

	picking = false;

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::undoOperation() {
	if (undo.empty()) return;
	cout << "undo operation" << endl;
	vector<JointsInfo> information = undo.top();
	saveState();
	redo.push(oldInfo);
	undo.pop();

	for (auto info : information) {
		info.apply();
	}
}

void A3::redoOperation() {
	if (redo.empty()) return;
	cout << "redo operation" << endl;
	vector<JointsInfo> information = redo.top();
	saveState();
	undo.push(oldInfo);
	redo.pop();

	for (auto info : information) {
		info.apply();
	}
}
//----------------------------------------------------------------------------------------
void A3::applyPositionChange(double xPos, double yPos) {
	double xOffset = xPos - mouseLastX;
	double yOffset = yPos - mouseLastY;

	if (mouse_leftdown) {
		// cout << "left down" << endl;
		translateTrans = translate(mat4(1.0f), vec3(xOffset * TRANSLATE_FACTOR, -yOffset * TRANSLATE_FACTOR, 0)) * translateTrans;
	}

	if (mouse_middledown) {
		// cout << "middle down" << endl;
		translateTrans = translate(mat4(1.0f),  vec3(0, 0, yOffset * TRANSLATE_FACTOR)) * translateTrans;
	}

	float center_x = m_framebufferWidth / 2.0f;
	float center_y = m_framebufferHeight / 2.0f;
	float radius = std::min(m_framebufferWidth / 4.0f, m_framebufferHeight / 4.0f);

	vec3 trackball_pos = vec3(xPos - center_x, -yPos + center_y, 0);
	trackball_pos /= radius;
	float xy_sqr = trackball_pos.x * trackball_pos.x + trackball_pos.y * trackball_pos.y;
	if (xy_sqr > 1) { // outside the ball, rotate around z
		trackball_pos /= sqrt(xy_sqr); // standarize to 1
	} else {
		trackball_pos.z = sqrt(1 - xy_sqr); // z pos
	}

	if (mouse_rightdown) { // trackball

		float cosine = dot(trackball_pos, lastTrackball);
		// if (cosine == 1) return; // angle is not a number
		float angle = acos(cosine);

		// cout << "cosine is " << cosine << endl;
		// cout << "angle is " << angle << endl;
		// cout << "cross is " << cross(trackball_pos, lastTrackball) << endl;
		// cout << "trackball_pos is " << trackball_pos << endl;
		if (angle >= -1 && angle <= 1 &&  cross(trackball_pos, lastTrackball) != vec3(0.0f)) {
			rotateTrans = rotate(mat4(1.0f), angle, cross(trackball_pos, lastTrackball)) * rotateTrans;
		}

		// cout << "rotateTrans is " << rotateTrans << endl;
		// cout << "translateTrans is " << translateTrans << endl;
	}

	lastTrackball = trackball_pos;
}

//----------------------------------------------------------------------------------------
void A3::applyJointsChange(double xPos, double yPos) {
	double yOffset = yPos - mouseLastY;
	if (mouse_rightdown) {
		for (auto node : selectedJoints) {
			node->rotate('x', yOffset * 0.1f);
		}
	}

	if (mouse_middledown) {
		for (auto node : selectedJoints) {
			node->rotate('y', yOffset * 0.1f);
		}
	}
}
//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
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
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);


	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Fill in with event handling code...
		switch (mode) {
			case Position:
				applyPositionChange(xPos, yPos);
				eventHandled = true;
				break;
			case Joints:
				applyJointsChange(xPos, yPos);
				eventHandled = true;
				break;
			default:
				break; 
		}


		mouseLastX = xPos;
		mouseLastY = yPos;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
void A3::saveState() {
	// cout << "current state copy" << endl;
	oldInfo.clear();

	for (auto node : selectedJoints) {
		JointsInfo copy(node);
		oldInfo.push_back(copy);
	}
}
//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);
	double xpos, ypos;
	glfwGetCursorPos( m_window, &xpos, &ypos );

	// Fill in with event handling code...
	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if (actions == GLFW_PRESS) { 
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				mouse_leftdown = true;
				if (mode == Joints) pickingSetup();
				eventHandled = true;
			}

			if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
				mouse_middledown = true;
				if (mode == Joints) saveState();
				eventHandled = true;
			}

			if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				mouse_rightdown = true;
				if (mode == Joints) saveState();
				eventHandled = true;
			}
		}

		if (actions == GLFW_RELEASE) {
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				mouse_leftdown = false;
				eventHandled = true;
			}

			if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
				mouse_middledown = false;
				if (mode == Joints) {
					// cout << "undo add" << endl;
					undo.push(oldInfo);
					redo = stack<vector<JointsInfo>>();
				}
				eventHandled = true;
			}

			if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				mouse_rightdown = false;
				if (mode == Joints) {
					// cout << "undo add" << endl;
					undo.push(oldInfo);
					redo = stack<vector<JointsInfo>>();
				}
				eventHandled = true;
			}
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
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
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		}

		if( key == GLFW_KEY_B ) {
			backCulling = !backCulling;
			eventHandled = true;
		}
		if( key == GLFW_KEY_F ) {
			frontCulling = !frontCulling;
			eventHandled = true;
		}
		if( key == GLFW_KEY_C ) {
			trackBall = !trackBall;
			eventHandled = true;
		}
		if( key == GLFW_KEY_Z ) {
			zBuffer = !zBuffer;
			eventHandled = true;
		}
		if( key == GLFW_KEY_I) {
			resetPosition();
			eventHandled = true;
		}
		if( key == GLFW_KEY_O) {
			resetRotation();
			eventHandled = true;
		}
		if( key == GLFW_KEY_S) {
			resetJoints(m_rootNode.get());
			eventHandled = true;
		}
		if( key == GLFW_KEY_A) {
			resetAll();
			eventHandled = true;
		}
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		}
		if (key == GLFW_KEY_P) {
			mode = Position;
			eventHandled = true;
		}
		if (key == GLFW_KEY_J) {
			mode = Joints;
			eventHandled = true;
		}

		if (key == GLFW_KEY_R) {
			redoOperation();
			eventHandled = true;
		}
		if (key == GLFW_KEY_U) {
			undoOperation();
			eventHandled = true;
		}

	}
	// Fill in with event handling code...

	return eventHandled;
}
