#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace Tile
{
    // An Arc-Ball Camera
    class Camera
    {

    public:
        explicit Camera(float aspectRatio);
        ~Camera() = default;

        Camera(const Camera& other) = default;
        Camera& operator=(const Camera& other) = default;

        inline const glm::mat4& GetProjectionView() const { return m_ProjectionView; }
        inline const glm::vec3& GetPosition() const { return m_Position; }

        inline void SetAspectRatio(float ar) { m_AspectRatio = ar; RecalculateProjection(); }
        inline void SetClippingPlanes(float near, float far)
        { 
            m_PlaneNear = near;
            m_PlaneFar = far; 
            RecalculateProjection();
        }

        inline void MoveBallCoords(float dpitch, float dyaw) 
        {
            m_ArcBallPitch += dpitch;
            m_ArcBallYaw += dyaw;
            RecalculateProjectionView();
        }

        inline void TranslateFocusPoint(const glm::vec3& translation)
        {
            m_FocusPoint += translation;
            RecalculateProjectionView();
        }

        inline void SetRadius(float radius) { m_BallRadius = radius; RecalculateProjectionView(); }

        glm::vec3 GetFowardDirection()  const;
        glm::vec3 GetUpDirection()      const;
        glm::vec3 GetRightDirection()   const;

        inline float GetRadius() const { return m_BallRadius; }

    private:
        void RecalculateProjection();
        void RecalculateProjectionView();

        glm::vec3 ApplyRotationTo(const glm::vec3& vec) const;

    private:
        glm::mat4 m_Projection      { 1.0f };
        glm::mat4 m_ProjectionView  { 1.0f };

        glm::vec3 m_Position        { 0.0f };
        glm::vec3 m_FocusPoint      { 0.0f };

        float m_BallRadius = 5.0f;
        float m_ArcBallPitch = 0.0f, m_ArcBallYaw = 0.0f;

        float m_AspectRatio;
        float m_PlaneNear, m_PlaneFar;
    };
}