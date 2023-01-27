#pragma once

#include "glm/glm.hpp"
#include "Types.h"
#include "Scene.h"

#include "string"
#include "atlimage.h"

class CTracer
{
public:
  SRay MakeCameraRay(glm::uvec2 pixelPos);  // Create ray for specified pixel
	SRay MakeLanternRay();

	glm::vec3 TraceRayBackward(SRay ray); // Trace ray, compute its color
	void TraceRayForward(SRay ray); // Trace ray, return position and color
	glm::vec3 Collect(std::vector< std::vector< std::pair<glm::vec3, float> > > *, glm::uvec2, glm::uvec2);

	void RenderImage(int xRes, int yRes, int max_photones, int ppu, bool use_collections, int min_photones, int max_radius);
  void SaveImageToFile(std::string fileName);
  CImage* LoadImageFromFile(std::string fileName);

public:
	SLantern m_lantern;
  SCamera m_camera;
  CScene* m_pScene;
};