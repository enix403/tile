#include <iterator>
#include <memory>
#include <iostream>

#include "tile/gl_wrappers.h"
#include "tile/opengl_inc.h"
#include "tile/Window.h"
#include "tile/Shader.h"
#include "tile/Camera.h"
#include "tile/CameraController.h"
#include "tile/Model.h"
#include "tile/Texture.h"
#include "tile/utils.h"

#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
// #include <imgui/imgui.h>

using namespace Tile;


class Application
{

public:
    Application() 
    : m_Camera(1.0f)
    {}

    void Start()
    {
        CreateWindowProps win_props { 1090, 700, "Tile Viewer", "tile-viewer" };
        m_MainWindow.reset(new Window(win_props));
    
        m_MainWindow->Init();
        m_MainWindow->SetVSync(true);

        auto win_handle = m_MainWindow->GetGLFWHandle();
        glfwSetWindowUserPointer(win_handle, this);

        glfwSetWindowSizeCallback(win_handle, [](GLFWwindow* glfw_win, int w, int h) {
            auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(glfw_win));
            app->m_MainWindow->OnResize(w, h);
            app->m_Camera.SetAspectRatio((float)w / h);
        });

        glfwSetScrollCallback(win_handle, [](GLFWwindow* glfw_win, double xOffset, double yOffset) {
            auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(glfw_win));
            app->m_CamController->OnScroll((float)yOffset);
        });

        m_Camera.SetClippingPlanes(0.01f, 100.0f);
        m_CamController = std::make_unique<CameraController>(m_Camera, win_handle);

        m_Running = true;

        BeforeLoop();
        while (!m_MainWindow->ShouldClose())
        {
            if (!m_Running)
                break;
            Loop();
        }

        m_MainWindow->Close();
    }

private:

    void BeforeLoop()
    {
        m_Camera.SetAspectRatio((float)m_MainWindow->GetWidth() / m_MainWindow->GetHeight());
        
        /* ------------------------------------------- Model Loading ------------------------------------------- */

        ModelBuilder builder;
        
        // m_TestModel = builder.LoadWavefrontObj("assets/_models/flat_vase.obj");
        // m_TestModel = builder.LoadWavefrontObj("assets/models/smooth_vase.obj");
        // m_Camera.TranslateFocusPoint({ 0.f, 0.2f, 0.f });
        // m_Camera.SetRadius(1.f);

        // m_TestModel = builder.LoadWavefrontObj("assets/models/cube.obj");
        m_TestModel = builder.LoadWavefrontObj("assets/models/cube_quads.obj");

        // m_TestModel = builder.LoadWavefrontObj("assets/_models/Porsche_911_GT2.obj");
        // m_Camera.SetRadius(6.f);

        /* ------------------------------------------- Texture ------------------------------------------- */

        // m_TestTexture = Texture2D::CreateFromFile("assets/textures/wiki.png");
        // m_TestTexture = Texture2D::CreateFromFile("assets/textures/monster.png");
        m_TestTexture = Texture2D::ImageFromFile("assets/textures/cosas.png");
        m_TestTexture->Bind(0);

        /* ------------------------------------------- Shader ------------------------------------------- */

        m_DefaultShader = Shader::LoadFromFile("assets/shaders/DiffuseModel.glsl", "Test Shader");
        m_DefaultShader->Bind();
        m_DefaultShader->SetUniformFloat3("u_Color", IRGB_TO_FRGB(174, 177, 189));

        m_DefaultShader->SetUniformInt("u_ShouldSampleTexture", 0);
        m_DefaultShader->SetUniformInt("u_Texture", 0); // the slot the texture is bound to

        /* ------------------------------------------- Grid ------------------------------------------- */

        m_GridShader = Shader::LoadFromFile("assets/shaders/WorldGrid.glsl", "Grid Shader");

        m_GridBuf = std::make_unique<VertexBuffer>();
        m_GridVAO = std::make_unique<VertexArray>();
        
        float gridTrigger[6] = { 0.f };
        m_GridBuf->SetData(gridTrigger, sizeof(float) * 6);
        m_GridVAO->AddVertexBuffer(*m_GridBuf, {{ 0, "Tigger", 1, VertAttribComponentType::Float, false }});

        /* ------------------------------------------- OpenGL Options ------------------------------------------- */

        gl::glClearColor(0.090196f, 0.090196f, 0.0901961f, 1.f);

        gl::glEnable(gl::GL_MULTISAMPLE);
        gl::glEnable(gl::GL_DEPTH_TEST);

        gl::glCullFace(gl::GL_BACK);
        gl::glFrontFace(gl::GL_CCW);

        gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA);
    }

    void Loop()
    {
        if (glfwGetKey(m_MainWindow->GetGLFWHandle(), GLFW_KEY_ESCAPE))
        {
            m_Running = false;
            return;
        }

        gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

        Draw();

        m_MainWindow->SwapBuffers();
        m_MainWindow->PollEvents();
    }

    void Draw()
    {
        m_CamController->Update();

        /* ============================================================================================================ */
        /* ============================================== Draw the model ============================================== */
        /* ============================================================================================================ */
        gl::glDisable(gl::GL_BLEND);
        gl::glEnable(gl::GL_CULL_FACE);

        m_TestModel->GetVA().Bind();
        m_DefaultShader->Bind();

        // TODO: multiply m_ProjectionView with model matrix to make up the actual
        // "tranform" matrix. Right now it is only the unit matrix so it doesn't matter
        m_DefaultShader->SetUniformMat4("u_Transform", m_Camera.GetProjectionView());
        m_DefaultShader->SetUniformMat4("u_Model", glm::mat4 { 1.0f });

        // light follows the camera
        m_DefaultShader->SetUniformFloat3("u_DirectionToLight", glm::normalize(-m_Camera.GetFowardDirection()));

        if (m_TestModel->HasIndexBuffer())
            gl::glDrawElements(gl::GL_TRIANGLES, m_TestModel->GetIndexCount(), gl::GL_UNSIGNED_INT, 0);
        else
            gl::glDrawArrays(gl::GL_TRIANGLES, 0,  m_TestModel->GetVertexCount());


        /* ============================================================================================================ */
        /* =============================================== Draw the grid ============================================== */
        /* ============================================================================================================ */
        gl::glDisable(gl::GL_CULL_FACE);
        gl::glEnable(gl::GL_BLEND);

        m_GridShader->Bind();
        m_GridShader->SetUniformMat4("u_ProjectionView", m_Camera.GetProjectionView());
        m_GridShader->SetUniformFloat("u_CamNear", m_Camera.GetNearPlane());
        m_GridShader->SetUniformFloat("u_CamFar", m_Camera.GetFarPlane());

        m_GridVAO->Bind();
        gl::glDrawArrays(gl::GL_TRIANGLES, 0, 6);
    }

private:
    bool m_Running = false;

    std::unique_ptr<Window> m_MainWindow;

    Camera m_Camera;

    std::shared_ptr<Model> m_TestModel;
    std::shared_ptr<Texture> m_TestTexture;

    std::unique_ptr<CameraController> m_CamController;
    std::shared_ptr<Shader> m_GridShader;
    std::shared_ptr<Shader> m_DefaultShader;

    std::unique_ptr<VertexBuffer> m_GridBuf;
    std::unique_ptr<VertexArray> m_GridVAO; 
};

