#pragma once

#include <GL/glew.h>

class CubeMesh
{
public:
    void initialize();
    void shutdown();
    void draw_face(int face_index) const;
    void bind() const;

private:
    GLuint vbo_ = 0;
    GLuint vao_ = 0;
};
