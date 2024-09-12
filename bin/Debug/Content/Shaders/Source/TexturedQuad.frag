#version 450

layout (location = 0) in vec2 TexCoord;
layout (location = 1) in vec4 Color;
layout (location = 0) out vec4 FragColor;
layout (set = 2, binding = 0) uniform sampler2D Sampler;

void main()
{
	FragColor = texture(Sampler, TexCoord) * Color;
	
	// if (FragColor.rgb == vec3(0, 0, 0)) {
	// 	FragColor = Color;
	// }
	
	// FragColor.rgb += dFdy(FragColor).rgb * vec3(3, 0, -3);
}
