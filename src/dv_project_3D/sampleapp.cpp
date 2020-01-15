#include "sampleapp.h"
#include "camera.h"
#include "shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace OGLAppFramework;

const float PI = 3.141592653;

// settings
const unsigned int SCR_WIDTH = 1366;
const unsigned int SCR_HEIGHT = 768;

//scene
bool mass[1000][1000][1000];

// camera
Camera camera(glm::vec3(8.0f, 20.0f, 8.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
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
ubo_point_light(0u), ubo_camera_position(0u), ubo_material(0u), vbo_handle_sky(0u),
ubo_projection_sky(0u), ubo_view_sky(0u) {
}

SampleApp::~SampleApp() {
}

void SampleApp::reshapeCallback(std::uint16_t width, std::uint16_t height) {
	std::cout << "Reshape..." << std::endl;
	std::cout << "New window size: " << width << " x " << height << std::endl;
	projection_matrix = glm::perspective(glm::radians(camera.Zoom), (float)width / height, 0.1f, 100.0f);
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
		float x = camera.Position.x + 2;
		float y = camera.Position.y;
		float z = camera.Position.z;

		int X, Y, Z;
		X = (int)x; Y = (int)y; Z = (int)z;
		int dist = 0;
		if (button == 1) {
			std::cout << "Mouse button pressed " << x << " " << y << " " << z << std::endl;

			mass[X][Y][Z] = 0;
		}
		else {
			std::cout << "Mouse button pressed " << X << " " << Y << " " << Z << std::endl;

			mass[X][Y][Z] = 1;
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
		for (int y = 0; y < 20; y++)
			for (int z = 0; z < 10; z++) {
				if ((y == 0) || rand() % 100 == 1) mass[x][y][z] = true;
			}

	bindObject();
	//bindSkybox();
	
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
	static float angle = 0.f;
	angle += delta_time * 1.5;
	deltaTime = delta_time * 1.1;
	camera.update(deltaTime, mass);
	//objects
	{
		// ustawienie programu, ktory bedzie uzywany podczas rysowania
		shader.use();

		// zbindowanie VAO modelu, ktorego bedziemy renderowac
		gl::glBindVertexArray(vao_handle);
		for (int x = 0; x < 10; x++)
			for (int y = 0; y < 10; y++)
				for (int z = 0; z < 10; z++) {
					if (!mass[x][y][z]) continue;
					glm::mat4x4 model_matrix = translationMatrix(glm::vec3(0.0f + x, 0.0f + y, 0.0f + z));
					glm::mat4x4 mvp_matrix = projection_matrix * camera.GetViewMatrix() * model_matrix;

					// uaktywnienie pierwszego slotu tekstur
					gl::glActiveTexture(gl::GL_TEXTURE0);
					// zbindowanie tekstury do aktywnego slotu
					gl::glBindTexture(gl::GL_TEXTURE_2D, tex_handle);
					//1 piramide send
					std::array<glm::mat4x4, 2u> matrices_1 = { mvp_matrix, model_matrix };
					sendData(matrices_1, ubo_mvp_matrix_handle);
					sendData(camera.Position, ubo_camera_position);
					// rozpoczynamy rysowanie uzywajac ustawionego programu (shader-ow) i ustawionych buforow
					gl::glDrawElements(gl::GL_TRIANGLES, 36, gl::GL_UNSIGNED_SHORT, nullptr);
				}

		gl::glBindVertexArray(0);
	}
	
	//skybox
	//drawSkybox();

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
	std::array<gl::GLfloat, 288u> vertices = {
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
	std::array<gl::GLushort, 36u> indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35 };

	std::cout << "Generating buffers..." << std::endl;
	// stworzenie bufora
	gl::glGenBuffers(1, &vbo_handle);
	// zbindowanie bufora jako VBO
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, vbo_handle);
	// alokacja pamieci dla bufora zbindowanego jako VBO i skopiowanie danych z tablicy "vertices"
	gl::glBufferData(gl::GL_ARRAY_BUFFER, vertices.size() * sizeof(gl::GLfloat), vertices.data(), gl::GL_STATIC_DRAW);
	// odbindowanie buffora zbindowanego jako VBO (zeby przypadkiem nie narobic sobie klopotow...)
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);

	// stworzenie bufora
	gl::glGenBuffers(1, &index_buffer_handle);
	// zbindowanie bufora jako IB
	gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, index_buffer_handle);
	// alokacja pamieci dla bufora zbindowanego jako IB i skopiowanie danych z tablicy "indeices"
	gl::glBufferData(gl::GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(gl::GLushort), indices.data(), gl::GL_STATIC_DRAW);
	// odbindowanie buffora zbindowanego jako IB (zeby przypadkiem nie narobic sobie klopotow...)
	gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);

	// stworzenie VAO
	gl::glGenVertexArrays(1, &vao_handle);
	// zbindowanie VAO
	gl::glBindVertexArray(vao_handle);

	// zbindowanie VBO do aktualnego VAO
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, vbo_handle);
	// ustalenie jak maja byc interpretowane dane o pozycji z VBO
	gl::glVertexAttribPointer(vertex_position_loction, 3, gl::GL_FLOAT, gl::GL_FALSE, sizeof(float) * 8, nullptr);
	// odblokowanie mozliwosci wczytywania danych o pozycji z danej lokalizacji
	gl::glEnableVertexAttribArray(vertex_position_loction);
	// ustalenie jak maja byc interpretowane dane o wspolrzednych uv z VBO
	gl::glVertexAttribPointer(vertex_tex_uv_loction, 2, gl::GL_FLOAT, gl::GL_FALSE, sizeof(float) * 8, reinterpret_cast<const gl::GLvoid*>(sizeof(float) * 3));
	// odblokowanie mozliwosci wczytywania danych o wspolrzednych uv z danej lokalizacji
	gl::glEnableVertexAttribArray(vertex_tex_uv_loction);
	// ustalenie jak maja byc interpretowane dane o wektorach normalnych z VBO
	gl::glVertexAttribPointer(vertex_normal_loction, 3, gl::GL_FLOAT, gl::GL_FALSE, sizeof(float) * 8, reinterpret_cast<const gl::GLvoid*>(sizeof(float) * 5));
	// odblokowanie mozliwosci wczytywania danych o wektorach noramlnych z danej lokalizacji
	gl::glEnableVertexAttribArray(vertex_normal_loction);
	// zbindowanie IB do aktualnego VAO
	gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, index_buffer_handle);
	// odbindowanie VAO (ma ono teraz informacje m.in. o VBO + IB, wiec gdy zajdzie potrzeba uzycia VBO + IB, wystarczy zbindowac VAO)
	gl::glBindVertexArray(0u);
	// odbindowanie buffora zbindowanego jako VBO (zeby przypadkiem nie narobic sobie klopotow...)
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);
	// odbindowanie buffora zbindowanego jako bufor indeksow (zeby przypadkiem nie narobic sobie klopotow...)
	gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);

	createBuffer(ub_mvp_binding_index, &ubo_mvp_matrix_handle);

	glm::vec3 ambient_light_color = glm::vec3(0.2f, 0.2f, 0.2f);
	createBuffer(ambient_light_color, ub_ambient_light_binding_index, &ubo_ambient_light);

	PointLight pointLight = PointLight();
	pointLight.position_ws = glm::vec3(5.5f, 2.5f, 0.5f);
	pointLight.r = 33.5;
	pointLight.color = glm::vec3(1.f, 1.f, 1.f);
	createBuffer(pointLight, ub_point_light_binding_index, &ubo_point_light);

	TextMaterial material = TextMaterial();
	material.color = glm::vec3(1.f, 1.f, 1.f);
	material.specular_intensity = 1.f;
	material.specular_power = 1.f;
	createBuffer(material, ub_material_binding_index, &ubo_material);

	createBuffer(ub_additional_data_binding_index, &ubo_camera_position);
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
	shader_sky = Shader(vs_path, fs_path);
	// ustawienie programu, ktory bedzie uzywany podczas rysowania
	shader_sky.use();

	std::array<gl::GLfloat, 108u> vertices = {
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

	std::array<gl::GLushort, 36u> indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
	18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};

	std::cout << "Generating skybox buffers..." << std::endl;
	// stworzenie bufora
	gl::glGenBuffers(1, &vbo_handle_sky);
	// zbindowanie bufora jako VBO
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, vbo_handle_sky);
	// alokacja pamieci dla bufora zbindowanego jako VBO i skopiowanie danych z tablicy "vertices"
	gl::glBufferData(gl::GL_ARRAY_BUFFER, vertices.size() * sizeof(gl::GLfloat), vertices.data(), gl::GL_STATIC_DRAW);
	// odbindowanie buffora zbindowanego jako VBO (zeby przypadkiem nie narobic sobie klopotow...)
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);

	// stworzenie bufora
	gl::glGenBuffers(1, &index_buffer_handle_sky);
	// zbindowanie bufora jako IB
	gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, index_buffer_handle_sky);
	// alokacja pamieci dla bufora zbindowanego jako IB i skopiowanie danych z tablicy "indeices"
	gl::glBufferData(gl::GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(gl::GLushort), indices.data(), gl::GL_STATIC_DRAW);
	// odbindowanie buffora zbindowanego jako IB (zeby przypadkiem nie narobic sobie klopotow...)
	gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);

	// stworzenie VAO
	gl::glGenVertexArrays(1, &vao_handle_sky);
	// zbindowanie VAO
	gl::glBindVertexArray(vao_handle_sky);

	// zbindowanie VBO do aktualnego VAO
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, vbo_handle_sky);
	// ustalenie jak maja byc interpretowane dane o pozycji z VBO
	gl::glVertexAttribPointer(0u, 3, gl::GL_FLOAT, gl::GL_FALSE, sizeof(float) * 3, nullptr);
	// odblokowanie mozliwosci wczytywania danych o pozycji z danej lokalizacji
	gl::glEnableVertexAttribArray(0u);
	// zbindowanie IB do aktualnego VAO
	gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, index_buffer_handle_sky);
	// odbindowanie VAO (ma ono teraz informacje m.in. o VBO + IB, wiec gdy zajdzie potrzeba uzycia VBO + IB, wystarczy zbindowac VAO)
	gl::glBindVertexArray(0u);
	// odbindowanie buffora zbindowanego jako VBO (zeby przypadkiem nie narobic sobie klopotow...)
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);
	// odbindowanie buffora zbindowanego jako bufor indeksow (zeby przypadkiem nie narobic sobie klopotow...)
	gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);

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

	const gl::GLuint ub_projection_index = 1u;
	const gl::GLuint ub_view_index = 2u;

	createBuffer(ub_projection_index, &ubo_projection_sky);
	createBuffer(ub_view_index, &ubo_view_sky);
}

