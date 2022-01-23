layout(location = POSITION_ATTRIBUTE_LOCATION) in vec3 aPos;
layout(location = NORMAL_ATTRIBUTE_LOCATION) in vec3 aNormal;
layout(location = TEXTURECOORDINATES_ATTRIBUTE_LOCATION) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

layout(location = 0) uniform mat4 projView;
layout(location = 1) uniform mat4 model;

void main()
{
	TexCoords = aTexCoords;
	WorldPos = vec3(model * vec4(aPos, 1.0));
	Normal = mat3(model) * aNormal;

	gl_Position =  projView * vec4(WorldPos, 1.0);
}
