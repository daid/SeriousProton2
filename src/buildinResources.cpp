#include <sp2/io/internalResourceProvider.h>

namespace sp {

static io::InternalResourceProvider buildin_resources({
    {"basic.shader", R"EOS(
[VERTEX]
attribute vec3 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_uv;

uniform mat4 projection_matrix;
uniform mat4 camera_matrix;
uniform mat4 object_matrix;
uniform vec3 object_scale;

varying vec2 v_uv;

void main()
{
    gl_Position = projection_matrix * camera_matrix * object_matrix * vec4(a_vertex.xyz * object_scale, 1.0);
    v_uv = a_uv.xy;
}

[FRAGMENT]
uniform sampler2D texture_map;
uniform vec4 color;

varying vec2 v_uv;

void main()
{
    gl_FragColor = texture2D(texture_map, v_uv) * color;
    if (gl_FragColor.a == 0.0)
        discard;
}
)EOS"},

    {"basic_shaded.shader", R"EOS(
[VERTEX]
attribute vec3 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_uv;

uniform mat4 projection_matrix;
uniform mat4 camera_matrix;
uniform mat4 object_matrix;
uniform vec3 object_scale;

varying vec2 v_uv;
varying vec3 v_offset;
varying vec3 v_normal;

void main()
{
    gl_Position = projection_matrix * camera_matrix * object_matrix * vec4(a_vertex.xyz * object_scale, 1.0);
    v_uv = a_uv.xy;
    v_normal = (camera_matrix * object_matrix * vec4(a_normal, 0.0)).xyz;
    v_offset = (camera_matrix * object_matrix * vec4(a_vertex.xyz * object_scale, 1.0)).xyz;
}

[FRAGMENT]
uniform sampler2D texture_map;
uniform vec4 color;

varying vec2 v_uv;
varying vec3 v_offset;
varying vec3 v_normal;

void main()
{
    gl_FragColor = texture2D(texture_map, v_uv) * color;
    gl_FragColor.rgb = gl_FragColor.rgb * -dot(normalize(v_normal), normalize(v_offset));
    if (gl_FragColor.a == 0.0)
        discard;
}
)EOS"},

    {"color.shader", R"EOS(
[VERTEX]
attribute vec3 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_uv;

uniform mat4 projection_matrix;
uniform mat4 camera_matrix;
uniform mat4 object_matrix;
uniform vec3 object_scale;

void main()
{
    gl_Position = projection_matrix * camera_matrix * object_matrix * vec4(a_vertex.xyz * object_scale, 1.0);
}

[FRAGMENT]
uniform sampler2D texture_map;
uniform vec4 color;

void main()
{
    gl_FragColor = color;
    if (gl_FragColor.a == 0.0)
        discard;
}
)EOS"},

    {"color_shaded.shader", R"EOS(
[VERTEX]
attribute vec3 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_uv;

uniform mat4 projection_matrix;
uniform mat4 camera_matrix;
uniform mat4 object_matrix;
uniform vec3 object_scale;

varying vec2 v_uv;
varying vec3 v_offset;
varying vec3 v_normal;

void main()
{
    gl_Position = projection_matrix * camera_matrix * object_matrix * vec4(a_vertex.xyz * object_scale, 1.0);
    v_normal = (camera_matrix * object_matrix * vec4(a_normal, 0.0)).xyz;
    v_offset = (camera_matrix * object_matrix * vec4(a_vertex.xyz * object_scale, 1.0)).xyz;
}

[FRAGMENT]
uniform sampler2D texture_map;
uniform vec4 color;

varying vec3 v_offset;
varying vec3 v_normal;

void main()
{
    gl_FragColor = color;
    gl_FragColor.rgb = gl_FragColor.rgb * -dot(normalize(v_normal), normalize(v_offset));
    if (gl_FragColor.a == 0.0)
        discard;
}
)EOS"},

    {"normal_as_color.shader", R"EOS(
[VERTEX]
attribute vec3 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_uv;

uniform mat4 projection_matrix;
uniform mat4 camera_matrix;
uniform mat4 object_matrix;
uniform vec3 object_scale;

varying vec3 v_col;

void main()
{
    gl_Position = projection_matrix * camera_matrix * object_matrix * vec4(a_vertex.xyz * object_scale, 1.0);
    v_col = a_normal;
}

[FRAGMENT]
uniform sampler2D texture_map;
uniform vec4 color;

varying vec3 v_col;

void main()
{
    gl_FragColor = vec4(v_col, color.a);
}
)EOS"},

    {"local_particle.shader", R"EOS(
[VERTEX]
attribute vec3 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_uv;

uniform mat4 projection_matrix;
uniform mat4 camera_matrix;
uniform mat4 object_matrix;
uniform vec3 object_scale;

varying vec4 color;
varying vec2 v_uv;

void main()
{
    vec4 position = camera_matrix * object_matrix * vec4(a_vertex.xyz * object_scale, 1.0);
    vec2 uv = vec2(a_uv.x, abs(a_uv.x) * sign(a_uv.y));
    position += vec4(uv, 0, 0);
    gl_Position = projection_matrix * position;
    v_uv.x = sign(uv.x) / 2.0 + 0.5;
    v_uv.y = sign(uv.y) / 2.0 + 0.5;
    color.rgb = a_normal.xyz;
    color.a = abs(a_uv.y);
}

[FRAGMENT]
uniform sampler2D texture_map;

varying vec4 color;
varying vec2 v_uv;

void main()
{
    vec4 c = texture2D(texture_map, v_uv);
    gl_FragColor = c * color;
}
)EOS"},

    {"global_particle.shader", R"EOS(
[VERTEX]
attribute vec3 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_uv;

uniform mat4 projection_matrix;
uniform mat4 camera_matrix;
uniform mat4 object_matrix;
uniform vec3 object_scale;

varying vec4 color;
varying vec2 v_uv;

void main()
{
    vec4 position = camera_matrix * vec4(a_vertex.xyz * object_scale, 1.0);
    vec2 uv = vec2(a_uv.x, abs(a_uv.x) * sign(a_uv.y));
    position += vec4(uv, 0, 0);
    gl_Position = projection_matrix * position;
    v_uv.x = sign(uv.x) / 2.0 + 0.5;
    v_uv.y = sign(uv.y) / 2.0 + 0.5;
    color.rgb = a_normal.xyz;
    color.a = abs(a_uv.y);
}

[FRAGMENT]
uniform sampler2D texture_map;

varying vec4 color;
varying vec2 v_uv;

void main()
{
    vec4 c = texture2D(texture_map, v_uv);
    gl_FragColor = c * color;
}
)EOS"},

});

}//namespace sp
