#version 440 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;

uniform dmat4 model;
uniform dmat4 view;
uniform dmat4 projection;


out vec3 norm;

void main()
{
	gl_Position = vec4(projection * view * model * vec4(aPos, 1.0));
	norm = normalize(vec3(model*vec4(aNorm,0.0)));
}
