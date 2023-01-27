#include "Types.h"

int PPU = 20;

void change_ppu(int new_ppu){
	PPU = new_ppu;
}

// Ray
SRay::SRay(glm::vec3 start, glm::vec3 dir, glm::vec3 intensity){
	m_start = start;
	m_dir = dir;
	m_intensity = intensity;
}


// Hit
SHit::SHit(){
	m_intensity = nullptr;
	has_hit = false;
}

SRay 
SHit::reflect(SRay ray){
	if (type == diffuse){
		return diffuse_reflect(ray);
	}

	if (type == mirror){
		return mirror_reflect(ray);
	}

	if (type == transperent){
		return refract(ray);
	}
}

SRay 
SHit::mirror_reflect(SRay ray){
	glm::vec3 u = glm::cross(m_normal, ray.m_dir);
	if (glm::length(u) < EPS){
		return SRay(m_pos + EPS*m_normal, m_normal);
	}
	glm::vec3 t = glm::cross(glm::normalize(u), m_normal);
	glm::vec3 new_dir = glm::normalize(glm::dot(ray.m_dir, t)*t - glm::dot(ray.m_dir, m_normal)*m_normal);
	return SRay(m_pos + EPS*m_normal, new_dir, ray.m_intensity);
}

SRay 
SHit::refract(SRay ray){
	glm::vec3 u = glm::cross(m_normal, ray.m_dir);
	if (glm::length(u) < EPS){
		return SRay(m_pos + EPS*m_normal*glm::dot(m_normal, ray.m_dir), m_normal*glm::dot(m_normal, ray.m_dir));
	}
	glm::vec3 t = glm::cross(glm::normalize(u), m_normal);
	glm::vec3 new_dir = 1.0f / n *glm::dot(ray.m_dir, t)*t + glm::dot(ray.m_dir, m_normal)*m_normal;
	return SRay(m_pos + EPS*new_dir, new_dir, ray.m_intensity);
}

SRay 
SHit::diffuse_reflect(SRay ray){
	glm::vec3 dir;
	do{
		dir = glm::vec3(
			(float)rand() / RAND_MAX - 0.5,
			(float)rand() / RAND_MAX - 0.5,
			(float)rand() / RAND_MAX - 0.5
			);
	} while (glm::length(dir) < EPS || abs(glm::dot(dir, m_normal)) < EPS);
	dir = glm::normalize(dir*glm::dot(dir, m_normal));
	return SRay(m_pos + EPS*dir, dir, DIFF_COEF * m_color * ray.m_intensity);
}


// Camera
void 
SCamera::set_angle(float angle, float x, float y){
	m_forward = glm::vec3(-1, 0, 0);

	m_up = glm::vec3(0, 0, 1) * sin(angle * PI / 360);
	m_right = glm::vec3(0, 1, 0) * x / y * sin(angle * PI / 360);
}


// Parabaloid
SHit 
SParabaloid::check_hit(SRay ray){
	SHit hit;
	static glm::mat3 T(m_up, m_right, m_dir);
	static glm::mat3 iT = glm::inverse(T);
	glm::vec3 p = iT * (ray.m_start - m_pos);
	glm::vec3 d = iT * ray.m_dir;
	float a = d.x*d.x + d.y*d.y;
	float b = p.x*d.x + p.y*d.y - 0.5*d.z;
	float c = p.x*p.x + p.y*p.y - p.z;
	float det = b*b - a*c;
	if (det < 0){
		return hit;
	}
	float t = (-b - sqrt(det)) / a;
	if (t < 0){
		t = (-b + sqrt(det)) / a;
	}
	if (t < 0){
		return hit;
	}
	p += t*d;
	if (p.z > m_lim){
		return hit;
	}
	hit.m_color = glm::vec3(0, 0, 1);
	hit.has_hit = true;
	hit.m_pos = T*p + m_pos;
	glm::vec3 norm;
	if (glm::length(p) < EPS){
		norm = glm::vec3(0, 0, 1);
	}
	else{
		glm::vec3 tg = glm::normalize(p - glm::vec3(p.x, p.y, 0) / (float)2.0);
		norm = glm::normalize(glm::cross(glm::cross(tg, p), tg));
	}
	hit.m_normal = -T*norm;

	hit.type = type;

	return hit;
}


