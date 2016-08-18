//  [1] Feature-Based Image Metamorphosis (siggraph '92) in GLSL

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

    GLfloat *pos;               // feature line poinst (4 * npos)
    GLuint npos;                // the number of feature line pairs
    GLuint debug;               // debug flag (1 or 0)
    GLfloat param[3];           // weight constants (a, b, p in [1])
    GLuint pos_loc, npos_loc, param_loc, debug_loc; // shader locations
} VideoTexture;

void vtexCreate(VideoTexture& vt, GLuint program_id, bool is_video, VideoCapture capture, Mat &image, GLfloat *vertex_buffer, int nvertex, GLfloat *uv_buffer, int nuv);
void vtexNextFrame(VideoTexture &vt);
void vtexDraw(VideoTexture &vt);
// Cleanup VBO
void vtexDelete(VideoTexture &vt);
void vtexSaveParams(VideoTexture &vt, string fname);

