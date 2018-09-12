#pragma once

using std::cout;
using std::endl;

#ifdef _WIN32 
#define platform "Windows"
#include <direct.h>
#define GetDir _getcwd
#endif
#ifdef __APPLE__
#define platform "Mac"
#include <unistd.h>
#define GetDir getcwd
#endif


class Window {
private:
	//Variable declarations
	int win_width = 1600;
	int win_height = 800;
	GLFWwindow* window;

	//Method Declarations
	void initializeWin() {
		char path[512];
		GetDir(path, 512);
		cout << path << endl;


		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		//check if mac, and add backwards commpatibility
		(platform == "Mac") ? glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE) : void();
		// Create GLFW context
		this->window = glfwCreateWindow(win_width, win_height, "Saphira", NULL, NULL);
		glfwSetWindowPos(this->window, 200, 200);
		// Check for problems with context initialization
		if (this->window == NULL) {
			cout << "Failed to create GLFW context" << endl;
			glfwTerminate();
			exit(-1);
		}
		// Make the window the current context
		glfwMakeContextCurrent(this->window);

		// Initialize GLAD
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			cout << "Failed to initialize GLAD" << endl;
			exit(-1);
		}

		// Set OpenGL viewport size
		glViewport(0, 0, win_width, win_height);
		// Set the window resize callback function in glfw


		//set the ColorClear Clear
		glClearColor(0.0,0.0,0.0,1.0);


	}
public:
	Window() {
		this->initializeWin();
		glEnable(GL_DEPTH_TEST);
	}
	void clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	//Getter Methods
	GLFWwindow* getWindow() {
		return this->window;
	}
	int getWidth() {
		return this->win_width;
	}
	int getHeight() {
		return this->win_height;
	}

	//Setter Methods
};