void SampleApp::drawSkybox() {
	// ustawienie programu, ktory bedzie uzywany podczas rysowania
	shader_sky.use();
	// skybox cube
	gl::glBindVertexArray(vao_handle_sky);
	glm::mat4x4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	gl::glDepthFunc(gl::GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
	sendData(view, ubo_view_sky);
	sendData(projection, ubo_projection_sky);
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

template <typename T>
void SampleApp::sendData(T object, gl::GLuint handle) {
	// zbindowanie bufora jako UBO
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, handle);
	// stworzenie bufora sierocego dla bufora zbindowanego jako UBO
	gl::glBufferData(gl::GL_UNIFORM_BUFFER, sizeof(object), nullptr, gl::GL_DYNAMIC_DRAW);
	// mapowanie pamieci bufora zbindowanego jako UBO z mozliwoscia nadpisywania danych
	if (void *data = gl::glMapBuffer(gl::GL_UNIFORM_BUFFER, gl::GL_WRITE_ONLY)) {
		// nadpisanie danych w buforze
		*reinterpret_cast<T*>(data) = object;
		// odmapowanie pamieci bufora zbindowanego jako UBO
		gl::glUnmapBuffer(gl::GL_UNIFORM_BUFFER);
	}
	// odbindowanie buffora zbindowanego jako UBO (zeby przypadkiem nie narobic sobie klopotow...)
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
}
void SampleApp::createBuffer(gl::GLuint index, gl::GLuint *handle) {
	// stworzenie bufora
	gl::glGenBuffers(1, handle);
	// zbindowanie bufora jako UBO
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, *handle);
	// odbindowanie buffora zbindowanego jako UBO (zeby przypadkiem nie narobic sobie klopotow...)
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
	// przyporzadkowanie UBO do indeksu bindowania unform block-u
	gl::glBindBufferBase(gl::GL_UNIFORM_BUFFER, index, *handle);
}
template <typename T>
void SampleApp::createBuffer(T object, gl::GLuint index, gl::GLuint *handle) {
	// stworzenie bufora
	gl::glGenBuffers(1, handle);
	// zbindowanie bufora jako UBO
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, *handle);
	// alokacja pamieci dla bufora zbindowanego jako UBO i skopiowanie danych
	gl::glBufferData(gl::GL_UNIFORM_BUFFER, sizeof(object), &object, gl::GL_DYNAMIC_DRAW);
	// odbindowanie buffora zbindowanego jako UBO (zeby przypadkiem nie narobic sobie klopotow...)
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
	// przyporzadkowanie UBO do indeksu bindowania unform block-u
	gl::glBindBufferBase(gl::GL_UNIFORM_BUFFER, index, *handle);
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
	if (ubo_projection_sky)
	{
		// usuniecie UBO
		gl::glDeleteBuffers(1, &ubo_projection_sky);
		ubo_projection_sky = 0u;
	}
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
	if (ubo_view_sky)
	{
		// usuniecie UBO
		gl::glDeleteBuffers(1, &ubo_view_sky);
		ubo_view_sky = 0u;
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
