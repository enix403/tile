#pragma once

#include <unordered_map>
#include <string>
#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>


using CnstStrRef = const std::string&;

namespace Tile
{
    enum class ShaderType 
    {
        Vertex,
        Fragment
    };

    using ShaderSources = std::unordered_map< ShaderType, std::string>;

    class Shader 
    {
    public:
        Shader(const ShaderSources& sources, const std::string& debug_name);

        inline unsigned int GetID() const { return m_ProgramID; }

        void Bind() const;
        void Unbind() const;

    public:

        void SetUniformIntArray(const std::string& name, int* values, uint32_t count);
        void SetUniformFloatArray(const std::string& name, float* values, uint32_t count);

        void SetUniformInt(const std::string& name, int value);
        void SetUniformFloat(const std::string& name, float values);

        void SetUniformFloat2(const std::string& name, const glm::vec2& value);
        void SetUniformFloat3(const std::string& name, const glm::vec3& value);
        void SetUniformFloat4(const std::string& name, const glm::vec4& value);

        void SetUniformMat3(const std::string& name, const glm::mat3& value);
        void SetUniformMat4(const std::string& name, const glm::mat4& value);

        static std::shared_ptr<Shader> LoadFromFile(const std::string& filepath, 
                                                    const std::string& debug_name);

    private:
        int GetUniformLocation(const std::string& name) const;

    private:
        unsigned int m_ProgramID;
        mutable std::unordered_map<std::string, int> m_UniformLocationCache;

        const std::string m_DebugName;
    };
}