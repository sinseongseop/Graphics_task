#version 150

in  vec4 vPosition;
in  vec4 vNormal;
in  vec2 vTexCoord;

out vec4 color;
out vec2 texCoord;

uniform mat4 mProject;
uniform mat4 mView;
uniform mat4 mModel;

void main() 
{
	gl_Position = mProject * mView * mModel * vPosition;

	//GOURAUD shading 이용 & Directional Light라 생각

	vec4 vColor = vec4(1, 1, 1, 1);
	vec4 L = normalize(vec4(3, 3, 5, 0));
	float kd = 0.8, ks = 1.0, ka = 0.2, shininess = 60;
	vec4 Id = vColor;
	vec4 Is = vec4(1, 1, 1, 1);
	vec4 Ia = vColor;

	// ambient
	float ambient = ka;

	// diffuse
	vec4 normal = transpose(inverse(mModel)) * vNormal;
	vec4 N = normalize(normal);
	float diff = kd * clamp(dot(N, L), 0, 1);

	// specular
	vec4 viewPos = inverse(mView) * vec4(0, 0, 0, 1);
	vec4 worldPos = mModel * vPosition;
	vec4 V =  normalize(viewPos - worldPos);
	vec4 R = reflect(-L, N);
	float spec = ks * pow(clamp(dot(V, R), 0, 1), shininess);

	color = ambient * Ia + diff * Id + spec * Is;

	// texture coordinate
	texCoord = vTexCoord;
}