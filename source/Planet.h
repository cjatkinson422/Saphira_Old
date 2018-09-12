#pragma once
#include <map>
#include "Transforms.h"
#include "SceneObject.h"
#include "Material.h"
#include "Globals.h"

# define PI  3.14159265358979323846


struct bodyState {
	double JDTDB;
	vec3 position;
	vec3 velocity;
};

struct bodyLoader {
	string name;
	string shaderType="DS_1";
	bool specular = true;
	bool normal = false;
	bool rings = false;
	bool atmosphere = false;
};

struct planetLoader : bodyLoader {
	int nrMoons = 0;
	bodyLoader* moonLoaders[16];
	
};

class Planet;
class Moon;
class PlanetaryBody;

class PlanetaryBody{
public:
	string name;
	Planet * parentBody=NULL;

	double mu;
	vec3 velocity;
	vec3 position;
	double angularV;
	vec3 rotationAxis = vec3{ 0.0,1.0,0.0 };

	SceneObject* bodyMesh;
	SceneObject* atmoMesh;
	SceneObject* ringMesh;
	Material* bodyMat;
	Material* atmoMat;
	Material* ringMat;

	std::vector<bodyState> EphemerisData;
	std::map<string, double> properties;

	PlanetaryBody(bodyLoader bodyToLoad);


	void loadEphemeris(string body);

	void setPosition(vec3 pos);
	void setVelocity(vec3 vel);
	vec3 getPosition();
	vec3 getVelocity();

	virtual void interpToTime(double julian);

	bodyState interpEphIndex(double julian);
	unsigned int curEphIndex = 0;
	void updateCurEphIndex(double julian);
protected:
	// ONLY CALL FROM INTERPEPHINDEX FUNCTION. OTHERWISE WILL MOST LIKELY GIVE NONSENSE
	bodyState interpbodyState(bodyState ps1, bodyState ps2, double julian);
};

class Moon : public PlanetaryBody {
public:
	Moon(bodyLoader moonToLoad, Planet* parent);

	vec3 getLocalPosition();
	void setLocalPosition(vec3 pos);
	vec3 getLocalVelocity();
	void setLocalVelocity(vec3 vel);

	unsigned int lineVAO, lineVBO, lineDataSize;
	Material* lineMat;
	void genOrbitLines();
	void updateOrbitLines();
	void drawLines();

	void interpToTime(double julian);

	void draw();

};

class Planet : public PlanetaryBody {
public:
	std::vector<Moon*> moons;
	int nrMoons=0;

	Planet(planetLoader planetToLoad);


	unsigned int lineVAO, lineVBO, lineDataSize;
	Material* lineMat;
	void genOrbitLines();
	void updateOrbitLines();
	void drawLines();

	void setUniform4dv(const char* uniformName, double* uniformPtr);
	void setUniformV3(const char* uniformName, float* vec);
	void setUniformV3(const char* uniformName, double* vec);
	void setUniformV3(const char* uniformName, vec3f vec);
	void setUniformV3(const char* uniformName, vec3 vec);

	void draw();

};


class PlanetarySystem {
public:
	int nrPlanets = 0;
	Skybox* skybox;
	Material* skyboxMat;
	vector<Planet*> Planets;

	PlanetarySystem();

	void loadPlanet(planetLoader loader);

	void draw();

	void setUniform4dv(const char* uniformName, double* uniformPtr);
	void setUniformV3(const char* uniformName, float* vec);
	void setUniformV3(const char* uniformName, double* vec);
	void setUniformV3(const char* uniformName, vec3f vec);
	void setUniformV3(const char* uniformName, vec3 vec);


	double getClosestPlanetDistance(vec3 position);

	void updateEphIndices(double julian);
	void updatePlanetPositions(double julian);

	void updateOrbitLines();
};

