#ShaderSegment:vertex
#version 420 core

layout (location = 0) in vec3 ia_Pos;
layout (location = 1) in vec3 ia_Normal;

uniform mat4 u_Transform;
uniform mat4 u_Model;

out vec3 fragNormal;

void main()
{   
    gl_Position = u_Transform * vec4(ia_Pos, 1.0);

    // FIXME: will not work if u_Model contains non-uniform scaling
    vec3 worldNormal = normalize(mat3(u_Model) * ia_Normal);
    fragNormal = worldNormal;
}

#ShaderSegment:fragment
#version 420 core

// const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, 1.5, -1.0));
const float AMBIENT_LIGHT = 0.55;

uniform vec3 u_Color;
uniform vec3 u_DirectionToLight;

in vec3 fragNormal;
out vec4 fout_FragColor;

void main()
{   
    float lightIntensity = AMBIENT_LIGHT + max(0, dot(normalize(fragNormal), u_DirectionToLight)) * 0.5;
    vec3 fragColor = u_Color * lightIntensity;

    fout_FragColor = vec4(fragColor, 1.0);
}
