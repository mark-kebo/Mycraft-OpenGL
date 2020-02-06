#include "sampleapp.h"

using namespace OGLAppFramework;


SampleApp::SampleApp() : OGLAppFramework::OGLApplication(1366, 768, "Game - 3D", 4u, 2u), SCR_WIDTH(1366), SCR_HEIGHT(768),
vbo_cube_handle(0u), index_buffer_handle(0u), vao_handle_sky(0u), vao_piramide_litle_handle(0u), vbo_piramide_litle_handle(0u),
vao_cube_handle(0u), ubo_mvp_matrix_handle(0u), ubo_intensity_handle(0u), tex_handle(0u), vao_piramide_handle(0u),
tex_so(0u), ubo_ambient_light(0u), tex_handle_sky(0u), index_buffer_handle_sky(0u), vbo_piramide_handle(0u), ubo_option(0u),
ubo_point_light(0u), ubo_camera_position(0u), ubo_material(0u), vbo_handle_sky(0u), ubo_skybox(0u), tex_piramide_handle(0u),
vbo_small_cube_handle(0u), vao_small_cube_handle(0u), shader_object(), shader_sky(), deltaTime(0u), elementsCount(0u),
firstMouse(true), camera(glm::vec3(50.0f, 5.0f, 50.0f)), objectType(smallCube), directionObject(diagonalForward), 
lastX(1366 / 2.0), lastY(768 / 2.0){
}

SampleApp::~SampleApp() {
}

void SampleApp::reshapeCallback(std::uint16_t width, std::uint16_t height) {
	std::cout << "Reshape..." << std::endl;
	std::cout << "New window size: " << width << " x " << height << std::endl;
	projection_matrix = glm::perspective(glm::radians(camera.Zoom), (float)width / height, 0.1f, 20.0f);
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	gl::glViewport(0, 0, width, height);
}

void SampleApp::keyCallback(int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_SPACE && (action == 0 || action == 2))
		camera.ProcessKeyboard(UP, deltaTime); 
	if (key == GLFW_KEY_W && (action == 0 || action == 2))
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (key == GLFW_KEY_S && (action == 0 || action == 2)) 
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (key == GLFW_KEY_D && (action == 0 || action == 2)) 
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (key == GLFW_KEY_A && (action == 0 || action == 2)) 
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (key == GLFW_KEY_P && (action == 0 || action == 2)) {
		switch (objectType) {
		case(smallCube):
			objectType = cube;
			break;
		case(cube):
			objectType = piramide;
			break;
		case(piramide):
			objectType = litlePiramide;
			break;
		case(litlePiramide):
			objectType = smallCube;
			break;
		}
	}
}

