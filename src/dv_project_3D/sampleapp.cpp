#include "sampleapp.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace OGLAppFramework;

// settings
const unsigned int SCR_WIDTH = 1366;
const unsigned int SCR_HEIGHT = 768;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;

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
simple_program(0u), vbo_handle(0u), index_buffer_handle(0u), 
vao_handle(0u), ubo_mvp_matrix_handle(0u), ubo_intensity_handle(0u), tex_handle(0u),
tex_so(0u), ubo_ambient_light(0u),
ubo_point_light(0u), ubo_camera_position(0u), ubo_material(0u) {
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
    //std::cout << "Key pressed" << std::endl;
	if (key == GLFW_KEY_W)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (key == GLFW_KEY_S)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (key == GLFW_KEY_D)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (key == GLFW_KEY_A)
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
    //std::cout << "Mouse button pressed" << std::endl;
}

bool SampleApp::init(void) {
    std::cout << "Init..." << std::endl;

	projection_matrix = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    // ustalamy domyślny kolor ekranu
    gl::glClearColor(0.f, 0.f, 0.2f, 1.f);

    // wlaczmy renderowanie tylko jednej strony poligon-ow
    //gl::glEnable(gl::GL_CULL_FACE);
    // ustalamy, ktora strona jest "przodem"
    gl::glFrontFace(gl::GL_CCW);
    // ustalamy, ktorej strony nie bedziemy renderowac
    gl::glCullFace(gl::GL_BACK);
	gl::glEnable(gl::GL_DEPTH_TEST);

	bindObject();

	// Tworzenie SO
	gl::glGenSamplers(1, &tex_so);
	// Ustawienie parametrów samplowania
	gl::glSamplerParameteri(tex_so, gl::GL_TEXTURE_WRAP_S, gl::GL_REPEAT);
	gl::glSamplerParameteri(tex_so, gl::GL_TEXTURE_WRAP_T, gl::GL_REPEAT);
	gl::glSamplerParameteri(tex_so, gl::GL_TEXTURE_WRAP_R, gl::GL_REPEAT);
	gl::glSamplerParameteri(tex_so, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR_MIPMAP_LINEAR);
	gl::glSamplerParameteri(tex_so, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
	gl::glBindSampler(0u, tex_so);
	// uaktywnienie pierwszego slotu tekstur
	gl::glActiveTexture(gl::GL_TEXTURE0);
	// zbindowanie tekstury do aktywnego slotu
	gl::glBindTexture(gl::GL_TEXTURE_2D, tex_handle);
	return true;
}

bool SampleApp::frame(float delta_time) {
	static float angle = 0.f;
	angle += delta_time * 1.5;
	deltaTime = delta_time * 2;

	//1 piramide mvp
	glm::mat4x4 model_matrix_1 = translationMatrix(model_positions[0]) * 
									rotationMatrix(angle, model_positions[0]);
	glm::mat4x4 mvp_matrix_1 = projection_matrix * camera.GetViewMatrix() * model_matrix_1;
	//2 piramide mvp
	glm::mat4x4 model_matrix_2 = rotationMatrix(angle, model_positions[0]) * 
									translationMatrix(model_positions[1]);
	glm::mat4x4 mvp_matrix_2 = projection_matrix * camera.GetViewMatrix() * model_matrix_2;
	//3 piramide mvp
	glm::mat4x4 model_matrix_3 = rotationMatrix(angle, model_positions[0]) *
									translationMatrix(glm::vec3(model_positions[1].x, 0.0f, model_positions[1].z)) *
									rotationMatrix(angle, model_positions[0]) *
									translationMatrix(model_positions[2]);
	glm::mat4x4 mvp_matrix_3 = projection_matrix * camera.GetViewMatrix() * model_matrix_3;

	// zbindowanie VAO modelu, ktorego bedziemy renderowac
	gl::glBindVertexArray(vao_handle);

	//1 piramide send
	std::array<glm::mat4x4, 2u> matrices_1 = { mvp_matrix_1, model_matrix_1 };
	sendData(matrices_1, ubo_mvp_matrix_handle);
	sendData(camera_position, ubo_camera_position);
	// rozpoczynamy rysowanie uzywajac ustawionego programu (shader-ow) i ustawionych buforow
	gl::glDrawElements(gl::GL_TRIANGLES, 18, gl::GL_UNSIGNED_SHORT, nullptr);
	
	//2 piramide send
	std::array<glm::mat4x4, 2u> matrices_2 = { mvp_matrix_2, model_matrix_2 };
	sendData(matrices_2, ubo_mvp_matrix_handle);
	sendData(camera_position, ubo_camera_position);
	// rozpoczynamy rysowanie uzywajac ustawionego programu (shader-ow) i ustawionych buforow
	gl::glDrawElements(gl::GL_TRIANGLES, 18, gl::GL_UNSIGNED_SHORT, nullptr);
	
	//3 piramide send
	std::array<glm::mat4x4, 2u> matrices_3 = { mvp_matrix_3, model_matrix_3 };
	sendData(matrices_3, ubo_mvp_matrix_handle);
	sendData(camera_position, ubo_camera_position);
	// rozpoczynamy rysowanie uzywajac ustawionego programu (shader-ow) i ustawionych buforow
	gl::glDrawElements(gl::GL_TRIANGLES, 18, gl::GL_UNSIGNED_SHORT, nullptr);

	return true;
}

void SampleApp::bindObject() {

	std::cout << "Shaders compilation..." << std::endl;
	// wczytanie z plikow i skompilowanie shaderow oraz utworzenie programu (VS + FS)
	std::string vs_path = "../../../dv_project/shaders/simple_lights_vs.glsl";
	std::string fs_path = "../../../dv_project/shaders/simple_lights_fs.glsl";
#if WIN32
	vs_path = "C:/Users/Mark/Desktop/dv_project/shaders/simple_lights_vs.glsl";
	fs_path = "C:/Users/Mark/Desktop/dv_project/shaders/simple_lights_fs.glsl";
#endif
	if (auto create_program_result = OGLAppFramework::createProgram(vs_path, fs_path)) {
		simple_program = create_program_result.value();
	}
	else {
		std::cerr << "Error - can't create program..." << std::endl;
		return;
	}
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
	gl::glUseProgram(simple_program);

	//	// stworzenie tablicy z danymi o wierzcholkach 3x (x, y, z)
	std::array<gl::GLfloat, 144u> vertices = {
		//		// Triangle 1
				0.0,0.5,0.0,        1.0,0.0,	0.0,1.0,1.0,
				-0.5,-0.5,0.5,      1.0,1.0,	0.0,1.0,1.0,
				0.5,-0.5,0.5,       0.0,1.0,	0.0,1.0,1.0,
				//		//Triangle 2
						0.0,0.5,0.0,        1.0,0.0,	1.0,1.0,0.0,
						0.5,-0.5,0.5,       1.0,1.0,	1.0,1.0,0.0,
						0.5,-0.5,-0.5,      0.0,1.0,	1.0,1.0,0.0,
						//		//Triangle 3
								0.0,0.5,0.0,        1.0,0.0,	0.0,1.0,-1.0,
								0.5,-0.5,-0.5,      1.0,1.0,	0.0,1.0,-1.0,
								-0.5,-0.5,-0.5,     0.0,1.0,	0.0,1.0,-1.0,
								//		//Triangle 4
										0.0,0.5,0.0,        1.0,0.0,	-1.0,1.0,0.0,
										-0.5,-0.5,-0.5,     1.0,1.0,	-1.0,1.0,0.0,
										-0.5,-0.5,0.5,      0.0,1.0,	-1.0,1.0,0.0,
										//		//Triangle 5-----
												0.5,-0.5,-0.5,      1.0,0.0,	0.0,-1.0,0.0,
												0.5,-0.5,0.5,       1.0,1.0,	0.0,-1.0,0.0,
												-0.5,-0.5,0.5,      0.0,1.0,	0.0,-1.0,0.0,
												//		//Triangle 6
														 0.5,-0.5,-0.5,     1.0,0.0,	0.0,-1.0,0.0,
														-0.5,-0.5,0.5,      0.0,1.0,	0.0,-1.0,0.0,
														-0.5,-0.5,-0.5,     1.0,1.0,	0.0,-1.0,0.0
	};

	//	// stworzenie tablicy z danymi o indeksach
	std::array<gl::GLushort, 18u> indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };

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

	// stworzenie bufora
	gl::glGenBuffers(1, &ubo_mvp_matrix_handle);
	// zbindowanie bufora jako UBO
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, ubo_mvp_matrix_handle);

	// stworzenie bufora
	gl::glGenBuffers(1, &ubo_ambient_light);
	// zbindowanie bufora jako UBO
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, ubo_ambient_light);
	// przygotowanie danych dla GPU
	glm::vec3 ambient_light_color = glm::vec3(0.2f, 0.2f, 0.2f);
	// alokacja pamieci dla bufora zbindowanego jako UBO i skopiowanie danych
	gl::glBufferData(gl::GL_UNIFORM_BUFFER, sizeof(ambient_light_color), &ambient_light_color, gl::GL_DYNAMIC_DRAW);
	// odbindowanie buffora zbindowanego jako UBO (zeby przypadkiem nie narobic sobie klopotow...)
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);

	// stworzenie bufora
	gl::glGenBuffers(1, &ubo_point_light);
	// zbindowanie bufora jako UBO
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, ubo_point_light);
	// przygotowanie danych dla GPU
	PointLight pointLight = PointLight();
	pointLight.position_ws = glm::vec3(5.5f, 2.5f, 0.5f);
	pointLight.r = 33.5;
	pointLight.color = glm::vec3(1.f, 0.f, 0.f);
	// alokacja pamieci dla bufora zbindowanego jako UBO i skopiowanie danych
	gl::glBufferData(gl::GL_UNIFORM_BUFFER, sizeof(pointLight), &pointLight, gl::GL_DYNAMIC_DRAW);
	// odbindowanie buffora zbindowanego jako UBO (zeby przypadkiem nie narobic sobie klopotow...)
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);

	// stworzenie bufora
	gl::glGenBuffers(1, &ubo_material);
	// zbindowanie bufora jako UBO
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, ubo_material);
	TextMaterial material = TextMaterial();
	material.color = glm::vec3(1.f, 1.f, 1.f);
	material.specular_intensity = 1.f;
	material.specular_power = 1.f;
	// alokacja pamieci dla bufora zbindowanego jako UBO i skopiowanie danych
	gl::glBufferData(gl::GL_UNIFORM_BUFFER, sizeof(material), &material, gl::GL_DYNAMIC_DRAW);
	// odbindowanie buffora zbindowanego jako UBO (zeby przypadkiem nie narobic sobie klopotow...)
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);

	// stworzenie bufora
	gl::glGenBuffers(1, &ubo_camera_position);
	// zbindowanie bufora jako UBO
	gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, ubo_camera_position);

	// przyporzadkowanie UBO do indeksu bindowania unform block-u
	gl::glBindBufferBase(gl::GL_UNIFORM_BUFFER, ub_mvp_binding_index, ubo_mvp_matrix_handle);
	// przyporzadkowanie UBO do indeksu bindowania unform block-u
	gl::glBindBufferBase(gl::GL_UNIFORM_BUFFER, ub_ambient_light_binding_index, ubo_ambient_light);
	// przyporzadkowanie UBO do indeksu bindowania unform block-u
	gl::glBindBufferBase(gl::GL_UNIFORM_BUFFER, ub_point_light_binding_index, ubo_point_light);
	// przyporzadkowanie UBO do indeksu bindowania unform block-u
	gl::glBindBufferBase(gl::GL_UNIFORM_BUFFER, ub_additional_data_binding_index, ubo_camera_position);
	// przyporzadkowanie UBO do indeksu bindowania unform block-u
	gl::glBindBufferBase(gl::GL_UNIFORM_BUFFER, ub_material_binding_index, ubo_material);

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

void SampleApp::release(void) {
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
    glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);
    if (index_buffer_handle)
    {
        // usuniecie IB
        gl::glDeleteBuffers(1, &index_buffer_handle);
        index_buffer_handle = 0u;
    }

    // ustawienie aktywnego programu na 0 (zaden)
    gl::glUseProgram(0);

    // usuniecie programu
    gl::glDeleteProgram(simple_program);
}
