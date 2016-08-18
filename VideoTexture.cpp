#include "VideoTexture.h"

static const GLfloat vertex_buffer[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
};

// Two UV coordinatesfor each vertex.
static const GLfloat uv_buffer[] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f
};


void VideoTexture::init(GLuint _program_id, bool _is_video, VideoCapture _capture, Mat &_image) {
    static int unit_number  = 0;
    // video capture (opencv)
    is_video = _is_video;
    if (is_video) {
        capture = _capture;
        width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
        height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
        printf("video size %d %d\n", width, height);
    } else {            // static image
        width = _image.cols;
        height = _image.rows;
        printf("static image size %d %d\n", width, height);
    }

    glGenTextures(1, &(texture_id));
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    // setup texture
    float x;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &x);//傾斜度合いの最大値
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, x);//適用する

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, is_video ? 0 : _image.data);
    glGenerateMipmap(GL_TEXTURE_2D);

    program_id = _program_id;
    unit = unit_number;
    unit_number++;
    
    glUseProgram(program_id); // Use our shader
    // Get a handle for our "myTextureSampler" uniform
    texture_loc  = glGetUniformLocation(program_id, "myTextureSampler");

    pos_loc  = glGetUniformLocation(program_id, "pos");
    npos_loc = glGetUniformLocation(program_id, "npos");
    param_loc = glGetUniformLocation(program_id, "param");
    debug_loc = glGetUniformLocation(program_id, "debug");
    morph_loc = glGetUniformLocation(program_id, "morph");
    alpha_loc = glGetUniformLocation(program_id, "alpha");

    // weight constants 
    param[0] = 0.05; // a
    param[1] = 2;   // b
    param[2] = 1; // p
    debug = true;
    morph = false;
    alpha = 0.5;

    cout << "Locations: pos_loc:" <<  pos_loc << " npos:" << npos_loc << endl;

    glGenVertexArrays(1, &(vertex_array_id));
    glBindVertexArray(vertex_array_id);
    
    // vertex buffer
    glGenBuffers(1, &(vertex_id));
    glBindBuffer(GL_ARRAY_BUFFER, vertex_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer), vertex_buffer, GL_STATIC_DRAW);

    // uv buffer
    glGenBuffers(1, &(uv_id));
    glBindBuffer(GL_ARRAY_BUFFER, uv_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer), uv_buffer, GL_STATIC_DRAW);

}

void VideoTexture::nextFrame() {
    if (!is_video) return;
    Mat frame;
    capture >> frame;
    if (frame.empty()) {  // auto rewind -- movie file playback only
        cout << "rewind" << unit << endl;
        capture.set(CV_CAP_PROP_POS_FRAMES, 0);
        capture >> frame;
    }
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, frame.data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void VideoTexture::draw() {
    glUseProgram(program_id); // Use our shader
    // Bind our texture in Texture Unit unit
    glActiveTexture(GL_TEXTURE0 + unit); 
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glUniform1i(texture_loc, unit);

    // morphing parameters
    glUniform2fv(pos_loc, npos, (float*)pos);
    glUniform1i(npos_loc, npos / 4);
    glUniform3f(param_loc, param[0], param[1], param[2]);
    glUniform1i(debug_loc, debug);
    glUniform1i(morph_loc, morph);
    glUniform1f(alpha_loc, alpha);

    // 1rst attribute buffer : vertices
    glBindVertexArray(vertex_array_id);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_id);
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
    glBindBuffer(GL_ARRAY_BUFFER, uv_id);
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
void VideoTexture::cleanup() {
//    glDeleteBuffers(1, (const GLuint*)&(vertex_buffer));
//    glDeleteBuffers(1, (const GLuint*)&(uv_buffer));
    glDeleteTextures(1, &(texture_loc));
    glDeleteVertexArrays(1, &(vertex_array_id));
}

void VideoTexture::saveParams(string fname) {
    cout << "saving parameters to " << fname << endl;
    showParams();
    ofstream out(fname, std::ios::out);
    out << param[0] << " " << param[1] << " " << param[2] << endl;
    out << npos << endl;
    for (int i = 0; i < npos; i++) {
        out << pos[i][0] << " " << pos[i][1] << endl;
    }
    out.close();
}

void VideoTexture::showParams() {
    cout << "param: a=" << param[0] << " b=" << param[1] << " p=" << param[2] << " npos=" << npos << endl;
}

int VideoTexture::loadParams(string fname) {
    cout << "loading params from " << fname << endl;
    ifstream in(fname);

    in >> param[0] >> param[1] >> param[2];
    in >> npos;
    for (int i = 0; i < npos; i++) {
        in >> pos[i][0];
        in >> pos[i][1];
    }

    showParams();

    return npos;
    
}
