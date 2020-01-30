#include "sampleapp.h"

using namespace OGLAppFramework;

enum ObjectType { piramide, litlePiramide, cube };

// settings
float SCR_WIDTH = 1366;
float SCR_HEIGHT = 768;

//scene
bool mass[100][100][100];

ObjectType objectType = cube;

// camera
Camera camera(glm::vec3(50.0f, 5.0f, 50.0f));
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;
int elementsCount = 0;

// timing
float deltaTime = 0.0f;

//shaders
Shader shader_cube = Shader();
Shader shader_piramide = Shader();
Shader shader_litle_piramide = Shader();
Shader shader_sky = Shader();

struct TextMaterial {
	glm::vec3 color;
	float specular_intensity;
	float specular_power;
};
struct Option {
	bool isHasLights;
	float some1;
	bool isUseTexture;
	float some2;
};
struct PointLight {
	glm::vec3 position_ws;
	float r;
	glm::vec3 color;
	float some1;
};
struct PointLightData {
	int n;
	float some1;
	float some2;
	float some3;
	PointLight lights[2];
};
struct ModelMatrices {
	glm::mat4x4 mvp_matrix;
	glm::mat4x4 model_matrix;
};

SampleApp::SampleApp() : OGLAppFramework::OGLApplication(SCR_WIDTH, SCR_HEIGHT, "Game - 3D", 4u, 2u),
vbo_cube_handle(0u), index_buffer_handle(0u), vao_handle_sky(0u), vao_piramide_litle_handle(0u), vbo_piramide_litle_handle(0u),
vao_cube_handle(0u), ubo_mvp_matrix_handle(0u), ubo_intensity_handle(0u), tex_handle(0u), vao_piramide_handle(0u),
tex_so(0u), ubo_ambient_light(0u), tex_handle_sky(0u), index_buffer_handle_sky(0u), vbo_piramide_handle(0u), ubo_option(0u),
ubo_point_light(0u), ubo_camera_position(0u), ubo_material(0u), vbo_handle_sky(0u), ubo_skybox(0u), tex_piramide_handle(0u) {
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
		case(cube):
			objectType = piramide;
			break;
		case(piramide):
			objectType = litlePiramide;
			break;
		case(litlePiramide):
			objectType = cube;
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
	for (int x = 0; x < 5; x++)
		for (int y = 0; y < 5; y++)
			for (int z = 0; z < 5; z++) {
				if ((y == 0) || rand() % 100 == 1) {
					mass[x + 50][y][z + 50] = true;
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
	bindObject(vertices_cube, indices_cube, &vbo_cube_handle, &vao_cube_handle, &shader_cube);
	
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
	bindObject(vertices_piramide, indices_piramide, &vbo_piramide_handle, &vao_piramide_handle, &shader_piramide);

	gl::GLfloat vertices_piramide_litle[] = {
		0.5, 0.0, 0.5,        1.0, 1.0,		1.0, 0.0, 1.0,
		0.5, 0.0, -0.5,	      0.0, 1.0,		1.0, 0.0, -1.0,
		-0.5, 0.0, -0.5,      1.0, 1.0,		-1.0, 0.0, -1.0,
		-0.5, 0.0, 0.5,       1.0, 0.0,		-1.0, 0.0, 1.0,
		0.0, 0.5, 0.0,		  0.0, 0.0,		0.0, 1.0, 0.0
	};
	gl::GLushort indices_piramide_litle[] = { 3, 2, 1,	 3, 1, 0,	3, 0, 4,	0, 1, 4,	1, 2, 4,	2, 3, 4 };
	bindObject(vertices_piramide_litle, indices_piramide_litle, &vbo_piramide_litle_handle, &vao_piramide_litle_handle, &shader_litle_piramide);

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

	deltaTime = delta_time * 1.1;
	camera.update(deltaTime, mass);

	switch (objectType) {
	case(cube):
		// uaktywnienie pierwszego slotu tekstur
		gl::glActiveTexture(gl::GL_TEXTURE0);
		// zbindowanie tekstury do aktywnego slotu
		gl::glBindTexture(gl::GL_TEXTURE_2D, tex_handle);
		drawObjects(&vao_cube_handle, 36, true, true, &shader_cube);
		break;
	case(piramide):
		// uaktywnienie pierwszego slotu tekstur
		gl::glActiveTexture(gl::GL_TEXTURE0);
		// zbindowanie tekstury do aktywnego slotu
		gl::glBindTexture(gl::GL_TEXTURE_2D, tex_piramide_handle);
		drawObjects(&vao_piramide_handle, 18, true, true, &shader_piramide);
		break;
	case(litlePiramide):
		drawObjects(&vao_piramide_litle_handle, 18, false, false, &shader_litle_piramide);
		break;
	}
	
	//skybox
	drawSkybox();

	return true;
}

void SampleApp::drawObjects(gl::GLuint *vao, gl::GLsizei size, bool isHasLights, bool isUseTexture, Shader *shader) {
	// ustawienie programu, ktory bedzie uzywany podczas rysowania
	shader->use();
	// zbindowanie VAO modelu, ktorego bedziemy renderowac
	gl::glBindVertexArray(*vao);

	std::array<ModelMatrices, 1000u> array;
	int index = 0;
	for (int x = 0; x < 100; x++)
		for (int y = 0; y < 100; y++)
			for (int z = 0; z < 100; z++) {
				if (!mass[x][y][z]) continue;
					ModelMatrices matrices = ModelMatrices();
					matrices.model_matrix = translationMatrix(glm::vec3(0.0f + x, 0.0f + y, 0.0f + z));
					matrices.mvp_matrix = projection_matrix * camera.GetViewMatrix() * matrices.model_matrix;
					array[index] = matrices;
					index++;
			}

	Option option = Option();
	option.isHasLights = isHasLights;
	option.isUseTexture = isUseTexture;
	shader->sendData(option, ubo_option);
	if (isHasLights)
		shader->sendData(camera.Position, ubo_camera_position);
	shader->sendData(array, ubo_mvp_matrix_handle);

	// rozpoczynamy rysowanie uzywajac ustawionego programu (shader-ow) i ustawionych buforow
	gl::glDrawElementsInstanced(gl::GL_TRIANGLES, size, gl::GL_UNSIGNED_SHORT, nullptr, elementsCount);
	gl::glBindVertexArray(0);
}

void SampleApp::bindObject(gl::GLfloat vertices[], gl::GLushort indices[], gl::GLuint *vbo, gl::GLuint *vao, Shader *shader) {
	std::cout << "Shaders compilation..." << std::endl;
	// wczytanie z plikow i skompilowanie shaderow oraz utworzenie programu (VS + FS)
	char* vs_path = "../../../dv_project/shaders/simple_lights_vs.glsl";
	char* fs_path = "../../../dv_project/shaders/simple_lights_fs.glsl";
#if WIN32
	vs_path = "C:/Users/Mark/Desktop/dv_project/shaders/simple_lights_vs.glsl";
	fs_path = "C:/Users/Mark/Desktop/dv_project/shaders/simple_lights_fs.glsl";
#endif
	*shader = Shader(vs_path, fs_path);

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
	shader->use();

	std::cout << "Generating buffers..." << std::endl;
	shader->createVAO(vao);
	shader->createVBO(vbo, vertices, 288);
	shader->createIndexBuffer(indices, 36, &index_buffer_handle);
	shader->bindVBOandIBtoVAO(vertex_position_loction, vertex_tex_uv_loction, vertex_normal_loction, &index_buffer_handle);
	shader->unbindVAOandVBO();

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

	shader->createBuffer(ambient_light_color, ub_ambient_light_binding_index, &ubo_ambient_light);
	shader->createBuffer(lights, ub_point_light_binding_index, &ubo_point_light);
	shader->createBuffer(material, ub_material_binding_index, &ubo_material);
	shader->createBuffer(0, ub_additional_data_binding_index, &ubo_camera_position);
	shader->createBuffer(0, ub_mvp_binding_index, &ubo_mvp_matrix_handle);
	shader->createBuffer(0, ub_option_binding_index, &ubo_option);
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

    // odbindowanie IB
    gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);
    if (index_buffer_handle)
    {
        // usuniecie IB
        gl::glDeleteBuffers(1, &index_buffer_handle);
        index_buffer_handle = 0u;
    }

	shader_cube.deleteProgram();
	shader_piramide.deleteProgram();
	shader_litle_piramide.deleteProgram();
	shader_sky.deleteProgram();
}
