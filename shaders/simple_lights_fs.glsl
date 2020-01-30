#version 420

in vec3 position_ws;
in vec3 normal_ws;
in vec2 tex_uv;

out vec4 frag_color;

layout(binding = 0) uniform sampler2D color_tex;

layout(std140, binding = 2) uniform AdditionalData
{
    vec3 camera_position_ws;
} additional_data;

layout(std140, binding = 3) uniform Material
{
    vec3 color;
    float specular_intensity;
    float specular_power;
} material;

layout(std140, binding = 4) uniform AmbientLight
{
    vec3 ambient_color;
} ambient_light;

struct PointLightData {
    vec3 position_ws;
    float r;
    vec3 color;
	float some1;
};

layout(std140, binding = 5) uniform PointLight
{
	int n;
	float some1;
	float some2;
	float some3;
	PointLightData pointLightData[16];
} point_light;

layout(std140, binding = 7) uniform Option
{
	bool isHasLights;
	float some1;
	bool isUseTexture;
	float some2;
} option;

//change color by chach
vec3 createColor(in vec2 uv) {
	uv = fract(uv);
	vec3 colors_tab[2] = {vec3(0.f,0.f,0.f), vec3(1.f,1.f,1.f)};
	uvec2 board_ids = uvec2(step(0.5f, uv));
	uint index = board_ids.x^board_ids.y;
	return colors_tab[index];
}

void main()
{
    vec3 diffuse = vec3(0.f, 0.f, 0.f);
    vec3 specular = vec3(0.f, 0.f, 0.f);

    //obliczenia zwiazane z oswietleniem...
	vec3 norm = normalize(normal_ws);

	for(int i = 0; i < point_light.n; ++i) {
		vec3 light_length = point_light.pointLightData[i].position_ws - position_ws;
		float intens = clamp((point_light.pointLightData[i].r - length(light_length))/point_light.pointLightData[i].r, 0.f, 1.f);
		intens = intens * intens;

		diffuse += ambient_light.ambient_color; 
		vec3 lightDir = normalize(point_light.pointLightData[i].position_ws - position_ws);  
		float diff = max(dot(norm, lightDir), 0.0);
		diffuse += diff * point_light.pointLightData[i].color * intens;

		vec3 viewDir = normalize(additional_data.camera_position_ws - position_ws);
		vec3 reflectDir = reflect(-lightDir, norm); 
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
		specular +=  spec * point_light.pointLightData[i].color * intens; 
	}

    vec3 result_color = clamp(diffuse * material.color * texture(color_tex, tex_uv).rgb + specular, 0.f, 1.f);
	vec4 text_color = option.isUseTexture ? vec4(texture(color_tex, tex_uv).rgb, 1.f) : vec4(createColor(tex_uv), 1.f);
    frag_color = option.isHasLights ? vec4(result_color, 1.f) : text_color;
	//frag_color = vec4(norm, 1.f); // <----- debug
}
