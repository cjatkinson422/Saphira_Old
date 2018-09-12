#include "Planet.h"

double SOLAR_MU;


PlanetaryBody::PlanetaryBody(bodyLoader bodyToLoad) {
	name = bodyToLoad.name;
	loadEphemeris(bodyToLoad.name);

	texHandler->loadTexture(bodyToLoad.name, bodyToLoad.specular, bodyToLoad.normal);

	bodyMesh = new SceneObject("uvsphere");
	bodyMat = new Material(bodyToLoad.shaderType, bodyToLoad.name);

	if (bodyToLoad.atmosphere) {
		atmoMesh = new SceneObject("icosphere");
		atmoMat = new Material("atmosphere", "NULL");
		atmoMat->matShader->setUniformV3("atmoColor", vec3{ 0.2,0.2,1.0 });
	}

	if (bodyToLoad.rings) {
		texHandler->loadTexture(bodyToLoad.name + "_rings");
		ringMesh = new SceneObject("rings");
		ringMat = new Material("ring", bodyToLoad.name + "_rings");
	}

	if (bodyMesh)
		bodyMat->addObj(this->bodyMesh);
	if (atmoMesh)
		atmoMat->addObj(this->atmoMesh);
	if (ringMesh)
		ringMat->addObj(this->ringMesh);


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
	if (ringMat) {
		ringMat->matShader->setUniformV3("sun.color", vec3f{ 1.0f,1.0f,0.95f });
		ringMat->matShader->setUniformV3("sun.position", vec3{ 0.0,0.0,0.0 });
		ringMat->matShader->setUniform1f("sun.attn_str", sunStr);
		ringMat->matShader->setUniform1f("sun.attn_lin", sunLin);
		ringMat->matShader->setUniform1f("sun.attn_quad", sunQuad);
	}
	if (atmoMat) {
		atmoMat->matShader->setUniformV3("sun.color", vec3f{ 1.0f,1.0f,0.95f });
		atmoMat->matShader->setUniformV3("sun.position", vec3{ 0.0,0.0,0.0 });
		atmoMat->matShader->setUniform1f("sun.attn_str", sunStr);
		atmoMat->matShader->setUniform1f("sun.attn_lin", sunLin);
		atmoMat->matShader->setUniform1f("sun.attn_quad", sunQuad);
	}


	// SCALE SETTING
	if (properties["radius"]) {
		if (bodyMesh) {
			bodyMesh->setScale(properties["radius"]);
			cout << "Setting " << bodyToLoad.name << " to scale of " << properties["radius"] << endl;
		}
		if (atmoMesh)
			atmoMesh->setScale(properties["radius"] * 1.01);
		if (ringMesh)
			ringMesh->setScale(properties["radius"]);
	}
	else if (properties["equ_radius"] && properties["pol_radius"]) {
		bodyMesh->setScale(vec3{ properties["equ_radius"],properties["pol_radius"],properties["equ_radius"] });
		if (atmoMesh)
			atmoMesh->setScale(1.01*vec3{ properties["equ_radius"],properties["pol_radius"],properties["equ_radius"] });
		if (ringMesh)
			ringMesh->setScale(properties["equ_radius"]);
		cout << "Setting " << bodyToLoad.name << " to scale of " << properties["pol_radius"] << " polar, and " << properties["equ_radius"] << " equatorial" << endl;;
	}
	else {
		cout << "ERROR: Failed to find scale data for " << bodyToLoad.name << endl;
	}

	// MASS SETTING

	if (properties["GM"]) {
		mu = properties["GM"];
	}
	if (bodyToLoad.name == "sol")
		SOLAR_MU = mu;

	if (properties["angularV"])
		angularV = properties["angularV"];
	//set coords

}

