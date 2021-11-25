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

        m_Camera.SetClippingPlanes(0.01f, 150.0f);
        m_CamController = std::make_shared<CameraController>(m_Camera, win_handle);

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

        m_DefaultShader->SetUniformInt("u_ShouldSampleTexture", 1);
        m_DefaultShader->SetUniformInt("u_Texture", 0); // the slot the texture is bound to

        /* ------------------------------------------- OpenGL Options ------------------------------------------- */

        gl::glClearColor(0.090196f, 0.090196f, 0.0901961f, 1.f);

        gl::glEnable(gl::GL_MULTISAMPLE);
        gl::glEnable(gl::GL_DEPTH_TEST);

        gl::glEnable(gl::GL_CULL_FACE);
        gl::glCullFace(gl::GL_BACK);
        
        gl::glFrontFace(gl::GL_CCW);
    }

    void Loop()
    {
        if (glfwGetKey(m_MainWindow->GetGLFWHandle(), GLFW_KEY_ESCAPE))
        {
            m_Running = false;
            return;
        }

        gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

        m_CamController->Update();
        m_TestModel->GetVA().Bind();

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

        m_MainWindow->SwapBuffers();
        m_MainWindow->PollEvents();
    }

private:
    bool m_Running = false;

    std::unique_ptr<Window> m_MainWindow;

    Camera m_Camera;

    std::shared_ptr<Model> m_TestModel;
    std::shared_ptr<Texture> m_TestTexture;

    std::shared_ptr<CameraController> m_CamController;
    std::shared_ptr<Shader> m_DefaultShader;
};

