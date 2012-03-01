#include <string>

#include <gl3w/gl3w.h> //Must be included before SFML
#include <time.h> //For setting up random

#include "FactoryRegister.h" //For initializing factory classes
#include "Singleton.h" //For accessing global classes
#include "SFMLCore.h" //For initializing the program
#include "GlobalPaths.h" //For setting data path

int main(int argc, char **argv)
{
	srand((unsigned int)time(NULL));
	std::string executablePath = std::string(argv[0]);
	std::string dataPath = executablePath + "/../../data/";
	Singleton<GlobalPaths>::Instance()->setDataPath(dataPath);
	Singleton<FactoryRegister>::Instance();
	Singleton<SFMLCore>::Instance();
	return 0;
}

/* 
TODO:
-Same mesh with different shaders
-Put GLBufferObject into GLMesh
-Make factory class registration work like the macro
-COLLADA mesh loading
-Hierarchical world storage
-Set name for world,meshes,materials
-Put all materials in one file
-Archetypes
-Better error handling (especially for XML loading)
-Allow adding to database
-Copying/ deep copying objects
*/