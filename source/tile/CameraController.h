#pragma once

#include "tile/Window.h"
#include "tile/Camera.h"

#include <memory>

namespace Tile {

    class CameraController
    {
    public:
        explicit CameraController(Camera& camera, GLFWwindow* window);

        CameraController(const CameraController& other) = delete;
        CameraController& operator=(const CameraController& other) = delete;

        void Update();

        // GLFW provides no way to poll mouse scroll state
        // so it must be passed down in an event-driven manner
        void OnScroll(float amount);

    private:
        void Rotate(float deltaX, float deltaY);
        void Pan(float deltaX, float deltaY);

    private:
        GLFWwindow* m_WinHandle;
        Camera& m_Camera;
        
        float m_MouseLastX, m_MouseLastY;
    };
}