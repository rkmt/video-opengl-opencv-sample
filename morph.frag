#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

#define NSEG 2
uniform vec2 P1[NSEG], Q1[NSEG];
uniform vec2 P2[NSEG], Q2[NSEG];
uniform int npos;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

#define hack(i) (abs(UV.x - P1[i].x) + abs(UV.y - P1[i].y) < 0.01)

void main_old(){
    // Output color = color of the texture at the specified UV
	color.rgb =	texture(myTextureSampler, UV).rgb;
    int i;
    for (i = 0; i < npos; i++) {
        if (abs(UV.x - P1[i].x) + abs(UV.y - P1[i].y) < 0.01) {
            color.rgb = vec3(1, 0, 1);
        }
        if (abs(UV.x - Q1[i].x) + abs(UV.y - Q1[i].y) < 0.01) {
            color.rgb = vec3(0, 1, 0);
        }
    }
    color.a = 0.5;
        
}

vec2 perpendicular(vec2 p) {
    return  vec2(p.y, -p.x);
}

/*
// distance to point a from line p--q
float distance(vec2 a, vec2 p, vec2 q) {
    float t;
    vec2 pq = p - q;
    // b = q + t * pq,  dot(ab, pq) == 0 (perpendicular)
    // (q.x + t * pq.x - a.x) * pq.x + (q.y + t * pq.y - a.y)* pq.y == 0
    // t * (pq.x * pq.x + pq.y * pq.y) - a.x * pq.x - a.y * pq.y == 0
    // t * lenth(pq) - dot(a, pq) == 0
    float t = dot(a, pq) / length(pq);
    return length(q + t * pq - a);
}
*/


// feature-based image metamorphosis 1segment version
void main_one() {
    vec2 XP = UV - P1[0];
    vec2 QP = Q1[0] - P1[0];
    float nQP = length(QP);
    vec2 QP2 = Q2[0] - P2[0];
    float nQP2 = length(QP2);

    int du = npos;

    float u = dot(XP, QP) / (nQP * nQP);
    float v = dot(XP, perpendicular(QP)) / nQP;
    vec2 xdash = P2[0] + u * QP2 + (v * perpendicular(QP2)) / nQP2;
    color.rgb = texture(myTextureSampler, xdash).rgb;
    color.a = 0.8;
    if (xdash.x < 0 || 1 < xdash.x || xdash.y < 0 || 1 < xdash.y) {
        color.rgb = vec3(1, 0, 1);
        color.a = 0.2;
    }
}

// feature-based image metamorphosis multi-segment version
void main_multi1() {
    float p = 1.2;
    float b = 1.2;
    float a = 0.5;
    
    vec2 dsum = vec2(0, 0);
    float weightsum = 0;
    int i;
    for (i = 0; i < npos; i++) {
        vec2 XP = UV - P1[i];
        vec2 QP = Q1[i] - P1[i];
        float nQP = length(QP);
        vec2 QP2 = Q2[i] - P2[i];
        float nQP2 = length(QP2);
    
        float u = dot(XP, QP) / (nQP * nQP);
        float v = dot(XP, perpendicular(QP)) / nQP;
        vec2 xdash = P2[i] + u * QP2 + (v * perpendicular(QP2)) / nQP2;

        vec2 Di = UV - xdash; // displacement
        float dist = v * nQP; // distance(UV, P1[i], Q1[i]);
        float weight = pow(pow(nQP, p) / (a + dist), b);
        dsum += weight * xdash;
        weightsum += weight;
    }
    vec2 xdash = dsum / weightsum;
    color.rgb = texture(myTextureSampler, xdash).rgb;
    color.a = 1;

    /*
    float dx = (xdash.x - 0.5) * 2;
    float dy = (xdash.y - 0.5) * 2;
    if (pow(dx, 4) + pow(dy, 4) > 0.5) {
        color.a = 0.1;
    }
    */

    if (xdash.x < 0 || 1 < xdash.x || xdash.y < 0 || 1 < xdash.y) {
        color.rgb = vec3(1, 0, 1);
        color.a = 0.1;
    }
}


// feature-based image metamorphosis multi-segment version
void main() {
    float p = 1.2;
    float b = 1.2;
    float a = 0.5;
    
    vec2 dsum = vec2(0, 0);
    float weightsum = 0;
    int i;
    for (i = 0; i < npos; i++) {
        vec2 XP = UV - P1[i];
        vec2 QP = Q1[i] - P1[i];
        float nQP = length(QP);
        vec2 QP2 = Q2[i] - P2[i];
        float nQP2 = length(QP2);
    
        float u = dot(XP, QP) / (nQP * nQP);
        float v = dot(XP, perpendicular(QP)) / nQP;
        vec2 xdash = P2[i] + u * QP2 + (v * perpendicular(QP2)) / nQP2;

        vec2 Di = UV - xdash; // displacement
        float dist = v * nQP; // distance(UV, P1[i], Q1[i]);
        float weight = pow(pow(nQP, p) / (a + dist), b);
        dsum += weight * dist;
        weightsum += weight;
    }
    vec2 xdash = UV + dsum / weightsum;
    color.rgb = texture(myTextureSampler, xdash).rgb;
    color.a = 0.5;
    if (xdash.x < 0 || 1 < xdash.x || xdash.y < 0 || 1 < xdash.y) {
        color.rgb = vec3(1, 0, 1);
        color.a = 0;
    }

    if (npos == 0) {
        color.a = 1;
        color.rgb = texture(myTextureSampler, UV).rgb;
    }
}



