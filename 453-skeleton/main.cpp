#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdio.h>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "MyCallbacks.h"

using namespace std;
using namespace glm;

//struct State {
//	int segments = 0; // no. of subdivisions
//	int scene = 1; // scene to display
//
//	// comparison operator in structs
//	bool operator == (State const& other) const { // syntax
//		return segments == other.segments && scene == other.scene;
//	}
//};

State::State() {
	segments = 0;
	scene = 1;
}

void State::set_scene(int sc) {
	this->scene = sc;
}

void State::set_segments(int sg) {
	this->segments = sg;
}

MyCallbacks::MyCallbacks(ShaderProgram& shader)
: shader(shader)
{
}

State MyCallbacks::state = State();

bool State::operator==(State const& other) const {
	return segments == other.segments && scene == other.scene;
}

State& MyCallbacks::getState() {
	return MyCallbacks::state;
}

void MyCallbacks::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) {
		case GLFW_KEY_R:
			break;
		case GLFW_KEY_UP: // increment level
			if (state.segments < 7) {
				state.segments++;
			}
			break;
		case GLFW_KEY_DOWN: // decrement level
			if (state.segments > 0) {
				state.segments--;
			}
			break;

			// cases to change scenes
		case GLFW_KEY_1:
			state.scene = 1;
			break;
		case GLFW_KEY_2:
			state.scene = 2;
			break;
		case GLFW_KEY_3:
			state.scene = 3;
			break;
		case GLFW_KEY_4:
			state.scene = 4;
			break;
		}
	}
}

// CALLBACKS 
//class MyCallbacks : public CallbackInterface {
//
//public:
//	MyCallbacks(ShaderProgram& shader) : shader(shader) {}
//
//	virtual void keyCallback(int key, int scancode, int action, int mods) {
//		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
//			switch (key) {
//			case GLFW_KEY_R:
//				shader.recompile();
//				break;
//			case GLFW_KEY_UP: // increment level
//				if (state.segments < 7) {
//					state.segments++;
//				}
//				break;
//			case GLFW_KEY_DOWN: // decrement level
//				if (state.segments > 0) {
//					state.segments--;
//				}
//				break;
//
//			// cases to change scenes
//			case GLFW_KEY_1:
//				state.scene = 1;
//				break;
//			case GLFW_KEY_2:
//				state.scene = 2;
//				break;
//			case GLFW_KEY_3:
//				state.scene = 3;
//				break;
//			case GLFW_KEY_4:
//				state.scene = 4;
//				break;
//			}
//		}
//	}
//
//	State getState() {
//		return state;
//	}
//
//private:
//	ShaderProgram& shader;
//	State state;
//};


// set 4 colours to choose from
const vec3 RED = vec3(1.f, 0.f, 0.f);
const vec3 GREEN = vec3(0.f, 1.f, 0.f);
const vec3 BLUE = vec3(0.f, 0.f, 1.f);
const vec3 YELLOW = vec3(1.f, 1.f, 0.f);

// stores the initial points and colours of the base triangle for the fractals
struct Triangle {
	vec3 p1, p2, p3, colour;
};

Triangle sierpinski = {
	vec3(-0.5f, -0.5f, 0.f),
	vec3(0.5f, -0.5f, 0.f),
	vec3(0.f, sqrt(0.75) - 0.5f, 0.f), // y is not 0.5 for equilateral triangle! use a^2 + b^2 = c^2 and remember to count from relative coordinates.
	vec3(0.05f, 0.05f, 0.05f) // grey
};

Triangle uniform = {
	vec3(-0.5f, -0.5f, 0.f),
	vec3(0.5f, -0.5f, 0.f),
	vec3(0.f, 0.5f, 0.f),
	vec3(0.05f, 0.f, 0.5f) // blue
};

Triangle koch = {
	vec3(-0.5f, -0.5f, 0.f),
	vec3(0.5f, -0.5f, 0.f),
	vec3(0.f, sqrt(0.75) - 0.5f, 0.f),
	vec3(0.5f, 0.f, 0.f) // red 
};

// clears verts and cols for cpu
void clear(CPU_Geometry& cpuGeom) {
	cpuGeom.verts.clear();
	cpuGeom.cols.clear();
}

// uploads verts and cols from cpu to gpu
void upload(CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom) {
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
}

