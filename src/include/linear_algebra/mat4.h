#ifndef LINEAR_ALGEBRA_MAT4_H
#define LINEAR_ALGEBRA_MAT4_H

#include "linear_algebra/vec3.h"
#include "utils.h"

typedef struct
{
    //      [col][raw]
    float data[4][4];
} Mat4;

static inline Mat4 mat4$()
{
    Mat4 res;
    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 4; j++)
            res.data[i][j] = 0;
    }
    
    return res;
}

static inline float* mat4_to_list(Mat4 mat){
    float* list = malloc(sizeof(float) * 4 * 4);
    int count = 0;
    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 4; j++)
            list[count++] = mat.data[i][j];
    }
    
    return list;
}

static inline Mat4 mat4_id(float val)
{
    Mat4 res;
    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 4; j++)
            res.data[i][j] = val * (i == j);
    }
    
    return res;
}

static inline Mat4 mat4_rotate(Mat4 mat, float angle, Vec3 vec) {
    float cos_teta = fast_cos(angle);
    float sin_teta = fast_sin(angle);

    Vec3 axis = vec3_unit(vec);
    Vec3 temp = vec3_mul_val(axis, 1.0f-cos_teta);

    Mat4 rot = mat4$();
    rot.data[0][0] = cos_teta + temp.x * axis.x;
    rot.data[0][1] = temp.x * axis.y + sin_teta * axis.z;
    rot.data[0][2] = temp.x * axis.z - sin_teta * axis.y;

    rot.data[1][0] = temp.y * axis.x - sin_teta * axis.z;
    rot.data[1][1] = cos_teta + temp.y * axis.y;
    rot.data[1][2] = temp.y * axis.z + sin_teta * axis.x;

    rot.data[2][0] = temp.z * axis.x + sin_teta * axis.y;
    rot.data[2][1] = temp.z * axis.y - sin_teta * axis.x;
    rot.data[2][2] = cos_teta + temp.z * axis.z;

    Mat4 res = mat4$();
    // SEE IF EQUIVALENT
    // res.data[0] = m[0] * rot.data[0][0] + m[1] * rot.data[0][1] + m[2] * rot.data[0][2];
    // res.data[1] = m[0] * rot.data[1][0] + m[1] * rot.data[1][1] + m[2] * rot.data[1][2];
    // res.data[2] = m[0] * rot.data[2][0] + m[1] * rot.data[2][1] + m[2] * rot.data[2][2];
    // res.data[3] = m[3];
    res.data[0][0] = mat.data[0][0] * rot.data[0][0] + mat.data[1][0] * rot.data[0][1] + mat.data[2][0] * rot.data[0][2];
    res.data[0][1] = mat.data[0][1] * rot.data[0][0] + mat.data[1][1] * rot.data[0][1] + mat.data[2][1] * rot.data[0][2];
    res.data[0][2] = mat.data[0][2] * rot.data[0][0] + mat.data[1][2] * rot.data[0][1] + mat.data[2][2] * rot.data[0][2];
    res.data[0][3] = mat.data[0][3] * rot.data[0][0] + mat.data[1][3] * rot.data[0][1] + mat.data[2][3] * rot.data[0][2];
    res.data[1][0] = mat.data[0][0] * rot.data[1][0] + mat.data[1][0] * rot.data[1][1] + mat.data[2][0] * rot.data[1][2];
    res.data[1][1] = mat.data[0][1] * rot.data[1][0] + mat.data[1][1] * rot.data[1][1] + mat.data[2][1] * rot.data[1][2];
    res.data[1][2] = mat.data[0][2] * rot.data[1][0] + mat.data[1][2] * rot.data[1][1] + mat.data[2][2] * rot.data[1][2];
    res.data[1][3] = mat.data[0][3] * rot.data[1][0] + mat.data[1][3] * rot.data[1][1] + mat.data[2][3] * rot.data[1][2];
    res.data[2][0] = mat.data[0][0] * rot.data[2][0] + mat.data[1][0] * rot.data[2][1] + mat.data[2][0] * rot.data[2][2];
    res.data[2][1] = mat.data[0][1] * rot.data[2][0] + mat.data[1][1] * rot.data[2][1] + mat.data[2][1] * rot.data[2][2];
    res.data[2][2] = mat.data[0][2] * rot.data[2][0] + mat.data[1][2] * rot.data[2][1] + mat.data[2][2] * rot.data[2][2];
    res.data[2][3] = mat.data[0][3] * rot.data[2][0] + mat.data[1][3] * rot.data[2][1] + mat.data[2][3] * rot.data[2][2];
    res.data[3][0] = mat.data[3][0];
    res.data[3][1] = mat.data[3][1];
    res.data[3][2] = mat.data[3][2];
    res.data[3][3] = mat.data[3][3];
    return res;
}

