#version 450

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec4 Color;
layout (location = 0) out vec2 outTexCoord;
layout (location = 1) out vec4 outColor;

layout (set = 1, binding = 0) uniform UniformBlock
{
	mat4x4 MatrixTransform;
};

void main()
{
	outColor = Color;
	outTexCoord = TexCoord;
	gl_Position = MatrixTransform * vec4(Position, 1);
}
