#version 450 core
out vec4 fColor;
uniform vec3 CameraPos;

in Vertex
{
	vec3 worldPosition;
	vec3 normal;
};

struct Material {
	vec4 color;
	float ambientK;
	float diffuseK;
	float specularK;
	float shininess;
};
uniform Material Mat;

vec3 lightColor = vec3(1,1,1);

void main() {
	vec3 L = normalize(CameraPos - worldPosition.xyz);
	float d = dot(L,normal);
	// the light is at camera position so it is always shining at the visible side of the face
	d *= sign(d);
	float di = Mat.diffuseK*d;
	fColor = vec4(clamp(di*lightColor, 0,1), 1);
}
