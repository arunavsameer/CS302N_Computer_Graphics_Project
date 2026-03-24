#version 120

uniform sampler2D texture1;

void main() {
    // Sample the color from the texture and multiply by the cube's base color
    vec4 texColor = texture2D(texture1, gl_TexCoord[0].st);
    gl_FragColor = texColor * gl_Color;
}