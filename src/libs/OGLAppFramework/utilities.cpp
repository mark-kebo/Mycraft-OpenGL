#include "utilities.h"
#include <fstream>
#include <sstream>

namespace OGLAppFramework
{
    // funkcja wczytujaca zawartosc pliku
    std::string loadFile(const std::string &file_path)
    {
        std::ifstream file(file_path, std::ios::in | std::ios::binary);
        if (file)
        {
            std::ostringstream contents;
            contents << file.rdbuf();
            file.close();
            return(contents.str());
        }
        return {};
    }

    // funkcja kompilujaca shader
    // przyjmuje na wejscie kod shader-a i informacje o rodzaju shader-a
    // w przypadku powodzenia kompilacji zwraca uchwyt (handle) do skompilowanego shader-a
    std::optional<gl::GLuint> createAndCompileShader(const std::string &shader_source, gl::GLenum shader_type)
    {
        if(shader_source.length() > 0u)
        {
            gl::GLuint shader = gl::glCreateShader(shader_type);
            const char *shader_source_ptr = shader_source.c_str();
            gl::glShaderSource(shader, 1, &shader_source_ptr, nullptr);
            gl::glCompileShader(shader);
            gl::GLint is_compiled = 0;
            gl::glGetShaderiv(shader, gl::GL_COMPILE_STATUS, &is_compiled);
            if(!is_compiled)
            {
                gl::GLint max_log_length = 0;
                gl::glGetShaderiv(shader, gl::GL_INFO_LOG_LENGTH, &max_log_length);
                std::string error_log;
                error_log.resize(max_log_length);
                gl::glGetShaderInfoLog(shader, max_log_length, &max_log_length, error_log.data());
                error_log.pop_back();
                gl::glDeleteShader(shader);
                std::cerr << (shader_type == gl::GL_VERTEX_SHADER ? "Vertex shader" : (shader_type == gl::GL_FRAGMENT_SHADER ? "Fragment shader" : "Shader")) << " compilation error: " << error_log << std::endl;
                return {};
            }
            return shader;
        }
        return {};
    }

    std::optional<gl::GLuint> createProgram(const std::string &vs_source_path, const std::string &fs_source_path)
    {
        std::string vs_source = loadFile(vs_source_path);
        std::string fs_source = loadFile(fs_source_path);

        if(vs_source.length() == 0u || fs_source.length() == 0u)
        {
            std::cerr << "Error - can't load file: " << (vs_source.length() == 0u ? vs_source_path : fs_source_path) << std::endl;
        }

        if(auto compile_vs_result = createAndCompileShader(loadFile(vs_source_path), gl::GL_VERTEX_SHADER))
        {
            if(auto compile_fs_result = createAndCompileShader(loadFile(fs_source_path), gl::GL_FRAGMENT_SHADER))
            {
                gl::GLuint vs = compile_vs_result.value();
                gl::GLuint fs = compile_fs_result.value();

                gl::GLuint program = gl::glCreateProgram();
                gl::glAttachShader(program, vs);
                gl::glAttachShader(program, fs);
                gl::glLinkProgram(program);

                gl::GLint is_linked = 0;
                gl::glGetProgramiv(program, gl::GL_LINK_STATUS, &is_linked);
                if(!is_linked)
                {
                    gl::GLint max_log_length = 0;
                    gl::glGetProgramiv(program, gl::GL_INFO_LOG_LENGTH, &max_log_length);
                    std::string error_log;
                    error_log.resize(max_log_length);
                    gl::glGetProgramInfoLog(program, max_log_length, &max_log_length, error_log.data());
                    error_log.pop_back();
                    gl::glDeleteProgram(program);
                    gl::glDeleteShader(vs);
                    gl::glDeleteShader(fs);
                    std::cerr << "Shaders linking error: " << error_log << std::endl;
                    return {};
                }

                return program;
            }
        }

        return {};
    }

