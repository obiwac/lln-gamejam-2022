#version 300 es

precision mediump float;

layout(location=0) out vec4 color;

in vec2 texCoord;

uniform sampler2D bloom_texture;
uniform vec2 TexelSize;

void main(void)
{
    vec4 A = texture(bloom_texture, texCoord + TexelSize * vec2(-1.0, -1.0));
	vec4 B = texture(bloom_texture, texCoord + TexelSize * vec2(0.0, -1.0));
	vec4 C = texture(bloom_texture, texCoord + TexelSize * vec2(1.0, -1.0));
	vec4 D = texture(bloom_texture, texCoord + TexelSize * vec2(-0.5, -0.5));
	vec4 E = texture(bloom_texture, texCoord + TexelSize * vec2(0.5, -0.5));
	vec4 F = texture(bloom_texture, texCoord + TexelSize * vec2(-1.0, 0.0));
	vec4 G = texture(bloom_texture, texCoord);
	vec4 H = texture(bloom_texture, texCoord + TexelSize * vec2(1.0, 0.0));
	vec4 I = texture(bloom_texture, texCoord + TexelSize * vec2(-0.5, 0.5));
	vec4 J = texture(bloom_texture, texCoord + TexelSize * vec2(0.5, 0.5));
	vec4 K = texture(bloom_texture, texCoord + TexelSize * vec2(-1.0, 1.0));
	vec4 L = texture(bloom_texture, texCoord + TexelSize * vec2(0.0, 1.0));
	vec4 M = texture(bloom_texture, texCoord + TexelSize * vec2(1.0, 1.0));

	vec2 div = (1.0 / 4.0) * vec2(0.5, 0.125);

	vec4 o = (D + E + I + J) * div.x;
	o += (A + B + G + F) * div.y;
	o += (B + C + H + G) * div.y;
	o += (F + G + L + K) * div.y;
	o += (G + H + M + L) * div.y;
    color = o;
}
