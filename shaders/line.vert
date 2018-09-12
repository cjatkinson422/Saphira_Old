#version 440 core
layout(location = 0) in dvec3 aPos;

uniform dmat4 view;
uniform dmat4 projection;
uniform dmat4 model;

void main()
{
	gl_Position = vec4(projection * view * model * dvec4(aPos, 1.0));
}
