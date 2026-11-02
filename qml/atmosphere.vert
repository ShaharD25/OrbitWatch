// SPDX-License-Identifier: GPL-3.0-or-later
VARYING vec3 viewPosition;
VARYING vec3 viewNormal;
VARYING vec3 worldNormal;
void MAIN() {
    viewPosition = (VIEW_MATRIX * MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
    worldNormal = normalize(NORMAL_MATRIX * NORMAL);
    viewNormal = normalize(mat3(VIEW_MATRIX) * NORMAL_MATRIX * NORMAL);
    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
}