// draws the base triangle with 1 solid colour
void drawTriangle(vec3 p0, vec3 p1, vec3 p2, vec3 colour, CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom) {

	// add vertices for triangle
	cpuGeom.verts.push_back(p0);
	cpuGeom.verts.push_back(p1);
	cpuGeom.verts.push_back(p2);

	// add colour for triangle
	for (int i = 0; i < 3; i++) {
		cpuGeom.cols.push_back(colour);
	}

	upload(cpuGeom, gpuGeom);
}

// recursive algorithm for the sierpinski triangle fractal
void subdivideSierpinskiTriangle(vec3 p0, vec3 p1, vec3 p2, vec3 colour, int i, CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom) {
	vec3 v0{}, v1{}, v2{}; // intermediate points for subdivision
	
	if (i <= 0) { // base case
		drawTriangle(p0, p1, p2, colour, cpuGeom, gpuGeom);
	}
	else {
		for (int j = 0; j < 2; j++) {
			v0[j] = (p0[j] + p1[j]) / 2;
			v1[j] = (p1[j] + p2[j]) / 2;
			v2[j] = (p2[j] + p0[j]) / 2;
		}

		// adjust the colours
		vec3 moreRed = normalize(colour + vec3(0.25f * i, -0.01f, -0.01f));
		vec3 moreGreen = normalize(colour + vec3(-0.01f, 0.25f * i, -0.01f));
		vec3 moreBlue = normalize(colour + vec3(-0.01f, -0.01f, 0.25f * i));

		// recursive calls
		subdivideSierpinskiTriangle(p0, v0, v2, moreRed, i - 1, cpuGeom, gpuGeom);
		subdivideSierpinskiTriangle(p1, v0, v1, moreGreen, i - 1, cpuGeom, gpuGeom);
		subdivideSierpinskiTriangle(p2, v1, v2, moreBlue, i - 1, cpuGeom, gpuGeom);
	}
}

// draws connecting lines to centroid of triangle
void drawCentroid(vec3 p0, vec3 p1, vec3 p2, vec3 colour, CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom) {
	vec3 centroid = vec3((p0[0] + p1[0] + p2[0]) / 3.f, (p0[1] + p1[1] + p2[1]) / 3.f, 0.f); // centroid coords

	// pass in 6 vertices for 3 lines
	cpuGeom.verts.push_back(p0);
	cpuGeom.verts.push_back(centroid);
	cpuGeom.verts.push_back(p1);
	cpuGeom.verts.push_back(centroid);
	cpuGeom.verts.push_back(p2);
	cpuGeom.verts.push_back(centroid);

	// add colours for lines
	for (int i = 0; i < 6; i++) {
		cpuGeom.cols.push_back(colour);
	}

	upload(cpuGeom, gpuGeom);
}

// recursive algorithm for the uniform triangle mass center fractal
void subdivideUniformTriangle(vec3 p0, vec3 p1, vec3 p2, vec3 colour, int i, CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom) {
	vec3 centroid{};
	if (i > 0) {
		centroid = vec3((p0[0] + p1[0] + p2[0]) / 3.f, (p0[1] + p1[1] + p2[1]) / 3.f, 0.f);
		// recursive calls
		drawCentroid(p0, p1, p2, colour, cpuGeom, gpuGeom);
		subdivideUniformTriangle(p0, p1, centroid, colour, i - 1, cpuGeom, gpuGeom);
		subdivideUniformTriangle(p1, p2, centroid, colour, i - 1, cpuGeom, gpuGeom);
		subdivideUniformTriangle(p2, p0, centroid, colour, i - 1, cpuGeom, gpuGeom);
	}
}

// gets points v0 and v2 along p0-p1 line based on length from p0.
vec3 calcKinkBase(vec3 p0, vec3 p1, float len) {
	return vec3{
		p0[0] + (p1[0] - p0[0]) * len, // x
		p0[1] + (p1[1] - p0[1]) * len, // y
		0.f							   // z
	};
}

// gets point v1 (protruding vertex of kink between v0 and v2).
vec3 rotateAboutV0(vec3 v0, vec3 v2, float angle) {
	// offset between v0 and v2
	float dx = v2[0] - v0[0];
	float dy = v2[1] - v0[1];

	// rotate v2 by degrees about v0
	return vec3{
		v0[0] + (cos(radians(angle)) * dx + sin(radians(angle)) * dy), // x
		v0[1] + (cos(radians(angle)) * dy - sin(radians(angle)) * dx), // y
		0.f														       // z
	};
}

