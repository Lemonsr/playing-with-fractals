#include "ShaderProgram.h"
#include <GLFW/glfw3.h>


class State {
public:
	//int segments = 0; // no. of subdivisions
	//int scene = 1; // scene to display

	int segments;
	int scene;

	State();

	bool operator==(State const& other) const;
	void set_scene(int scene);
	void set_segments(int segments);
	// comparison operator in structs
	//bool operator == (State const& other) const { // syntax
	//	return segments == other.segments && scene == other.scene;
	//}
};

class MyCallbacks : public CallbackInterface {
	static State state;
public:
	MyCallbacks(ShaderProgram& shader);
	State &getState();
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
private:
	ShaderProgram& shader;
};
