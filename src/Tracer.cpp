#include "Tracer.h"
#include <iostream>
#include <tuple>
#include <algorithm>

int MAX_PHOTONES = 100000000;
const float DEATH_P = 0.3;
int MIN_PHOTONES = 750;
int MAX_RADIUS = 25;
bool USE_COLLECTION = true;

//int PPU = 20;

SRay CTracer::MakeCameraRay(glm::uvec2 pixelPos)
{
	float shift_x = ((pixelPos.x + 0.5) / m_camera.m_resolution.x - 0.5);
	float shift_y = ((pixelPos.y + 0.5) / m_camera.m_resolution.y - 0.5);
	glm::vec3 dir = m_camera.m_forward +
						 shift_x * m_camera.m_right +
						 shift_y * m_camera.m_up;
  return SRay(m_camera.m_pos, dir);
}

SRay CTracer::MakeLanternRay(){
	glm::vec3 pos = m_lantern.m_pos + ((float)rand() / RAND_MAX - (float)0.5) * m_lantern.m_up + 
																		((float)rand() / RAND_MAX - (float)0.5) * m_lantern.m_right;
	glm::vec3 dir;
	do{
		dir = glm::vec3((float)rand() / RAND_MAX - 0.5, (float)rand() / RAND_MAX - 0.5, (float)rand() / RAND_MAX - 0.5);
	} while (glm::length(dir) < EPS || abs(glm::dot(dir, m_lantern.m_dir)) < EPS);
	dir = glm::normalize(dir*glm::dot(dir, m_lantern.m_dir));
	float intensity = glm::dot(dir, m_lantern.m_dir);

	SRay ray(pos, dir);
	SHit hit;
	do{
		hit = m_lantern.parabaloid.check_hit(ray);
		if (hit.has_hit){
			ray = hit.reflect(ray);
		}
	} while (hit.has_hit);

	do{
		hit = m_lantern.linse.check_hit(ray);
		if (hit.has_hit){
			ray = hit.reflect(ray);
		}
	} while (hit.has_hit);

	return ray;
}

glm::vec3 CTracer::TraceRayBackward(SRay ray)
{
	SHit hit;

	hit = m_lantern.parabaloid.check_hit(ray);
	if (hit.has_hit){
		return glm::vec3(0, 0, 0);
	}

	hit = m_pScene->check_hit(ray);

	if (!hit.has_hit){
		return glm::vec3(0, 0, 1);
	}

	if (hit.type == mirror || hit.type == transperent){
		return TraceRayBackward(hit.reflect(ray));
	}

	if (hit.type == diffuse){
		if (USE_COLLECTION){
			return Collect(hit.photone_map, hit.size, hit.uv) * hit.m_color;
		}
		return hit.m_intensity->first / (hit.m_intensity->second + EPS) * hit.m_color;
	}
}

void CTracer::TraceRayForward(SRay ray){
	SHit hit;

	do{
		hit = m_pScene->check_hit(ray);

		if (!hit.has_hit){
			return;
		}

		if (hit.m_intensity != nullptr){
			hit.m_intensity->first += ray.m_intensity;
			hit.m_intensity->second += 1;
		}

		ray = hit.reflect(ray);

	} while ((float)rand() / RAND_MAX > DEATH_P && glm::length(ray.m_intensity) > 0.002);
}