static inline Mat4 mat4_mult(Mat4 mat1, Mat4 mat2) {
    Mat4 res = mat4$();

    res.data[0][0] = (mat1.data[0][0]*mat2.data[0][0])+(mat1.data[1][0]*mat2.data[0][1])+(mat1.data[2][0]*mat2.data[0][2])+(mat1.data[3][0]*mat2.data[0][3]);
    res.data[0][1] = (mat1.data[0][1]*mat2.data[0][0])+(mat1.data[1][1]*mat2.data[0][1])+(mat1.data[2][1]*mat2.data[0][2])+(mat1.data[3][1]*mat2.data[0][3]);
    res.data[0][2] = (mat1.data[0][2]*mat2.data[0][0])+(mat1.data[1][2]*mat2.data[0][1])+(mat1.data[2][2]*mat2.data[0][2])+(mat1.data[3][2]*mat2.data[0][3]);
    res.data[0][3] = (mat1.data[0][3]*mat2.data[0][0])+(mat1.data[1][3]*mat2.data[0][1])+(mat1.data[2][3]*mat2.data[0][2])+(mat1.data[3][3]*mat2.data[0][3]);

    res.data[1][0] = (mat1.data[0][0]*mat2.data[1][0])+(mat1.data[1][0]*mat2.data[1][1])+(mat1.data[2][0]*mat2.data[1][2])+(mat1.data[3][0]*mat2.data[1][3]);
    res.data[1][1] = (mat1.data[0][1]*mat2.data[1][0])+(mat1.data[1][1]*mat2.data[1][1])+(mat1.data[2][1]*mat2.data[1][2])+(mat1.data[3][1]*mat2.data[1][3]);
    res.data[1][2] = (mat1.data[0][2]*mat2.data[1][0])+(mat1.data[1][2]*mat2.data[1][1])+(mat1.data[2][2]*mat2.data[1][2])+(mat1.data[3][2]*mat2.data[1][3]);
    res.data[1][3] = (mat1.data[0][3]*mat2.data[1][0])+(mat1.data[1][3]*mat2.data[1][1])+(mat1.data[2][3]*mat2.data[1][2])+(mat1.data[3][3]*mat2.data[1][3]);

    res.data[2][0] = (mat1.data[0][0]*mat2.data[2][0])+(mat1.data[1][0]*mat2.data[2][1])+(mat1.data[2][0]*mat2.data[2][2])+(mat1.data[3][0]*mat2.data[2][3]);
    res.data[2][1] = (mat1.data[0][1]*mat2.data[2][0])+(mat1.data[1][1]*mat2.data[2][1])+(mat1.data[2][1]*mat2.data[2][2])+(mat1.data[3][1]*mat2.data[2][3]);
    res.data[2][2] = (mat1.data[0][2]*mat2.data[2][0])+(mat1.data[1][2]*mat2.data[2][1])+(mat1.data[2][2]*mat2.data[2][2])+(mat1.data[3][2]*mat2.data[2][3]);
    res.data[2][3] = (mat1.data[0][3]*mat2.data[2][0])+(mat1.data[1][3]*mat2.data[2][1])+(mat1.data[2][3]*mat2.data[2][2])+(mat1.data[3][3]*mat2.data[2][3]);

    res.data[3][0] = (mat1.data[0][0]*mat2.data[3][0])+(mat1.data[1][0]*mat2.data[3][1])+(mat1.data[2][0]*mat2.data[3][2])+(mat1.data[3][0]*mat2.data[3][3]);
    res.data[3][1] = (mat1.data[0][1]*mat2.data[3][0])+(mat1.data[1][1]*mat2.data[3][1])+(mat1.data[2][1]*mat2.data[3][2])+(mat1.data[3][1]*mat2.data[3][3]);
    res.data[3][2] = (mat1.data[0][2]*mat2.data[3][0])+(mat1.data[1][2]*mat2.data[3][1])+(mat1.data[2][2]*mat2.data[3][2])+(mat1.data[3][2]*mat2.data[3][3]);
    res.data[3][3] = (mat1.data[0][3]*mat2.data[3][0])+(mat1.data[1][3]*mat2.data[3][1])+(mat1.data[2][3]*mat2.data[3][2])+(mat1.data[3][3]*mat2.data[3][3]);

    return res;
}

