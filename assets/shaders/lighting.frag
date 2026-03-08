uniform sampler2D texture;
uniform vec2 u_lightPos;
uniform float u_time;

void main() {
    vec2 uv = gl_TexCoord[0].xy;
    vec4 color = texture2D(texture, uv);

    float distToLight = distance(uv, u_lightPos);
    float spotlight = smoothstep(0.48, 0.0, distToLight);

    float distToCenter = distance(uv, vec2(0.5, 0.5));
    float vignette = smoothstep(0.98, 0.22, distToCenter);

    float pulse = 0.03 * sin(u_time * 2.7 + uv.y * 35.0);
    float brightness = 0.70 + spotlight * 0.56 + pulse;
    float contrast = 0.88 + vignette * 0.24;

    color.rgb *= brightness * contrast;
    gl_FragColor = color;
}