glm::vec3 CTracer::Collect(std::vector< std::vector< std::pair<glm::vec3, float> > > *photone_map, 
	glm::uvec2 size,
	glm::uvec2 p)
{
	glm::vec3 color = (*photone_map)[p.x][p.y].first;
	float count = (*photone_map)[p.x][p.y].second;
	int radius = 0;

	while (count < MIN_PHOTONES && radius < MAX_RADIUS){
		radius += 1;
		int i, j;

		j = p.y - radius;
		if (0 <= j && j < size.y){
			for (i = p.x - radius; i <= p.x + radius; ++i){
				if (0 <= i && i < size.x){
					color += (*photone_map)[i][j].first;
					count += (*photone_map)[i][j].second;
				}
			}
		}

		j = p.y + radius;
		if (0 <= j && j < size.y){
			for (i = p.x - radius; i <= p.x + radius; ++i){
				if (0 <= i && i < size.x){
					color += (*photone_map)[i][j].first;
					count += (*photone_map)[i][j].second;
				}
			}
		}

		i = p.x - radius;
		if (0 <= i && i < size.x){
			for (j = p.y - (radius-1); j <= p.y + (radius-1); ++j){
				if (0 <= j && j < size.y){
					color += (*photone_map)[i][j].first;
					count += (*photone_map)[i][j].second;
				}
			}
		}
		i = p.x + radius;
		if (0 <= i && i < size.x){
			for (j = p.y - (radius - 1); j <= p.y + (radius - 1); ++j){
				if (0 <= j && j < size.y){
					color += (*photone_map)[i][j].first;
					count += (*photone_map)[i][j].second;
				}
			}
		}
	}

	return color / count;
}

void CTracer::RenderImage(int xRes, int yRes, int max_photones, int ppu, bool use_collections, int min_photones, int max_radius){
	MAX_PHOTONES = max_photones;
	USE_COLLECTION = use_collections;
	MIN_PHOTONES = min_photones;
	MAX_RADIUS = max_radius;
	change_ppu(ppu);

	m_camera = m_pScene->camera;

  m_camera.m_resolution = glm::uvec2(xRes, yRes);
  m_camera.m_pixels.resize(xRes * yRes);
	m_camera.set_angle(90, xRes, yRes);

	m_lantern = m_pScene->lantern;

	// Making map of photones, trace forward
	std::cout << "trace forward" << std::endl;
	for (int i = 0; i < MAX_PHOTONES; ++i){
		if ((i + 1) * 100 % MAX_PHOTONES == 0){
			std::cout << (i + 1) * 100 / MAX_PHOTONES << "%" << std::endl;
		}
		SRay ray = MakeLanternRay();
		TraceRayForward(ray);
	}

	// Trace backward
	std::cout << "trace backward" << std::endl;
  for(int i = 0; i < yRes; i++){
    for(int j = 0; j < xRes; j++)
    {
			if ((i * xRes + j + 1) % (xRes*yRes/100) == 0){
				std::cout << (i * xRes + j + 1) / (xRes*yRes/100) << "%" << std::endl;
			}
      SRay ray = MakeCameraRay(glm::uvec2(j, i));
      m_camera.m_pixels[i * xRes + j] = TraceRayBackward(ray);
    }
	}

	std::cout << "saving image..." << std::endl;
}

void CTracer::SaveImageToFile(std::string fileName)
{
  CImage image;

  int width = m_camera.m_resolution.x;
  int height = m_camera.m_resolution.y;

  image.Create(width, height, 24);
    
	int pitch = image.GetPitch();
	unsigned char* imageBuffer = (unsigned char*)image.GetBits();

	if (pitch < 0)
	{
		imageBuffer += pitch * (height - 1);
		pitch =- pitch;
	}

	int i, j;
	int imageDisplacement = 0;
	int textureDisplacement = 0;

	for (i = 0; i < height; i++)
	{
    for (j = 0; j < width; j++)
    {
      glm::vec3 color = m_camera.m_pixels[textureDisplacement + j];

      imageBuffer[imageDisplacement + j * 3] = glm::clamp(color.b, 0.0f, 1.0f) * 255.0f;
      imageBuffer[imageDisplacement + j * 3 + 1] = glm::clamp(color.g, 0.0f, 1.0f) * 255.0f;
      imageBuffer[imageDisplacement + j * 3 + 2] = glm::clamp(color.r, 0.0f, 1.0f) * 255.0f;
    }

		imageDisplacement += pitch;
		textureDisplacement += width;
	}

  image.Save(fileName.c_str());
	image.Destroy();
}

CImage* CTracer::LoadImageFromFile(std::string fileName)
{
  CImage* pImage = new CImage;

  if(SUCCEEDED(pImage->Load(fileName.c_str())))
    return pImage;
  else
  {
    delete pImage;
    return NULL;
  }
}