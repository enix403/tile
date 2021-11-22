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

        BeforeLoop();
        while (!m_MainWindow->ShouldClose())
        {
            Loop();
        }

        m_MainWindow->Close();
    }

private:

    void BeforeLoop()
    {
        gl::glClearColor(0.090196f, 0.090196f, 0.0901961f, 1.f);

        m_Camera.SetAspectRatio((float)m_MainWindow->GetWidth() / m_MainWindow->GetHeight());
        
        ModelBuilder builder;

        // m_TestModel = builder.LoadObjFromFile("assets/_models/flat_vase.obj");
        m_TestModel = builder.LoadObjFromFile("assets/models/smooth_vase.obj");
        m_Camera.TranslateFocusPoint({ 0.f, 0.2f, 0.f });
        m_Camera.SetRadius(1.f);

        // m_TestModel = builder.LoadObjFromFile("assets/models/cube.obj");
        // m_TestModel = builder.LoadObjFromFile("assets/_models/Handgun.obj", "Gun_Cube.001");
        // m_TestModel = builder.LoadObjFromFile("assets/_models/Porsche_911_GT2.obj");
        // m_TestModel = builder.LoadObjFromFile("assets/_models/Lowpoly_Notebook_2.obj");
        // m_TestModel = builder.LoadObjFromFile("assets/_models/Sting-Sword-lowpoly.obj");
        // m_TestModel = builder.LoadObjFromFile("assets/_models/Lowpoly_tree_sample.obj");

        // m_TestModel = builder.LoadObjFromFile("assets/_models/CraneoOBJ.obj");
        // m_Camera.TranslateFocusPoint({ 0.f, 3.f, 0.f });
        // m_Camera.SetRadius(12.f);
        // m_Camera.MoveBallCoords(0.0f, glm::radians(-45.0f));

        // m_TestModel = builder.LoadObjFromFile("assets/_models/Speaker.obj");
        // m_Camera.TranslateFocusPoint({ 0.f, 1.f, 0.f });
        // m_Camera.MoveBallCoords(0.0f, glm::radians(-45.0f));


        m_DefaultShader = Tile::Shader::LoadFromFile("assets/shaders/DiffuseModel.glsl", "Test Shader");
        m_DefaultShader->Bind();
        // m_DefaultShader->SetUniformFloat3("u_Color", IRGB_TO_FRGB(252, 3, 40));
        // m_DefaultShader->SetUniformFloat3("u_Color", IRGB_TO_FRGB(3, 186, 252));
        // m_DefaultShader->SetUniformFloat3("u_Color", IRGB_TO_FRGB(232, 231, 213));
        m_DefaultShader->SetUniformFloat3("u_Color", IRGB_TO_FRGB(174, 177, 189));
        // m_DefaultShader->SetUniformFloat3("u_Color", IRGB_TO_FRGB(119, 119, 122));

        gl::glEnable(gl::GL_MULTISAMPLE);
        gl::glEnable(gl::GL_DEPTH_TEST);
    }

    void Loop()
    {
        gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

        m_CamController->Update();
        m_TestModel->GetVA().Bind();

        // TODO: multiply m_ProjectionView with model matrix to make up the actual
        // "tranform" matrix. Right now it is only the unit matrix so it doesn't matter
        m_DefaultShader->SetUniformMat4("u_Transform", m_Camera.GetProjectionView());
        m_DefaultShader->SetUniformMat4("u_Model", glm::mat4 { 1.0f });

        if (m_TestModel->HasIndexBuffer())
            gl::glDrawElements(gl::GL_TRIANGLES, m_TestModel->GetIndexCount(), gl::GL_UNSIGNED_INT, 0);
        else
            gl::glDrawArrays(gl::GL_TRIANGLES, 0,  m_TestModel->GetVertexCount());

        m_MainWindow->SwapBuffers();
        m_MainWindow->PollEvents();
    }

private:
    std::unique_ptr<Window> m_MainWindow;


    Camera m_Camera;

    std::shared_ptr<Model> m_TestModel;
    std::shared_ptr<CameraController> m_CamController;
    std::shared_ptr<Shader> m_DefaultShader;
};

