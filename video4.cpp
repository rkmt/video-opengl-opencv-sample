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
#include "VideoTexture.h"

VideoTexture vtex0, vtex1;

static void onKey(GLFWwindow *win, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        cout << "key=" << key << " " << action << endl;
        switch (key) {
        case GLFW_KEY_M:
            vtex1.morph = !vtex1.morph;
            break;
            
        case GLFW_KEY_D:
            vtex1.debug = !vtex1.debug;
            break;
        }
    }
}

int main( void ) {
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
    
    glfwSetKeyCallback(window, onKey);

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

    // background face texture (static image)
    Mat face_image = imread("../face-texture.jpg", CV_LOAD_IMAGE_COLOR);
    vtex0.init(programID, false, 0, face_image);        // static image texture
    vtex0.alpha = 1;
    
    // video テクスチャを準備する
    VideoCapture capture1("../IMG_6155.MOV");
    if (!capture1.isOpened()) {
        cerr << "Failed to open the video device, video file or image sequence!\n" << endl;
        return 0;
    }
    Mat dummy_frame;
    vtex1.init(programID, true, capture1, dummy_frame); // video texture
    vtex1.alpha = 0.5;
    vtex1.loadParams("../vtex1.txt");
    vtex1.morph = true;

    // ブレンドを有効にする  to enable alpha values in shaders
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    startFPS();
    do{
        tickFPS();

        // Get next video frames
        vtex1.nextFrame();
        
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        vtex0.draw();
        vtex1.draw();

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    vtex0.cleanup();
    vtex1.cleanup();
    glDeleteProgram(programID);
    glfwTerminate(); // Close OpenGL window and terminate GLFW

    return 0;
}
