#version 300 es

precision mediump float;

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add here your custom variables
uniform int renderWidth;
uniform int renderHeight;
uniform float pixelSize;

uniform int impactFrame;

void main()
{
    float dx = pixelSize * (1.0 / float(renderWidth));
    float dy = pixelSize * (1.0 / float(renderHeight));

    vec2 coord = vec2(dx*floor(fragTexCoord.x/dx), dy*floor(fragTexCoord.y/dy));

    vec3 tc = texture(texture0, coord).rgb;

    if (impactFrame == 1) {
        float avg = (tc.r + tc.g + tc.b) / 3.0;
        //tc = vec3(avg, avg, avg);
        float impactFactor = 5.0;
        if (avg < 0.5) {
            tc = vec3(avg / impactFactor, avg / impactFactor, avg / impactFactor);
        } else {
            tc = vec3(avg * impactFactor, avg * impactFactor, avg * impactFactor);
        }

    }

    finalColor = vec4(tc, 1.0);
}