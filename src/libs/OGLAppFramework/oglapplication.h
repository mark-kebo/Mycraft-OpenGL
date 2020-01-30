#pragma once

#include "externallibs.h"
#include <cstdint>

namespace OGLAppFramework
{
    class OGLApplication
    {
    public:
        OGLApplication(std::uint16_t window_width = 1366u, std::uint16_t window_height = 768u, std::string app_name = "OGLSample", std::uint8_t ogl_context_version_major = 4u, std::uint8_t ogl_context_version_minor = 2u, bool log = true);
        virtual ~OGLApplication();

    private:
        static bool instance_created_;

    private:
        // GLFW callbacks
        static void glfwErrorCallback(int error, const char* description);
        static void glfwReshapeCallback(GLFWwindow* window, int width, int height);
        static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos);
        static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    private:
        GLFWwindow* window_ptr_;
        const bool log_;

    private:
        void releaseLibs();

    public:
        virtual void run() final;
        virtual void disableCursor() final;
        virtual void enableCursor() final;

    public:
        // metoda wywolywana podczas zmiany rozmiaru okna
        virtual void reshapeCallback(std::uint16_t width, std::uint16_t height) = 0;
        // metoda wywolywana podczas wcisniecia przycisku
        virtual void keyCallback(int key, int scancode, int action, int mods) = 0;
        // metoda wywolywana podczas zmiany pozycji kursora myszy
        virtual void cursorPosCallback(double xpos, double ypos) = 0;
        // metoda wywolywana podczas wcisniecia przycisku myszy
        virtual void mouseButtonCallback(int button, int action, int mods) = 0;
        // metoda wywolywana na poczatku (przy starcie aplikacji, po inicjalizacji OGL)
        virtual bool init() = 0;
        // metoda wywolywana co klatke
        virtual bool frame(float delta_time) = 0;
        // metoda wywolywana przy zamknieciu aplikacji
        virtual void release() = 0;
    };
}  // namespace OGLAppFramework
