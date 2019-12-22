#pragma once

#include "libs.h"

class SampleApp : public OGLAppFramework::OGLApplication
{
	gl::GLuint tex_so;
	gl::GLuint tex_handle;
    gl::GLuint simple_program;
    gl::GLuint vbo_handle;
    gl::GLuint index_buffer_handle;
    gl::GLuint vao_handle;
    gl::GLuint ubo_mvp_matrix_handle;
	gl::GLuint ubo_intensity_handle;
	gl::GLuint ubo_ambient_light;
	gl::GLuint ubo_point_light;
	gl::GLuint ubo_camera_position;
	gl::GLuint ubo_material;

	glm::mat4x4 projection_matrix;
	glm::vec3 camera_position;
	glm::vec3 camera_front;
	glm::vec3 camera_up;

public:
    SampleApp();
    virtual ~SampleApp() override;
	template <typename T>
	void sendData(T object, gl::GLuint handle);
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
