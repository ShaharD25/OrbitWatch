// SPDX-License-Identifier: GPL-3.0-or-later
// Visual approximation only: a subtle sunlit atmospheric limb, not a scattering model.
VARYING vec3 viewPosition;
VARYING vec3 viewNormal;
VARYING vec3 worldNormal;
void MAIN() {
    float facing = max(dot(normalize(viewNormal), normalize(-viewPosition)), 0.0);
    float rim = pow(1.0 - facing, 5.0);
    float daylight = smoothstep(-0.1, 0.25, dot(normalize(worldNormal), normalize(sunDirection)));
    FRAGCOLOR = vec4(0.20, 0.45, 0.70, rim * daylight * 0.22);
}
