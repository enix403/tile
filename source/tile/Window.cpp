#include "tile/Window.h"
#include "tile/opengl_inc.h"

#include <iostream>
#include <stdexcept>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace {
    void handle_glfw_error()
    {
        const char* err_desc;
        int code = glfwGetError(&err_desc);

        if (code != GLFW_NO_ERROR)
            std::cerr << "Failed to create window -> GLFW error [code=" << code << "]: " << err_desc << std::endl;
    }
};

namespace Tile {

    Window::Window(const CreateWindowProps& props) :
        m_WinProps(props)
    { }

    bool Window::Init() {
        
        if (!glfwInit())
        {
            handle_glfw_error();
            return false;
        }

        /* Set GLFW hints before creating the window */
        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // This `GLFW_X11_CLASS_NAME` straight up does not work....
        // but according to the documentation it should, unless I do not 
        // know English
        glfwWindowHintString(GLFW_X11_CLASS_NAME, m_WinProps.X11WinClass);
        m_Handle = glfwCreateWindow(
            m_WinProps.Width,
            m_WinProps.Height,
            m_WinProps.Title,
            NULL, NULL
        );

        if (!m_Handle)
        {
            handle_glfw_error();
            glfwTerminate();
            return false;
        }


        glfwMakeContextCurrent(m_Handle);
        glfwSetWindowUserPointer(m_Handle, this);

        SetVSync(m_IsVSync);
        InitGl();

        int win_width, win_height;
        glfwGetWindowSize(m_Handle, &win_width, &win_height);
        OnResize(win_width, win_height);

        return true;
    }

    bool Window::InitGl() const
    {
        try
        {
            // Loads the functions
            // Throws an exception if OpenGL library could not be loaded
            gl::init();
            std::cout << "Using OpenGL Version: " << gl::glGetString(gl::GL_VERSION) << std::endl;
        }
        catch(const std::runtime_error& e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }

        return true;
    }

    void Window::OnResize(int width, int height)
    {
        m_WinProps.Width = width;
        m_WinProps.Height = height;

        gl::glViewport(0, 0, width, height);
    }

    void Window::SetVSync(bool is_vsync)
    {
        if (is_vsync)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);

        m_IsVSync = is_vsync;
    }

    bool Window::ShouldClose() const
    {
        return glfwWindowShouldClose(m_Handle);
    }

    void Window::SwapBuffers() const 
    {
        glfwSwapBuffers(m_Handle);
    }

    void Window::PollEvents() const 
    {
        glfwPollEvents();
    }

    void Window::Close()
    {
        glfwDestroyWindow(m_Handle);
        glfwTerminate();

        // gl::deinit();

        m_Handle = nullptr;
    }
}