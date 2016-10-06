#version 410

in vec3 vp;
uniform mat4 M,V,P;

void main () {
	gl_Position = P*V*M*vec4(vp, 1.0);
}