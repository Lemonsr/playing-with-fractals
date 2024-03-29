#include "Geometry.h"

#include <utility>


GPU_Geometry::GPU_Geometry()
	: vao()
	, vertBuffer(0, 3, GL_FLOAT)
	, colBuffer(1, 3, GL_FLOAT)
{}


void GPU_Geometry::setVerts(const std::vector<vec3>& verts) {
	vertBuffer.uploadData(sizeof(vec3) * verts.size(), verts.data(), GL_STATIC_DRAW);
}


void GPU_Geometry::setCols(const std::vector<vec3>& cols) {
	colBuffer.uploadData(sizeof(vec3) * cols.size(), cols.data(), GL_STATIC_DRAW);
}
