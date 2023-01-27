#include "Scene.h"
#include <cmath>
#include "glm/gtx/matrix_operation.hpp"

SLantern make_lantern(){
	float s = 0.1;
	SLantern lantern;
	lantern.m_pos = glm::vec3(6.2, -0.5, 0.5);
	lantern.m_dir = glm::normalize(glm::vec3(2.5, 0.3, -0.3) - lantern.m_pos);
	lantern.m_up = glm::normalize(glm::cross(glm::vec3(0, 1, 0), lantern.m_dir)) * (float)0.035;
	lantern.m_right = glm::normalize(glm::cross(lantern.m_dir, glm::vec3(0, 0, 1))) * (float)0.035;

	lantern.parabaloid.m_pos = lantern.m_pos - lantern.m_dir*(float)0.0125;
	lantern.parabaloid.m_dir = glm::normalize(lantern.m_dir)*(float)0.05;
	lantern.parabaloid.m_up = glm::normalize(lantern.m_up)*(float)0.05;
	lantern.parabaloid.m_right = glm::normalize(lantern.m_right)*(float)0.05;
	lantern.parabaloid.m_lim = 5;
	lantern.parabaloid.type = mirror;

	lantern.linse = SLinse(lantern.m_pos + 0.35f*lantern.m_dir, lantern.m_dir, 0.25, 0.1, 1.7);

	return lantern;
}

SCamera make_camera(){
	SCamera camera;
	camera.m_pos = glm::vec3(7, 0, 0.2);

	return camera;
}

CScene::CScene(){
	cornellbox = SCube(glm::diagonal3x3(glm::vec3(10.0, 2.0, 2.0)), glm::vec3(0, -1, -1), glm::vec3(1, 1, 1), false);
	cornellbox.m_colors[8] = glm::vec3(1, 0, 0);
	cornellbox.m_colors[9] = glm::vec3(1, 0, 0);
	cornellbox.m_colors[10] = glm::vec3(0, 1, 0);
	cornellbox.m_colors[11] = glm::vec3(0, 1, 0);

	lantern = make_lantern();
	sphere = SSphere(glm::vec3(3, -0.2, -0.5), 0.5, glm::vec3(1, 1, 1));
	mirror_sphere = SSphere(glm::vec3(1, 0.2, 0.4), 0.4, glm::vec3(1, 1, 1));
	mirror_sphere.type = mirror;

	linse = SLinse(glm::vec3(3.8, 0.55, -0.2), glm::normalize(glm::vec3(1,0.5,0)), 0.3, 0.15, 1.4);

	parabaloid.m_pos = glm::vec3(1, 0, 0.5);
	parabaloid.m_dir = glm::vec3(0.5, 0, 0);
	parabaloid.m_up = glm::vec3(0, 0, 0.5);
	parabaloid.m_right = glm::vec3(0, 0.5, 0);
	parabaloid.m_lim = 0.5;

	cube = SCube(glm::diagonal3x3(glm::vec3(1.5, 0.5, 1.0)), glm::vec3(1, 0.33, -1), glm::vec3(1, 1, 0), true);

	objects = { &cornellbox, &sphere, &cube, &linse, &mirror_sphere };

	camera = make_camera();
}

SHit CScene::check_hit(SRay ray){
	float min_dist = 1e10;
	SHit hit;
	for (int i = 0; i < objects.size(); ++i){
		SHit h = objects[i]->check_hit(ray);
		if (h.has_hit && glm::distance(h.m_pos, ray.m_start) < min_dist){
			hit = h;
			min_dist = glm::distance(h.m_pos, ray.m_start);
		}
	}
	return hit;
}