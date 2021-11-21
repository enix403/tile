#ShaderSegment:vertex
#version 420 core

layout (location = 0) in vec3 ia_Pos;
layout (location = 1) in vec3 ia_Normal;

uniform mat4 u_Transform;
uniform mat4 u_Model;
uniform vec3 u_Color;

out vec4 i_FragColor;


const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, 3.0, -1.0));
const float AMBIENT_LIGHT = 0.55;


void main()
{   
    gl_Position = u_Transform * vec4(ia_Pos, 1.0);

    vec3 worldNormal = normalize(mat3(u_Model) * ia_Normal);

    float lightIntensity = AMBIENT_LIGHT + max(0, dot(worldNormal, DIRECTION_TO_LIGHT)) * 0.5;

    i_FragColor = vec4(u_Color * lightIntensity, 1.0);
}

#ShaderSegment:fragment
#version 420 core

in vec4 i_FragColor;
out vec4 FragColor;

void main()
{   
    FragColor = i_FragColor;
}
