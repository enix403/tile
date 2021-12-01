#include "tile/Shader.h"

#include "tile/opengl_inc.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

namespace {
    using namespace Tile;

    std::string shader_type_string(ShaderType type)
    {
        switch(type)
        {
        case ShaderType::Vertex:
            return "SHADER_VERTEX";
        case ShaderType::Fragment:
            return "SHADER_FRAGMENT";

        default:
            return "";
        }
    }

    bool read_shader_source_from_file(const std::string& filepath, ShaderSources& outSources);
}

namespace Tile
{
    Shader::Shader(const ShaderSources& sources, const std::string& debug_name) 
    : m_DebugName(debug_name)
    {
        int programId = gl::glCreateProgram();

        std::vector<unsigned int> children;

        for(auto& it : sources)
        {
            uint openglType;
            switch(it.first)
                {
                case ShaderType::Vertex: {
                    openglType = gl::GL_VERTEX_SHADER;
                    break;
                }

                case ShaderType::Fragment: {
                    openglType = gl::GL_FRAGMENT_SHADER;
                    break;
                }
            }

            const char* sourceStr = it.second.c_str();

            uint sid = gl::glCreateShader(openglType);
            children.push_back(sid);
            gl::glShaderSource(sid, 1, &sourceStr, NULL);
            gl::glCompileShader(sid);
            
            int success;
            gl::glGetShaderiv(sid, gl::GL_COMPILE_STATUS, &success);

            if(success)
            {
                gl::glAttachShader(programId, sid);
            }
            else
            {
                int len;
                gl::glGetShaderiv(sid, gl::GL_INFO_LOG_LENGTH, &len);
                char* infoLog = (char*)alloca(len * sizeof(char));

                gl::glGetShaderInfoLog(sid, len, NULL, infoLog);

                std::cerr 
                    << "[ERROR] Shader compilation error ("
                    << "name=\"" << debug_name << "\", "
                    << "type="   << shader_type_string(it.first)
                    << "): " << infoLog;
            }
        }

        /* =================================================================== */
        /* =================================================================== */
        /* ========================== Linking Stage ========================== */
        /* =================================================================== */
        /* =================================================================== */

        gl::glLinkProgram(programId);

        for(int sid : children)
        {
            gl::glDeleteShader(sid);
        }

        int success;
        gl::glGetProgramiv(programId, gl::GL_LINK_STATUS, &success);

        if(success == gl::GL_FALSE)
        {
            int len;
            gl::glGetProgramiv(programId, gl::GL_INFO_LOG_LENGTH, &len);
            char* infoLog = (char*)alloca(len * sizeof(char));

            gl::glGetProgramInfoLog(programId, len, NULL, infoLog);

            std::cerr 
                << "[ERROR] Shader link error ("
                << "name=\"" << debug_name << "\", "
                << "): " << infoLog;
        }
        else {
            m_ProgramID = programId;
        }
    }

    void Shader::Bind() const
    {
        gl::glUseProgram(m_ProgramID);
    }

    void Shader::Unbind() const
    {
        gl::glUseProgram(0);
    }


    void Shader::SetUniformIntArray(const std::string& name, int* values, uint32_t count)
    {
        int location = GetUniformLocation(name);
        gl::glUniform1iv(location, count, values);
    }

    void Shader::SetUniformFloatArray(const std::string& name, float* values, uint32_t count)
    {
        int location = GetUniformLocation(name);
        gl::glUniform1fv(location, count, values);
    }

    void Shader::SetUniformInt(const std::string& name, int value)
    {
        int location = GetUniformLocation(name);
        gl::glUniform1i(location, value);
    }

    void Shader::SetUniformFloat(const std::string& name, float value)
    {
        int location = GetUniformLocation(name);
        gl::glUniform1f(location, value);
    }
    
    void Shader::SetUniformFloat2(const std::string& name, const glm::vec2& value)
    {
        int location = GetUniformLocation(name);
        gl::glUniform2f(location, value.x, value.y);
    }

    void Shader::SetUniformFloat3(const std::string& name, const glm::vec3& value)
    {
        int location = GetUniformLocation(name);
        gl::glUniform3f(location, value.x, value.y, value.z);
    }

    void Shader::SetUniformFloat4(const std::string& name, const glm::vec4& value)
    {
        int location = GetUniformLocation(name);
        gl::glUniform4f(location, value.x, value.y, value.z, value.w);
    }

    void Shader::SetUniformMat3(const std::string& name, const glm::mat3& matrix)
    {
        int location = GetUniformLocation(name);
        gl::glUniformMatrix3fv(location, 1, gl::GL_FALSE, glm::value_ptr(matrix));
    }

    void Shader::SetUniformMat4(const std::string& name, const glm::mat4& matrix)
    {
        int location = GetUniformLocation(name);
        gl::glUniformMatrix4fv(location, 1, gl::GL_FALSE, glm::value_ptr(matrix));
    }

    int Shader::GetUniformLocation(const std::string& name) const 
    {
        if(m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        {
            return m_UniformLocationCache[name];
        }

        int location = gl::glGetUniformLocation(m_ProgramID, name.c_str());
        if(location == -1)
        {
            std::cerr 
                << "[ERROR] Uniform \"" << name << "\" not found for Shader("
                << "name=" << m_DebugName << ")";
        }

        m_UniformLocationCache[name] = location;

        return location;
    }

    std::shared_ptr<Shader> Shader::LoadFromFile(const std::string& filepath, 
                                                 const std::string& debug_name)
    {
        ShaderSources sources;
        read_shader_source_from_file(filepath, sources);
        return std::make_shared<Shader>(sources, debug_name);
    }
    
}

namespace {
    bool read_shader_source_from_file(const std::string& filepath, ShaderSources& outSources) 
    {
        std::fstream fileStream(filepath);

        if(fileStream.fail())
        {
            std::cerr << "[ERROR] Could not open shader file: " << filepath;
            return false;
        }

        std::stringstream writer;
        bool writing = false;

        ShaderType currentType;

        std::string line;
        int lineNumber = 0;

        while(getline(fileStream, line))
        {
            lineNumber++;
            // if(line.empty())
                // continue;

            if(line.rfind("#ShaderSegment:", 0) == 0)
            {
                if(writing)
                {
                    outSources[currentType] = writer.str();
                    std::stringstream().swap(writer);
                }

                writing = true;

                if(line.find("vertex") != std::string::npos)
                    currentType = ShaderType::Vertex;

                else if(line.find("fragment") != std::string::npos)
                    currentType = ShaderType::Fragment;

                else
                {
                    std::cout << 
                        "[WARN] Invalid shader segment at line " << lineNumber << ", Skipping segment";
                    writing = false;
                }

                continue;
            }

            if(writing)
                writer << line << "\n";
        }

        if(writing)
        {
            outSources[currentType] = writer.str();
        }

        return true;
    }
}