#include "tile/Camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>


namespace Tile {
    Camera::Camera(float aspectRatio)
    :   m_AspectRatio(aspectRatio), 
        m_PlaneNear(0.1f), 
        m_PlaneFar(150.0f)
    {
        RecalculateProjection();
    }

    void Camera::RecalculateProjection()
    {
        m_Projection = glm::perspective(glm::radians(45.0f), m_AspectRatio, m_PlaneNear, m_PlaneFar);
        RecalculateProjectionView();
    }

    void Camera::RecalculateProjectionView()
    {
        // Set m_ProjectionView
        m_Position = m_FocusPoint - GetFowardDirection() * m_BallRadius;

        // (Translation * Rotation) ^ -1 = (Rotation ^ -1) * (Translation ^ -1)
        // And rotation inverse is inverse of is quaternion (which itself is its conjugate)
        // and translation inverse is, well, the opposite translation
        glm::mat4 inverseTransform = glm::toMat4(glm::conjugate(
            glm::quat({-m_ArcBallPitch, -m_ArcBallYaw, 0.0f})) // inverse of a unit quaternion is its conjugate
        );
        inverseTransform *= glm::translate(glm::mat4{1.0f}, -m_Position);
        m_ProjectionView = m_Projection * inverseTransform;
    }

    glm::vec3 Camera::ApplyRotationTo(const glm::vec3& vec) const
    {
        return glm::rotate(glm::quat(glm::vec3 {-m_ArcBallPitch, -m_ArcBallYaw, 0.0f}), vec);
    }

    glm::vec3 Camera::GetFowardDirection() const
    {
        return ApplyRotationTo({ 0.0f, 0.0f, -1.0f });
    }

    glm::vec3 Camera::GetUpDirection() const
    {
        return ApplyRotationTo({ 0.0f, 1.0f, 0.0f });
    }

    glm::vec3 Camera::GetRightDirection() const
    {
        return ApplyRotationTo({ 1.0f, 0.0f, 0.0f });
    }

}