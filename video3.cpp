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
//#include <glm/glm.hpp>
//using namespace glm;

#include "common/shader.hpp"
#include "common/texture.hpp"

typedef struct {
    GLuint unit;
    GLuint texture_id;
    GLuint texture_loc;
    GLuint vertex_array_id;
    GLuint vertex_id;
    GLfloat *vertex_buffer;
    GLuint uv_id;
    GLfloat *uv_buffer;
    int width;
    int height;
    const char* texture_name;
    Mat frame;
    VideoCapture capture;
} VideoTexture;

void textureCreate(VideoTexture& vt, GLuint unit, GLuint programID, VideoCapture capture, GLfloat *vertex_buffer, int nvertex, GLfloat *uv_buffer, int nuv) {
    vt.unit = unit;
    vt.vertex_buffer = vertex_buffer;
    vt.uv_buffer = uv_buffer;
    glGenTextures(1, &(vt.texture_id));
    //glBindTexture(GL_TEXTURE_RECTANGLE, Texture);
    glBindTexture(GL_TEXTURE_2D, vt.texture_id);

    vt.texture_name = "myTextureSampler";
    // Get a handle for our "myTextureSampler" uniform
    vt.texture_loc  = glGetUniformLocation(programID, vt.texture_name);

    glGenVertexArrays(1, &(vt.vertex_array_id));
    glBindVertexArray(vt.vertex_array_id);
    glGenBuffers(1, &(vt.vertex_id));
    glBindBuffer(GL_ARRAY_BUFFER, vt.vertex_id);
    glBufferData(GL_ARRAY_BUFFER, nvertex*sizeof(GLfloat), vt.vertex_buffer, GL_STATIC_DRAW);

    glGenBuffers(1, &(vt.uv_id));
    glBindBuffer(GL_ARRAY_BUFFER, vt.uv_id);
    glBufferData(GL_ARRAY_BUFFER, nuv*sizeof(GLfloat), vt.uv_buffer, GL_STATIC_DRAW);

    vt.capture = capture;
    vt.width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    vt.height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    printf("video size %d %d\n", vt.width, vt.height);

    float x;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &x);//傾斜度合いの最大値
    cout << "ANISOTROPY_EXT " << x << endl;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, x);//適用する

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, vt.width, vt.height, 0, GL_BGR, GL_UNSIGNED_BYTE, 0);
    
    glGenerateMipmap(GL_TEXTURE_2D);
}

void textureNextFrame(VideoTexture &vt) {
    vt.capture >> vt.frame;
    if (vt.frame.empty()) {  // auto rewind -- movie file playback only
        cout << "rewind" << vt.unit << endl;
        vt.capture.set(CV_CAP_PROP_POS_FRAMES, 0);
        vt.capture >> vt.frame;
    }
}

//void setTexture(GLuint unit, GLuint texture, GLuint textureLoc, int width, int height, unsigned char *data) {
void textureDraw(VideoTexture &vt) {
    // Bind our texture in Texture Unit unit
    glActiveTexture(GL_TEXTURE0 + vt.unit);

//    cout << "setTexture" << vt.unit << " " << vt.texture_id << " " << vt.texture_loc << endl;
    glBindTexture(GL_TEXTURE_2D, vt.texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, vt.width, vt.height, 0, GL_BGR, GL_UNSIGNED_BYTE, vt.frame.data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glUniform1i(vt.texture_loc, vt.unit);

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
    VideoCapture capture1("../dejavu.mp4");
    if (!capture1.isOpened()) {
        cerr << "Failed to open the video device, video file or image sequence!\n" << endl;
        return 0;
    }

    VideoCapture capture2("../video.mp4");
    if (!capture2.isOpened()) {
        cerr << "Failed to open the video device, video file or image sequence!\n" << endl;
        return 0;
    }

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
    window = glfwCreateWindow(1024, 720, "video", NULL, NULL);
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

    // Textureをそのまま出す
    GLuint programID = LoadShaders( "../simple.vert", "../simple.frag" );

    // 三角形Mesh

    static const GLfloat g_vertex_buffer_data[] = {
        /*
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        */
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

    VideoTexture vTexture1, vTexture2;
    // テクスチャを準備する
    textureCreate(vTexture1, 0, programID, capture1, (GLfloat*)g_vertex_buffer_data, sizeof(g_vertex_buffer_data) / sizeof(GLfloat), (GLfloat*)g_uv_buffer_data, sizeof(g_uv_buffer_data) / sizeof(GLfloat));
    textureCreate(vTexture2, 1, programID, capture2, (GLfloat*)g_vertex_buffer_data2, sizeof(g_vertex_buffer_data2) / sizeof(GLfloat), (GLfloat*)g_uv_buffer_data, sizeof(g_uv_buffer_data) / sizeof(GLfloat));
    
    startFPS();
    
    do{
        tickFPS();

        // Get next video frames
        textureNextFrame(vTexture1);
        textureNextFrame(vTexture2);
        
        // Clear the screen
        glClear( GL_COLOR_BUFFER_BIT );

        // Use our shader
        glUseProgram(programID);

        textureDraw(vTexture1);
        textureDraw(vTexture2);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    textureDelete(vTexture1);
    textureDelete(vTexture2);

    glDeleteProgram(programID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