// draws a line
void drawLine(vec3 p0, vec3 p1, vec3 colour, CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom) {

	// add vertices for line
	cpuGeom.verts.push_back(p0);
	cpuGeom.verts.push_back(p1);

	// add colour for line
	for (int i = 0; i < 2; i++) {
		cpuGeom.cols.push_back(colour);
	}

	upload(cpuGeom, gpuGeom);
}

// recursive algorithm for the koch snowflake fractal
void subdivideKoch(vec3 p0, vec3 p1, vec3 colour, int i, CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom) {
	vec3 v0{}, v1{}, v2{};

	if (i == 0) { // base case
		drawLine(p0, p1, colour, cpuGeom, gpuGeom);
	} else {
		v0 = calcKinkBase(p0, p1, 1.f / 3.f);
		v2 = calcKinkBase(p0, p1, 2.f / 3.f);
		v1 = rotateAboutV0(v0, v2, 60.f);

		// adjust the colours such that each subdivision is differentiable
		vec3 newColour = vec3(i / 10.f, cos(i), sin(i));

		// recursive calls for _/\_
		subdivideKoch(p0, v0, colour, i - 1, cpuGeom, gpuGeom);
		subdivideKoch(v0, v1, newColour, i - 1, cpuGeom, gpuGeom);
		subdivideKoch(v1, v2, newColour, i - 1, cpuGeom, gpuGeom);
		subdivideKoch(v2, p1, colour, i - 1, cpuGeom, gpuGeom);
	}
}

// main function to draw Koch starting with a triangle shape
void drawKoch(vec3 p0, vec3 p1, vec3 p2, vec3 colour, int i, CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom) {
	subdivideKoch(p0, p1, colour, i, cpuGeom, gpuGeom);
	subdivideKoch(p1, p2, colour, i, cpuGeom, gpuGeom);
	subdivideKoch(p2, p0, colour, i, cpuGeom, gpuGeom);
}

// add the colours such that each subdivision is differentiable
void addColoursDragon(int i, CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom) {
	int colourInt = 0; // loop through 4 colours
	for (int j = 0; j <= i;  j++) {
		for (int k = 0; k < pow(2, j + 1); k++) {
			switch (colourInt % 4) {
			case 0:
				cpuGeom.cols.push_back(RED);
				break;
			case 1:
				cpuGeom.cols.push_back(YELLOW);
				break;
			case 2:
				cpuGeom.cols.push_back(GREEN);
				break;
			case 3:
				cpuGeom.cols.push_back(BLUE);
				break;
			}
		}
		colourInt++;
	}

	upload(cpuGeom, gpuGeom);
}

