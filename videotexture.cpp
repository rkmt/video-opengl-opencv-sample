#include "videotexture.h"

void vtexCreate(VideoTexture& vt, GLuint program_id, bool is_video, VideoCapture capture, Mat &image, GLfloat *vertex_buffer, int nvertex, GLfloat *uv_buffer, int nuv) {
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
    vt.texture_loc  = glGetUniformLocation(program_id, "myTextureSampler");

    vt.pos_loc  = glGetUniformLocation(program_id, "pos");
    vt.npos_loc = glGetUniformLocation(program_id, "npos");
    vt.param_loc = glGetUniformLocation(program_id, "param");
    vt.debug_loc = glGetUniformLocation(program_id, "debug");

    // weight constants 
    vt.param[0] = 0.01; // a
    vt.param[1] = 2;   // b
    vt.param[2] = 1; // p
    vt.debug = 1;

    cout << "Locations: pos_loc:" <<  vt.pos_loc << " npos:" << vt.npos_loc << endl;

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

void vtexNextFrame(VideoTexture &vt) {
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

void vtexDraw(VideoTexture &vt) {
    glUseProgram(vt.program_id); // Use our shader
    // Bind our texture in Texture Unit unit
    glActiveTexture(GL_TEXTURE0 + vt.unit); 
    glBindTexture(GL_TEXTURE_2D, vt.texture_id);
    glUniform1i(vt.texture_loc, vt.unit);

    // morphing parameters
    glUniform2fv(vt.pos_loc, vt.npos * 4, (float*)vt.pos); // npos * 4 points
    glUniform1i(vt.npos_loc, vt.npos);
    glUniform3f(vt.param_loc, vt.param[0], vt.param[1], vt.param[2]);
    glUniform1i(vt.debug_loc, vt.debug);

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
void vtexDelete(VideoTexture &vt) {
    glDeleteBuffers(1, (const GLuint*)&(vt.vertex_buffer));
    glDeleteBuffers(1, (const GLuint*)&(vt.uv_buffer));
    glDeleteTextures(1, &(vt.texture_loc));
    glDeleteVertexArrays(1, &(vt.vertex_array_id));
}

void vtexSaveParams(VideoTexture &vt, string fname) {
    ofstream out;
    out.open(fname, std::ios::out);
    out << vt.param[0] << " " << vt.param[1] << " " << vt.param[2] << endl;
}
