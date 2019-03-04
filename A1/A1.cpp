// Winter 2019

#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <sys/types.h>
#include <unistd.h>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

static const size_t DIM = 16;

// Distance to rotate 1 round
static const size_t DRR = 2048;
static const float PAI = 3.14159;
static const float ScaleRate = 0.1f;
static const float ScaleUpRange = 2.0f;
static const float ScaleLowRange = 0.5f;
static float Avatar_Radius = 0.5f;  

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 ), maze(DIM)
{
	colour[0] = 0.0f;
	colour[1] = 0.0f;
	colour[2] = 0.0f;
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{
}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
	// Initialize random number generator
	int rseed=getpid();
	srandom(rseed);
	// Print random number seed in case we want to rerun with
	// same random numbers
	cout << "Random number seed = " << rseed << endl;
	
	initSettings();

	// maze initilization
	maze.digMaze();
	maze.printMaze();

	// find avatar start position
	for (size_t i = 0; i < DIM; i++) {
		for (size_t j = 0; j < DIM; j++) {
			// find the entrance
			if (i == DIM-1 || i == 0 || j == DIM-1 || j == 0) {
				if (maze.getValue(i,j) == 0) {
					avatar_pos = vec3(i,0,j);
				}
			}
		}
	}
	
	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );

	initGrid();
	initCube();
	initFloor();
	initAvatar();

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt( 
		glm::vec3( 0.0f, 2.*float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	proj = glm::perspective( 
		glm::radians( 30.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );
}

void A1::initGrid()
{
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to recoblock_colorrd buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

void A1::initAvatar() {
	// a semi-circle vertices
	size_t sphere_sz = 180 * 3 * 360 * 2 * 3;
	float* verts = new float[sphere_sz];
	size_t index = 0;
	float r = Avatar_Radius;
	// rotate semi-circle by y-axis
	for (int alpha = 0; alpha < 360; alpha++) {

		// semi-circle
		for (int beta = 90; beta > -90; beta--) {
			// first triangle 
			verts[index + 0] = r * cos(beta * PAI / 180.0f) * cos(alpha * PAI / 180.0f); // x
			verts[index + 1] = r * sin(beta * PAI / 180.0f); // y
			verts[index + 2] = r * cos(beta * PAI / 180.0f) * sin(alpha * PAI / 180.0f); // z

			verts[index + 3] = r * cos((beta-1) * PAI / 180.0f) * cos(alpha * PAI / 180.0f); 
			verts[index + 4] = r * sin((beta-1) * PAI / 180.0f); 
			verts[index + 5] = r * cos((beta-1) * PAI / 180.0f) * sin(alpha * PAI / 180.0f); 

			verts[index + 6] = r * cos(beta * PAI / 180.0f) * cos((alpha+1) * PAI / 180.0f); 
			verts[index + 7] = r * sin(beta * PAI / 180.0f); 
			verts[index + 8] = r * cos(beta * PAI / 180.0f) * sin((alpha+1) * PAI / 180.0f); 

			// second triangle
			verts[index + 9] = r * cos((beta-1) * PAI / 180.0f) * cos(alpha * PAI / 180.0f); 
			verts[index + 10] = r * sin((beta-1) * PAI / 180.0f); 
			verts[index + 11] = r * cos((beta-1) * PAI / 180.0f) * sin(alpha * PAI / 180.0f); 

			verts[index + 12] = r * cos(beta * PAI / 180.0f) * cos((alpha+1) * PAI / 180.0f); 
			verts[index + 13] = r * sin(beta * PAI / 180.0f); 
			verts[index + 14] = r * cos(beta * PAI / 180.0f) * sin((alpha+1) * PAI / 180.0f); 

			verts[index + 15] = r * cos((beta-1) * PAI / 180.0f) * cos((alpha+1) * PAI / 180.0f); 
			verts[index + 16] = r * sin((beta-1) * PAI / 180.0f); 
			verts[index + 17] = r * cos((beta-1) * PAI / 180.0f) * sin((alpha+1) * PAI / 180.0f); 

			index += 18;
		}
	}

	// Create the vertex array to recoblock_colorrd buffer assignments.
	glGenVertexArrays( 1, &m_avatar_vao );
	glBindVertexArray( m_avatar_vao );

	// Create the avatar vertex buffer
	glGenBuffers( 1, &m_avatar_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_avatar_vbo );
	glBufferData( GL_ARRAY_BUFFER, sphere_sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

void A1::initCube() {

	float verts[] = {
		// front bottom left
		0.0f, 0.0f, 1.0f,
		// front top left
		0.0f, block_height, 1.0f,
		// front top right
		1.0f, block_height, 1.0f,
		// front bottom right
		1.0f, 0.0f, 1.0f,
		// back top right
		1.0f, block_height, 0.0f,
		// back bottom right
		1.0f, 0.0f, 0.0f,
		// back top left
		0.0f, block_height, 0.0f,
		// back bottom left
		0.0f, 0.0f, 0.0f
	};

	unsigned int indices[] = {
		// front
		0, 1, 2,
		0, 2, 3,
		// left
		2, 3, 4,
		3, 4, 5,
		// back
		4, 5, 6,
		5, 6, 7,
		// right
		0, 1, 6,
		0, 6, 7,
		// top
		1, 2, 6,
		2, 4 ,6,
		// bottom
		0, 3, 7,
		3, 5, 7
	};



	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_cube_vao );
	glBindVertexArray( m_cube_vao );


	// Create the cube vertex buffer
	glGenBuffers( 1, &m_cube_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_cube_vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(verts),
		verts, GL_STATIC_DRAW );

	// Create the cube element buffer
	glGenBuffers( 1, &m_cube_ebo );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_cube_ebo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),
		indices, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );
	
	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS;
}

void A1::initFloor()
{
	size_t sz = 3 * 2 * 3;

	float verts[] = {
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,
	};

	// Create the vertex array to recoblock_colorrd buffer assignments.
	glGenVertexArrays( 1, &m_floor_vao );
	glBindVertexArray( m_floor_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_floor_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_floor_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	CHECK_GL_ERRORS;
}

void A1::loadColor(int selected_obj, float * color) {
	vec3 tmp_color(0.0f,0.0f,0.0f);
	switch (selected_obj) {
		// block
		case 0:
			tmp_color = block_color;
			break;
		case 1:
			tmp_color = floor_color;
			break;
		case 2:
			tmp_color = avatar_color;
			break;
		default:
			break;
	}
	color[0] = tmp_color.r;
	color[1] = tmp_color.g;
	color[2] = tmp_color.b;
}

void A1::saveColor(int selected_obj, float * color) {
	vec3 tmp_color(color[0], color[1], color[2]);
	switch (selected_obj) {
		// block
		case 0:
			block_color = tmp_color;
			break;
		case 1:
			floor_color = tmp_color;
			break;
		case 2:
			avatar_color = tmp_color;
			break;
		default:
			break;
	}
}

void A1::initSettings() {
	last_obj = -1;
	scale = 1.0f;
	block_height = 1.0f;
	rotate_proportion = 0.0f;
	self_rotate_rate = 0.0f;
	avatar_pos = vec3(0.0f, 0.0f, 0.0f);
	block_color = vec3(0.0f, 1.0f, 1.0f);
	floor_color = vec3(0.0f, 0.5f, 1.0f);
	avatar_color = vec3(1.0f, 0.0f, 0.0f);
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...
	rotate_proportion += self_rotate_rate;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for 
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if( ImGui::Button( "Reset" ) ) {
			maze.reset();
			initSettings();
		}

		if( ImGui::Button( "Dig" ) ) {
			maze.digMaze();
			// find avatar start position
			for (size_t i = 0; i < DIM; i++) {
				for (size_t j = 0; j < DIM; j++) {
					// find the entrance
					if (i == DIM-1 || i == 0 || j == DIM-1 || j == 0) {
						if (maze.getValue(i,j) == 0) {
							avatar_pos = vec3(i,0,j);
						}
					}
				}
			}
		}
		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.
		ImGui::PushID("widget");
		static int selected_obj = 0;
        ImGui::RadioButton("Block", &selected_obj, 0); ImGui::SameLine();
        ImGui::RadioButton("Floor", &selected_obj, 1); ImGui::SameLine();
        ImGui::RadioButton("Avatar", &selected_obj, 2);
		ImGui::PopID();

		if (last_obj != selected_obj) {

			// save original color to current selected_object
			saveColor(last_obj, origin_colour);

			// load  new selected_object's information to original color and color variable
			loadColor(selected_obj, colour);
			loadColor(selected_obj, origin_colour);
			last_obj = selected_obj;
		}

		// Prefixing a widget name with "##" keeps it from being
		// displayed.

		ImGui::PushID( 0 );
		ImGui::ColorEdit3( "##Colour", colour );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col", &current_col, 0 ) ) {
			// Select this colour.

			// original color is changuiiiiiiiiiiiiiiiiiiiiiiii`							`1ed, so the color of selected_object is truely changed
			loadColor(selected_obj, origin_colour);
		}
		ImGui::PopID();

		// use color variable to display on screen
		saveColor(selected_obj, colour);


		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in 
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}


		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	mat4 W;
	
	W = glm::scale(W, vec3(scale));
	W = glm::rotate( W, 2 * PAI * rotate_proportion, vec3( 0, 1, 0) );
	W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );

	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao );
		glUniform3f( col_uni, 1, 1, 1 );
		glDrawArrays( GL_LINES, 0, (3+DIM)*4 );


		// Draw the cubes
		mat4 origin = W;

		for (int i = 0; i < DIM + 2; i++) {
			for (int j = 0; j < DIM + 2; j++) {
				W = glm::translate( W, vec3( i - 1, 0, j - 1) );
				glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );
				
				if (i-1 == -1 || j - 1 == -1 || i - 1 == DIM || j - 1 == DIM 
				|| maze.getValue(i-1,j-1) == 0) {
					// draw floors
					glBindVertexArray( m_floor_vao );
					glUniform3f( col_uni, floor_color.r, floor_color.g, floor_color.b );
					glDrawArrays( GL_TRIANGLES, 0, 6 );

				} else {
					// draw cubes

					//cout << "i is " << i << " j is " << j << endl;
					glBindVertexArray( m_cube_vao );
					glUniform3f( col_uni, block_color.r, block_color.g, block_color.b );
					glDrawElements( GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
				}
				W = origin;
			}
		}

		// draw the avatar
		glBindVertexArray( m_avatar_vao );
		glUniform3f( col_uni, avatar_color.r, avatar_color.g, avatar_color.b );
		W = glm::translate( W, avatar_pos + vec3(Avatar_Radius));
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );
		glDrawArrays( GL_TRIANGLES, 0, 360*180*2*3);
		W = origin;

		// Highlight the active square.
		
	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
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
bool A1::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
		double hori_pos = xPos;
		if (ImGui::IsMouseDragging(0)) {
			rotate_proportion += (hori_pos - last_mouse_pos) / DRR;
			self_rotate_rate = (hori_pos - last_mouse_pos) / DRR;
			drag = true;
		}
		last_mouse_pos = hori_pos;

		eventHandled = true;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);
	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
		if (actions == GLFW_PRESS) {
			drag = false;
			eventHandled = true;
		}
		if (actions == GLFW_RELEASE) {
			if (drag) {
				drag = false;
			} else {
				self_rotate_rate = 0.0f;
			}
			
			eventHandled = true;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Zoom in or out.
	if (yOffSet > 0) {
		if (scale < ScaleUpRange) scale += ScaleRate;
	} else {
		if (scale > ScaleLowRange) scale -= ScaleRate;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		// Respond to some key events.
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		}

		if (key == GLFW_KEY_R) {
			maze.reset();
			initSettings();
			eventHandled = true;
		}

		if (key == GLFW_KEY_D) {
			maze.digMaze();
			// find avatar start position
			for (size_t i = 0; i < DIM; i++) {
				for (size_t j = 0; j < DIM; j++) {
					// find the entrance
					if (i == DIM-1 || i == 0 || j == DIM-1 || j == 0) {
						if (maze.getValue(i,j) == 0) {
							avatar_pos = vec3(i,0,j);
						}
					}
				}
			}
			eventHandled = true;
		}


		if (key == GLFW_KEY_SPACE) {
			if (block_height < 2.0f) block_height += 0.1f;
			initCube();
			eventHandled = true;
		}

		if (key == GLFW_KEY_BACKSPACE) {
			if (block_height > 0) block_height -= 0.1f;
			initCube();
			eventHandled = true;
		}

		if (key == GLFW_KEY_LEFT && avatar_pos.x - 1 >= -1) {
			if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
				glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
				maze.setValue(avatar_pos.x - 1,avatar_pos.z, 0);
				avatar_pos.x -= 1;
			}
			else if (maze.getValue(avatar_pos.x - 1,avatar_pos.z) == 0) {
				avatar_pos.x -= 1;
			}
			eventHandled = true;
		}

		if (key == GLFW_KEY_RIGHT && avatar_pos.x + 1 <= DIM) {
			if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
				glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
				maze.setValue(avatar_pos.x + 1,avatar_pos.z, 0);
				avatar_pos.x += 1;
			}
			else if (maze.getValue(avatar_pos.x + 1,avatar_pos.z) == 0) {
				avatar_pos.x += 1;
			}
			eventHandled = true;
		}

		if (key == GLFW_KEY_UP && avatar_pos.z - 1 >= -1) {
			if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
				glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
				maze.setValue(avatar_pos.x,avatar_pos.z - 1, 0);
				avatar_pos.z -= 1;
			}
			else if (maze.getValue(avatar_pos.x,avatar_pos.z - 1) == 0) {
				avatar_pos.z -= 1;
			}
			eventHandled = true;
		}

		if (key == GLFW_KEY_DOWN && avatar_pos.z + 1 <= DIM) {
			if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
				glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
					maze.setValue(avatar_pos.x,avatar_pos.z + 1, 0);
					avatar_pos.z += 1;
			}
			else if (maze.getValue(avatar_pos.x,avatar_pos.z + 1) == 0) {
				avatar_pos.z += 1;
			}
			eventHandled = true;
		}
	}

	return eventHandled;
}
