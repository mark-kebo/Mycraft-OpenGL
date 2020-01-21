#include "libs.h"

#include <fstream>
#include <sstream>

// TODO: Move implementation to CPP file.

namespace OGLAppFramework {

	class Shader {
	public:
		unsigned int ID;
		gl::GLuint *vao; gl::GLuint *vbo;

		Shader() = default;

		Shader(const char* vertexPath, const char* fragmentPath) {
			if (auto create_program_result = OGLAppFramework::createProgram(vertexPath, fragmentPath)) {
				ID = create_program_result.value();
			}
			else {
				std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
			}
		}

		void use() {
			gl::glUseProgram(ID);
		}

		void deleteProgram() {
			// ustawienie aktywnego programu na 0 (zaden)
			gl::glUseProgram(0);

			// usuniecie programu
			gl::glDeleteProgram(ID);
		}

		template <typename T>
		void sendData(T object, gl::GLuint handle) {
			use();
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

		template <typename T>
		void createBuffer(T object, gl::GLuint index, gl::GLuint *handle) {
			use();
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

		void createVBO(gl::GLuint *vbo, gl::GLfloat vertices[], int count) {
			this->vbo = vbo;
			// stworzenie bufora
			gl::glGenBuffers(1, this->vbo);
			// zbindowanie bufora jako VBO
			gl::glBindBuffer(gl::GL_ARRAY_BUFFER, *this->vbo);
			// alokacja pamieci dla bufora zbindowanego jako VBO i skopiowanie danych z tablicy "vertices"
			gl::glBufferData(gl::GL_ARRAY_BUFFER, count * sizeof(gl::GLfloat), vertices, gl::GL_STATIC_DRAW);
			// odbindowanie buffora zbindowanego jako VBO (zeby przypadkiem nie narobic sobie klopotow...)
			gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);
		}

		void createVAO(gl::GLuint *vao) {
			this->vao = vao;
			// stworzenie VAO
			gl::glGenVertexArrays(1, this->vao);
			// zbindowanie VAO
			gl::glBindVertexArray(*this->vao);
		}

		void createIndexBuffer(gl::GLushort indices [], int count, gl::GLuint *handle) {
			// stworzenie bufora
			gl::glGenBuffers(1, handle);
			// zbindowanie bufora jako IB
			gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, *handle);
			// alokacja pamieci dla bufora zbindowanego jako IB i skopiowanie danych z tablicy "indeices"
			gl::glBufferData(gl::GL_ELEMENT_ARRAY_BUFFER, sizeof(gl::GLushort) * count, indices, gl::GL_STATIC_DRAW);
			// odbindowanie buffora zbindowanego jako IB (zeby przypadkiem nie narobic sobie klopotow...)
			gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		void bindVBOandIBtoVAO(gl::GLuint vertex_position_loction, gl::GLuint vertex_tex_uv_loction, 
			gl::GLuint vertex_normal_loction, gl::GLuint *index_buffer_handle) {
			// zbindowanie VBO do aktualnego VAO
			gl::glBindBuffer(gl::GL_ARRAY_BUFFER, *this->vbo);
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
			gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, *index_buffer_handle);
		}

		void bindVBOandIBtoVAO(gl::GLuint vertex_position_loction, gl::GLuint *index_buffer_handle) {
			// zbindowanie VBO do aktualnego VAO
			gl::glBindBuffer(gl::GL_ARRAY_BUFFER, *this->vbo);
			// ustalenie jak maja byc interpretowane dane o pozycji z VBO
			gl::glVertexAttribPointer(vertex_position_loction, 3, gl::GL_FLOAT, gl::GL_FALSE, sizeof(float) * 3, nullptr);
			// odblokowanie mozliwosci wczytywania danych o pozycji z danej lokalizacji
			gl::glEnableVertexAttribArray(vertex_position_loction);
			// zbindowanie IB do aktualnego VAO
			gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, *index_buffer_handle);
		}

		void unbindVAOandVBO() {
			// odbindowanie VAO (ma ono teraz informacje m.in. o VBO + IB, wiec gdy zajdzie potrzeba uzycia VBO + IB, wystarczy zbindowac VAO)
			gl::glBindVertexArray(0u);
			// odbindowanie buffora zbindowanego jako VBO (zeby przypadkiem nie narobic sobie klopotow...)
			gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);
			// odbindowanie buffora zbindowanego jako bufor indeksow (zeby przypadkiem nie narobic sobie klopotow...)
			gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	};
}