void PlanetaryBody::loadEphemeris(string body) {
	string path = "Ephemeris/Lander/" + body + ".epm";
	FILE* file;
	errno_t err = fopen_s(&file, path.c_str(), "r");


	if (err) {
		cout << "Could not open file: " << path << endl;
		return;
	}
	cout << "Beginning ephemeris read of " << path << endl;
	while (1) {
		char key[64];
		key[63] = 0;
		int res = fscanf_s(file, "%63s", key, 63);
		if (res == EOF)
			break;
		if (strcmp(key, "$$SOE") == 0) {
			int index = 0;
			while (res != EOF) {
				double JDTDB;
				int year;
				char month[] = "NLL";
				int day;
				int hour;
				int min;
				float second;
				vec3 position;
				vec3 velocity;
				//res = fscanf_s(file, "%lf, A.D. %d-%3c-%d %d:%d:%f, %lf, %lf, %lf, %lf, %lf, %lf,\n", &JDTDB, &year, month, &day, &hour, &min, &second, &position[0], &position[1], &position[2], &velocity[0], &velocity[1], &velocity[2]);
				res = fscanf_s(file, "%lf, A.D. %d-%3c-", &JDTDB, &year, month);
				res = fscanf_s(file, "%d %d:%d:%f, %lg, %lg, %lg, %lg, %lg, %lg,\n", &day, &hour, &min, &second, &position[2], &position[0], &position[1], &velocity[2], &velocity[0], &velocity[1]);
				bodyState state;
				state.JDTDB = JDTDB;
				state.position = position;
				state.velocity = velocity;
				this->EphemerisData.push_back(state);
			}
			break;
		}
		else {
			double value;
			int res = fscanf_s(file, "%lg", &value);
			properties[key] = value;
		}
	}
	fclose(file);
	cout << "Successfully loaded ephemeris data for " << body << ", " << EphemerisData.size() << " states found." << endl;
}

void PlanetaryBody::setPosition(vec3 pos) {
	this->position = pos;
	if (bodyMesh)
		this->bodyMesh->setPos(pos);
	if (ringMesh)
		this->ringMesh->setPos(pos);
	if (atmoMesh)
		atmoMesh->setPos(pos);
}

void PlanetaryBody::setVelocity(vec3 vel) {
	this->velocity = vel;
}

vec3 PlanetaryBody::getPosition() {
	return this->position;
}

vec3 PlanetaryBody::getVelocity() {
	return this->velocity;
}

void PlanetaryBody::interpToTime(double julian)
{
	bodyState newState = interpEphIndex(julian);
	this->setPosition(newState.position);
	this->setVelocity(newState.velocity);
}

bodyState PlanetaryBody::interpEphIndex(double julian)
{
	if (julian >= EphemerisData[curEphIndex].JDTDB && julian < EphemerisData[curEphIndex + 1].JDTDB) {
		return interpbodyState(EphemerisData[curEphIndex], EphemerisData[curEphIndex + 1], julian);
	}
	else if (julian >= EphemerisData[curEphIndex + 1].JDTDB && julian < EphemerisData[curEphIndex + 2].JDTDB) {
		curEphIndex += 1;
		return interpbodyState(EphemerisData[curEphIndex], EphemerisData[curEphIndex + 1], julian);
	}
	else{
		updateCurEphIndex(julian);
		interpEphIndex(julian);
	}
}

void PlanetaryBody::updateCurEphIndex(double julian)
{
	bool overflow = true;
	unsigned int ephSize = EphemerisData.size()-1;
	for (unsigned int i = 0; i < ephSize; ++i) {
		if (julian >= EphemerisData[i].JDTDB && julian < EphemerisData[i + 1].JDTDB) {
			curEphIndex = i;
			overflow = false;
			break;
		}
	}
	if (overflow) {
		cout << "REACHED END OF EPHEMERIS DATA FOR " << name << ". Julian time: "<< julian <<" Breaking out." << endl;
		exit(0);
	}
}
// ONLY CALL FROM INTERPEPHINDEX FUNCTION. OTHERWISE WILL MOST LIKELY GIVE NONSENSE OR AN ERROR
bodyState PlanetaryBody::interpbodyState(bodyState ps1, bodyState ps2, double julian)
{
	bodyState returnState;
	returnState.JDTDB = julian;
	double timeFrac = (julian - ps1.JDTDB) / (ps2.JDTDB - ps1.JDTDB);
	double slopeFac = 60 * 60 * 24 * abs(ps2.JDTDB-ps1.JDTDB);
	double posx = cubicInterpolation(ps1.position[0], ps2.position[0], slopeFac*ps1.velocity[0], slopeFac*ps2.velocity[0], timeFrac);
	double posy = cubicInterpolation(ps1.position[1], ps2.position[1], slopeFac*ps1.velocity[1], slopeFac*ps2.velocity[1], timeFrac);
	double posz = cubicInterpolation(ps1.position[2], ps2.position[2], slopeFac*ps1.velocity[2], slopeFac*ps2.velocity[2], timeFrac);
	returnState.velocity = timeFrac*(ps2.velocity-ps1.velocity)+ps1.velocity;
	returnState.position = vec3{ posx, posy, posz };
	return returnState;
}

