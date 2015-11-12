//#include "glm.h"
#include "MyVector.h"

typedef Sketch2Scene::MyVector Vec;

#ifndef BASIS_H
#define BASIS_H

struct Basis
{
	Vec front;
	Vec up;
	Vec right;
};

#endif