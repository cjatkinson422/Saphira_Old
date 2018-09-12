#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

#include "TexHandler.h"
#include "Shader.h"
#include "Window.h"
#include "Transforms.h"
#include "Camera.h"
#include "Material.h"
#include "Planet.h"
#include "Spacecraft.h"
#include "Globals.h"

void inputHandler(double dTime);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void loadPlanets(PlanetarySystem* solarSystem);
vec6 motionEqn(double time, vec6 input);

TexHandler* texHandler;

vector<planetLoader> bodiesToLoad;

PlanetarySystem* solarSystem;

Window mainWindow = Window();
GLFWwindow* window = mainWindow.getWindow();



Camera mainCamera = Camera(window, mainWindow.getWidth(), mainWindow.getHeight());

int main() {
	//create the main texture handler. External global variable
	texHandler = new TexHandler();
	//create the solar system and then load the planets defined in function below
	solarSystem = new PlanetarySystem();
	loadPlanets(solarSystem);

	//set the start time to somewhere in the ephemeris table
	double startTime = 2459580.500000000;
	//update the solar system to given start time
	solarSystem->updateEphIndices(startTime);
	solarSystem->updatePlanetPositions(startTime);
	solarSystem->updateOrbitLines();

	//set the spacecraft initial data
	vec3 startPos = solarSystem->Planets[1]->getPosition() + vec3{ 0.0,150000.0,0.0 };
	vec3 startVel = solarSystem->Planets[1]->getVelocity() + vec3{30.0,0.0,0.0};
	//vec3 startVel = vec3{ 0.0,0.0,0.0 };
	
	//create the craft to propagate
	Spacecraft* falcon = new Spacecraft(startTime, startPos, startVel, solarSystem);
	falcon->solver = motionEqn;
	//propagate the craft through given time and error
	falcon->propagate(startTime, 5.0, 1e-5);
	//falcon->loadEpm("craft1.epm");   //this loads saved propagation data so you dont have to propagate each time you run
	falcon->genOrbitLines(solarSystem->Planets[1]);
	//after propagation, solar system is in wrong state, so reset to intial
	solarSystem->updateEphIndices(startTime);
	solarSystem->updatePlanetPositions(startTime);

	//set the main camera position. need to change this later
	vec3 camPos;
	camPos = falcon->position + vec3{ 0.0,0.0,-1200.0 };
	cout << "Setting camera position to " << camPos[0] << " " << camPos[1] << " " << camPos[2] << endl;
	mainCamera.setPos(camPos);

	//some graphics stuff
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPos(window, (float)mainWindow.getWidth() / 2.0, (float)mainWindow.getHeight() / 2.0);
	//more graphics stuff
	solarSystem->setUniform4dv("projection", mainCamera.getProjPtr());
	falcon->setUniform4dv("projection", mainCamera.getProjPtr());

	//time handling variables
	double prevTimeSeconds = glfwGetTime();
	double dtimeSeconds;
	float simSpeed = 1.0;
	double timeJulian = startTime;
	// Main Loop
	while (1) {
		if (glfwWindowShouldClose(window))
			break;
		mainWindow.clear();

		//update time
		dtimeSeconds = glfwGetTime() - prevTimeSeconds;
		timeJulian += 10.0*secondsToJulianOffset(dtimeSeconds); // 10.0 is a simulation speed. can change to any arbitrary number. 1.0 is realtime
		prevTimeSeconds = glfwGetTime();
		//update the solar system to the new time
		//solarSystem->updatePlanetPositions(timeJulian);
		//solarSystem->updateOrbitLines();


		inputHandler(dtimeSeconds);
		solarSystem->skybox->setPos(mainCamera.getPosition());
		solarSystem->setUniform4dv("view", mainCamera.getViewPtr());
		solarSystem->setUniformV3("camPos", mainCamera.getPosPtr());

		falcon->setUniform4dv("view", mainCamera.getViewPtr());
		falcon->setUniformV3("camPos", mainCamera.getPosPtr());

		//sets the camera max speed to the distance from a planet. makes moving around the system a lot smoother and faster
		float speed = 0.1*solarSystem->getClosestPlanetDistance(mainCamera.getPosition());
		mainCamera.setSpeed(speed);

		//draw the system and the craft to the screen
		solarSystem->draw();
		falcon->draw();

		
		//check events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
		
	}

	// Exit the program
	glfwTerminate();
	return 0;
}

