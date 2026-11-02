// SPDX-License-Identifier: GPL-3.0-or-later
// Historical day/night composites; fixed presentation light, unrelated to scenario time.
void MAIN() {
    vec3 day = pow(texture(dayMap, UV0).rgb, vec3(2.2));
    vec3 nightImage = pow(texture(nightMap, UV0).rgb, vec3(2.2));
    float sunlight = dot(normalize(VAR_WORLD_NORMAL), normalize(sunDirection));
    float night = 1.0 - smoothstep(-0.15, 0.15, sunlight);
    float lights = max(nightImage.r, max(nightImage.g, nightImage.b));
    BASE_COLOR = vec4(day, 1.0);
    ROUGHNESS = 0.95;
    SPECULAR_AMOUNT = 0.08;
    // A little surface visibility keeps the globe readable on the night side.
    EMISSIVE_COLOR = night * (day * 0.025 + vec3(1.0, 0.65, 0.30) * pow(lights, 1.2) * 0.9);
}
