#include "libs.h"

#include <fstream>
#include <sstream>

// TODO: Move implementation to CPP file.

namespace OGLAppFramework {

	class Shader {
	public:
		unsigned int ID;

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
	};
}