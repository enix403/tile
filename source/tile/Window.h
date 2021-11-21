#pragma once

#include <string>

struct GLFWwindow;

namespace Tile {

    struct CreateWindowProps 
    {
        int Width, Height;

        const char* Title;
        const char* X11WinClass;
    };

    class Window 
    {
    public:
        Window(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(const Window& other) = delete;


        explicit Window(const CreateWindowProps& props);
        ~Window() = default;

    public:

        inline GLFWwindow* GetGLFWHandle() const { return m_Handle; }

        bool Init();
        void SetVSync(bool is_vsync);

        bool ShouldClose() const;
        void SwapBuffers() const;
        void PollEvents() const;

        void Close();

        inline int GetWidth()  const { return m_WinProps.Width;  }
        inline int GetHeight() const { return m_WinProps.Height; }
        
        void OnResize(int width, int height);

    private:
        bool InitGl() const;
        // void RegisterEventHandlers();

    private:
        CreateWindowProps m_WinProps;
        bool m_IsVSync = false;
        
        /* The GLFW window handle pointer */
        GLFWwindow* m_Handle = nullptr;
    };
}