Moon::Moon(bodyLoader moonToLoad, Planet * parent) :PlanetaryBody(moonToLoad) {
	parentBody = parent;
	genOrbitLines();
	cout << "Setting " << name << " to local coords: " << EphemerisData[0].position[0] << " " << EphemerisData[0].position[1] << " " << EphemerisData[0].position[2] << endl;
	setLocalPosition(EphemerisData[0].position);
	cout << "Setting " << name << " to local velocity: " << EphemerisData[0].velocity[0] << " " << EphemerisData[0].velocity[1] << " " << EphemerisData[0].velocity[2] << endl;
	setLocalVelocity(EphemerisData[0].velocity);
}

vec3 Moon::getLocalPosition() {
	return this->position - parentBody->position;
}

void Moon::setLocalPosition(vec3 pos) {
	pos += parentBody->position;
	this->position = pos;
	if (bodyMesh)
		this->bodyMesh->setPos(position);
	if (ringMesh)
		this->ringMesh->setPos(position);
	if (atmoMesh)
		atmoMesh->setPos(position);
}

vec3 Moon::getLocalVelocity()
{
	return this->velocity - parentBody->velocity;
}

void Moon::setLocalVelocity(vec3 vel)
{
	this->velocity = parentBody->velocity + vel;
}

void Moon::genOrbitLines() {
	lineMat = new Material("line", "NULL");
	lineMat->matShader->setUniformV3("lineColor", vec3f{ 0.0,0.0,1.0 });
	lineMat->matShader->use();
	glGenVertexArrays(1, &lineVAO);
	glGenBuffers(1, &lineVBO);


	unsigned int numPts = 360 * 4;
	std::vector<double> lineData;
	vec6 oe = rv2oe(EphemerisData[curEphIndex].position, EphemerisData[curEphIndex].velocity, parentBody->mu);
	for (double nu = 0; nu <= 360.0; nu += (360.0 / numPts)) {
		oe[5] = radians(nu);
		std::array<vec3, 2> rv = oe2rv(oe, parentBody->mu);
		lineData.push_back(rv[0][0]);
		lineData.push_back(rv[0][1]);
		lineData.push_back(rv[0][2]);

	}


	lineDataSize = lineData.size() / 3;



	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, lineDataSize * 3 * sizeof(double), lineData.data(), GL_STATIC_DRAW);
	glVertexAttribLPointer(0, 3, GL_DOUBLE, sizeof(double) * 3, (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

}

void Moon::updateOrbitLines()
{
	unsigned int numPts = 360;
	std::vector<double> lineData;
	vec6 oe = rv2oe(getLocalPosition(), getLocalVelocity(), parentBody->mu);
	for (double nu = 0; nu <= 360.0; nu += (360.0 / numPts)) {
		oe[5] = radians(nu);
		std::array<vec3, 2> rv = oe2rv(oe, parentBody->mu);
		lineData.push_back(rv[0][0]);
		lineData.push_back(rv[0][1]);
		lineData.push_back(rv[0][2]);

	}

	lineDataSize = lineData.size() / 3;



	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, lineDataSize * 3 * sizeof(double), lineData.data(), GL_STATIC_DRAW);
	glVertexAttribLPointer(0, 3, GL_DOUBLE, sizeof(double) * 3, (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void Moon::drawLines() {
	if (lineMat) {
		lineMat->matShader->use();
		mat4 modelMat = TransMat(parentBody->position);
		lineMat->matShader->setUniform4dv("model", &modelMat[0][0]);
		glBindVertexArray(lineVAO);
		glDrawArrays(GL_LINE_STRIP, 0, lineDataSize);
		glBindVertexArray(0);
	}
	else {
		cout << "ERROR: no line material found. " << endl;
	}

}

void Moon::interpToTime(double julian)
{
	bodyState newState = interpEphIndex(julian);
	this->setLocalPosition(newState.position);
	this->setLocalVelocity(newState.velocity);
}

void Moon::draw() {
	if (bodyMat) 
		bodyMat->draw();
	if (ringMat)
		ringMat->draw();
	if (atmoMat)
		atmoMat->draw();
	if (lineMat)
		drawLines();
}

Planet::Planet(planetLoader planetToLoad) :PlanetaryBody(planetToLoad) {
	cout << "Setting " << name << " to global coords: " << EphemerisData[0].position[0] << " " << EphemerisData[0].position[1] << " " << EphemerisData[0].position[2] << endl;
	setPosition(EphemerisData[0].position);
	cout << "Setting " << name << " to global velocity: " << EphemerisData[0].velocity[0] << " " << EphemerisData[0].velocity[1] << " " << EphemerisData[0].velocity[2] << endl;
	setVelocity(EphemerisData[0].velocity);
	if (planetToLoad.name != "sol")
		this->genOrbitLines();
	for (int i = 0; i < planetToLoad.nrMoons; ++i) {
		cout << endl;
		Moon* m = new Moon(*planetToLoad.moonLoaders[i], this);
		m->parentBody = this;
		moons.push_back(m);
		++nrMoons;
	}
	
}

void Planet::genOrbitLines() {
	lineMat = new Material("line", "NULL");
	lineMat->matShader->use();
	lineMat->matShader->setUniformV3("lineColor", vec3f{ 0.0f,0.8f,0.8f });
	glGenVertexArrays(1, &lineVAO);
	glGenBuffers(1, &lineVBO);


	unsigned int numPts = 360 * 4;
	std::vector<double> lineData;
	vec6 oe = rv2oe(EphemerisData[curEphIndex].position, EphemerisData[curEphIndex].velocity, SOLAR_MU);
	for (double nu = 0; nu <= 360.0; nu += (360.0 / numPts)) {
		oe[5] = radians(nu);
		std::array<vec3, 2> rv = oe2rv(oe, SOLAR_MU);
		lineData.push_back(rv[0][0]);
		lineData.push_back(rv[0][1]);
		lineData.push_back(rv[0][2]);

	}

	lineDataSize = lineData.size() / 3;
	

	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, lineDataSize * 3 * sizeof(double), lineData.data(), GL_STATIC_DRAW);
	glVertexAttribLPointer(0, 3, GL_DOUBLE, sizeof(double) * 3, (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void Planet::updateOrbitLines()
{
	unsigned int numPts = 360*4 ;
	std::vector<double> lineData;
	vec6 oe = rv2oe(position,velocity, SOLAR_MU);
	for (double nu = 0; nu <= 360.0; nu += (360.0 / numPts)) {
		oe[5] = radians(nu);
		std::array<vec3, 2> rv = oe2rv(oe, SOLAR_MU);
		lineData.push_back(rv[0][0]);
		lineData.push_back(rv[0][1]);
		lineData.push_back(rv[0][2]);

	}

	lineDataSize = lineData.size() / 3;


	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, lineDataSize * 3 * sizeof(double), lineData.data(), GL_STATIC_DRAW);
	glVertexAttribLPointer(0, 3, GL_DOUBLE, sizeof(double) * 3, (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void Planet::drawLines() {
	if (lineMat) {
		lineMat->matShader->use();
		mat4 modelMat = eye4();
		lineMat->matShader->setUniform4dv("model", &modelMat[0][0]);
		glBindVertexArray(lineVAO);
		glDrawArrays(GL_LINE_STRIP, 0, lineDataSize);
		glBindVertexArray(0);
	}
}

void Planet::setUniform4dv(const char * uniformName, double * uniformPtr) {
	if (bodyMat)
		bodyMat->matShader->setUniform4dv(uniformName, uniformPtr);
	if (atmoMat)
		atmoMat->matShader->setUniform4dv(uniformName, uniformPtr);
	if (ringMat)
		ringMat->matShader->setUniform4dv(uniformName, uniformPtr);
	if (lineMat)
		lineMat->matShader->setUniform4dv(uniformName, uniformPtr);

	for (int i = 0; i < nrMoons; ++i) {
		if (moons[i]->bodyMat)
			moons[i]->bodyMat->matShader->setUniform4dv(uniformName, uniformPtr);
		if (moons[i]->lineMat)
			moons[i]->lineMat->matShader->setUniform4dv(uniformName, uniformPtr);
		if (moons[i]->atmoMat)
			moons[i]->atmoMat->matShader->setUniform4dv(uniformName, uniformPtr);
		if (moons[i]->ringMat)
			moons[i]->ringMat->matShader->setUniform4dv(uniformName, uniformPtr);
	}

}

void Planet::setUniformV3(const char * uniformName, float * vec) {
	if (bodyMat)
		bodyMat->matShader->setUniformV3(uniformName, vec);
	if (atmoMat)
		atmoMat->matShader->setUniformV3(uniformName, vec);
	if (ringMat)
		ringMat->matShader->setUniformV3(uniformName, vec);
	for (int i = 0; i < nrMoons; ++i) {
		if (moons[i]->bodyMat)
			moons[i]->bodyMat->matShader->setUniformV3(uniformName, vec);
		if (moons[i]->lineMat)
			moons[i]->lineMat->matShader->setUniformV3(uniformName, vec);
		if (moons[i]->atmoMat)
			moons[i]->atmoMat->matShader->setUniformV3(uniformName, vec);
		if (moons[i]->ringMat)
			moons[i]->ringMat->matShader->setUniformV3(uniformName, vec);
	}
}

void Planet::setUniformV3(const char * uniformName, double * vec) {
	if (bodyMat)
		bodyMat->matShader->setUniformV3(uniformName, vec);
	if (atmoMat)
		atmoMat->matShader->setUniformV3(uniformName, vec);
	if (ringMat)
		ringMat->matShader->setUniformV3(uniformName, vec);
	for (int i = 0; i < nrMoons; ++i) {
		if (moons[i]->bodyMat)
			moons[i]->bodyMat->matShader->setUniformV3(uniformName, vec);
		if (moons[i]->lineMat)
			moons[i]->lineMat->matShader->setUniformV3(uniformName, vec);
		if (moons[i]->atmoMat)
			moons[i]->atmoMat->matShader->setUniformV3(uniformName, vec);
		if (moons[i]->ringMat)
			moons[i]->ringMat->matShader->setUniformV3(uniformName, vec);
	}
}

void Planet::setUniformV3(const char * uniformName, vec3f vec) {
	if (bodyMat)
		bodyMat->matShader->setUniformV3(uniformName, vec);
	if (atmoMat)
		atmoMat->matShader->setUniformV3(uniformName, vec);
	if (ringMat)
		ringMat->matShader->setUniformV3(uniformName, vec);
	for (int i = 0; i < nrMoons; ++i) {
		if (moons[i]->bodyMat)
			moons[i]->bodyMat->matShader->setUniformV3(uniformName, vec);
		if (moons[i]->lineMat)
			moons[i]->lineMat->matShader->setUniformV3(uniformName, vec);
		if (moons[i]->atmoMat)
			moons[i]->atmoMat->matShader->setUniformV3(uniformName, vec);
		if (moons[i]->ringMat)
			moons[i]->ringMat->matShader->setUniformV3(uniformName, vec);
	}
}

void Planet::setUniformV3(const char * uniformName, vec3 vec) {
	if (bodyMat)
		bodyMat->matShader->setUniformV3(uniformName, vec);
	if (atmoMat)
		atmoMat->matShader->setUniformV3(uniformName, vec);
	if (ringMat)
		ringMat->matShader->setUniformV3(uniformName, vec);
	for (int i = 0; i < nrMoons; ++i) {
		if (moons[i]->bodyMat)
			moons[i]->bodyMat->matShader->setUniformV3(uniformName, vec);
		if (moons[i]->lineMat)
			moons[i]->lineMat->matShader->setUniformV3(uniformName, vec);
		if (moons[i]->atmoMat)
			moons[i]->atmoMat->matShader->setUniformV3(uniformName, vec);
		if (moons[i]->ringMat)
			moons[i]->ringMat->matShader->setUniformV3(uniformName, vec);
	}
}

void Planet::draw() {
	if (bodyMat)
		bodyMat->draw();
	if (ringMat)
		ringMat->draw();
	if (atmoMat)
		atmoMat->draw();
	if (lineMat)
		drawLines();

	for (int i = 0; i < nrMoons; ++i) {
		moons[i]->draw();
	}
}

PlanetarySystem::PlanetarySystem() {

	texHandler->loadTexture("skybox");
	skybox = new Skybox("uvsphere");
	skybox->setScale(10000.0);
	skyboxMat = new Material("light", "skybox");
	skyboxMat->addObj(skybox);

}

void PlanetarySystem::loadPlanet(planetLoader loader) {

	Planet* p = new Planet(loader);

	this->Planets.push_back(p);
	++nrPlanets;

}

void PlanetarySystem::draw() {

	if (skyboxMat) {
		skyboxMat->draw(true);
	}

	for (int i = 0; i < nrPlanets; ++i) {
		Planets[i]->draw();
	}
}

void PlanetarySystem::setUniform4dv(const char * uniformName, double * uniformPtr) {
	if (skyboxMat)
		skyboxMat->matShader->setUniform4dv(uniformName, uniformPtr);
	for (int i = 0; i < nrPlanets; ++i) {
		Planets[i]->setUniform4dv(uniformName, uniformPtr);
	}

}

void PlanetarySystem::setUniformV3(const char * uniformName, float * vec) {
	if (skyboxMat)
		skyboxMat->matShader->setUniformV3(uniformName, vec);
	for (int i = 0; i < nrPlanets; ++i) {
		Planets[i]->setUniformV3(uniformName, vec);
	}
}

void PlanetarySystem::setUniformV3(const char * uniformName, double * vec) {
	if (skyboxMat)
		skyboxMat->matShader->setUniformV3(uniformName, vec);
	for (int i = 0; i < nrPlanets; ++i) {
		Planets[i]->setUniformV3(uniformName, vec);
	}
}

void PlanetarySystem::setUniformV3(const char * uniformName, vec3f vec) {
	if (skyboxMat)
		skyboxMat->matShader->setUniformV3(uniformName, vec);
	for (int i = 0; i < nrPlanets; ++i) {
		Planets[i]->setUniformV3(uniformName, vec);
	}
}

void PlanetarySystem::setUniformV3(const char * uniformName, vec3 vec) {
	if (skyboxMat)
		skyboxMat->matShader->setUniformV3(uniformName, vec);
	for (int i = 0; i < nrPlanets; ++i) {
		Planets[i]->setUniformV3(uniformName, vec);
	}
}

double PlanetarySystem::getClosestPlanetDistance(vec3 position) {
	double dist = INFINITY;
	for (int i = 0; i < nrPlanets; ++i) {
		if (len(position - Planets[i]->position) < dist) {
			dist = len(position - Planets[i]->position);
		}
		for (int j = 0; j < Planets[i]->nrMoons; ++j) {
			if (len(position - Planets[i]->moons[j]->getPosition()) < dist) {
				dist = len(position - Planets[i]->moons[j]->getPosition());
			}
		}
	}
	return dist;
}

void PlanetarySystem::updateEphIndices(double julian)
{
	for (int i = 0; i < nrPlanets; ++i) {
		Planets[i]->updateCurEphIndex(julian);
		for (int j = 0; j < Planets[i]->nrMoons; ++j) {
			Planets[i]->moons[j]->updateCurEphIndex(julian);
		}
	}
}

void PlanetarySystem::updatePlanetPositions(double julian)
{
	for (int i = 0; i < nrPlanets; ++i) {
		if (Planets[i]->name == "sol")
			continue;
			Planets[i]->interpToTime(julian);
			for (int j = 0; j < Planets[i]->nrMoons; ++j) {
				Planets[i]->moons[j]->interpToTime(julian);

			}
	}
}

void PlanetarySystem::updateOrbitLines()
{
	for (int i = 0; i < nrPlanets; ++i) {
		Planets[i]->updateOrbitLines();
		for (int j = 0; j < Planets[i]->nrMoons; ++j) {
			Planets[i]->moons[j]->updateOrbitLines();
		}
	}
}


