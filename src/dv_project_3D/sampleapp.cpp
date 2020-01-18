#include "sampleapp.h"
#include "camera.h"
#include "shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace OGLAppFramework;

// settings
float SCR_WIDTH = 1366;
float SCR_HEIGHT = 768;

//scene
bool mass[100][100][100];

// camera
Camera camera(glm::vec3(8.0f, 8.0f, 8.0f));
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;

//shaders
Shader shader = Shader();
Shader shader_sky = Shader();

const glm::vec3 model_positions[3] = {
	glm::vec3(0.0f,  1.0f,  0.0f),
	glm::vec3(0.0f, 1.0f, 4.0f),
	glm::vec3(0.0f,  1.0f, -2.0f)
};

struct TextMaterial {
	glm::vec3 color;
	float specular_intensity;
	float specular_power;
};

struct PointLight {
	glm::vec3 position_ws;
	float r;
	glm::vec3 color;
};

SampleApp::SampleApp() : OGLAppFramework::OGLApplication(SCR_WIDTH, SCR_HEIGHT, "Game - 3D", 4u, 2u),
vbo_handle(0u), index_buffer_handle(0u), vao_handle_sky(0u),
vao_handle(0u), ubo_mvp_matrix_handle(0u), ubo_intensity_handle(0u), tex_handle(0u),
tex_so(0u), ubo_ambient_light(0u), tex_handle_sky(0u), index_buffer_handle_sky(0u),
ubo_point_light(0u), ubo_camera_position(0u), ubo_material(0u), vbo_handle_sky(0u), ubo_skybox(0u) {
}

SampleApp::~SampleApp() {
}

void SampleApp::reshapeCallback(std::uint16_t width, std::uint16_t height) {
	std::cout << "Reshape..." << std::endl;
	std::cout << "New window size: " << width << " x " << height << std::endl;
	projection_matrix = glm::perspective(glm::radians(camera.Zoom), (float)width / height, 0.1f, 100.0f);
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
		std::cout << "Mouse  " << camera.getBeforePlayerPosition().x << " " << camera.getBeforePlayerPosition().y << " " << camera.getBeforePlayerPosition().z << std::endl;
		if (button == 1) {
			mass[(int)camera.getBeforePlayerPosition().x][(int)camera.getBeforePlayerPosition().y][(int)camera.getBeforePlayerPosition().z] = 0;
		} else {
			mass[(int)camera.getBeforePlayerPosition().x][(int)camera.getBeforePlayerPosition().y][(int)camera.getBeforePlayerPosition().z] = 1;
		}
	}
}

bool SampleApp::init(void) {
    std::cout << "Init..." << std::endl;

	projection_matrix = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

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
	for (int x = 0; x < 10; x++)
		for (int y = 0; y < 10; y++)
			for (int z = 0; z < 10; z++) {
				if ((y == 0) || rand() % 100 == 1) mass[x][y][z] = true;
			}

	bindSkybox();
	bindObject();

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
	deltaTime = delta_time * 1.1;
	camera.update(deltaTime, mass);
	//objects
	{
		// ustawienie programu, ktory bedzie uzywany podczas rysowania
		shader.use();

		// zbindowanie VAO modelu, ktorego bedziemy renderowac
		gl::glBindVertexArray(vao_handle);
		// uaktywnienie pierwszego slotu tekstur
		gl::glActiveTexture(gl::GL_TEXTURE0);
		// zbindowanie tekstury do aktywnego slotu
		gl::glBindTexture(gl::GL_TEXTURE_2D, tex_handle);

		for (int x = 0; x < 10; x++)
			for (int y = 0; y < 10; y++)
				for (int z = 0; z < 10; z++) {
					if (!mass[x][y][z]) continue;
					glm::mat4x4 model_matrix = translationMatrix(glm::vec3(0.0f + x, 0.0f + y, 0.0f + z));
					glm::mat4x4 mvp_matrix = projection_matrix * camera.GetViewMatrix() * model_matrix;

					std::array<glm::mat4x4, 2u> matrices = { mvp_matrix, model_matrix };
					shader.sendData(matrices, ubo_mvp_matrix_handle);
					shader.sendData(camera.Position, ubo_camera_position);
					// rozpoczynamy rysowanie uzywajac ustawionego programu (shader-ow) i ustawionych buforow
					gl::glDrawElements(gl::GL_TRIANGLES, 36, gl::GL_UNSIGNED_SHORT, nullptr);
				}

		gl::glBindVertexArray(0);
	}
	
	//skybox
	drawSkybox();

	return true;
}

