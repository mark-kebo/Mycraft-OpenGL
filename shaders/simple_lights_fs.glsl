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

layout(std140, binding = 5) uniform PointLight
{
    vec3 position_ws;
    float r;
    vec3 color;
} point_light;

void main()
{
    vec3 diffuse = vec3(0.f, 0.f, 0.f);
    vec3 specular = vec3(0.f, 0.f, 0.f);

    //obliczenia zwiazane z oswietleniem...
	vec3 norm = normalize(normal_ws);

	diffuse += ambient_light.ambient_color; 

	vec3 intens = 1 - point_light.position_ws/point_light.r; // (1-t/r)^2
	//float theta     = dot(lightDir, normalize(-light.direction));
	//float epsilon   = light.cutOff - light.outerCutOff;
	//float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);  

	vec3 lightDir = normalize(point_light.position_ws - position_ws);  
	float diff = max(dot(norm, lightDir), 0.0);
	diffuse += diff * point_light.color * intens;

	vec3 viewDir = normalize(additional_data.camera_position_ws - position_ws);
	vec3 reflectDir = reflect(-lightDir, norm); 
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 256);
	specular +=  spec * point_light.color * intens; 

    vec3 result_color = clamp(diffuse * material.color * texture(color_tex, tex_uv).rgb + specular, 0.f, 1.f);
    frag_color = vec4(result_color, 1.f);
	//frag_color = vec4(norm, 1.f); // <----- debug
}