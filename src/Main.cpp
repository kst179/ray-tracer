#include "Tracer.h"
#include "stdio.h"

void main(int argc, char** argv)
{
  CTracer tracer;
  CScene scene;

	int xRes = 512;//1024;  // Default resolution
	int yRes = 512;//768;

	int max_photones = 0;
	int ppu = 0;
	int use_collections=0;
	int min_photones=0;
	int max_radius=0;

  if(argc == 2) // There is input file in parameters
  {
    FILE* file = fopen(argv[1], "r");
    if(file)
    {
      int xResFromFile = 0;
      int yResFromFile = 0;
			
      if(fscanf(file, "%d %d %d %d %d", &xResFromFile, &yResFromFile, &max_photones, &ppu, &use_collections) == 5)
      {
        xRes = xResFromFile;
        yRes = yResFromFile;
				
				if (use_collections && fscanf(file, "%d %d", &min_photones, &max_radius) != 2){
					printf("Invalid config format! Using default parameters.\r\n");
					min_photones = 100;
					max_radius = 5;
				}
      }
			else{
				printf("Invalid config format! Using default parameters.\r\n");
				ppu = 20;
				max_photones = 100000;
				use_collections = false;
			}

      fclose(file);
    }
		else{
			printf("Invalid config path! Using default parameters.\r\n");
			ppu = 20;
			max_photones = 100000;
			use_collections = false;
		}
  }
	else{
		printf("No config! Using default parameters.\r\n");
		ppu = 20;
		max_photones = 100000;
		use_collections = false;
	}

  tracer.m_pScene = &scene;
  tracer.RenderImage(xRes, yRes, max_photones, ppu, use_collections, min_photones, max_radius);
  tracer.SaveImageToFile("../img/Result.png");
}