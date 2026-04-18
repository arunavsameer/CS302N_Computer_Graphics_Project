#version 120

void main() {
    // Transform vertex position to screen space
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    
    // Pass along texture coordinates and vertex colours
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_FrontColor = gl_Color;
}