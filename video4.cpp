//  Feature-Based Image Metamorphosis (siggraph '92) in shader

#include <opencv2/features2d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

#include "fps.h"

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include "common/shader.hpp"
#include "common/texture.hpp"

#define NSEG 10

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

void textureCreate(VideoTexture& vt, GLuint program_id, bool is_video, VideoCapture capture, Mat &image, GLfloat *vertex_buffer, int nvertex, GLfloat *uv_buffer, int nuv) {
    static int unit_number  = 0;
    // video capture (opencv)
    vt.is_video = is_video;
    if (is_video) {
        vt.capture = capture;
        vt.width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
        vt.height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
        printf("video size %d %d\n", vt.width, vt.height);
    } else {            // static image
        vt.width = image.cols;
        vt.height = image.rows;
        printf("static image size %d %d\n", vt.width, vt.height);
    }

    glGenTextures(1, &(vt.texture_id));
    glBindTexture(GL_TEXTURE_2D, vt.texture_id);
    
    // setup texture
    float x;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &x);//傾斜度合いの最大値
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, x);//適用する

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, vt.width, vt.height, 0, GL_BGR, GL_UNSIGNED_BYTE, vt.is_video ? 0 : image.data);
    glGenerateMipmap(GL_TEXTURE_2D);

    vt.program_id = program_id;
    vt.unit = unit_number;
    unit_number++;
    vt.vertex_buffer = vertex_buffer;
    vt.uv_buffer = uv_buffer;
    
    glUseProgram(vt.program_id); // Use our shader
    // Get a handle for our "myTextureSampler" uniform
    vt.texture_name = "myTextureSampler";
    vt.texture_loc  = glGetUniformLocation(program_id, vt.texture_name);

    vt.P1_loc  = glGetUniformLocation(program_id, "P1");
    vt.Q1_loc  = glGetUniformLocation(program_id, "Q1");
    vt.P2_loc  = glGetUniformLocation(program_id, "P2");
    vt.Q2_loc  = glGetUniformLocation(program_id, "Q2");
    vt.npos_loc = glGetUniformLocation(program_id, "npos");

    cout << "P1:" << vt.P1_loc << " Q1:" << vt.Q1_loc << " P2:" << vt.P2_loc << " Q2:" << vt.Q2_loc << " npos:" << vt.npos_loc << endl;

    glGenVertexArrays(1, &(vt.vertex_array_id));
    glBindVertexArray(vt.vertex_array_id);
    
    // vertex buffer
    glGenBuffers(1, &(vt.vertex_id));
    glBindBuffer(GL_ARRAY_BUFFER, vt.vertex_id);
    glBufferData(GL_ARRAY_BUFFER, nvertex*sizeof(GLfloat), vt.vertex_buffer, GL_STATIC_DRAW);

    // uv buffer
    glGenBuffers(1, &(vt.uv_id));
    glBindBuffer(GL_ARRAY_BUFFER, vt.uv_id);
    glBufferData(GL_ARRAY_BUFFER, nuv*sizeof(GLfloat), vt.uv_buffer, GL_STATIC_DRAW);

}

