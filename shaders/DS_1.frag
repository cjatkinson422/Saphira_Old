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

struct DirLight{
	vec3 angle;
	vec3 color;
	float strength;
};


uniform sampler2D tex_0_diffuse;
uniform sampler2D tex_0_spec;

uniform dvec3 camPos;

//uniform int num_pt_lights;
uniform PointLight sun;

vec3 CalDirLight(DirLight light, vec3 viewDir);
vec3 CalPointLight(PointLight light, vec3 viewDir);


void main()
{
	float ambientStrength = 0.002;
	
	vec3 ambient = ambientStrength*vec3(1.0,1.0,1.0);
	vec3 viewDir = normalize(vec3(-camPos)-FragPos);

	vec3 lightFac = vec3(0.0,0.0,0.0);


	//lightFac += CalDirLight(sun, viewDir);
	lightFac += CalPointLight(sun, viewDir);

	FragColor = texture2D(tex_0_diffuse, texCoord) * vec4((ambient+lightFac),1.0);
}



vec3 CalPointLight(PointLight light, vec3 viewDir){
	vec3 lightVec = vec3(light.position) - FragPos;
	vec3 lightDir = normalize(lightVec);
	
	float angle = clamp(dot(norm, lightDir),0,1);
	float dist = length(lightVec);
	float attn = light.attn_str/(1.0+light.attn_lin*dist+light.attn_quad*dist*dist);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(clamp(dot(reflectDir, viewDir),0.0,1.0),128);

	vec3 specMap = vec3(texture2D(tex_0_spec, texCoord));

	vec3 diffuse = angle*light.color*attn;
	vec3 specular = light.color*spec*specMap*attn;

	vec3 lightFac = diffuse + specular;
	return lightFac;
}

vec3 CalDirLight(DirLight light, vec3 viewDir){
	vec3 refDir = reflect(light.angle, norm);
	float spec = pow(max(dot(refDir, viewDir),0.0),64);
	
	float angle = clamp(dot(norm, -light.angle),0,1);
	vec3 specMap = vec3(texture2D(tex_0_spec, texCoord));

	vec3 diffuse = light.strength*angle*light.color;
	vec3 specular = light.strength*light.color*spec*specMap;
	vec3 lightFac = diffuse+specular;
	return lightFac;
}