// Sphere
SSphere::SSphere(glm::vec3 center, float radius, glm::vec3 color)
{
	m_center = center;
	m_radius = radius;
	m_color = color;
	texsize = 2 * radius*PPU;
	photone_map.resize(2 * texsize + 1);
	for (int i = 0; i < 2 * texsize + 1; ++i){
		photone_map[i].resize(texsize + 1);
	}
}

SHit 
SSphere::check_hit(SRay ray){
	SHit hit;
	glm::vec3 p = ray.m_start - m_center;
	float pd = glm::dot(p, ray.m_dir);
	float p2 = glm::dot(p, p);
	float d2 = glm::dot(ray.m_dir, ray.m_dir);
	float det = pd*pd - (p2 - m_radius*m_radius)*d2;
	hit.has_hit = (det > EPS);
	if (!hit.has_hit)
		return hit;
	float t = (-pd - sqrt(det)) / d2;
	if (t < EPS){
		t = (-pd + sqrt(det)) / d2;
	}
	if (t < EPS){
		hit.has_hit = false;
		return hit;
	}
	hit.m_pos = ray.m_start + t*ray.m_dir;

	hit.m_normal = glm::normalize(hit.m_pos - m_center);
	hit.m_color = m_color;

	glm::vec3 dh = hit.m_pos - m_center;
	float u = atan2(dh.y, dh.x) / (2 * PI) + 0.5;
	float v = atan2(dh.z, hypot(dh.x, dh.y)) / PI + 0.5;
	if (u > 1.0) u = 1.0;
	if (v > 1.0) v = 1.0;
	if (u < 0.0) u = 0.0;
	if (v < 0.0) v = 0.0;

	hit.m_intensity = &photone_map[int(u * 2 * texsize)][int(v*texsize)];

	hit.type = type;

	if (glm::distance(ray.m_start, m_center) < m_radius){
		hit.n = 1 / n;
	}
	else{
		hit.n = n;
	}

	hit.photone_map = &photone_map;
	hit.size = glm::uvec2(2 * texsize + 1, texsize + 1);
	hit.uv = glm::vec2(int(u * 2 * texsize), int(v*texsize));

	return hit;
}


// Linse
SLinse::SLinse(glm::vec3 pos, glm::vec3 dir, float radius, float width, float n){
	m_s1 = SSphere(pos + (radius - width / 2)*dir, radius, glm::vec3(1, 1, 1));
	m_s2 = SSphere(pos - (radius - width / 2)*dir, radius, glm::vec3(1, 1, 1));
	m_s1.type = transperent;
	m_s1.n = n;
	m_s2.type = transperent;
	m_s2.n = n;
}

SHit 
SLinse::check_hit(SRay ray){
	SHit hit1 = m_s1.check_hit(ray);
	SHit hit2 = m_s2.check_hit(ray);
	if (!hit1.has_hit || !hit2.has_hit){
		return SHit();
	}

	if (glm::distance(m_s1.m_center, hit2.m_pos) > m_s1.m_radius + EPS &&
		glm::distance(m_s2.m_center, hit1.m_pos) > m_s2.m_radius + EPS){
		return SHit();
	}

	if (glm::distance(m_s1.m_center, hit2.m_pos) > m_s1.m_radius + EPS){
		return hit1;
	}
	if (glm::distance(m_s2.m_center, hit1.m_pos) > m_s2.m_radius + EPS){
		return hit2;
	}
	if (glm::distance(ray.m_start, hit1.m_pos) < glm::distance(ray.m_start, hit2.m_pos)){
		return hit1;
	}
	else{
		return hit2;
	}
}


