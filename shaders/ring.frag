#version 440 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 norm;
in vec3 FragPos;

struct PointLight{
	vec3 color;
	dvec3 position;
	float attn_str;
	float attn_lin;
	float attn_quad;
};



uniform sampler2D tex_0_diffuse;



//uniform int num_pt_lights;
uniform PointLight sun;

vec3 CalPointLight(PointLight light);


void main()
{
	float ambientStrength = 0.00;
	
	vec3 ambient = ambientStrength*sun.color;

	vec3 lightFac = vec3(0.0,0.0,0.0);

	lightFac += CalPointLight(sun);


	FragColor = texture2D(tex_0_diffuse, texCoord) * vec4((ambient+lightFac),1.0);
}



vec3 CalPointLight(PointLight light){
	vec3 lightVec = vec3(light.position) - FragPos;
	vec3 lightDir = normalize(lightVec);
	
	float angle = max(dot(norm, lightDir),dot(-norm, lightDir));
	float dist = length(lightVec);
	float attn = light.attn_str/(1.0+light.attn_lin*dist+light.attn_quad*dist*dist);

	vec3 diffuse = 100.0*angle*light.color*attn;

	return diffuse;
}
