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

static GLfloat points[NSEG][2]; // Pi, Qi, P'i, Q'i, ....  [center (0, 0)]
static int npoints = 0;

// Render lines
void vbo_display(GLuint &vao) {
	glBindVertexArray(vao);
	glDrawArrays(GL_LINES, 0, npoints);
}

void vbo_setup(GLuint &vbo, GLfloat pos[], int size) {
	// Allocate space and upload the data from CPU to GPU
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, pos);
}

void vbo_initialize(GLuint &vao, GLuint &vbo) {
	// Use a Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create a Vector Buffer Object that will store the vertices on video memory
	glGenBuffers(1, &vbo);

	// Allocate space and upload the data from CPU to GPU
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_position), vertices_position, GL_STATIC_DRAW);	
	glBufferData(GL_ARRAY_BUFFER, NSEG * 2 * sizeof(GLfloat), 0, GL_DYNAMIC_DRAW);	

    GLuint shaderProgram = LoadShaders( "../line.vert", "../line.frag" );

	// Get the location of the attributes that enters in the vertex shader
	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");

	// Specify how the data for position can be accessed
	glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable the attribute
	glEnableVertexAttribArray(position_attribute);

    glBindVertexArray(0);
}



static double x_press, y_press;

static void onKey(GLFWwindow *win, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        cout << "key=" << key << " " << action << endl;
        switch (key) {
        case GLFW_KEY_M:
            vtex1.morph = !vtex1.morph;
            if (vtex1.morph) {
                for (int i = 0; i < npoints; i++) {
                    vtex1.pos[i][0] = (points[i][0] / 2.0) + 0.5;
                    vtex1.pos[i][1] = (-points[i][1] / 2.0) + 0.5;
                }
                vtex1.npos = npoints;
                //vtex1.saveParams("../vtex1.txt");
            }
            vtex1.showParams();
            break;
            
        case GLFW_KEY_D:
            vtex1.debug = !vtex1.debug;
            break;

        case GLFW_KEY_L:
            vtex1.loadParams("../vtex1.txt");
            vtex1.showParams();
            for (int i = 0; i < vtex1.npos; i++) {
                points[i][0] = (vtex1.pos[i][0] - 0.5) * 2.0;
                points[i][1] = (0.5 - vtex1.pos[i][1]) * 2.0;
            }
            npoints = vtex1.npos;
            vtex1.morph = true;
            break;

        case GLFW_KEY_S:
            vtex1.saveParams("../vtex1.txt");
            break;

        case GLFW_KEY_DELETE:
        case GLFW_KEY_BACKSPACE:
            if (npoints > 0) {
                cout << "D" << key << " " << npoints << endl;
                npoints = ((npoints - 1) / 4) * 4;
            }
            cout << "delte " << npoints << endl;
            break;

            // morph weight constants    
        case GLFW_KEY_A:
            vtex1.param[0] *= (GLFW_MOD_SHIFT & mods) ? 1.1 : (1.0/1.1);
            vtex1.showParams();
            break;
        case GLFW_KEY_B:
            vtex1.param[1] += (GLFW_MOD_SHIFT & mods) ? 0.1 : -0.1;
            vtex1.showParams();
            break;
        case GLFW_KEY_P:
            vtex1.param[2] += (GLFW_MOD_SHIFT & mods) ? 0.1 : -0.1;
            vtex1.showParams();
            break;
        }
    }
}

static void onMouseButton(GLFWwindow *win, int button, int action , int mods) {
    double x, y;
    int width, height;
    glfwGetWindowSize(win, &width, &height);
    glfwGetCursorPos(win, &x, &y);

    double xpos = x / (double)width;
    double ypos = y / (double)height;

    switch (action) {
    case 1: // press
        x_press = xpos;
        y_press = ypos;
        break;
    case 0: // release
        points[npoints][0] = (x_press - 0.5) * 2.0;
        points[npoints][1] = (0.5 - y_press) * 2.0;
        npoints++;
        points[npoints][0] = (xpos - 0.5) * 2.0;
        points[npoints][1] = (0.5 - ypos) * 2.0;
        npoints++;
        break;
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
    glfwSetMouseButtonCallback(window, onMouseButton);

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
    
    // video テクスチャを準備する
    VideoCapture capture1("../IMG_6155.MOV");
    if (!capture1.isOpened()) {
        cerr << "Failed to open the video device, video file or image sequence!\n" << endl;
        return 0;
    }
    Mat dummy_frame;
    vtex1.init(programID, true, capture1, dummy_frame); // video texture

    GLuint vao, vbo;  // display line segments for morphing debug
    vbo_initialize(vao, vbo);
    
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

        for (int i = npoints; i < NSEG; i++) {
            points[i][0] = points[i][1] = 0;
        }
        vbo_setup(vbo, (GLfloat*)points, NSEG * 2 * sizeof(GLfloat));

        vbo_display(vao);

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