// Mesh
SHit 
SMesh::check_hit(SRay ray){
	SHit hit;
	hit.m_intensity = nullptr;
	hit.has_hit = false;
	for (int i = 0; i < m_triangles.size(); ++i){
		glm::uvec3 tri = m_triangles[i];
		glm::vec3 a = m_vertices[tri.x];
		glm::vec3 b = m_vertices[tri.y];
		glm::vec3 c = m_vertices[tri.z];
		glm::vec3 n = glm::cross(a - c, b - c);
		float t = -glm::dot(ray.m_start - c, n) / glm::dot(ray.m_dir, n);
		glm::vec3 h = ray.m_start + t * ray.m_dir;

		if (t>0 && glm::length(glm::cross(a - c, b - c)) + 1e-5 >
			glm::length(glm::cross(a - c, h - c)) +
			glm::length(glm::cross(b - c, h - c)) +
			glm::length(glm::cross(b - a, h - a)))
		{
			float u;
			float v;

			if (glm::length(a - c) < EPS)
				u = 0;
			else
				u = glm::dot(glm::normalize(a - c), h - c) / glm::length(a - c);

			if (glm::length(b - c) < EPS)
				v = 0;
			else
				v = glm::dot(glm::normalize(b - c), h - c) / glm::length(b - c);

			if (i % 2 == 1){
				u = 1 - u;
				v = 1 - v;
			}

			if (hit.has_hit && glm::distance(ray.m_start, h) < glm::distance(ray.m_start, hit.m_pos)){
				hit.m_pos = h;
				hit.m_color = m_colors[i];
				hit.m_normal = glm::normalize(n);
				hit.m_intensity = &m_photone_maps[i / 2][int(u*texsize_u[i / 2])][int(v*texsize_v[i / 2])];

				hit.photone_map = &m_photone_maps[i / 2];
				hit.size = glm::uvec2(texsize_u[i / 2], texsize_v[i / 2]);
				hit.uv = glm::uvec2(int(u*texsize_u[i / 2]), int(v*texsize_v[i / 2]));
			}
			if (!hit.has_hit){
				hit.m_pos = h;
				hit.m_color = m_colors[i];
				hit.m_normal = glm::normalize(n);
				hit.m_intensity = &m_photone_maps[i / 2][int(u*texsize_u[i / 2])][int(v*texsize_v[i / 2])];
				hit.has_hit = true;

				hit.photone_map = &m_photone_maps[i / 2];
				hit.size = glm::uvec2(texsize_u[i / 2], texsize_v[i / 2]);
				hit.uv = glm::uvec2(int(u*texsize_u[i / 2]), int(v*texsize_v[i / 2]));
			}
		}
	}

	hit.type = type;

	return hit;
}


// Cube
SCube::SCube(glm::mat3 T, glm::vec3 shift, glm::vec3 color, bool normout){
	for (int i = 0; i < 2; ++i){
		for (int j = 0; j < 2; ++j){
			for (int k = 0; k < 2; ++k){
				m_vertices.push_back(T * glm::vec3(i, j, k) + shift);
			}
		}
	}
	m_triangles = {
		// -x
		glm::uvec3(1, 2, 0),
		glm::uvec3(2, 1, 3),

		// +x
		glm::uvec3(6, 5, 4),
		glm::uvec3(5, 6, 7),

		// +z
		glm::uvec3(5, 3, 1),
		glm::uvec3(3, 5, 7),

		// -z
		glm::uvec3(2, 4, 0),
		glm::uvec3(4, 2, 6),

		// -y
		glm::uvec3(0, 5, 1),
		glm::uvec3(5, 0, 4),

		// +y
		glm::uvec3(7, 2, 3),
		glm::uvec3(2, 7, 6)
	};
	m_photone_maps.resize(6);
	for (int i = 0; i < m_triangles.size(); ++i){
		if (!normout){
			std::swap(m_triangles[i].x, m_triangles[i].y);
		}
	}
	for (int i = 0; i < 12; ++i){
		m_colors.push_back(color);
	}

	for (int i = 0; i < m_photone_maps.size(); ++i){
		glm::uvec3 tri = m_triangles[i * 2];
		glm::vec3 a = m_vertices[tri.x];
		glm::vec3 b = m_vertices[tri.y];
		glm::vec3 c = m_vertices[tri.z];
		texsize_u.push_back(glm::length(a - c)*PPU);
		texsize_v.push_back(glm::length(b - c)*PPU);
		m_photone_maps[i].resize(texsize_u[i] + 1);
		for (int j = 0; j < m_photone_maps[i].size(); ++j){
			m_photone_maps[i][j].resize(texsize_v[i] + 1);
		}
	}
}