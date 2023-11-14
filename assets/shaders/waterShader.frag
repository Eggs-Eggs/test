#version 330 core
in vec2 TexCoords;
in vec3 fragNormal;
in vec3 worldPos;

out vec4 color;

uniform sampler2D texture_diffuse;
uniform vec3 viewPos;

vec3 getAmbientLight(float ambientStrength)
{
	return ambientStrength * vec3(1.0f, 1.0f, 1.0f);
}

vec3 getDiffuseLight(vec3 lightColor,vec3 normal,vec3 lightDir)
{
	float diff = max(dot(normal, lightDir), 0.0);
	return diff * lightColor;
}

vec3 getSpecularLight(float specularStrength,vec3 lightColor,vec3 normal,vec3 viewDir,vec3 lightDir)
{
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	return specularStrength * spec * lightColor; 
}

void main( )
{
	vec3 objectColor = vec3(0.0f, 0.0f, 0.5f);
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
	vec3 lightPos = vec3(0.0f, 100.0f, 0.0f);

	vec3 ambient = getAmbientLight(0.5);

	vec3 norm = normalize(fragNormal);
	vec3 lightDir = normalize(lightPos - worldPos);  

	vec3 diffuse = getDiffuseLight(lightColor, norm, lightDir);

	float specularStrength = 1;

	vec3 viewDir = normalize(viewPos - worldPos);
	vec3 specular = getSpecularLight(specularStrength, lightColor, norm, viewDir, lightDir); 

	vec3 result = (ambient + diffuse + specular)* objectColor;
	color = vec4(result , 1.0f);
}