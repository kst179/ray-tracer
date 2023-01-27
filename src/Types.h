#pragma once

#include "glm/glm.hpp"
#include "vector"
#include <iostream>

//const int PPU = 20; // texture pixels per unit
const float PI = 3.14159;
const float EPS = 1e-4;

const float DIFF_COEF = 0.6;

enum{diffuse, mirror, transperent};

void change_ppu(int new_ppu);

struct SRay
{
  glm::vec3 m_start;
  glm::vec3 m_dir;
	glm::vec3 m_intensity;
	SRay(glm::vec3 start, glm::vec3 dir, glm::vec3 intensity = glm::vec3(1, 1, 1));
};

struct SHit
{
	glm::vec3 m_pos;
	glm::vec3 m_normal;
	glm::vec3 m_color;
	std::pair<glm::vec3, float> *m_intensity;
	std::vector< std::vector< std::pair<glm::vec3, float> > > *photone_map;
	glm::uvec2 size;
	glm::uvec2 uv;
	bool has_hit;
	int type;
	float n;

	SHit();
	SRay reflect(SRay ray);
	SRay mirror_reflect(SRay ray);
	SRay refract(SRay ray);
	SRay diffuse_reflect(SRay ray);
};

struct SObject{
	int type = diffuse;
	float n = 1.0;
	virtual SHit check_hit(SRay ray) = 0;
};

struct SCamera
{
  glm::vec3 m_pos;          // Camera position and orientation
  glm::vec3 m_forward;      // Orthonormal basis
  glm::vec3 m_right;
  glm::vec3 m_up;

  glm::vec2 m_viewAngle;    // View angles, rad
  glm::uvec2 m_resolution;  // Image resolution: w, h

  std::vector<glm::vec3> m_pixels;  // Pixel array

	void set_angle(float angle, float x, float y);
};

struct SParabaloid : SObject
{
	glm::vec3 m_pos;
	glm::vec3 m_dir;
	glm::vec3 m_right;
	glm::vec3 m_up;
	float m_lim;

	SHit check_hit(SRay ray);
};

struct SSphere : public SObject
{
	glm::vec3 m_center;
	glm::vec3 m_color;
	float m_radius;
	int texsize;
	std::vector< std::vector< std::pair<glm::vec3, float> > > photone_map;

	SSphere(){}
	SSphere(glm::vec3 center, float radius, glm::vec3 color);
	SHit check_hit(SRay ray);
};

struct SLinse : SObject{
	SSphere m_s1;
	SSphere m_s2;

	SLinse(){}
	SLinse(glm::vec3 pos, glm::vec3 dir, float radius, float width, float n);
	SHit check_hit(SRay ray);
};

struct SLantern
{
	glm::vec3 m_pos;
	glm::vec3 m_dir;
	glm::vec3 m_up;
	glm::vec3 m_right;

	SParabaloid parabaloid;
	SLinse linse;
};

struct SMesh : public SObject
{
  std::vector<glm::vec3> m_vertices;  // vertex positions
  std::vector<glm::uvec3> m_triangles;  // vetrex indices
	std::vector<glm::vec3> m_colors; // triangle colors
	std::vector< std::vector< std::vector< std::pair<glm::vec3, float> > > > m_photone_maps;
	std::vector<int> texsize_u;
	std::vector<int> texsize_v;

	SMesh(){}
	SHit check_hit(SRay ray);
};

struct SCube : public SMesh
{
	SCube(){}
	SCube(glm::mat3 T, glm::vec3 shift, glm::vec3 color = glm::vec3(1, 1, 1), bool normout = true);
};

