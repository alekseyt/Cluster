$input a_position, a_normal, a_tangent, a_texcoord0
$output v_worldpos, v_normal, v_tangent, v_texcoord0

#include <bgfx_shader.sh>

void main()
{
    v_worldpos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    v_normal = mul(u_modelView, vec4(a_normal, 0.0)).xyz;
    v_tangent = mul(u_modelView, vec4(a_tangent, 0.0)).xyz;
    v_texcoord0 = a_texcoord0;
}
