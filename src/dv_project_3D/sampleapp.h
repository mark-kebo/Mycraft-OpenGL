#pragma once

#include "libs.h"
#include "camera.h"
#include "shader.h"

enum ObjectType { piramide, litlePiramide, cube, smallCube };
enum Direction { forward, back, left, right, diagonalForward, diagonalBack };

static bool mass[100][100][100];

class SampleApp : public OGLAppFramework::OGLApplication
{
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

	gl::GLuint vao_cube_handle;
	gl::GLuint vbo_cube_handle;
	gl::GLuint vao_small_cube_handle;
	gl::GLuint vbo_small_cube_handle;
	gl::GLuint vao_piramide_handle;
	gl::GLuint vbo_piramide_handle;
	gl::GLuint vao_piramide_litle_handle;
	gl::GLuint vbo_piramide_litle_handle;
	gl::GLuint tex_so;
	gl::GLuint tex_handle;
	gl::GLuint tex_piramide_handle;
    gl::GLuint index_buffer_handle;
    gl::GLuint ubo_mvp_matrix_handle;
	gl::GLuint ubo_intensity_handle;
	gl::GLuint ubo_ambient_light;
	gl::GLuint ubo_point_light;
	gl::GLuint ubo_camera_position;
	gl::GLuint ubo_material;
	gl::GLuint ubo_option;

	gl::GLuint tex_handle_sky;
	gl::GLuint index_buffer_handle_sky;
	gl::GLuint vao_handle_sky;
	gl::GLuint vbo_handle_sky;
	gl::GLuint ubo_skybox;

	glm::mat4x4 projection_matrix;
	
	float SCR_WIDTH;
	float SCR_HEIGHT;
	Shader shader_object;
	Shader shader_sky;
	float deltaTime;
	int elementsCount;
	bool firstMouse;
	Camera camera;
	ObjectType objectType;
	Direction directionObject;
	float lastX;
	float lastY;

public:
    SampleApp();
    virtual ~SampleApp() override;

	void drawObjects(gl::GLuint *vao, gl::GLsizei size, bool isHasLights, bool isUseTexture);
	void drawObject(gl::GLuint *vao, gl::GLsizei size, bool isHasLights, bool isUseTexture, const glm::vec3& translation);
	void bindObject(gl::GLfloat vertices[], gl::GLushort indices[], gl::GLuint *vbo, gl::GLuint *vao);
	void bindSkybox();
	void drawSkybox();
	void useTexture(gl::GLuint *id);

    // metoda wywolywana podczas zmiany rozmiaru okna
    virtual void reshapeCallback(std::uint16_t width, std::uint16_t height) override;
    // metoda wywolywana podczas wcisniecia przycisku
    virtual void keyCallback(int key, int scancode, int action, int mods) override;
    // metoda wywolywana podczas zmiany pozycji kursora myszy
    virtual void cursorPosCallback(double xpos, double ypos) override;
    // metoda wywolywana podczas wcisniecia przycisku myszy
    virtual void mouseButtonCallback(int button, int action, int mods) override;
    // metoda wywolywana na poczatku (przy starcie aplikacji, po inicjalizacji OGL)
    virtual bool init(void) override;
    // metoda wywolywana co klatke
    virtual bool frame(float delta_time) override;
    // metoda wywolywana przy zamknieciu aplikacji
    virtual void release() override;
};
