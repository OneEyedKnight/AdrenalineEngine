/*
	main.cpp
	Adrenaline Engine

    This starts the entire program.
*/

#include "engine/adrenaline.h"
#define DEBUG

int main() {
    Config config{};
    
    /*Model sponza("../engine/resources/models/sponza/Sponza.gltf");

    //Model newSponza("../engine/resources/models/newsponza/NewSponza_Main_Blender_glTF.gltf");

    //Model scientist("../engine/resources/models/scientist/scene.gltf");
    //scientist.scale = 10.0f;
    //scientist.rotationAngle = -90.0f;
    //scientist.rotationAxis = ADREN_Y_AXIS;

    // Model deccer("../engine/resources/models/deccer/SM_Deccer_Cubes_Textured_Embedded.gltf");
    
    //Model batman("../engine/resources/models/batman.gltf");

    //Model loba("../engine/resources/models/loba/scene.gltf");
    
    //Model cj("../engine/resources/models/CJ/scene.gltf");*/

    Adren::Engine engine;

    try {
    	engine.run();
    } catch (const std::exception& e) {
 		std::cerr << e.what() << std::endl;
 		return EXIT_FAILURE; 
 	}

	return EXIT_SUCCESS;
}
