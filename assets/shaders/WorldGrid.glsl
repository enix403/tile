#ShaderSegment:vertex
#version 420 core

layout (location = 0) in float _trigger;

uniform mat4 u_ProjectionView;

vec2 gridPlane[6] = vec2[](
    vec2(-1, -1), vec2(-1,  1), vec2( 1,  1),
    vec2(-1, -1), vec2( 1,  1), vec2( 1, -1)
);

out vec3 fragNearPoint;
out vec3 fragFarPoint;

void main()
{   
    vec2 p = gridPlane[gl_VertexID];

    mat4 invProjView = inverse(u_ProjectionView);
    
    vec4 unprojectedNear = invProjView * vec4(p.xy, -1.0, 1.0);
    vec4 unprojectedFar  = invProjView * vec4(p.xy, +1.0, 1.0);

    fragNearPoint = unprojectedNear.xyz / unprojectedNear.w;
    fragFarPoint = unprojectedFar.xyz / unprojectedFar.w;

    gl_Position = vec4(p, 0.0, 1.0);
}

#ShaderSegment:fragment
#version 420 core

precision highp float;

in vec3 fragNearPoint;
in vec3  fragFarPoint;

uniform mat4 u_ProjectionView;

uniform float u_CamNear;
uniform float u_CamFar;

out vec4 fragColor;

vec4 GridColor(vec3 fragPos3D, float scale, bool drawAxis) {
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
    // z axis
    if(fragPos3D.x > -0.1 * minimumx && fragPos3D.x < 0.1 * minimumx)
        color.z = 1.0;
    // x axis
    if(fragPos3D.z > -0.1 * minimumz && fragPos3D.z < 0.1 * minimumz)
        color.x = 1.0;
    return color;
}

float CalculateFragDepth(vec3 pos) {
    vec4 clipSpacePosition = u_ProjectionView * vec4(pos.xyz, 1.0);
    return clipSpacePosition.z / clipSpacePosition.w;
}

float ComputeLinearDepth(vec3 pos) {
    vec4 clipSpacePosition = u_ProjectionView * vec4(pos.xyz, 1.0);
    float depth = clipSpacePosition.z / clipSpacePosition.w;

    // get linear value between 0.01 and 100
    float linearDepth = (2.0 * u_CamNear * u_CamFar) / (u_CamFar + u_CamNear - depth * (u_CamFar - u_CamNear)); 
    return linearDepth / u_CamFar; // normalize
}


void main()
{
    float t = -fragNearPoint.y / (fragFarPoint.y - fragNearPoint.y);
    vec3 surfaceIntersection = mix(fragNearPoint, fragFarPoint, t);

    float worldDepth = CalculateFragDepth(surfaceIntersection);

    // convert depth from range [-1, +1] to [0, +1]
    gl_FragDepth = (worldDepth + 1.0) / 2.0;

    if (t <= 0 || t >= 1)
        discard;

    float linearDepth = ComputeLinearDepth(surfaceIntersection);
    float fading = max(0, (0.5 - linearDepth));

    // adding multiple resolution for the grid
    fragColor = (GridColor(surfaceIntersection, 5, true) + GridColor(surfaceIntersection, 1, true));
    fragColor.a *= fading;
}
