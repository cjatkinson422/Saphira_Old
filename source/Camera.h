 #pragma once
#include <array>
#include <GLFW/glfw3.h>
#include "Transforms.h"
#include "Window.h"
#define ROT_CAM 1
#define SHP_CAM 2

class Camera {
private:
	mat4 viewMat = eye4();
	mat4 orientationMat = eye4();
	mat4 projectionMat;

	vec3 position = { 0.0,0.0,0.0 };
	vec3 globalUp = { 0.0,1.0,0.0 };

	vec3 shpUp = { 0.0,1.0,0.0 };
	vec3 shpFw = { 0.0,0.0,1.0 };

	double fovy;
	double aspect;
	double nearClip;
	double farClip;

	int MODE = SHP_CAM;
	bool mCap = false;

	int winWidth;
	int winHeight;

public:
	float moveSpeed = 5000.0f;
	void setSpeed(float speed) {
		this->moveSpeed = speed;
	}
	float mouseSens = 0.02f;
	float rollSpeed = 100.0f;
	Camera(GLFWwindow* window, int width, int height) {
		this->aspect = 16.0 / 9.0;
		this->nearClip = 100;
		this->farClip = 10E9;
		this->fovy = 45.0;
		this->frameResize(width, height);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		mCap = true;
	}
	vec3 getPosition() {
		return -1.0*this->position;
	}
	mat4 getViewMat() {
		return this->viewMat;
	}
	void addPos(vec3 pos) {
		this->position[0] += pos[0];
		this->position[1] += pos[1];
		this->position[2] += pos[2];
		this->updateViewMat();
		return;
	}
	void setPos(vec3 pos) {
		this->position = -1.0*pos;
		this->updateViewMat();
	}


	void lookAt(vec3 target) {
		vec3 camForward = Normalize(vec3{ position[0] - target[0], position[1] - target[1], position[2] - target[2] });
		vec3 globalUp = { 0.0,1.0,0.0 };
		vec3 camRight = Normalize(Cross(camForward, globalUp));
		vec3 camUp = Normalize(Cross(camForward, camRight));
		this->orientationMat = eye4();
		this->orientationMat[0] = { camRight[0],camRight[1],camRight[2], 0.0 };
		this->orientationMat[1] = { camUp[0],camUp[1],camUp[2], 0.0 };
		this->orientationMat[2] = { camForward[0],camForward[1],camForward[2], 0.0 };
		this->updateViewMat();

	}
	void rotate(double angle, vec3 axis) {
		this->orientationMat = orientationMat * RotMat(angle, axis);
	}
	void updateViewMat() {
		if (MODE == SHP_CAM) {
			vec3 shpRt = Cross(shpUp, shpFw);
			orientationMat = eye4();
			orientationMat[0] = { shpRt[0],shpRt[1] ,shpRt[2], 0.0 };
			orientationMat[1] = { shpUp[0],shpUp[1] ,shpUp[2], 0.0 };
			orientationMat[2] = { shpFw[0],shpFw[1] ,shpFw[2], 0.0 };
			this->viewMat = orientationMat * TransMat(position);
		}
		else if (MODE == ROT_CAM) {
			this->viewMat = TransMat(vec3{ -position[0],-position[1],-position[2] }) * this->orientationMat;
		}

	}
	void frameResize(int width, int height) {
		this->winWidth = width;
		this->winHeight = height;
		this->aspect = (float)width / (float)height;
		this->projectionMat = ProjectionMat(radians(this->fovy), this->aspect, this->nearClip, this->farClip);
	}
	double* getProjPtr() {
		return &projectionMat[0][0];
	}
	double* getViewPtr() {
		return &viewMat[0][0];

	}
	double* getPosPtr() {
		return &position[0];
	}



	//Branches the cursor input from cursor_callback into the different camera modes
	void mouse_callback(GLFWwindow* window, double xpos, double ypos, double xrset, double yrset) {

		if (mCap) {
			double xoffset = xrset - xpos;
			double yoffset = yrset - ypos;

			if (MODE == SHP_CAM) {
				this->shpMouseInput(window, xoffset, yoffset);
			}
			else if (MODE == ROT_CAM) {
				this->fpsMouseInput(window, xoffset, yoffset);
			}
			glfwSetCursorPos(window, xrset, yrset);
		}
	}
	//Input function for fps style camera; ie on a planet surface
	void fpsMouseInput(GLFWwindow* window, double xoffset, double yoffset) {

	}
	//Input function for ship style camera
	void shpMouseInput(GLFWwindow* window, double xoffset, double yoffset) {
		this->shpFw = Normalize(qrotl(shpFw, -xoffset*mouseSens, shpUp));
		this->shpFw = Normalize(qrotl(shpFw, -yoffset*mouseSens, Cross(shpUp, shpFw)));
		this->shpUp = Normalize(qrotl(shpUp, -yoffset*mouseSens, Cross(shpUp, shpFw)));
		this->updateViewMat();
	}

	//key input for ship camera
	void shpKeyInput(GLFWwindow* window, double deltaT) {
		if (mCap) {
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				moveSpeed *= 10;
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				addPos(-deltaT * moveSpeed*shpFw);
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				addPos(deltaT * moveSpeed*shpFw);
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				addPos(deltaT * moveSpeed*Cross(shpUp, shpFw));
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				addPos(-deltaT * moveSpeed*Cross(shpUp, shpFw));
			if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
				this->shpUp = qrotl(shpUp, rollSpeed * deltaT, shpFw);
				this->updateViewMat();
			}
			if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
				this->shpUp = qrotl(shpUp, -rollSpeed*deltaT, shpFw);
				this->updateViewMat();
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
				this->addPos(deltaT * moveSpeed*shpUp);
			}
			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
				this->addPos(-deltaT * moveSpeed*shpUp);
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				moveSpeed /= 1000;
		}
	}

	//key press input for ship camera
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			if (mCap) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				mCap = false;
			}
			else {
				glfwSetWindowShouldClose(window, true);
			}

		}

	}
	//mouse button callback
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
		if (!mCap && action == GLFW_PRESS) {
			this->resetCursor(window);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			mCap = true;
			
		}

	}
	void resetCursor(GLFWwindow* window) {
		glfwSetCursorPos(window, (double)winWidth / 2.0, (double)winHeight / 2.0);
	}

};
