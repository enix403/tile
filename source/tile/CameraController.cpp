#include "tile/CameraController.h"

#include <algorithm>

#include <GLFW/glfw3.h>


namespace Tile {
    constexpr float CAMERA_ARCBALL_MAX_RADIUS = 40.0f;

    // TODO: Extract into its own "Input" Component/Manager
    bool is_key_pressed(GLFWwindow* window, int keyCode) 
    {
        int state = glfwGetKey(window, keyCode); 
        return state == GLFW_PRESS || state == GLFW_REPEAT;   
    }

    bool is_mouse_pressed(GLFWwindow* window, int mouseBtnCode, bool checkRelease = false)
    {
        int state = glfwGetMouseButton(window, mouseBtnCode); 

        if (checkRelease)
            return state == GLFW_RELEASE;

        return state == GLFW_PRESS;
    }
}

namespace Tile {
    CameraController::CameraController(Camera& camera, GLFWwindow* window)
    :   m_Camera(camera),
        m_WinHandle(window)
    {}

    void CameraController::Update()
    {
        bool isCtrlPressed      = is_key_pressed(m_WinHandle, GLFW_KEY_LEFT_CONTROL);
        bool isShiftPressed     = is_key_pressed(m_WinHandle, GLFW_KEY_LEFT_SHIFT);
        bool isLeftMousePressed = is_mouse_pressed(m_WinHandle, GLFW_MOUSE_BUTTON_LEFT);
        
        double mouseX, mouseY;
        glfwGetCursorPos(m_WinHandle, &mouseX, &mouseY);

        float deltaX = mouseX - m_MouseLastX;
        float deltaY = mouseY - m_MouseLastY;

        if (isCtrlPressed && isLeftMousePressed)
        {
            Rotate(deltaX, deltaY);
        }

        if (isShiftPressed && isLeftMousePressed)
        {
            Pan(deltaX, deltaY);
        }

        m_MouseLastX = static_cast<float>(mouseX);
        m_MouseLastY = static_cast<float>(mouseY);
    }

    void CameraController::Rotate(float deltaX, float deltaY)
    {
        constexpr float speed = 0.005f;

        float dpitch = deltaY * speed;
        float dyaw;

        if (m_Camera.GetUpDirection().y > 0)
            dyaw = deltaX;
        else
            dyaw = -deltaX;

        dyaw *= speed;

        m_Camera.MoveBallCoords(dpitch, dyaw);
    }

    void CameraController::Pan(float deltaX, float deltaY)
    {
        float speed = 0.005f * m_Camera.GetRadius();

        glm::vec3 translation = m_Camera.GetRightDirection() * deltaX + m_Camera.GetUpDirection() * -deltaY;
        m_Camera.TranslateFocusPoint(translation * -speed);
    }

    void CameraController::OnScroll(float amount)
    {
        // Zoom
        constexpr float speed = 0.1f;
        float newRadius = std::clamp(m_Camera.GetRadius() - amount * speed, 0.1f, CAMERA_ARCBALL_MAX_RADIUS);
        m_Camera.SetRadius(newRadius);
    }
}