#ShaderSegment:vertex

#version 420 core
layout (location = 0) in vec3 ia_Pos;
layout (location = 1) in vec3 ia_Normal;

uniform mat4 u_ProjectionView;
uniform mat4 u_Model;

out vec3 o_Normal;

void main()
{
    gl_Position = u_ProjectionView * u_Model * vec4(ia_Pos, 1.0);
    o_Normal = ia_Normal;
}

#ShaderSegment:fragment

#version 420 core

in vec3 o_Normal;

out vec4 FragColor;

uniform vec3 u_Color;

void main()
{   
    FragColor = vec4(u_Color + normalize(o_Normal) * 0.8, 1.0);
}
