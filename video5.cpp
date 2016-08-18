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

VideoTexture vTexture0, vTexture1, vTexture2;

#define NSEG 100
static GLfloat points[NSEG][2]; // Pi, Qi, P'i, Q'i, ....  [center (0, 0)]
static int npoints = 0;
static GLfloat gpoints[NSEG][2]; // Pi, Qi, P'i, Q'i, .... [UV coord]

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

    GLuint shaderProgram = LoadShaders( "../vert.shader", "../frag.shader" );

	// Get the location of the attributes that enters in the vertex shader
	GLint position_attribute = glGetAttribLocation(shaderProgram, "position");

	// Specify how the data for position can be accessed
	glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable the attribute
	glEnableVertexAttribArray(position_attribute);

    glBindVertexArray(0);
}

void showParam(GLfloat param[]) {
    cout << "param: a=" << param[0] << ", b=" << param[1] << ", p=" << param[2] << endl;
}


static double x_press, y_press;

bool morph_mode = false;

static void onKey(GLFWwindow *win, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        cout << "key=" << key << " " << action << endl;
        switch (key) {
        case GLFW_KEY_M:
            morph_mode = !morph_mode;
            if (morph_mode) {
                for (int i = 0; i < npoints; i++) {
                    gpoints[i][0] = (points[i][0] / 2.0) + 0.5;
                    gpoints[i][1] = (-points[i][1] / 2.0) + 0.5;
                }
                vTexture2.pos = (GLfloat*)gpoints;
                vTexture2.npos = npoints / 4;
                vtexSaveParams(vTexture2, "vTexture2.txt");
            } else {
                vTexture2.npos = 0; // nor morph
            }
            cout << "toggle morph mode" << morph_mode << "nops: " << vTexture2.npos << endl;
            break;
            
        case GLFW_KEY_D:
            vTexture2.debug = !vTexture2.debug;
            break;

        case GLFW_KEY_DELETE:
            if (npoints > 0) {
                cout << "D" << key << " " << npoints << endl;
                npoints = ((npoints - 1) / 4) * 4;
            }
            cout << "delte " << npoints << endl;
            break;


        case GLFW_KEY_A:
            vTexture2.param[0] += (GLFW_MOD_SHIFT & mods) ? 0.1 : -0.1;
            showParam(vTexture2.param);
            break;
        case GLFW_KEY_B:
            vTexture2.param[1] += (GLFW_MOD_SHIFT & mods) ? 0.1 : -0.1;
            showParam(vTexture2.param);
            break;
        case GLFW_KEY_P:
            vTexture2.param[2] += (GLFW_MOD_SHIFT & mods) ? 0.1 : -0.1;
            showParam(vTexture2.param);
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



static void onCursorPos(GLFWwindow *win, double x, double y) {
    cout << "Cursor: " << x << ", " << y << endl;
}

int main( void ) {
    VideoCapture capture1("../IMG_6154.MOV");
    if (!capture1.isOpened()) {
        cerr << "Failed to open the video device, video file or image sequence!\n" << endl;
        return 0;
    }

    VideoCapture capture2("../IMG_6169.MOV");
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

    static const GLfloat g_vertex_buffer_data0[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
    };

    // Two UV coordinatesfor each vertex.
    static const GLfloat g_uv_buffer_data[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f
    };

    // テクスチャを準備する

    Mat dummy_frame;
    // background face texture (static image)
    vtexCreate(vTexture0, programID, false, 0, face_image, (GLfloat*)g_vertex_buffer_data0, sizeof(g_vertex_buffer_data0) / sizeof(GLfloat), (GLfloat*)g_uv_buffer_data, sizeof(g_uv_buffer_data) / sizeof(GLfloat));

    // video texture 1
    vtexCreate(vTexture1, programID, true, capture1, dummy_frame, (GLfloat*)g_vertex_buffer_data0, sizeof(g_vertex_buffer_data0) / sizeof(GLfloat), (GLfloat*)g_uv_buffer_data, sizeof(g_uv_buffer_data) / sizeof(GLfloat));

    // video texture 2
    vtexCreate(vTexture2, programID, true, capture2, dummy_frame, (GLfloat*)g_vertex_buffer_data0, sizeof(g_vertex_buffer_data0) / sizeof(GLfloat), (GLfloat*)g_uv_buffer_data, sizeof(g_uv_buffer_data) / sizeof(GLfloat));

    /*
    // morphing setup sample
    // p1-q1 の線分が p2-q2の線分に対応するようにtexture全体が変形する (morph.frag)
    vTexture2.npos = 0;         // the number of feature segment pairs
    vTexture2.P2 = (GLfloat*)p2;
    vTexture2.Q2 = (GLfloat*)q2;

    vTexture2.npos = 0;
    vTexture2.P1 = (GLfloat*)p1;
    vTexture2.Q1 = (GLfloat*)q1;
    */


    GLuint vao, vbo;
    vbo_initialize(vao, vbo);
    
    startFPS();
    
    // ブレンドを有効にする
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    do{
        tickFPS();

        // Get next video frames
        // vtexNextFrame(vTexture1);
        vtexNextFrame(vTexture2);
        
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        vtexDraw(vTexture0);
//        vtexDraw(vTexture1);
        vtexDraw(vTexture2);

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

    vtexDelete(vTexture0);
    vtexDelete(vTexture1);
    vtexDelete(vTexture2);
    glDeleteProgram(programID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
