#version 330 core
layout ( location = 0 ) in vec3 position;
layout ( location = 1 ) in vec3 normal;
layout ( location = 2 ) in vec2 texCoords;

out vec2 TexCoords;
out vec3 fragNormal;
out vec3 worldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float water_time;

void main( )
{
	vec3 p = position;
	float speed = 0.1;
	float a = 0.1;
	float k = 10;
	float f = k * (position.x + speed * water_time);

	p.y = a * sin(f);

	gl_Position = projection * view * model * vec4( p, 1.0f );

	vec3 tangent = normalize(vec3(1, k * a * cos(f), 0));

	TexCoords = texCoords;
	fragNormal = vec3(-tangent.y, tangent.x, 0);
	worldPos = vec3(model * vec4( p, 1.0f ));
}