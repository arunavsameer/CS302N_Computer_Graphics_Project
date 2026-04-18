#version 120

uniform sampler2D texture1;
uniform int u_nightMode;   // 1 = night, 0 = day

void main() {
    // Sample texture, modulate by vertex colour (same as before)
    vec4 col = texture2D(texture1, gl_TexCoord[0].st) * gl_Color;

    if (u_nightMode == 1) {
        // Darken scene and push toward a cool blue-indigo tint,
        // simulating moonlight / low-ambient night sky.
        // Tweak the multipliers here to taste.
        col.rgb *= vec3(0.22, 0.26, 0.42);
    }

    gl_FragColor = col;
}