// recursive algorithm for the dragon curve
void subdivideDragon(vec3 p0, float len, float angle, int i, CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom) {
	vec3 p1 = vec3(p0[0] + len * cos(angle), p0[1] + len * sin(angle), 0.f); // 2nd point for the line

	if (i == 0) { // base case
		cpuGeom.verts.push_back(p0);
		cpuGeom.verts.push_back(p1);
	} else {
		float newLength = len / sqrt(2.f);
		float p0Angle = angle - quarter_pi<float>();
		float p1Angle = angle + pi<float>() * 5/4;

		subdivideDragon(p0, newLength, p0Angle, i - 1, cpuGeom, gpuGeom);
		subdivideDragon(p1, newLength, p1Angle, i - 1, cpuGeom, gpuGeom);
	}
}

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window glfwWindow(10, 10, "CPSC 453"); // can set callbacks at construction if desired
	GLFWwindow* window = glfwCreateWindow(1600, 1600, "Assignment 1", NULL, NULL);
	//if (window == NULL)
	//	return 1;
	glfwMakeContextCurrent(window);
	//glfwSwapInterval(1); // Enable vsync

	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// GUI
	glfwSetErrorCallback(glfw_error_callback);

	// Decide GL+GLSL versions
	#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
		const char* glsl_version = "#version 100";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	#elif defined(__APPLE__)
		// GL 3.2 + GLSL 150
		const char* glsl_version = "#version 150";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
	#else
		// GL 3.0 + GLSL 130
		const char* glsl_version = "#version 130";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	#endif

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\tahoma.ttf", 36.0f); // increase font size
	
	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true); // install_callbacks = true to install own callbacks
	ImGui_ImplOpenGL3_Init(glsl_version);

	ImGui::StyleColorsDark();

	// CALLBACKS
	auto callbacks = make_shared<MyCallbacks>(shader); // callbacks is a pointer
	//glfwWindow.setCallbacks(callbacks);
	//GLFWkeyfun callbacks =
	//GLFWkeyfun gkf = mcb.keyCallback;

	glfwSetKeyCallback(window, (*callbacks).keyCallback);

	// GEOMETRY
	CPU_Geometry cpSierpinski, cpUniform1, cpUniform2, cpKoch, cpDragon;
	GPU_Geometry gpSierpinski, gpUniform1, gpUniform2, gpKoch, gpDragon;

	State state;

	//ImGuiContext* GImGui = NULL;
	//ImGuiContext& g = *GImGui;

	// display part I on load
	subdivideSierpinskiTriangle(sierpinski.p1, sierpinski.p2, sierpinski.p3, sierpinski.colour, state.segments, cpSierpinski, gpSierpinski);

	// RENDER LOOP
	while (!glfwWindowShouldClose(window)) { // cannot assign window.shouldClose() to bool for while loop, the window cannot close!
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();https://stackoverflow.com/collectives
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// if state changed: either segment/scene change
		if (!(state == callbacks->getState())) {
			state = callbacks->getState(); // update state

			// clear all loaded cpuGeoms
			clear(cpSierpinski);
			clear(cpUniform1);
			clear(cpUniform2);
			clear(cpKoch);
			clear(cpDragon);

			switch (state.scene) {
			case 1: // PART I: Sierpinski Triangle
				subdivideSierpinskiTriangle(sierpinski.p1, sierpinski.p2, sierpinski.p3, sierpinski.colour, state.segments, cpSierpinski, gpSierpinski);
				break;
			case 2: // PART II: Uniform Triangle Mass Center
				drawTriangle(uniform.p1, uniform.p2, uniform.p3, uniform.colour, cpUniform1, gpUniform1); // draw outermost triangle (with no inner lines)
				subdivideUniformTriangle(uniform.p1, uniform.p2, uniform.p3, uniform.colour, state.segments, cpUniform2, gpUniform2);
				break;
			case 3: // PART III: Koch Snowflake
				drawKoch(koch.p1, koch.p2, koch.p3, koch.colour, state.segments, cpKoch, gpKoch);
				break;
			case 4: // PART IV: Dragon Curve
				subdivideDragon(vec3(-0.5f, 0.f, 0.f), 1, 0, state.segments, cpDragon, gpDragon);
				addColoursDragon(state.segments, cpDragon, gpDragon);
				break;
			case 5: // BONUS: Hilbert Curve
				// todo
				break;
			}
		}

		// settings go after the vertex processing function calls
		shader.use();

		glEnable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw arrays based on the scene
		switch (state.scene) {
		case 1: // PART I: Sierpinski Triangle
			gpSierpinski.bind();
			for (int i = 0; i < pow(3, state.segments); i++) {
				glDrawArrays(GL_TRIANGLES, i * 3, 3);
			}
			break;
		case 2: // PART II: Uniform Triangle Mass Center
			gpUniform1.bind();
			glDrawArrays(GL_LINE_LOOP, 0, 3);

			gpUniform2.bind();
			for (int i = 0; i < pow(3, state.segments); i++) {
				glDrawArrays(GL_LINES, i * 6, GLsizei(cpUniform2.verts.size()));
			}
			break;
		case 3: // PART III: Koch Snowflake
			gpKoch.bind();
			for (int i = 0; i < pow(3, state.segments); i++) {
				glDrawArrays(GL_LINES, i * 2, GLsizei(cpKoch.verts.size()));
			}
			break;
		case 4: // PART IV: Dragon Curve
			gpDragon.bind();
			for (int i = 0; i < pow(2, state.segments); i++) {
				glDrawArrays(GL_LINES, i * 2, GLsizei(cpDragon.verts.size()));
			}
			break;
		case 5: // BONUS: Hilbert Curve
			// todo
			break;
		}

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		// show window
		{
			int currScene = state.scene;
			int currSegments = state.segments;

			ImGui::Begin("Fractals by Liaw Xin Yan", NULL, ImGuiWindowFlags_AlwaysAutoResize); // window resizes with font size

			ImGui::Text("Change scenes by using the 'Scene' slider.");
			ImGui::Text("Change subdivisions by using the 'Subdivisions' slider.");
			ImGui::Spacing();

			if (ImGui::SliderInt("Scene", &currScene, 1, 4)) {
				callbacks->getState().set_scene(currScene);
			}
			if (ImGui::SliderInt("Subdivisions", &currSegments, 0, 7)) {
				callbacks->getState().set_segments(currSegments);
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//glfwWindow.swapBuffers();
		glfwSwapBuffers(window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
