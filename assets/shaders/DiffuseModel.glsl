#ShaderSegment:vertex
#version 420 core

layout (location = 0) in vec3 ia_Pos;
layout (location = 1) in vec3 ia_Normal;
layout (location = 2) in vec2 ia_TexCoords;

uniform mat4 u_Transform;
uniform mat4 u_Model;

out vec3 fragNormal;
out vec2 texCoords;

void main()
{   
    gl_Position = u_Transform * vec4(ia_Pos, 1.0);

    // FIXME: will not work if u_Model contains non-uniform scaling
    vec3 worldNormal = normalize(mat3(u_Model) * ia_Normal);
    fragNormal = worldNormal;
    texCoords = ia_TexCoords;
}

#ShaderSegment:fragment
#version 420 core

// const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, 1.5, -1.0));
const float AMBIENT_LIGHT = 0.55;

uniform vec3 u_Color;
uniform vec3 u_DirectionToLight;

// Whether to sample from texture or use solid color.
//
// An int is used here because of shaky support of bool in various OpenGL drivers
uniform int u_ShouldSampleTexture;
uniform sampler2D u_Texture;

in vec3 fragNormal;
in vec2 texCoords;

out vec4 fout_FragColor;

void main()
{   
    float lightIntensity = AMBIENT_LIGHT + max(0, dot(normalize(fragNormal), u_DirectionToLight)) * 0.5;
    vec3 fragSampleColor;

    if (u_ShouldSampleTexture == 1) 
        fragSampleColor = texture(u_Texture, texCoords).xyz;
    else
        fragSampleColor = u_Color * lightIntensity;
        
    fout_FragColor = vec4(fragSampleColor, 1.0);
}