    std::optional<gl::GLuint> loadTexFromFileAndCreateTO(const std::string &file_path)
    {
        gli::texture texture = gli::load(file_path);

        if(texture.empty())
            return {};

        gli::gl GL(gli::gl::PROFILE_GL33);
        gli::gl::format const format = GL.translate(texture.format(), texture.swizzles());
        gl::GLenum target = static_cast<gl::GLenum>(GL.translate(texture.target()));

        gl::GLuint texture_handle = 0u;
        gl::glGenTextures(1, &texture_handle);
        gl::glBindTexture(target, texture_handle);
        glTexParameteri(target, gl::GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(target, gl::GL_TEXTURE_MAX_LEVEL, static_cast<gl::GLint>(texture.levels() - 1));
        glTexParameteri(target, gl::GL_TEXTURE_SWIZZLE_R, format.Swizzles[0]);
        glTexParameteri(target, gl::GL_TEXTURE_SWIZZLE_G, format.Swizzles[1]);
        glTexParameteri(target, gl::GL_TEXTURE_SWIZZLE_B, format.Swizzles[2]);
        glTexParameteri(target, gl::GL_TEXTURE_SWIZZLE_A, format.Swizzles[3]);

        glm::tvec3<gl::GLsizei> const extent(texture.extent());
        gl::GLsizei const face_total = static_cast<gl::GLsizei>(texture.layers() * texture.faces());

        switch(texture.target())
        {
        case gli::TARGET_1D:
            gl::glTexStorage1D(
                target, static_cast<gl::GLint>(texture.levels()), static_cast<gl::GLenum>(format.Internal), extent.x);
            break;
        case gli::TARGET_1D_ARRAY:
        case gli::TARGET_2D:
        case gli::TARGET_CUBE:
            gl::glTexStorage2D(
                target, static_cast<gl::GLint>(texture.levels()), static_cast<gl::GLenum>(format.Internal),
                extent.x, texture.target() == gli::TARGET_2D ? extent.y : face_total);
            break;
        case gli::TARGET_2D_ARRAY:
        case gli::TARGET_3D:
        case gli::TARGET_CUBE_ARRAY:
            gl::glTexStorage3D(
                target, static_cast<gl::GLint>(texture.levels()), static_cast<gl::GLenum>(format.Internal),
                extent.x, extent.y,
                texture.target() == gli::TARGET_3D ? extent.z : face_total);
            break;
        default:
            assert(0);
            break;
        }

        for(std::size_t layer = 0; layer < texture.layers(); ++layer)
        for(std::size_t face = 0; face < texture.faces(); ++face)
        for(std::size_t level = 0; level < texture.levels(); ++level)
        {
            gl::GLsizei const layer_GL = static_cast<gl::GLsizei>(layer);
            glm::tvec3<gl::GLsizei> extent(texture.extent(level));
            target = gli::is_target_cube(texture.target())
                ? static_cast<gl::GLenum>(gl::GL_TEXTURE_CUBE_MAP_POSITIVE_X + face)
                : target;

            switch(texture.target())
            {
            case gli::TARGET_1D:
                if(gli::is_compressed(texture.format()))
                    gl::glCompressedTexSubImage1D(
                        target, static_cast<gl::GLint>(level), 0, extent.x,
                        static_cast<gl::GLenum>(format.Internal), static_cast<gl::GLsizei>(texture.size(level)),
                        texture.data(layer, face, level));
                else
                    gl::glTexSubImage1D(
                        target, static_cast<gl::GLint>(level), 0, extent.x,
                        static_cast<gl::GLenum>(format.External), static_cast<gl::GLenum>(format.Type),
                        texture.data(layer, face, level));
                break;
            case gli::TARGET_1D_ARRAY:
            case gli::TARGET_2D:
            case gli::TARGET_CUBE:
                if(gli::is_compressed(texture.format()))
                    gl::glCompressedTexSubImage2D(
                        target, static_cast<gl::GLint>(level),
                        0, 0,
                        extent.x,
                        texture.target() == gli::TARGET_1D_ARRAY ? layer_GL : extent.y,
                        static_cast<gl::GLenum>(format.Internal), static_cast<gl::GLsizei>(texture.size(level)),
                        texture.data(layer, face, level));
                else
                    gl::glTexSubImage2D(
                        target, static_cast<gl::GLint>(level),
                        0, 0,
                        extent.x,
                        texture.target() == gli::TARGET_1D_ARRAY ? layer_GL : extent.y,
                        static_cast<gl::GLenum>(format.External), static_cast<gl::GLenum>(format.Type),
                        texture.data(layer, face, level));
                break;
            case gli::TARGET_2D_ARRAY:
            case gli::TARGET_3D:
            case gli::TARGET_CUBE_ARRAY:
                if(gli::is_compressed(texture.format()))
                    gl::glCompressedTexSubImage3D(
                        target, static_cast<gl::GLint>(level),
                        0, 0, 0,
                        extent.x, extent.y,
                        texture.target() == gli::TARGET_3D ? extent.z : layer_GL,
                        static_cast<gl::GLenum>(format.Internal), static_cast<gl::GLsizei>(texture.size(level)),
                        texture.data(layer, face, level));
                else
                    gl::glTexSubImage3D(
                        target, static_cast<gl::GLint>(level),
                        0, 0, 0,
                        extent.x, extent.y,
                        texture.target() == gli::TARGET_3D ? extent.z : layer_GL,
                        static_cast<gl::GLenum>(format.External), static_cast<gl::GLenum>(format.Type),
                        texture.data(layer, face, level));
                break;
            default: assert(0); break;
            }
        }
        return texture_handle;
    }

    glm::mat4x4 scaleMatrix(float x, float y, float z)
    {
        return glm::scale(glm::mat4x4(1.f), glm::vec3(x, y, z));
    }

    glm::mat4x4 rotationMatrix(float angle, const glm::vec3 &axis)
    {
        return glm::rotate(glm::mat4x4(1.f), angle, axis);
    }

    glm::mat4x4 translationMatrix(const glm::vec3 &translation)
    {
        return glm::translate(glm::mat4x4(1.f), translation);
    }
}
