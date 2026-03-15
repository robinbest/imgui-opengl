#version 330 core

out vec4 FragColor;
in vec3 vertexColor;
uniform vec3 tint;

void main()
{
    FragColor = vec4(tint * vertexColor, 1.0);
}