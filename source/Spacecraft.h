#pragma once
#include <iostream>
#include "Transforms.h"
#include "Planet.h"
#include "OdeSolver.h"



class Spacecraft {
public:
	vec3 position;
	vec3 velocity;
	vec3 angularVelocity;

	vector<state> propagationData;

	SceneObject* bodyMesh;
	Material* bodyMat;
	Material* lineMat;

	vec6(*solver)(double, vec6);
	PlanetarySystem* parentSystem;
	PlanetaryBody* parentBody;

	Spacecraft(double startTime, vec3 startPos, vec3 startVel, PlanetarySystem* syst, string craftFile = "craft") {
		parentSystem = syst;
		bodyMesh = new SceneObject("craft",startPos);
		bodyMesh->setScale(10.0);
		texHandler->loadTexture("craft", true, false);
		bodyMat = new Material("DS_1", "craft");
		bodyMat->addObj(bodyMesh);
		this->position = startPos;
		this->velocity = startVel;

		float sunStr = 5.0;
		float sunLin = 5e-9;
		float sunQuad = 0.0;
		if (bodyMat) {
			bodyMat->matShader->setUniformV3("sun.color", vec3f{ 1.0f,1.0f,0.95f });
			bodyMat->matShader->setUniformV3("sun.position", vec3{ 0.0,0.0,0.0 });
			bodyMat->matShader->setUniform1f("sun.attn_str", sunStr);
			bodyMat->matShader->setUniform1f("sun.attn_lin", sunLin);
			bodyMat->matShader->setUniform1f("sun.attn_quad", sunQuad);
		}

	}
	void addPos(vec3 pos) {
		this->position += pos;
		this->bodyMesh->addPos(pos);
	}
	void addVel(vec3 vel) {
		this->velocity += vel;
	}
	void propagate(double initTime, double deltaTime, double tolerance) {
		double unitConv = 24.0*60.0*60.0;
		cout << "Beginning spacecraft propagation. Working...";
		vec6 odeInput = vec6{ position[0], position[1], position[2], unitConv*velocity[0], unitConv*velocity[1], unitConv*velocity[2] };
		vec3_rkf45(odeInput, initTime, deltaTime, tolerance, solver, &propagationData);
		cout << " done!" << endl;
		FILE* file;
		std::ofstream craftEpm("Ephemeris/Lander/craft1.epm");
		if (craftEpm.is_open()) {
			craftEpm.precision(20);
			for (int i = 0; i < propagationData.size(); ++i) {
				craftEpm << propagationData[i].JDTDB<<",";
				craftEpm << propagationData[i].position << ",";
				craftEpm << propagationData[i].velocity << "\n";
			}
			craftEpm.close();
		}
	}
	void loadEpm(string filename) {
		string path = "Ephemeris/Lander/" + filename;
		FILE* file;
		errno_t err = fopen_s(&file, path.c_str(), "r");
		if (err) {
			cout << "Could not open file: " << path << endl;
			return;
		}
		cout << "Beginning ephemeris read of " << path << endl;
		int res=1;
		while (res!=EOF) {
			double julian;
			vec3 pos;
			vec3 vel;
			res = fscanf_s(file, "%lg,%lg,%lg,%lg,%lg,%lg,%lg\n", &julian, &pos[0], &pos[1], &pos[2], &vel[0], &vel[1], &vel[2]);
			state newState = { julian,pos,vel };
			propagationData.push_back(newState);
		}
		fclose(file);
	}

	unsigned int lineVAO, lineVBO, lineDataSize;
	void genOrbitLines() {
		lineMat = new Material("line", "NULL");
		lineMat->matShader->use();
		lineMat->matShader->setUniformV3("lineColor", vec3f{ 0.8f,0.8f,0.8f });
		glGenVertexArrays(1, &lineVAO);
		glGenBuffers(1, &lineVBO);


		std::vector<double> lineData;
		
		for (int i = 0; i < propagationData.size(); ++i) {
			lineData.push_back(propagationData[i].position[0]);
			lineData.push_back(propagationData[i].position[1]);
			lineData.push_back(propagationData[i].position[2]);

		}

		lineDataSize = lineData.size() / 3;


		glBindVertexArray(lineVAO);
		glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
		glBufferData(GL_ARRAY_BUFFER, lineDataSize * 3 * sizeof(double), lineData.data(), GL_STATIC_DRAW);
		glVertexAttribLPointer(0, 3, GL_DOUBLE, sizeof(double) * 3, (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
	}
	void genOrbitLines(PlanetaryBody* parent) {
		parentBody = parent;
		lineMat = new Material("line", "NULL");
		lineMat->matShader->use();
		lineMat->matShader->setUniformV3("lineColor", vec3f{ 0.8f,0.8f,0.8f });
		glGenVertexArrays(1, &lineVAO);
		glGenBuffers(1, &lineVBO);


		std::vector<double> lineData;
		bodyState newState;
		bodyState newStateP;
		for (int i = 0; i < propagationData.size(); ++i) {
			newState = parentBody->interpEphIndex(propagationData[i].JDTDB);
			if (parentBody->parentBody != NULL) {
				newStateP = parentBody->parentBody->interpEphIndex(propagationData[i].JDTDB);
				newState.position += newStateP.position;
			}
			lineData.push_back(propagationData[i].position[0] - newState.position[0]);
			lineData.push_back(propagationData[i].position[1] - newState.position[1]);
			lineData.push_back(propagationData[i].position[2] - newState.position[2]);

		}

		lineDataSize = lineData.size() / 3;


		glBindVertexArray(lineVAO);
		glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
		glBufferData(GL_ARRAY_BUFFER, lineDataSize * 3 * sizeof(double), lineData.data(), GL_STATIC_DRAW);
		glVertexAttribLPointer(0, 3, GL_DOUBLE, sizeof(double) * 3, (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);

	}

	void updateCraftPosition(double julian) {

	}


	void draw() {
		bodyMat->draw();
		if (lineMat) {
			lineMat->matShader->use();
			mat4 modelMat;
			modelMat = (parentBody) ? TransMat(parentBody->getPosition()) : eye4();
			lineMat->matShader->setUniform4dv("model", &modelMat[0][0]);
			glBindVertexArray(lineVAO);
			glDrawArrays(GL_LINE_STRIP, 0, lineDataSize);
			glBindVertexArray(0);
		}
	}

	void setUniform4dv(const char * uniformName, double * uniformPtr) {
		if (bodyMat)
			bodyMat->matShader->setUniform4dv(uniformName, uniformPtr);
		if (lineMat)
			lineMat->matShader->setUniform4dv(uniformName, uniformPtr);
	}
	void setUniformV3(const char * uniformName, float * vec) {
		if (bodyMat)
			bodyMat->matShader->setUniformV3(uniformName, vec);
		if (lineMat)
			lineMat->matShader->setUniformV3(uniformName, vec);
	}
	void setUniformV3(const char * uniformName, double * vec) {
		if (bodyMat)
			bodyMat->matShader->setUniformV3(uniformName, vec);
		if (lineMat)
			lineMat->matShader->setUniformV3(uniformName, vec);
	}
	void setUniformV3(const char * uniformName, vec3 vec) {
		if (bodyMat)
			bodyMat->matShader->setUniformV3(uniformName, vec);
		if (lineMat)
			lineMat->matShader->setUniformV3(uniformName, vec);
	}
	void setUniformV3(const char * uniformName, vec3f vec) {
		if (bodyMat)
			bodyMat->matShader->setUniformV3(uniformName, vec);
		if (lineMat)
			lineMat->matShader->setUniformV3(uniformName, vec);
	}

};
