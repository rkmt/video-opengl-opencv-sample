#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

#define NSEG 400
uniform vec2 pos[NSEG];
uniform int npos;
uniform vec3 param;
uniform int debug;
uniform int morph;
uniform float alpha;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

vec2 perpendicular(vec2 p) {
    return  vec2(p.y, -p.x);
}

// feature-based image metamorphosis 1segment version
void main_single() {
    vec2 P1 = pos[0];
    vec2 Q1 = pos[1];
    vec2 P2 = pos[2];
    vec2 Q2 = pos[3];

    vec2 XP = UV - P1;
    vec2 QP = Q1 - P1;
    float nQP = length(QP);
    vec2 QP2 = Q2 - P2;
    float nQP2 = length(QP2);

    float u = dot(XP, QP) / (nQP * nQP);
    float v = dot(XP, perpendicular(QP)) / nQP;
    vec2 xdash = P2[0] + u * QP2 + (v * perpendicular(QP2)) / nQP2;
    color.rgb = texture(myTextureSampler, xdash).rgb;
    color.a = 0.5;

    if (xdash.x < 0 || 1 < xdash.x || xdash.y < 0 || 1 < xdash.y) {
        color.rgb = vec3(1, 0, 1);
        color.a = 0.1;
    }

    if (npos != 2) {
        color.rgb = texture(myTextureSampler, UV).rgb;
        color.a = 1;
    }
}

// feature-based image metamorphosis multi-segment version
void main_morph() {
    // wight parameters
    float a = param[0]; // 300.0
    float b = param[1]; // 1.0
    float p = param[2]; // 0.0
    
    vec2 dsum = vec2(0, 0);
    float weightsum = 0;
    for (int i = 0; i < npos; i++) {
        int j = i * 4;
        vec2 P1 = pos[j];   // dest
        vec2 Q1 = pos[j+1];
        vec2 P2 = pos[j+2]; // soruce
        vec2 Q2 = pos[j+3];
        
        vec2 XP = UV - P1;
        vec2 QP = Q1 - P1;
        float nQP = length(QP);
        vec2 QP2 = Q2 - P2;
        float nQP2 = length(QP2);
    
        float u = dot(XP, QP) / (nQP * nQP);
        float v = dot(XP, perpendicular(QP)) / nQP;
        vec2 xdash = P2 + u * QP2 + (v * perpendicular(QP2)) / nQP2;

        vec2 Di = xdash - UV; // displacement
        float dist = abs(v);
        //float dist = v * nQP; // distance(UV, P1, Q1);
        if (u < 0) {
            dist = length(P1 - UV);
        }
        if (u > 1) {
            dist = length(Q1 - UV);
        }
        float weight = pow(pow(nQP, p) / (a + dist), b);
        dsum += weight * Di;
        weightsum += weight;
    }
    vec2 xdash = UV + dsum / weightsum;
    color.rgb = texture(myTextureSampler, xdash).rgb;
    color.a = 0.5;

    if (xdash.x < 0 || 1 < xdash.x || xdash.y < 0 || 1 < xdash.y) {
        color.a = 0;
    }
    
    if (debug != 0) {
        for (int i = 0; i < npos; i++) {
            int j = i * 4;
            vec2 P1 = pos[j];   // dest
            vec2 Q1 = pos[j+1];
            vec2 P2 = pos[j+2]; // soruce
            vec2 Q2 = pos[j+3];
            if (length(xdash - P2) < 0.01) {
                color.rgb = vec3(1, 1, 0);
            }
            if (length(xdash - Q2) < 0.01) {
                color.rgb = vec3(0, 1, 1);
            }
            if (length(UV - P1) < 0.005) {
                color.rgb = vec3(1, 0, 0);
            }
            if (length(UV - Q1) < 0.005) {
                color.rgb = vec3(0, 1, 0);
            }
        }
    }
}


void main() {
    if (morph != 0) {
        main_morph();
    } else {
        color.a = 0.5;
        color.rgb = texture(myTextureSampler, UV).rgb;
    }
}