//Framebuffer callback. Cannot be a member, so need to leave it here for now
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	glfwSetCursorPos(window, (float)width / 2.0, (float)height / 2.0);
	mainCamera.frameResize(width, height);
	solarSystem->setUniform4dv("projection", mainCamera.getProjPtr());
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	mainCamera.mouse_callback(window, xpos, ypos, (double)mainWindow.getWidth() / 2.0, (double)mainWindow.getHeight() / 2.0);

}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	mainCamera.mouse_button_callback(window, button, action, mods);
}
void inputHandler(double dTime) {
	mainCamera.shpKeyInput(window, dTime);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	mainCamera.key_callback(window, key, scancode, action, mods);
}


void loadPlanets(PlanetarySystem* solarSystem) {
	//load sun
	planetLoader solLoader;
	solLoader.name = "sol"; solLoader.shaderType = "light"; solLoader.specular = false;
	solarSystem->loadPlanet(solLoader);
	cout << endl;
	//load mercury
	planetLoader mercuryLoader;
	mercuryLoader.name = "mercury"; mercuryLoader.shaderType = "DS_1"; mercuryLoader.specular = true;
	//solarSystem->loadPlanet(mercuryLoader);
	cout << endl;
	//load venus
	planetLoader venusLoader;
	venusLoader.name = "venus"; venusLoader.shaderType = "DS_1"; venusLoader.specular = true;
	//solarSystem->loadPlanet(venusLoader);
	cout << endl;
	//luna loader
	bodyLoader lunaLoader;
	lunaLoader.name = "luna"; lunaLoader.shaderType = "DS_1"; lunaLoader.specular = true;
	//earth loader
	planetLoader earthLoader;
	earthLoader.name = "earth"; earthLoader.shaderType = "DS_1"; earthLoader.specular = true;
	earthLoader.nrMoons = 1; earthLoader.moonLoaders[0] = &lunaLoader;
	//solarSystem->loadPlanet(earthLoader);
	cout << endl;
	//mars loader
	planetLoader marsLoader;
	marsLoader.name = "mars"; marsLoader.shaderType = "DS_1"; marsLoader.specular = true;
	//solarSystem->loadPlanet(marsLoader);
	cout << endl;
	//jupiter loader
	bodyLoader europaLoader; europaLoader.name = "europa";
	bodyLoader callistoLoader; callistoLoader.name = "callisto";
	bodyLoader ioLoader; ioLoader.name = "io";
	bodyLoader ganymedeLoader; ganymedeLoader.name = "ganymede";
	planetLoader jupiterLoader;
	jupiterLoader.name = "jupiter";
	jupiterLoader.nrMoons = 4;
	jupiterLoader.moonLoaders[0] = &ioLoader; jupiterLoader.moonLoaders[1] = &europaLoader; jupiterLoader.moonLoaders[2] = &callistoLoader; jupiterLoader.moonLoaders[3] = &ganymedeLoader;
	solarSystem->loadPlanet(jupiterLoader);
	cout << endl;
	//saturn loader
	planetLoader saturnLoader;
	saturnLoader.name = "saturn"; saturnLoader.shaderType = "DS_1"; saturnLoader.specular = true; saturnLoader.rings = true;
	solarSystem->loadPlanet(saturnLoader);
	cout << endl;
	//uranus loade
	planetLoader uranusLoader;
	uranusLoader.name = "uranus"; uranusLoader.shaderType = "DS_1"; uranusLoader.specular = true;
	//solarSystem->loadPlanet(uranusLoader);
	cout << endl;
	//neptune loader
	planetLoader neptuneLoader;
	neptuneLoader.name = "neptune"; neptuneLoader.shaderType = "DS_1"; neptuneLoader.specular = true;
	//solarSystem->loadPlanet(neptuneLoader);
	cout << endl;

}

vec6 motionEqn(double time, vec6 input) {
	double unitConv = 24.0*60.0*60.0;
	vec3 position = vec3{ input[0],input[1],input[2] };
	vec3 acceleration = vec3{ 0.0,0.0,0.0 };
	vec3 dir;
	bodyState pstate, mstate;
	double r;
	for (int i = 0; i < solarSystem->nrPlanets; ++i) {
		pstate = solarSystem->Planets[i]->interpEphIndex(time);
		dir = pstate.position - position;
		r = len(dir);
		acceleration += solarSystem->Planets[i]->mu*unitConv*unitConv*dir / (r*r*r);
		for (int j = 0; j < solarSystem->Planets[i]->nrMoons; ++j) {
			mstate = solarSystem->Planets[i]->moons[j]->interpEphIndex(time);
			dir = (mstate.position+pstate.position) - position;
			r = len(dir);
			acceleration += solarSystem->Planets[i]->moons[j]->mu*unitConv*unitConv*dir / (r*r*r);
		}
	}
	
	vec6 dx = { input[3],input[4],input[5], acceleration[0], acceleration[1], acceleration[2] };
	return dx;
}
