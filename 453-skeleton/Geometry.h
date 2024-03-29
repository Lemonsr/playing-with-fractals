#pragma once

//------------------------------------------------------------------------------
// This file contains simple classes for storing geomtery on the CPU and the GPU
// Later assignments will require you to expand these classes or create your own
// similar classes with the needed functionality
//------------------------------------------------------------------------------

#include "VertexArray.h"
#include "VertexBuffer.h"

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>

// added
using namespace glm;

// List of vertices and colour using std::vector and vec3
struct CPU_Geometry {
	std::vector<vec3> verts;
	std::vector<vec3> cols;
};


// VAO and two VBOs for storing vertices and colours, respectively
class GPU_Geometry {

public:
	GPU_Geometry();

	// Public interface
	void bind() { vao.bind(); }

	void setVerts(const std::vector<vec3>& verts);
	void setCols(const std::vector<vec3>& cols);

private:
	// note: due to how OpenGL works, vao needs to be 
	// defined and initialized before the vertex buffers
	VertexArray vao;

	VertexBuffer vertBuffer;
	VertexBuffer colBuffer;
};
