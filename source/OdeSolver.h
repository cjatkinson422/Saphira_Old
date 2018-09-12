#pragma once
#include <array>
#include "Transforms.h"

struct state {
	double JDTDB;
	vec3 position;
	vec3 velocity;
};

//there is some issue here as the errors are very large for the timesteps im using. Still trying to find the bug
inline void vec3_rkf45(vec6 initial, double initTime, double deltaTime, double tolerance, vec6(*fun)(double, vec6),vector<state>* stateOutput) {
	stateOutput->push_back(state{ initTime, vec3{ initial[0],initial[1],initial[2] }, vec3{ initial[3],initial[4],initial[5] } });
	double h = deltaTime / 100.0;
	double hmin = 1.0 / (24.0 * 60.0*60.0); // .01 second
	double hmax = 1.0 / (24.0 * 4.0); // 15 minutes
	double t = initTime;
	double err,s,err2;
	vec6 y = initial;
	vec6 y1;
	vec6 z1;
	vec6 k1, k2, k3, k4, k5, k6;

	double r1 = (1.0 / 36.0); double r3 = (-128.0 / 4275.0); double r4 = (-2197.0 / 75240.0); double r5 = (1.0 / 50.0); double r6 = (2.0 / 55.0);
	double outputStep = 0.0;
	double outputTime = 1.0 / (24.0*60.0*4.0);
	bool contFlag = false;
	bool warnFlag = false;
	while (t + h < initTime + deltaTime) {
		k1 = h * fun(t, y);
		k2 = h * fun(t + 0.25*h, y + 0.25*k1);
		k3 = h * fun(t + (3.0 / 8.0)*h, y + (3.0 / 32.0)*k1 + (9.0 / 32.0)*k2);
		k4 = h * fun(t + (12.0 / 13.0)*h, y + (1932.0 / 2197.0)*k1 - (7200.0 / 2197.0)*k2 + (7296.0 / 2197.0)*k3);
		k5 = h * fun(t + h, y + (439.0 / 216.0)*k1 - 8.0*k2 + (3680.0 / 513.0)*k3 - (845.0 / 4104.0)*k4);
		k6 = h * fun(t + 0.5*h, y - (8.0 / 27.0)*k1 + 2.0*k2 - (3544.0 / 2565.0)*k3 + (1859.0 / 4104.0)*k4 - (11.0 / 40.0)*k5);

		y1 = y + (25.0 / 216.0)*k1 + (1408.0 / 2565.0)*k3 + (2197.0 / 4101.0)*k4 - (1.0/5.0)*k5;
		z1 = y + (16.0 / 135.0)*k1 + (6656.0 / 12825.0)*k3 + (28561.0 / 56430.0)*k4 - (9.0 / 50.0)*k5 + (2.0 / 55.0)*k6;

		err2 = len(r1*k1+r3*k3+r4*k4+r5*k5+r6*k6);
		err = len(z1 - y1);
		s = sqrt(sqrt((tolerance*h)/(2*err)));

		if (err > tolerance) {
			if (h == hmin) {
				contFlag = true;
				if (!warnFlag) {
					cout << endl;
					cout << "Warning: error exceeding limits. " << endl;
					warnFlag = true;
				}
			}
			else if (h*s < hmin) {
				h = hmin;
				
				continue;

			}
			else {
				h = h * s;
				continue;
			}
		}
		if (err<=tolerance || contFlag) {
			contFlag = false;
			t += h;
			y = y1;
			outputStep += h;
			if (outputStep>outputTime) {
				stateOutput->push_back(state{ t, vec3{ y1[0],y1[1],y1[2] }, vec3{ y1[3],y1[4],y1[5] } });
				outputStep = 0;
			}
			
			if (h*s > hmax) {
				h = hmax;
			}
			else {
				//h = h * s;
			}
		}
		
	}
	h = (initTime + deltaTime) - t;

	k1 = h * fun(t, y);
	k2 = h * fun(t + 0.25*h, y + 0.25*k1);
	k3 = h * fun(t + (3.0 / 8.0)*h, y + (3.0 / 32.0)*k1 + (9.0 / 32.0)*k2);
	k4 = h * fun(t + (12.0 / 13.0)*h, y + (1932.0 / 2197.0)*k1 - (7200.0 / 2197.0)*k2 + (7296.0 / 2197.0)*k3);
	k5 = h * fun(t + h, y + (439.0 / 216.0)*k1 - 8.0*k2 + (3680.0 / 513.0)*k3 - (845.0 / 4104.0)*k4);
	k6 = h * fun(t + 0.5*h, y - (8.0 / 27.0)*k1 + 2.0*k2 - (3544.0 / 2565.0)*k3 + (1859.0 / 4104)*k4 - (11.0 / 40.0)*k5);

	y1 = y + (25.0 / 216.0)*k1 + (1408.0 / 2565.0)*k3 + (2197.0 / 4101.0)*k4 - (0.2)*k5;
	t = initTime + deltaTime;
	stateOutput->push_back(state{ t, vec3{ y1[0],y1[1],y1[2] }, vec3{ y1[3],y1[4],y1[5] } });
}
