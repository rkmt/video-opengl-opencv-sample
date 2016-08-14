//  Feature-Based Image Metamorphosis (siggraph '92) in shader

#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

#include <GL/glew.h>        // GLEW
#include <glfw3.h>          // GLFW
#include <glm/glm.hpp>      // GLM
using namespace glm;

// videoから読み込んでtextureとして処理するための構造体
typedef struct {
    bool is_video;              // video / static image
    GLuint program_id;          // shader program handle
    GLuint unit;                // 0, 1, 2, ...
    GLuint texture_id;          // glGenTexturesで採番されるID
    GLuint vertex_array_id;     // glGenVertexArraysで採番されるID
    GLuint vertex_id;           // vertex buffer object id
    GLfloat *vertex_buffer;     // vertex_idに対応する実体
    GLuint uv_id;               // uv buffer object id
    GLfloat *uv_buffer;         // uv_idに対応する実体
    int width, height;          // videoサイズ(=textureサイズ)
    const char* texture_name;   // fragment shaderでの名前
    GLuint texture_loc;         // fragment shaderでの位置
    VideoCapture capture;       // (OpenCV) video capture (is_video == 1)

    GLfloat *P1;
    GLfloat *Q1;
    GLfloat *P2;
    GLfloat *Q2;
    GLuint npos;
    GLuint Q1_loc, P1_loc, P2_loc, Q2_loc, npos_loc;
} VideoTexture;

void textureCreate(VideoTexture& vt, GLuint program_id, bool is_video, VideoCapture capture, Mat &image, GLfloat *vertex_buffer, int nvertex, GLfloat *uv_buffer, int nuv);
void textureNextFrame(VideoTexture &vt);
void textureDraw(VideoTexture &vt);
// Cleanup VBO
void textureDelete(VideoTexture &vt);

