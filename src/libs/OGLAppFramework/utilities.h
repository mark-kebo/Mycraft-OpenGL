#pragma once

#include "externallibs.h"
#include <optional>

namespace OGLAppFramework
{
    // funkcja wczytujaca shader-y z plikow, kompilujaca je i tworzaca program
    // przyjmuje na wejscie sciezki do VS i FS
    // w przypadku powodzenia zwraca uchwyt (handle) do programu
    std::optional<gl::GLuint> createProgram(const std::string &vs_source_path, const std::string &fs_source_path);

    // funkcja wczytujaca teksture z pliku (format .dds, lub .ktx), tworzaca "Texture Object" i w przypadku powodzenia zwracająca uchwyt (handle) do niego
    std::optional<gl::GLuint> loadTexFromFileAndCreateTO(const std::string &file_path);

    // funckja zwracajaca macierz skalowania
    glm::mat4x4 scaleMatrix(float x, float y, float z);
    // funckja zwracajaca macierz obrotu
    glm::mat4x4 rotationMatrix(float angle, const glm::vec3 &axis);
    // funckja zwracajaca macierz translacji
    glm::mat4x4 translationMatrix(const glm::vec3 &translation);
}
