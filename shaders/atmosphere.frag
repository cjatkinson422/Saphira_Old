#version 440 core

out vec4 FragColor;

in vec3 norm;

uniform dvec3 atmoColor;

void main()
{
	FragColor = vec4(vec3(atmoColor), 0.2);
}
