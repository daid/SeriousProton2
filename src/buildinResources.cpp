#include <sp2/io/internalResourceProvider.h>

namespace sp {

static io::InternalResourceProvider buildin_resources({
    {"basic.shader", R"EOS(
[VERTEX]
#version 110

uniform mat4 projection_matrix;
uniform mat4 camera_matrix;
uniform mat4 object_matrix;
uniform vec3 object_scale;

varying vec2 v_uv;

void main()
{
    gl_Position = projection_matrix * camera_matrix * object_matrix * vec4(gl_Vertex.xyz * object_scale, 1.0);
    v_uv = gl_MultiTexCoord0.xy;
}

[FRAGMENT]
#version 110
uniform sampler2D texture_map;
uniform vec4 color;

varying vec2 v_uv;

void main()
{
    gl_FragColor = texture2D(texture_map, v_uv) * color;
}
)EOS"},


    {"color.shader", R"EOS(
[VERTEX]
#version 110

uniform mat4 projection_matrix;
uniform mat4 camera_matrix;
uniform mat4 object_matrix;
uniform vec3 object_scale;

void main()
{
    gl_Position = projection_matrix * camera_matrix * object_matrix * vec4(gl_Vertex.xyz * object_scale, 1.0);
}

[FRAGMENT]
#version 110
uniform sampler2D texture_map;
uniform vec4 color;

varying vec2 v_uv;

void main()
{
    gl_FragColor = color;
}
)EOS"},

    {"particle.shader", R"EOS(
[VERTEX]
#version 110

uniform mat4 projection_matrix;
uniform mat4 camera_matrix;
uniform mat4 object_matrix;
uniform vec3 object_scale;

varying vec3 color;
varying vec2 v_uv;

void main()
{
    vec4 position = camera_matrix * object_matrix * vec4(gl_Vertex.xyz * object_scale, 1.0);
    position += vec4(gl_MultiTexCoord0.xy, 0, 0);
    gl_Position = projection_matrix * position;
    v_uv.x = (gl_MultiTexCoord0.x / abs(gl_MultiTexCoord0.x)) / 2.0 + 0.5;
    v_uv.y = (gl_MultiTexCoord0.y / abs(gl_MultiTexCoord0.y)) / 2.0 + 0.5;
    color.xyz = gl_Normal.xyz;
}

[FRAGMENT]
#version 110
uniform sampler2D texture_map;

varying vec3 color;
varying vec2 v_uv;

void main()
{
    vec4 c = texture2D(texture_map, v_uv);
    gl_FragColor.rgb = c.xyz * color;
    gl_FragColor.a = c.a;
}
)EOS"},

});

};//!namespace sp