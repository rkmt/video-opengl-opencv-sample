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

#define NSEG 1000

// 動画/静止画の表示を抽象化
// feature-based image metamorphosisによる画像変形をサポート
// http://dl.acm.org/citation.cfm?id=134003
class VideoTexture {
  public:
    bool is_video;              // video / static image
    GLuint program_id;          // shader program handle
    GLuint unit;                // 0, 1, 2, ...
    GLuint texture_id;          // glGenTexturesで採番されるID
    GLuint vertex_array_id;     // glGenVertexArraysで採番されるID
    GLuint vertex_id;           // vertex buffer object id
    GLuint uv_id;               // uv buffer object id
    int width, height;          // videoサイズ(=textureサイズ)
    GLuint texture_loc;         // fragment shaderでの位置
    VideoCapture capture;       // (OpenCV) video capture (is_video == 1)
    
    // feature-based image metamorphosis 関連
    //  dest   {pos[i][0], pos[i][1]} <--> {pos[i+1][0], pos[i+1][1]}
    //  source {pos[i+2][0], pos[i+2][1]} <--> {pos[i+3][0], pos[i+3][1]}
    GLfloat pos[NSEG][2];               // feature line points  (npos * 4)
    GLuint npos;                // the number of feature-segment pairs
    GLfloat param[3];           // weight constants (a, b, p in [1])
    
    GLuint debug;               // debug flag (1 or 0)
    GLuint morph;               // morph flat
    GLfloat alpha;
    GLuint pos_loc, npos_loc, param_loc, debug_loc, morph_loc, alpha_loc; // shader locations

    void init(GLuint program_id, bool is_video, VideoCapture capture, Mat &image);
    void nextFrame();           // fetech next video frame (auto rewind)
    void draw();                // draw current frame with morphing
    
    void saveParams(string fname);
    int loadParams(string fname);
    void showParams();

    void cleanup();
};

