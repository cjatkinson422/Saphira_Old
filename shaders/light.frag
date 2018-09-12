#version 440 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 norm;

uniform sampler2D tex_0_diffuse;

void main()
{
	FragColor = texture2D(tex_0_diffuse, texCoord);
}
