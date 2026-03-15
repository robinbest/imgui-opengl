#include "cube_mesh.h"

void CubeMesh::initialize()
{
    const float cube_vertices[] = {
        -0.5f, -0.5f,  0.5f,    1.0f, 0.2f, 0.2f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.2f, 0.2f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.2f, 0.2f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.2f, 0.2f,
        -0.5f,  0.5f,  0.5f,    1.0f, 0.2f, 0.2f,
        -0.5f, -0.5f,  0.5f,    1.0f, 0.2f, 0.2f,

        -0.5f, -0.5f, -0.5f,    0.2f, 1.0f, 0.2f,
        -0.5f,  0.5f, -0.5f,    0.2f, 1.0f, 0.2f,
         0.5f,  0.5f, -0.5f,    0.2f, 1.0f, 0.2f,
         0.5f,  0.5f, -0.5f,    0.2f, 1.0f, 0.2f,
         0.5f, -0.5f, -0.5f,    0.2f, 1.0f, 0.2f,
        -0.5f, -0.5f, -0.5f,    0.2f, 1.0f, 0.2f,

        -0.5f, -0.5f, -0.5f,    0.2f, 0.4f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.2f, 0.4f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.2f, 0.4f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.2f, 0.4f, 1.0f,
        -0.5f,  0.5f, -0.5f,    0.2f, 0.4f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.2f, 0.4f, 1.0f,

         0.5f, -0.5f, -0.5f,    1.0f, 0.8f, 0.2f,
         0.5f,  0.5f, -0.5f,    1.0f, 0.8f, 0.2f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.8f, 0.2f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.8f, 0.2f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.8f, 0.2f,
         0.5f, -0.5f, -0.5f,    1.0f, 0.8f, 0.2f,

        -0.5f,  0.5f, -0.5f,    0.7f, 0.2f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.7f, 0.2f, 1.0f,
         0.5f,  0.5f,  0.5f,    0.7f, 0.2f, 1.0f,
         0.5f,  0.5f,  0.5f,    0.7f, 0.2f, 1.0f,
         0.5f,  0.5f, -0.5f,    0.7f, 0.2f, 1.0f,
        -0.5f,  0.5f, -0.5f,    0.7f, 0.2f, 1.0f,

        -0.5f, -0.5f, -0.5f,    0.2f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,    0.2f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,    0.2f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,    0.2f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.2f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.2f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void CubeMesh::shutdown()
{
    if (vbo_ != 0)
        glDeleteBuffers(1, &vbo_);
    if (vao_ != 0)
        glDeleteVertexArrays(1, &vao_);
    vbo_ = 0;
    vao_ = 0;
}

void CubeMesh::bind() const
{
    glBindVertexArray(vao_);
}

void CubeMesh::draw_face(int face_index) const
{
    glDrawArrays(GL_TRIANGLES, face_index * 6, 6);
}
