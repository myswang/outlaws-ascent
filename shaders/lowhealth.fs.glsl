#version 330
// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE


uniform sampler2D screen_texture;
uniform float time;
uniform float darken_screen_factor;
uniform bool isHealthy;
uniform bool isBlinded;
uniform bool isWet;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec2 distort(vec2 uv)
{
	if (!isHealthy && darken_screen_factor == 0.0)
	{
		float scale = 0.5 * (sin(time * 0.7) + 1.0);

		uv = texcoord * 2.0 - 1.0;
		vec2 dist = abs(uv);
		float factor = 1.0 + (0.03 * pow(max(dist.x, dist.y), 2)) * scale;

		uv /= factor;
		uv++;
		uv /= 2.0;
	}
	return uv;
}

vec4 color_shift(vec4 in_color) 
{
	if (!isHealthy && darken_screen_factor == 0.0)
	{
		vec2 center_dist = abs(texcoord - vec2(0.5, 0.5));
		float intensity = smoothstep(0.0, 0.7, pow(max(center_dist.x, center_dist.y), 2));

		vec4 shift = vec4(0.8, -0.4, -0.4, 0.0); // Shift towards a more intense red
		in_color += shift * intensity;
	}

	if (isWet && darken_screen_factor == 0.0)
   	{
   		vec2 center_dist = abs(texcoord - vec2(0.5, 0.5));
   		float intensity = smoothstep(0.0, 0.5, pow(max(center_dist.x, center_dist.y), 2));
       	vec4 shift = vec4(-0.4, -0.4, 0.8, 0.0); // Shift towards a more intense red
   		in_color += shift * intensity;
   	}
	
	if (isBlinded && darken_screen_factor == 0.0)
	{
		vec2 center_dist = abs(texcoord - vec2(0.5, 0.5));
		float intensity = smoothstep(0.0, 0.7, pow(max(center_dist.x, center_dist.y), 2));

		vec4 shift = vec4(-10.0, -10.0, -10.0, 0.0); // Shift towards a more intense red
		in_color += shift * intensity;
	}
	return in_color;
}

vec4 fade_color(vec4 in_color) 
{
	if (darken_screen_factor > 0)
		in_color -= darken_screen_factor * vec4(0.8, 0.8, 0.8, 0);
	return in_color;
}

void main()
{
	vec2 in_texcoord = texcoord;
	in_texcoord = distort(in_texcoord);
    vec4 in_color = texture(screen_texture, in_texcoord);
	
    color = color_shift(in_color);
	color = fade_color(color);
}