void SampleApp::cursorPosCallback(double xpos, double ypos) {
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void SampleApp::mouseButtonCallback(int button, int action, int mods) {
	if (action == 0) {
		if (button == 1) {
			if (mass[(int)camera.getBeforePlayerPosition().x][(int)camera.getBeforePlayerPosition().y][(int)camera.getBeforePlayerPosition().z] != 0) {
				mass[(int)camera.getBeforePlayerPosition().x][(int)camera.getBeforePlayerPosition().y][(int)camera.getBeforePlayerPosition().z] = 0;
				elementsCount--;
			}
		} else {
			if (mass[(int)camera.getBeforePlayerPosition().x][(int)camera.getBeforePlayerPosition().y][(int)camera.getBeforePlayerPosition().z] != 1) {
				mass[(int)camera.getBeforePlayerPosition().x][(int)camera.getBeforePlayerPosition().y][(int)camera.getBeforePlayerPosition().z] = 1;
				elementsCount++;
			}
		}
		std::cout << "Mouse  " << camera.getBeforePlayerPosition().x << " " << camera.getBeforePlayerPosition().y << " " << camera.getBeforePlayerPosition().z << std::endl;
		std::cout << "Count  " << elementsCount << std::endl;
	}
}

bool SampleApp::init(void) {
    std::cout << "Init..." << std::endl;

	projection_matrix = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 20.0f);
	
	camera.MovementSpeed = 8.f;
   
	// ustalamy domyślny kolor ekranu
	gl::glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // wlaczmy renderowanie tylko jednej strony poligon-ow
    gl::glEnable(gl::GL_CULL_FACE);
    // ustalamy, ktora strona jest "przodem"
    gl::glFrontFace(gl::GL_CCW);
    // ustalamy, ktorej strony nie bedziemy renderowac
    gl::glCullFace(gl::GL_BACK);
	gl::glEnable(gl::GL_DEPTH_TEST);

	srand(time(0));
	for (int x = 0; x < 15; x++)
		for (int y = 0; y < 2; y++)
			for (int z = 0; z < 15; z++) {
				if ((y == 0) || rand() % 100 == 1) {
					mass[x + 40][y][z + 40] = true;
					elementsCount++;
				}
			}

	bindSkybox();

	std::string texture_p = "../../../dv_project/data/box.dds";
#if WIN32
	texture_p = "C:/Users/Mark/Desktop/dv_project/data/box.dds";
#endif
	if (auto load_t_r = OGLAppFramework::loadTexFromFileAndCreateTO(texture_p)) {
		tex_handle = load_t_r.value();
	}
	else {
		return false;
	}
	std::string texture_piramide = "../../../dv_project/data/piram.dds";
#if WIN32
	texture_p = "C:/Users/Mark/Desktop/dv_project/data/piram.dds";
#endif
	if (auto load_t_r = OGLAppFramework::loadTexFromFileAndCreateTO(texture_p)) {
		tex_piramide_handle = load_t_r.value();
	}
	else {
		return false;
	}

	gl::GLfloat vertices_cube[] = {
		-0.5f, 0.5f, -0.5f,		1.0f,  0.5f,	0.0f,  0.0f, -1.0f,
		 0.5f, 0.5f, -0.5f,		1.0f,  1.0f,	0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,	0.5f,  1.0f,	0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,	0.5f,  1.0f,	0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,	1.0f,  1.0f,	0.0f,  0.0f, -1.0f,
		-0.5f, 0.5f, -0.5f,		1.0f,  0.5f,	0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,	1.0f,  0.5f,	0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,	1.0f,  1.0f,	0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,	0.5f,  1.0f,	0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,	0.5f,  1.0f,	0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,	1.0f,  1.0f,	0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,	1.0f,  0.5f,	0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f,	1.0f,  0.5f,	-1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,	1.0f,  1.0f,	-1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,	0.5f,  1.0f,	-1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,	0.5f,  1.0f,	-1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,	1.0f,  1.0f,	-1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,	1.0f,  0.5f,	-1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  -0.5f,	1.0f,  0.5f,	1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,   0.5f,	1.0f,  1.0f,	1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,   0.5f,	0.5f,  1.0f,	1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,   0.5f,	0.5f,  1.0f,	1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  -0.5f,	1.0f,  1.0f,	1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  -0.5f,	1.0f,  0.5f,	1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,	1.0f,  0.5f,	0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,	1.0f,  1.0f,	0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,	0.5f,  1.0f,	0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,	0.5f,  1.0f,	0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,	1.0f,  1.0f,	0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,	1.0f,  0.5f,	0.0f, -1.0f,  0.0f,

		0.5f,  0.5f, -0.5f,		0.5f,  0.5f,	0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f,		0.5f,  1.0f,	0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f,		0.0f,  1.0f,	0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f,		0.0f,  1.0f,	0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,		0.5f,  1.0f,	0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,		0.5f,  0.5f,	0.0f,  1.0f,  0.0f
	};
	gl::GLushort indices_cube[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35 };
	bindObject(vertices_cube, indices_cube, &vbo_cube_handle, &vao_cube_handle);

	gl::GLfloat vertices_small_cube[] = {
	-0.5f, 0.25f, -0.5f,	1.0f,  0.5f,	0.0f,  0.0f, -1.0f,
	 0.5f, 0.25f, -0.5f,	1.0f,  1.0f,	0.0f,  0.0f, -1.0f,
	 0.5f, -0.5f, -0.5f,	0.5f,  1.0f,	0.0f,  0.0f, -1.0f,
	 0.5f, -0.5f, -0.5f,	0.5f,  1.0f,	0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,	1.0f,  1.0f,	0.0f,  0.0f, -1.0f,
	-0.5f, 0.25f, -0.5f,	1.0f,  0.5f,	0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,	1.0f,  0.5f,	0.0f,  0.0f,  1.0f,
	 0.5f, -0.5f,  0.5f,	1.0f,  1.0f,	0.0f,  0.0f,  1.0f,
	 0.5f,  0.25f,  0.5f,	0.5f,  1.0f,	0.0f,  0.0f,  1.0f,
	 0.5f,  0.25f,  0.5f,	0.5f,  1.0f,	0.0f,  0.0f,  1.0f,
	-0.5f,  0.25f,  0.5f,	1.0f,  1.0f,	0.0f,  0.0f,  1.0f,
	-0.5f, -0.5f,  0.5f,	1.0f,  0.5f,	0.0f,  0.0f,  1.0f,

	-0.5f,  0.25f,  0.5f,	1.0f,  0.5f,	-1.0f,  0.0f,  0.0f,
	-0.5f,  0.25f, -0.5f,	1.0f,  1.0f,	-1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,	0.5f,  1.0f,	-1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,	0.5f,  1.0f,	-1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,	1.0f,  1.0f,	-1.0f,  0.0f,  0.0f,
	-0.5f,  0.25f,  0.5f,	1.0f,  0.5f,	-1.0f,  0.0f,  0.0f,

	 0.5f,  0.25f,  -0.5f,	1.0f,  0.5f,	1.0f,  0.0f,  0.0f,
	 0.5f,  0.25f,   0.5f,	1.0f,  1.0f,	1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f,   0.5f,	0.5f,  1.0f,	1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f,   0.5f,	0.5f,  1.0f,	1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f,  -0.5f,	1.0f,  1.0f,	1.0f,  0.0f,  0.0f,
	 0.5f,  0.25f,  -0.5f,	1.0f,  0.5f,	1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,	1.0f,  0.5f,	0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,	1.0f,  1.0f,	0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,	0.5f,  1.0f,	0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,	0.5f,  1.0f,	0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,	1.0f,  1.0f,	0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,	1.0f,  0.5f,	0.0f, -1.0f,  0.0f,

	0.5f,  0.25f, -0.5f,	0.5f,  0.5f,	0.0f,  1.0f,  0.0f,
   -0.5f,  0.25f, -0.5f,	0.5f,  1.0f,	0.0f,  1.0f,  0.0f,
   -0.5f,  0.25f,  0.5f,	0.0f,  1.0f,	0.0f,  1.0f,  0.0f,
   -0.5f,  0.25f,  0.5f,	0.0f,  1.0f,	0.0f,  1.0f,  0.0f,
	0.5f,  0.25f,  0.5f,	0.5f,  1.0f,	0.0f,  1.0f,  0.0f,
	0.5f,  0.25f, -0.5f,	0.5f,  0.5f,	0.0f,  1.0f,  0.0f
	};
	bindObject(vertices_small_cube, indices_cube, &vbo_small_cube_handle, &vao_small_cube_handle);
	
	gl::GLfloat vertices_piramide[] = {
		// Triangle 1
				0.0,-0.5,0.0,      1.0,0.5,		0.0,1.0,1.0,
				0.5,0.5,0.5,       0.5,1.0,		0.0,1.0,1.0,
				-0.5,0.5,0.5,      1.0,1.0,		0.0,1.0,1.0,
		//Triangle 2
				0.0,-0.5,0.0,      1.0,0.5,		1.0,1.0,0.0,
				0.5,0.5,-0.5,      0.5,1.0,		1.0,1.0,0.0,
				0.5,0.5,0.5,       1.0,1.0,		1.0,1.0,0.0,
		//Triangle 3
				0.0,-0.5,0.0,      1.0,0.5,		0.0,1.0,-1.0,
				-0.5,0.5,-0.5,     0.5,1.0,		0.0,1.0,-1.0,
				0.5,0.5,-0.5,      1.0,1.0,		0.0,1.0,-1.0,
		//Triangle 4
				0.0,-0.5,0.0,      1.0,0.5,		-1.0,1.0,0.0,
				-0.5,0.5,0.5,      0.5,1.0,		-1.0,1.0,0.0,
				-0.5,0.5,-0.5,     1.0,1.0,		-1.0,1.0,0.0,
		//Triangle 5-----
				-0.5,0.5,0.5,      0.5,1.0,		0.0,1.0,0.0,
				0.5,0.5,0.5,       1.0,1.0,		0.0,1.0,0.0,
				0.5,0.5,-0.5,      1.0,0.5,		0.0,1.0,0.0,
		//Triangle 6
				 0.5,0.5,-0.5,     1.0,0.5,		0.0,1.0,0.0,
				-0.5,0.5,-0.5,     1.0,1.0,		0.0,1.0,0.0,
				-0.5,0.5,0.5,      0.5,1.0,		0.0,1.0,0.0
	};
	gl::GLushort indices_piramide[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
	bindObject(vertices_piramide, indices_piramide, &vbo_piramide_handle, &vao_piramide_handle);

	gl::GLfloat vertices_piramide_litle[] = {
		0.5, 0.0, 0.5,        1.0, 1.0,		1.0, 0.0, 1.0,
		0.5, 0.0, -0.5,	      0.0, 1.0,		1.0, 0.0, -1.0,
		-0.5, 0.0, -0.5,      1.0, 1.0,		-1.0, 0.0, -1.0,
		-0.5, 0.0, 0.5,       1.0, 0.0,		-1.0, 0.0, 1.0,
		0.0, 0.5, 0.0,		  0.0, 0.0,		0.0, 1.0, 0.0
	};
	gl::GLushort indices_piramide_litle[] = { 3, 2, 1,	 3, 1, 0,	3, 0, 4,	0, 1, 4,	1, 2, 4,	2, 3, 4 };
	bindObject(vertices_piramide_litle, indices_piramide_litle, &vbo_piramide_litle_handle, &vao_piramide_litle_handle);

	// Tworzenie SO
	gl::glGenSamplers(1, &tex_so);
	// Ustawienie parametrów samplowania
	gl::glSamplerParameteri(tex_so, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
	gl::glSamplerParameteri(tex_so, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
	gl::glSamplerParameteri(tex_so, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR);
	gl::glSamplerParameteri(tex_so, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
	gl::glBindSampler(0u, tex_so);

	return true;
}

bool SampleApp::frame(float delta_time) {
	//std::cout << "FPS:" << 1/delta_time << std::endl;
	static float angle = 0.f;
	static float x = 45.f;
	static float z = 45.f;
	angle += delta_time * 1.1;
	deltaTime = delta_time;
	camera.update(deltaTime, mass);

	switch (objectType) {
	case(smallCube):
		useTexture(&tex_handle);
		drawObjects(&vao_small_cube_handle, 36, true, true);
		break;
	case(cube):
		useTexture(&tex_handle);
		drawObjects(&vao_cube_handle, 36, true, true);
		break;
	case(piramide):
		useTexture(&tex_piramide_handle);
		drawObjects(&vao_piramide_handle, 18, true, true);
		break;
	case(litlePiramide):
		drawObjects(&vao_piramide_litle_handle, 18, false, false);
		break;
	}

	useTexture(&tex_piramide_handle);
	srand(time(0));
	switch (rand() % 10 + 1) {
	case 1: 	
		directionObject = diagonalForward;
		break;
	case 2:
		directionObject = diagonalBack;
		break;
	case 3:
		directionObject = forward;
		break;
	case 4:
		directionObject = right;
		break;
	case 5:
		directionObject = back;
		break;
	case 6:
		directionObject = left;
		break;
	}
	switch (directionObject) {
	case forward:
		x += 0.01;
		break;
	case back:
		x -= 0.01;
		break;
	case left:
		z -= 0.01;
		break;
	case right:
		z += 0.01;
		break;
	case diagonalForward:
		x += 0.01;
		z += 0.01;
		break;
	case diagonalBack:
		x -= 0.01;
		z -= 0.01;
		break;
	}

	drawObject(&vao_piramide_handle, 18, true, true, glm::vec3(x, 1.0f, z));
	if ((int)camera.Position.x == (int)x && (int)camera.Position.y == 1 && (int)camera.Position.z == (int)z) {
		std::cout << "You caught lava !!!" << std::endl;
		camera.fall();
	}
	//skybox
	drawSkybox();

	return true;
}

void SampleApp::useTexture(gl::GLuint *id) {
	// uaktywnienie pierwszego slotu tekstur
	gl::glActiveTexture(gl::GL_TEXTURE0);
	// zbindowanie tekstury do aktywnego slotu
	gl::glBindTexture(gl::GL_TEXTURE_2D, *id);
}

void SampleApp::drawObjects(gl::GLuint *vao, gl::GLsizei size, bool isHasLights, bool isUseTexture) {
	// ustawienie programu, ktory bedzie uzywany podczas rysowania
	shader_object.use();
	// zbindowanie VAO modelu, ktorego bedziemy renderowac
	gl::glBindVertexArray(*vao);

	std::array<ModelMatrices, 1000u> array;
	int index = 0;
	for (int x = 0; x < 100; x++)
		for (int y = 0; y < 100; y++)
			for (int z = 0; z < 100; z++) {
				if (!mass[x][y][z]) continue;
					ModelMatrices matrices = ModelMatrices();
					if (objectType == smallCube) {
						matrices.model_matrix = translationMatrix(glm::vec3(0.0f + x, 0.0f + y - y * 0.25, 0.0f + z));
					} else {
						matrices.model_matrix = translationMatrix(glm::vec3(0.0f + x, 0.0f + y, 0.0f + z));
					}
					matrices.mvp_matrix = projection_matrix * camera.GetViewMatrix() * matrices.model_matrix;
					array[index] = matrices;
					index++;
			}

	Option option = Option();
	option.isHasLights = isHasLights;
	option.isUseTexture = isUseTexture;
	shader_object.sendData(option, ubo_option);
	if (isHasLights)
		shader_object.sendData(camera.Position, ubo_camera_position);
	shader_object.sendData(array, ubo_mvp_matrix_handle);

	// rozpoczynamy rysowanie uzywajac ustawionego programu (shader-ow) i ustawionych buforow
	gl::glDrawElementsInstanced(gl::GL_TRIANGLES, size, gl::GL_UNSIGNED_SHORT, nullptr, elementsCount);
	gl::glBindVertexArray(0);
}

void SampleApp::drawObject(gl::GLuint *vao, gl::GLsizei size, bool isHasLights, bool isUseTexture, const glm::vec3& translation) {
	// ustawienie programu, ktory bedzie uzywany podczas rysowania
	shader_object.use();
	// zbindowanie VAO modelu, ktorego bedziemy renderowac
	gl::glBindVertexArray(*vao);

	ModelMatrices matrices = ModelMatrices();
	matrices.model_matrix = translationMatrix(translation);
	matrices.mvp_matrix = projection_matrix * camera.GetViewMatrix() * matrices.model_matrix;
	Option option = Option();
	option.isHasLights = isHasLights;
	option.isUseTexture = isUseTexture;
	shader_object.sendData(option, ubo_option);
	if (isHasLights)
		shader_object.sendData(camera.Position, ubo_camera_position);
	shader_object.sendData(matrices, ubo_mvp_matrix_handle);

	// rozpoczynamy rysowanie uzywajac ustawionego programu (shader-ow) i ustawionych buforow
	gl::glDrawElements(gl::GL_TRIANGLES, size, gl::GL_UNSIGNED_SHORT, nullptr);
	gl::glBindVertexArray(0);
}

void SampleApp::bindObject(gl::GLfloat vertices[], gl::GLushort indices[], gl::GLuint *vbo, gl::GLuint *vao) {
	std::cout << "Shaders compilation..." << std::endl;
	// wczytanie z plikow i skompilowanie shaderow oraz utworzenie programu (VS + FS)
	char* vs_path = "../../../dv_project/shaders/simple_lights_vs.glsl";
	char* fs_path = "../../../dv_project/shaders/simple_lights_fs.glsl";
#if WIN32
	vs_path = "C:/Users/Mark/Desktop/dv_project/shaders/simple_lights_vs.glsl";
	fs_path = "C:/Users/Mark/Desktop/dv_project/shaders/simple_lights_fs.glsl";
#endif
	shader_object = Shader(vs_path, fs_path);

	// ustawienie informacji o lokalizacji atrybutu pozycji w vs (musi sie zgadzac z tym co mamy w VS!!!)
	const gl::GLuint vertex_position_loction = 0u;
	// ustawienie informacji o lokalizacji atrybutu uv w vs (musi sie zgadzac z tym co mamy w VS!!!)
	const gl::GLuint vertex_tex_uv_loction = 1u;
	// ustawienie informacji o lokalizacji atrybutu wektora normalnego w vs (musi sie zgadzac z tym co mamy w VS!!!)
	const gl::GLuint vertex_normal_loction = 2u;
	// ustawienie informacji o indeksie bindowania uniform block-u z danymi dotyczacymi macierzy
	const gl::GLuint ub_mvp_binding_index = 1u;
	// ustawienie informacji o indeksie bindowania uniform block-u z danymi dotyczacymi dodatkowych danych (np. kamery)
	const gl::GLuint ub_additional_data_binding_index = 2u;
	// ustawienie informacji o indeksie bindowania uniform block-u z danymi dotyczacymi materialu
	const gl::GLuint ub_material_binding_index = 3u;
	// ustawienie informacji o indeksie bindowania uniform block-u z danymi dotyczacymi swiatla otoczenia
	const gl::GLuint ub_ambient_light_binding_index = 4u;
	// ustawienie informacji o indeksie bindowania uniform block-u z danymi dotyczacymi swiatla punktowego
	const gl::GLuint ub_point_light_binding_index = 5u;
	const gl::GLuint ub_option_binding_index = 7u;


	// ustawienie programu, ktory bedzie uzywany podczas rysowania
	shader_object.use();

	std::cout << "Generating buffers..." << std::endl;
	shader_object.createVAO(vao);
	shader_object.createVBO(vbo, vertices, 288);
	shader_object.createIndexBuffer(indices, 36, &index_buffer_handle);
	shader_object.bindVBOandIBtoVAO(vertex_position_loction, vertex_tex_uv_loction, vertex_normal_loction, &index_buffer_handle);
	shader_object.unbindVAOandVBO();

	PointLightData lights = PointLightData();
	PointLight pointLight = PointLight();
	PointLight pointLight2 = PointLight();
	pointLight.position_ws = glm::vec3(10.f, 100.f, 90.f);
	pointLight.r = 300.f;
	pointLight.color = glm::vec3(1.f, 0.5f, 0.4f);
	pointLight2.r = 300.f;
	pointLight2.position_ws = glm::vec3(90.f, 100.f, 10.f);
	pointLight2.color = glm::vec3(1.f, 1.f, 1.f);
	lights.n = 2;
	lights.lights[0] = pointLight;
	lights.lights[1] = pointLight2;

	glm::vec3 ambient_light_color = glm::vec3(0.2f, 0.2f, 0.2f);
	TextMaterial material = TextMaterial();
	material.color = glm::vec3(1.f, 1.f, 1.f);
	material.specular_intensity = 1.f;
	material.specular_power = 1.f;

	shader_object.createBuffer(ambient_light_color, ub_ambient_light_binding_index, &ubo_ambient_light);
	shader_object.createBuffer(lights, ub_point_light_binding_index, &ubo_point_light);
	shader_object.createBuffer(material, ub_material_binding_index, &ubo_material);
	shader_object.createBuffer(0, ub_additional_data_binding_index, &ubo_camera_position);
	shader_object.createBuffer(0, ub_mvp_binding_index, &ubo_mvp_matrix_handle);
	shader_object.createBuffer(0, ub_option_binding_index, &ubo_option);
}

void SampleApp::bindSkybox() {
	std::cout << "Shaders skybox compilation..." << std::endl;
	// wczytanie z plikow i skompilowanie shaderow oraz utworzenie programu (VS + FS)
	char* vs_path = "../../../dv_project/shaders/simple_skybox_vs.glsl";
	char* fs_path = "../../../dv_project/shaders/simple_skybox_fs.glsl";
#if WIN32
	vs_path = "C:/Users/Mark/Desktop/dv_project/shaders/simple_skybox_vs.glsl";
	fs_path = "C:/Users/Mark/Desktop/dv_project/shaders/simple_skybox_fs.glsl";
#endif

	// ustawienie informacji o lokalizacji atrybutu pozycji w vs (musi sie zgadzac z tym co mamy w VS!!!)
	const gl::GLuint vertex_position_loction = 0u;
	const gl::GLuint ub_skybox_index = 6u;

	shader_sky = Shader(vs_path, fs_path);
	// ustawienie programu, ktory bedzie uzywany podczas rysowania
	shader_sky.use();

	gl::GLfloat vertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	gl::GLushort indices[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
	18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};

	std::cout << "Generating skybox buffers..." << std::endl;
	shader_sky.createVAO(&vao_handle_sky);
	shader_sky.createVBO(&vbo_handle_sky, vertices, 108);
	shader_sky.createIndexBuffer(indices, 36, &index_buffer_handle_sky);
	shader_sky.bindVBOandIBtoVAO(vertex_position_loction, &index_buffer_handle_sky);
	shader_sky.unbindVAOandVBO();

	std::string texture_p = "../../../dv_project/data/SkyDawn.dds";
#if WIN32
	texture_p = "C:/Users/Mark/Desktop/dv_project/data/SkyDawn.dds";
#endif
	if (auto load_t_r = OGLAppFramework::loadTexFromFileAndCreateTO(texture_p)) {
		tex_handle_sky = load_t_r.value();
	}
	else {
		return;
	}
	
	glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
	std::array<glm::mat4x4, 2u> matrices = { projection_matrix, view };
	shader_sky.createBuffer(matrices, ub_skybox_index, &ubo_skybox);
}

void SampleApp::drawSkybox() {
	// ustawienie programu, ktory bedzie uzywany podczas rysowania
	shader_sky.use();
	// skybox cube
	gl::glBindVertexArray(vao_handle_sky);
	gl::glDepthFunc(gl::GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
	std::array<glm::mat4x4, 2u> matrices = { projection_matrix, view };
	shader_sky.sendData(matrices, ubo_skybox);
	gl::glActiveTexture(gl::GL_TEXTURE0);
	gl::glBindTexture(gl::GL_TEXTURE_CUBE_MAP, tex_handle_sky);
	gl::glDrawElements(gl::GL_TRIANGLES, 36, gl::GL_UNSIGNED_SHORT, nullptr);
	gl::glDepthFunc(gl::GL_LESS); // set depth function back to default
	gl::glBindVertexArray(0);
}

void SampleApp::release(void) {
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
	if (index_buffer_handle_sky)
	{
		// usuniecie UBO
		gl::glDeleteBuffers(1, &index_buffer_handle_sky);
		index_buffer_handle_sky = 0u;
	}
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
	if (tex_handle_sky)
	{
		// usuniecie UBO
		gl::glDeleteBuffers(1, &tex_handle_sky);
		tex_handle_sky = 0u;
	}
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
	if (vao_handle_sky)
	{
		// usuniecie UBO
		gl::glDeleteBuffers(1, &vao_handle_sky);
		vao_handle_sky = 0u;
	}
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
	if (vbo_handle_sky)
	{
		// usuniecie UBO
		gl::glDeleteBuffers(1, &vbo_handle_sky);
		vbo_handle_sky = 0u;
	}
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);

	if (vbo_piramide_litle_handle)
	{
		// usuniecie UBO
		gl::glDeleteBuffers(1, &vbo_piramide_litle_handle);
		vbo_piramide_litle_handle = 0u;
	}
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
	if (vao_piramide_litle_handle)
	{
		// usuniecie UBO
		gl::glDeleteBuffers(1, &vao_piramide_litle_handle);
		vao_piramide_litle_handle = 0u;
	}
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
	
	if (vbo_piramide_handle)
	{
		// usuniecie UBO
		gl::glDeleteBuffers(1, &vbo_piramide_handle);
		vbo_piramide_handle = 0u;
	}
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
	if (vao_piramide_handle)
	{
		// usuniecie UBO
		gl::glDeleteBuffers(1, &vao_piramide_handle);
		vao_piramide_handle = 0u;
	}
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
	if (ubo_skybox)
	{
		// usuniecie UBO
		gl::glDeleteBuffers(1, &ubo_skybox);
		ubo_skybox = 0u;
	}
	// odbindowanie UBO
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
	if (ubo_ambient_light)
	{
		// usuniecie UBO
		gl::glDeleteBuffers(1, &ubo_ambient_light);
		ubo_ambient_light = 0u;
	}
	
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
	if (ubo_option)
	{
		// usuniecie UBO
		gl::glDeleteBuffers(1, &ubo_option);
		ubo_option = 0u;
	}
    // odbindowanie UBO
    gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
    if(ubo_mvp_matrix_handle)
    {
        // usuniecie UBO
        gl::glDeleteBuffers(1, &ubo_mvp_matrix_handle);
        ubo_mvp_matrix_handle = 0u;
    }

	// odbindowanie UBO
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
	if (tex_piramide_handle)
	{
		// usuniecie UBO
		gl::glDeleteBuffers(1, &tex_piramide_handle);
		tex_piramide_handle = 0u;
	}

    // odbindowanie VAO
    gl::glBindVertexArray(0);
    if (vao_cube_handle)
    {
        // usuniecie VAO
        gl::glDeleteVertexArrays(1, &vao_cube_handle);
        vao_cube_handle = 0u;
    }

    // odbindowanie VBO
    gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);
    if (vbo_cube_handle)
    {
        // usuniecie VBO
        gl::glDeleteBuffers(1, &vbo_cube_handle);
        vbo_cube_handle = 0u;
    }

	// odbindowanie VAO
	gl::glBindVertexArray(0);
	if (vao_small_cube_handle)
	{
		// usuniecie VAO
		gl::glDeleteVertexArrays(1, &vao_small_cube_handle);
		vao_small_cube_handle = 0u;
	}

	// odbindowanie VBO
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);
	if (vbo_small_cube_handle)
	{
		// usuniecie VBO
		gl::glDeleteBuffers(1, &vbo_small_cube_handle);
		vbo_small_cube_handle = 0u;
	}

    // odbindowanie IB
    gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);
    if (index_buffer_handle)
    {
        // usuniecie IB
        gl::glDeleteBuffers(1, &index_buffer_handle);
        index_buffer_handle = 0u;
    }

	shader_object.deleteProgram();
	shader_sky.deleteProgram();
}
