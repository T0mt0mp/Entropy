#version 330 core

in vec3 fragColor;
in vec3 vertexPos;

out vec3 color;

uniform float negativeBorder = -0.9;
uniform float positiveBorder = 0.9;
uniform vec3 borderColor = vec3(0.46, 0.46, 0.82);

void main()
{
	bool xBorder = (vertexPos.x < negativeBorder || vertexPos.x > positiveBorder);
	bool yBorder = (vertexPos.y < negativeBorder || vertexPos.y > positiveBorder);
	bool zBorder = (vertexPos.z < negativeBorder || vertexPos.z > positiveBorder);
    if ((xBorder && zBorder) || (yBorder && zBorder) || (yBorder && xBorder))
	{
	    color = borderColor;
	}
	else
	{
	    color = fragColor;
	}
	//color = fragColor;
}