void textureNextFrame(VideoTexture &vt) {
    if (!vt.is_video) return;
    Mat frame;
    vt.capture >> frame;
    if (frame.empty()) {  // auto rewind -- movie file playback only
        cout << "rewind" << vt.unit << endl;
        vt.capture.set(CV_CAP_PROP_POS_FRAMES, 0);
        vt.capture >> frame;
    }
    glBindTexture(GL_TEXTURE_2D, vt.texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, vt.width, vt.height, 0, GL_BGR, GL_UNSIGNED_BYTE, frame.data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

//void setTexture(GLuint unit, GLuint texture, GLuint textureLoc, int width, int height, unsigned char *data) {
void textureDraw(VideoTexture &vt) {
    glUseProgram(vt.program_id); // Use our shader
    // Bind our texture in Texture Unit unit
    glActiveTexture(GL_TEXTURE0 + vt.unit); 
    glBindTexture(GL_TEXTURE_2D, vt.texture_id);
    glUniform1i(vt.texture_loc, vt.unit);

    // morphing parameters
    glUniform2fv(vt.P1_loc, vt.npos, (float*)vt.P1);
    glUniform2fv(vt.Q1_loc, vt.npos, (float*)vt.Q1);
    glUniform2fv(vt.P2_loc, vt.npos, (float*)vt.P2);
    glUniform2fv(vt.Q2_loc, vt.npos, (float*)vt.Q2);
    glUniform1i(vt.npos_loc, vt.npos);

    // 1rst attribute buffer : vertices
    glBindVertexArray(vt.vertex_array_id);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vt.vertex_id);
    glVertexAttribPointer(
            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader. 
            3,                  // size  was 3
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vt.uv_id);
    glVertexAttribPointer(
            1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
            2,                                // size : U+V => 2
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
            );
    // Draw the triangles !
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // 4 indices starting at 0 -> 1 rectangle
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

// Cleanup VBO
void textureDelete(VideoTexture &vt) {
    glDeleteBuffers(1, (const GLuint*)&(vt.vertex_buffer));
    glDeleteBuffers(1, (const GLuint*)&(vt.uv_buffer));
    glDeleteTextures(1, &(vt.texture_loc));
    glDeleteVertexArrays(1, &(vt.vertex_array_id));
}

int main( void ) {
    VideoCapture capture1("../IMG_6154.MOV");
    if (!capture1.isOpened()) {
        cerr << "Failed to open the video device, video file or image sequence!\n" << endl;
        return 0;
    }

    VideoCapture capture2("../IMG_6155.MOV");
    if (!capture2.isOpened()) {
        cerr << "Failed to open the video device, video file or image sequence!\n" << endl;
        return 0;
    }

    Mat face_image = imread("../face-texture.jpg", CV_LOAD_IMAGE_COLOR);

    // Initialise GLFW
    if( !glfwInit() ) {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1200, 800, "video", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    GLuint programID = LoadShaders( "../simple.vert", "../morph.frag" );

    static const GLfloat g_vertex_buffer_data0[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
    };

    static const GLfloat g_vertex_buffer_data1[] = {
        0, 0, 0,
        0.75, 0.25, 0,
        1, 1, 0,
        0.25, 0.75, 0,
    };
    

    static const GLfloat g_vertex_buffer_data2[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
    };

    // Two UV coordinatesfor each vertex.
    static const GLfloat g_uv_buffer_data[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f
    };

    VideoTexture vTexture0, vTexture1, vTexture2;
    // テクスチャを準備する

    Mat null_frame;
    // background face texture (static)
    textureCreate(vTexture0, programID, false, 0, face_image, (GLfloat*)g_vertex_buffer_data0, sizeof(g_vertex_buffer_data0) / sizeof(GLfloat), (GLfloat*)g_uv_buffer_data, sizeof(g_uv_buffer_data) / sizeof(GLfloat));
    
    textureCreate(vTexture1, programID, true, capture1, null_frame, (GLfloat*)g_vertex_buffer_data0, sizeof(g_vertex_buffer_data0) / sizeof(GLfloat), (GLfloat*)g_uv_buffer_data, sizeof(g_uv_buffer_data) / sizeof(GLfloat));
    
    textureCreate(vTexture2, programID, true, capture2, null_frame, (GLfloat*)g_vertex_buffer_data0, sizeof(g_vertex_buffer_data0) / sizeof(GLfloat), (GLfloat*)g_uv_buffer_data, sizeof(g_uv_buffer_data) / sizeof(GLfloat));

    vTexture2.npos = 2;
    static const GLfloat p1[][2] =  {{0.5, 0.1}, {0.25, 0.4}};
    static const GLfloat q1[][2] = {{0.5, 0.6}, {0.4, 0.2}};
    vTexture2.P1 = (GLfloat*)p1;
    vTexture2.Q1 = (GLfloat*)q1;
    static const GLfloat p2[][2] =  {{0.45, 0.15}, {0.3, 0.3}};
    static const GLfloat q2[][2] = {{0.6, 0.85}, {0.35, 0.5}};
    vTexture2.P2 = (GLfloat*)p2;
    vTexture2.Q2 = (GLfloat*)q2;

    vTexture1.npos = 0;
    /*
    static const GLfloat p1[][2] =  {{0.5, 0.1}, {0.25, 0.4}};
    static const GLfloat q1[][2] = {{0.5, 0.9}, {0.4, 0.2}};
    vTexture1.P1 = (GLfloat*)p1;
    vTexture1.Q1 = (GLfloat*)q1;
    static const GLfloat p2[][2] =  {{0.45, 0.15}, {0.3, 0.3}};
    static const GLfloat q2[][2] = {{0.6, 0.85}, {0.35, 0.5}};
    vTexture1.P2 = (GLfloat*)p2;
    vTexture1.Q2 = (GLfloat*)q2;
    */
    

    
    
    startFPS();
    
    // ブレンドを有効にする
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    do{
        tickFPS();

        // Get next video frames
        //textureNextFrame(vTexture1);
        textureNextFrame(vTexture2);
        
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        textureDraw(vTexture0);
        //textureDraw(vTexture1);
        textureDraw(vTexture2);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    textureDelete(vTexture0);
    textureDelete(vTexture1);
    textureDelete(vTexture2);
    glDeleteProgram(programID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
