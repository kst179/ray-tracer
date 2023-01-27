#pragma once

#include "Types.h"

class CScene
{
public:
	SMesh cornellbox;
	SSphere sphere;
	SSphere mirror_sphere;
	SCube cube;
	SLantern lantern;
	SCamera camera;
	SParabaloid parabaloid;
	SLinse linse;

	std::vector<SObject *> objects;

	CScene();
	SHit check_hit(SRay ray);
};