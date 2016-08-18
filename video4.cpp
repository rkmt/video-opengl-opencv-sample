//  Feature-Based Image Metamorphosis (siggraph '92) in shader

#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

#include <GL/glew.h>        // GLEW
#include <glfw3.h>          // GLFW
GLFWwindow* window;

#include <glm/glm.hpp>      // GLM
using namespace glm;

#include "common/shader.hpp"
#include "common/texture.hpp"

#include "fps.h"
#include "videotexture.h"

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

    /*
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
    */

    // Two UV coordinatesfor each vertex.
    static const GLfloat g_uv_buffer_data[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f
    };

    VideoTexture vTexture0, vTexture1, vTexture2;
    // テクスチャを準備する

    Mat dummy_frame;
    // background face texture (static image)
    textureCreate(vTexture0, programID, false, 0, face_image, (GLfloat*)g_vertex_buffer_data0, sizeof(g_vertex_buffer_data0) / sizeof(GLfloat), (GLfloat*)g_uv_buffer_data, sizeof(g_uv_buffer_data) / sizeof(GLfloat));

    // video texture 1
    textureCreate(vTexture1, programID, true, capture1, dummy_frame, (GLfloat*)g_vertex_buffer_data0, sizeof(g_vertex_buffer_data0) / sizeof(GLfloat), (GLfloat*)g_uv_buffer_data, sizeof(g_uv_buffer_data) / sizeof(GLfloat));

    // video texture 2
    textureCreate(vTexture2, programID, true, capture2, dummy_frame, (GLfloat*)g_vertex_buffer_data0, sizeof(g_vertex_buffer_data0) / sizeof(GLfloat), (GLfloat*)g_uv_buffer_data, sizeof(g_uv_buffer_data) / sizeof(GLfloat));

    // morphing setup sample
    // p1-q1 の線分が p2-q2の線分に対応するようにtexture全体が変形する (morph.frag)
    vTexture2.npos = 3;         // the number of feature segment pairs
    static const GLfloat p2[][2] =  {{0.494417, 0.617314}, {0.391364, 0.182178}, {0.192998, 0.369263}};
    static const GLfloat q2[][2] = {{0.858242, 0.570093}, {0.877008, 0.203149}, {0.147721, 0.650742}};
    static const GLfloat p1[][2] =  {{0.545622, 0.405601}, {0.53417, 0.345142}, {0.471393, 0.342964}};
    static const GLfloat q1[][2] = {{0.618337, 0.409629}, {0.63667, 0.364585}, {0.455713, 0.406392}};
    vTexture2.P1 = (GLfloat*)p1;
    vTexture2.Q1 = (GLfloat*)q1;
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
