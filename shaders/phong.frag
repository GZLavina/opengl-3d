#version 460

in vec3 finalColor;
in vec2 texCoord;
in vec3 scaledNormal;
in vec3 fragPos;

//surface
uniform float ka;
uniform float ks;
uniform float kd;
uniform float q;

//light properties
uniform vec3 lightPos;
uniform vec3 lightColor;

//camera properties
uniform vec3 cameraPos;

out vec4 color;

void main() {

    vec3 ambient = ka * lightColor;

    vec3 N = normalize(scaledNormal);
    vec3 L = normalize(lightPos - fragPos);
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = kd * diff * lightColor;

    vec3 V = normalize(cameraPos - fragPos);
    vec3 R = normalize(reflect(-L, N));
    float spec = max(dot(R, V), 0.0);
    spec = pow(spec, q);
    vec3 specular = ks * spec * lightColor;

    vec3 result = (ambient + diffuse) * finalColor + specular;

    color = vec4(result, 1.0);
}