#version 330 core

layout(location = 0) in vec3 vertex_position_modelspace;
uniform mat4 model_view_projection;

// Notice that the "1" here equals the "1" in glVertexAttribPointer
layout(location = 1) in vec3 vertex_color;

// Output data ; will be interpolated for each fragment.
out vec3 fragment_color;

void main() {
    // Output position of the vertex, in clip space : model_view_projection * position
    vec4 v = vec4(vertex_position_modelspace, 1);
    gl_Position = model_view_projection * v;

    fragment_color = vertex_color;
}
