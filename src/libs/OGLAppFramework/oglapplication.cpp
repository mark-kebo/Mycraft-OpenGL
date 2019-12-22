#include "oglapplication.h"

namespace OGLAppFramework
{
    bool OGLApplication::instance_created_ = false;

    OGLApplication::OGLApplication(std::uint16_t window_width, std::uint16_t window_height, std::string app_name, std::uint8_t ogl_context_version_major, std::uint8_t ogl_context_version_minor, bool log) : window_ptr_(nullptr), log_(log)
    {
        std::string error_string;

        if(!instance_created_)
        {
            if(log_) std::cout << "OGLApplication - start" << std::endl;

            // ustawienie funkcji, ktora ma sie wywolac podczas bledu GLFW
            glfwSetErrorCallback(glfwErrorCallback);

            // inicjalizacja GLFW
            if(glfwInit())
            {
                // ustawienie wartosci dla glownego framebuffer-a
                glfwWindowHint(GLFW_RED_BITS, 8);
                glfwWindowHint(GLFW_GREEN_BITS, 8);
                glfwWindowHint(GLFW_BLUE_BITS, 8);
                glfwWindowHint(GLFW_DEPTH_BITS, 32);
                glfwWindowHint(GLFW_STENCIL_BITS, 0);

                // wymuszenie minimalnej wersji OGL wymaganej dla naszej aplikacji
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, ogl_context_version_major);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, ogl_context_version_minor);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);

                // stworzenie okna
                if(window_ptr_ = glfwCreateWindow(window_width, window_height, app_name.c_str(), nullptr, nullptr))
                {
                    // ustawienie kontekstu OGL dla okna
                    glfwMakeContextCurrent(window_ptr_);

                    // inicjalizacja glbinding
                    glbinding::Binding::initialize(glfwGetProcAddress);

                    // wlaczenie V-sync jesli jest obslugiwane...
                    glfwSwapInterval(1);

                    // ustawienie callback-a po każdej funkcji OGL, tak aby wywolywana byla funkcja glGetError, sprawdzajaca czy nie wystapil jakis blad
                   // glbinding::setCallbackMaskExcept(glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue, { "glGetError" });
                    glbinding::setAfterCallback([log](const glbinding::FunctionCall &call)
                    {
                        const auto error = gl::glGetError();
                        if(error != gl::GL_NO_ERROR)
                        {
                            std::cerr << "OpenGL error in function call:" << std::endl;
                            std::cerr << call.function->name() << "(";
                            for(std::size_t i = 0u; i < call.parameters.size(); ++i)
                            {
                                std::cerr << call.parameters[i].get();
                                if(i < call.parameters.size() - 1)
                                    std::cerr << ", ";
                            }
                            std::cerr << ");";

                            if(call.returnValue)
                                std::cerr << " -> " << call.returnValue.get();
                            std::cerr << std::endl;
                            std::cerr << "Error: " << error << std::endl;
                            throw std::runtime_error(std::to_string(static_cast<int>(error)));
                        }
                    });

                    // przyporzadkowanie obiektu klasy OGLApplication do okna - dzieki temu majac dostep do okna, mozemy otrzymac adres obiektu
                    glfwSetWindowUserPointer(window_ptr_, this);

                    // ustawienie funkcji wywolywanej podczas zmiany rozmiaru okna
                    glfwSetFramebufferSizeCallback(window_ptr_, glfwReshapeCallback);

                    // ustawienie funkcji wywolywanej podczas wcisniecia klawisza
                    glfwSetKeyCallback(window_ptr_, glfwKeyCallback);

                    // ustawienie funkcji wywolywanej podczas zmiany pozycji kursora myszy
                    glfwSetCursorPosCallback(window_ptr_, glfwCursorPosCallback);

                    // ustawienie funkcji wywolywanej podczas wcisniecia przycisku myszy
                    glfwSetMouseButtonCallback(window_ptr_, glfwMouseButtonCallback);

                    // all done
                    instance_created_ = true;

                    return;
                }
                else
                {
                    error_string = "Error - glfwCreateWindow failed";
                }
            }
            else
            {
                error_string = "Error - glfwInit failed";
            }

            releaseLibs();
        }
        else
        {
            error_string = "Error - too many instances of OGLApplication";
        }

        if(log_) std::cerr << error_string << std::endl;

        throw std::runtime_error(error_string);
    }

    OGLApplication::~OGLApplication()
    {
        instance_created_ = false;

        if(log_) std::cout << "OGLApplication - shutdown" << std::endl;

        releaseLibs();
    }

    void OGLApplication::glfwErrorCallback(int error, const char *description)
    {
        std::cerr << "GLFW error - " << std::hex << error << " - " << description << std::endl;
    }

    void OGLApplication::glfwReshapeCallback(GLFWwindow *window, int width, int height)
    {
        if(auto app_ptr = reinterpret_cast<OGLApplication*>(glfwGetWindowUserPointer(window)))
        {
            app_ptr->reshapeCallback(width, height);
        }
    }

    void OGLApplication::glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        if(auto app_ptr = reinterpret_cast<OGLApplication*>(glfwGetWindowUserPointer(window)))
        {
            app_ptr->keyCallback(key, scancode, action, mods);
        }
    }

    void OGLApplication::glfwCursorPosCallback(GLFWwindow *window, double xpos, double ypos)
    {
        if(auto app_ptr = reinterpret_cast<OGLApplication*>(glfwGetWindowUserPointer(window)))
        {
            app_ptr->cursorPosCallback(xpos, ypos);
        }
    }

    void OGLApplication::glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
    {
        if(auto app_ptr = reinterpret_cast<OGLApplication*>(glfwGetWindowUserPointer(window)))
        {
            app_ptr->mouseButtonCallback(button, action, mods);
        }
    }

    void OGLApplication::releaseLibs()
    {
        if(window_ptr_)
        {
            // zniszczenie okna
            glfwDestroyWindow(window_ptr_);
            window_ptr_ = nullptr;
        }

        // posprzatanie po GLFW
        glfwTerminate();
    }

    void OGLApplication::run()
    {
        if(log_) std::cout << "OGLApplication - run" << std::endl;

        if(log_) std::cout << "OGLApplication - init" << std::endl;

        try{
            // wywolanie funkcji init
            if(init())
            {
                if(log_) std::cout << "OGLApplication - frames" << std::endl;

                double time_from_start_to_last_frame = 0.;
                // glowna petla programu
                while(!glfwWindowShouldClose(window_ptr_))
                {
                    // pobranie i obsluga event-ow
                    glfwPollEvents();

                    double time_from_start = glfwGetTime();
                    double last_frame_time = time_from_start - time_from_start_to_last_frame;
                    time_from_start_to_last_frame = time_from_start;

                    // wyczyszczenie ekranu
                    gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

                    // wywolanie funkcji frame (co klatke)
                    frame(static_cast<float>(last_frame_time));

                    // podmiana backbuffer-a
                    glfwSwapBuffers(window_ptr_);
                }
            }
        }
        catch(const std::exception &e)
        {
            std::cout << "Exception caught: " << e.what() << std::endl;
        }
        catch(...)
        {
            std::cout << "Exception caught..." << std::endl;
        }

        if(log_) std::cout << "OGLApplication - release" << std::endl;
        // wywolanie funkcji release (na zakonczenie)
        release();
    }

    void OGLApplication::disableCursor()
    {
        glfwSetInputMode(window_ptr_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void OGLApplication::enableCursor()
    {
        glfwSetInputMode(window_ptr_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}
