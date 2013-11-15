#version 330

uniform mat4 viewProjection;
in vec3 pos;


void main() {
	gl_Position = viewProjection*vec4(pos, 1.0);
}