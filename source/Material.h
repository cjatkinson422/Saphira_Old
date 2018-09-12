#pragma once
#include <vector>
#include "TexHandler.h"
#include "Shader.h"
#include "SceneObject.h"
#include "Globals.h"

using std::vector;

class Material {
private:	
	Texture* textures[16];

public:
	short numTextures = 0;
	int numObjs = 0;
	string materialName;
	vector<SceneObject*> objVec;

	float shininess = 64;
	float specStr = 1.0;
	Shader* matShader;

	Material(string shaderType, string name) {
		materialName = name;
		matShader = new Shader();
		matShader->loadShader(shaderType);

		if (name != "NULL") {
			addTexture(texHandler->textureMap[name]);
			setTexUniforms();
		}
	}

	void addObj(SceneObject* argB) {
		objVec.push_back(argB);
		++numObjs;
	}
	void addObj(Skybox* argB) {
		objVec.push_back(argB);
		++numObjs;
	}
	void removeObj(SceneObject* argB) {
		auto it = std::find(objVec.begin(), objVec.end(), argB);
		if (it != objVec.end()) {
			objVec.erase(it);
		}
	}
	
	void draw(bool cast_to_skybox = false) {
		matShader->use();
		
		if (numTextures > 0) {
			if (textures[0]->diffID) {
				glActiveTexture(GL_TEXTURE0 + 2);
				glBindTexture(GL_TEXTURE_2D, textures[0]->diffID);
			}
			if (textures[0]->specID) {
				glActiveTexture(GL_TEXTURE0 + 4);
				glBindTexture(GL_TEXTURE_2D, textures[0]->specID);
			}
			if (textures[0]->normID) {
				glActiveTexture(GL_TEXTURE0 + 6);
				glBindTexture(GL_TEXTURE_2D, textures[0]->normID);
			}
		}
		setTexUniforms();
		for (int i = 0; i < numObjs; ++i) {
			if (!cast_to_skybox) {
				objVec[i]->draw(matShader);
			}
			else {
				(dynamic_cast<Skybox*>(objVec[i]))->draw(matShader);
				
			}
		}
	}

	void addTexture(int index) {
		if (this->numTextures < 16) {
			this->textures[numTextures] = texHandler->getTexturePtr(index);
			numTextures++;
		}
		else {
			cout << "Attempted to bind too many textures to material: " << this->materialName << ". Skipping." << endl;
			return;
		}

	}

	void setTexUniforms() {
		if (numTextures == 0) { return; }
		for (int i = 0; i < numTextures; ++i) {
			string diffuseName = "tex_" + std::to_string(i) + "_diffuse";
			matShader->setUniform1i(diffuseName, 2);
			if (textures[0]->specID) {
				string specName = "tex_" + std::to_string(i) + "_spec";
				matShader->setUniform1i(specName, 4);
			}
			if (textures[0]->normID) {
				string normName = "tex_" + std::to_string(i) + "_norm";
				matShader->setUniform1i(normName, 6);
			}
		}

	}
};