static inline Mat4 mat4_perspective(float fovy, float aspect, float zNear, float zFar) {
    float tanHalfFovy = fast_sin(fovy / 2.0f)/fast_cos(fovy / 2.0f);
    Mat4 res = mat4$();
    res.data[0][0] = 1.0f / (aspect * tanHalfFovy);
    res.data[1][1] = 1.0f / (tanHalfFovy);
    res.data[2][2] = -(zFar + zNear) / (zFar - zNear);
    res.data[2][3] = -1.0f;
    res.data[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
    return res;
}

static inline Mat4 mat4_translate(Mat4 mat, Vec3 vec) {

    Mat4 res = mat4$();
    res.data[0][0] = mat.data[0][0];
    res.data[0][1] = mat.data[0][1];
    res.data[0][2] = mat.data[0][2];
    res.data[0][3] = mat.data[0][3];
    res.data[1][0] = mat.data[1][0];
    res.data[1][1] = mat.data[1][1];
    res.data[1][2] = mat.data[1][2];
    res.data[1][3] = mat.data[1][3];
    res.data[2][0] = mat.data[2][0];
    res.data[2][1] = mat.data[2][1];
    res.data[2][2] = mat.data[2][2];
    res.data[2][3] = mat.data[2][3];
    res.data[3][0] = mat.data[0][0] * vec.x + mat.data[1][0] * vec.y + mat.data[2][0] * vec.z + mat.data[3][0];
    res.data[3][1] = mat.data[0][1] * vec.x + mat.data[1][1] * vec.y + mat.data[2][1] * vec.z + mat.data[3][1];
    res.data[3][2] = mat.data[0][2] * vec.x + mat.data[1][2] * vec.y + mat.data[2][2] * vec.z + mat.data[3][2];
    res.data[3][3] = mat.data[0][3] * vec.x + mat.data[1][3] * vec.y + mat.data[2][3] * vec.z + mat.data[3][3];
    return res;
}

static inline Mat4 mat4_lookAt(Vec3 eye, Vec3 center, Vec3 up){
    Vec3 f = vec3_unit(vec3_sub(center, eye));
    Vec3 s = vec3_unit(vec3_cross(up, f));
    Vec3 u = vec3_cross(f, s);

    Mat4 res = mat4_id(1.0f);
    res.data[0][0] = s.x;
    res.data[1][0] = s.y;
    res.data[2][0] = s.z;
    res.data[0][1] = u.x;
    res.data[1][1] = u.y;
    res.data[2][1] = u.z;
    res.data[0][2] = f.x;
    res.data[1][2] = f.y;
    res.data[2][2] = f.z;
    res.data[3][0] = -vec3_dot(s, eye);
    res.data[3][1] = -vec3_dot(u, eye);
    res.data[3][2] = -vec3_dot(f, eye);
    return res;
}

#endif