void SampleApp::bindObject() {
	std::cout << "Shaders compilation..." << std::endl;
	// wczytanie z plikow i skompilowanie shaderow oraz utworzenie programu (VS + FS)
	char* vs_path = "../../../dv_project/shaders/simple_lights_vs.glsl";
	char* fs_path = "../../../dv_project/shaders/simple_lights_fs.glsl";
#if WIN32
	vs_path = "C:/Users/Mark/Desktop/dv_project/shaders/simple_lights_vs.glsl";
	fs_path = "C:/Users/Mark/Desktop/dv_project/shaders/simple_lights_fs.glsl";
#endif
	shader = Shader(vs_path, fs_path);
	std::string texture_p = "../../../dv_project/data/box.dds";
#if WIN32
	texture_p = "C:/Users/Mark/Desktop/dv_project/data/box.dds";
#endif
	if (auto load_t_r = OGLAppFramework::loadTexFromFileAndCreateTO(texture_p)) {
		tex_handle = load_t_r.value();
	}
	else {
		return;
	}

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

	// ustawienie programu, ktory bedzie uzywany podczas rysowania
	shader.use();

	// stworzenie tablicy z danymi o wierzcholkach 3x (x, y, z)
	gl::GLfloat vertices [] = {
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

	// stworzenie tablicy z danymi o indeksach
	gl::GLushort indices []= { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35 };

	std::cout << "Generating buffers..." << std::endl;
	shader.createVAO(&vao_handle);
	shader.createVBO(&vbo_handle, vertices, 288);
	shader.createIndexBuffer(indices, 36, &index_buffer_handle);
	shader.bindVBOandIBtoVAO(vertex_position_loction, vertex_tex_uv_loction, vertex_normal_loction, &index_buffer_handle);
	shader.unbindVAOandVBO();

	glm::mat4x4 model_matrix = translationMatrix(glm::vec3(0.0f, 0.0f, 0.0f));
	glm::mat4x4 mvp_matrix = projection_matrix * camera.GetViewMatrix() * model_matrix;
	std::array<glm::mat4x4, 2u> matrices = { mvp_matrix, model_matrix };
	glm::vec3 ambient_light_color = glm::vec3(0.2f, 0.2f, 0.2f);
	PointLight pointLight = PointLight();
	pointLight.position_ws = glm::vec3(5.5f, 2.5f, 0.5f);
	pointLight.r = 33.5;
	pointLight.color = glm::vec3(1.f, 1.f, 1.f);
	TextMaterial material = TextMaterial();
	material.color = glm::vec3(1.f, 1.f, 1.f);
	material.specular_intensity = 1.f;
	material.specular_power = 1.f;

	shader.createBuffer(matrices, ub_mvp_binding_index, &ubo_mvp_matrix_handle);
	shader.createBuffer(ambient_light_color, ub_ambient_light_binding_index, &ubo_ambient_light);
	shader.createBuffer(pointLight, ub_point_light_binding_index, &ubo_point_light);
	shader.createBuffer(material, ub_material_binding_index, &ubo_material);
	shader.createBuffer(camera.Position, ub_additional_data_binding_index, &ubo_camera_position);
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

	// load textures
	// -------------
	std::string faces[6] = {
		"../../../dv_project/data/skybox/stormydays_ft.tga",
		"../../../dv_project/data/skybox/stormydays_bk.tga",
		"../../../dv_project/data/skybox/stormydays_up.tga",
		"../../../dv_project/data/skybox/stormydays_dn.tga",
		"../../../dv_project/data/skybox/stormydays_rt.tga",
		"../../../dv_project/data/skybox/stormydays_lf.tga"
	};
#if WIN32
	faces[0] = "C:/Users/Mark/Desktop/dv_project/data/skybox/stormydays_ft.tga";
	faces[1] = "C:/Users/Mark/Desktop/dv_project/data/skybox/stormydays_bk.tga";
	faces[2] = "C:/Users/Mark/Desktop/dv_project/data/skybox/stormydays_up.tga";
	faces[3] = "C:/Users/Mark/Desktop/dv_project/data/skybox/stormydays_dn.tga";
	faces[4] = "C:/Users/Mark/Desktop/dv_project/data/skybox/stormydays_rt.tga";
	faces[5] = "C:/Users/Mark/Desktop/dv_project/data/skybox/stormydays_lf.tga";
#endif
	tex_handle_sky = loadCubemap(faces);

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
	gl::glDrawArrays(gl::GL_TRIANGLES, 0, 36);
	gl::glDepthFunc(gl::GL_LESS); // set depth function back to default
	gl::glBindVertexArray(0);
}

unsigned int SampleApp::loadCubemap(std::string faces[])
{
	unsigned int textureID;
	//gl::glCreateTextures(gl::GL_TEXTURE_CUBE_MAP, 1, &textureID);
	gl::glGenTextures(1, &textureID);
	gl::glBindTexture(gl::GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < 6; i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			gl::glTexImage2D(gl::GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, gl::GL_RGB,
				width, height, 0, gl::GL_RGB, gl::GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	gl::glTexParameteri(gl::GL_TEXTURE_CUBE_MAP, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR);
	gl::glTexParameteri(gl::GL_TEXTURE_CUBE_MAP, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
	gl::glTexParameteri(gl::GL_TEXTURE_CUBE_MAP, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
	gl::glTexParameteri(gl::GL_TEXTURE_CUBE_MAP, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
	gl::glTexParameteri(gl::GL_TEXTURE_CUBE_MAP, gl::GL_TEXTURE_WRAP_R, gl::GL_CLAMP_TO_EDGE);

	return textureID;
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

    // odbindowanie UBO
    gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
    if(ubo_mvp_matrix_handle)
    {
        // usuniecie UBO
        gl::glDeleteBuffers(1, &ubo_mvp_matrix_handle);
        ubo_mvp_matrix_handle = 0u;
    }

    // odbindowanie VAO
    gl::glBindVertexArray(0);
    if (vao_handle)
    {
        // usuniecie VAO
        gl::glDeleteVertexArrays(1, &vao_handle);
        vao_handle = 0u;
    }

    // odbindowanie VBO
    gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);
    if (vbo_handle)
    {
        // usuniecie VBO
        gl::glDeleteBuffers(1, &vbo_handle);
        vbo_handle = 0u;
    }

    // odbindowanie IB
    gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);
    if (index_buffer_handle)
    {
        // usuniecie IB
        gl::glDeleteBuffers(1, &index_buffer_handle);
        index_buffer_handle = 0u;
    }

	shader.deleteProgram();
	shader_sky.deleteProgram();
}
