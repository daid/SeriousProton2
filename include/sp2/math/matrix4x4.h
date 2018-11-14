#ifndef SP2_MATRIX_4X4_H
#define SP2_MATRIX_4X4_H

#include <sp2/math/matrix.impl.h>
#include <sp2/math/quaternion.h>
#include <sp2/math/vector.h>

#include <cmath>
#include <cstring>

#define ARITHMETIC(X) typename X, typename = typename std::enable_if<std::is_arithmetic<X>::value, X>::type

namespace sp {

template<ARITHMETIC(T)> class Matrix4x4 : public Matrix<T, 4>
{
protected:
    template<ARITHMETIC(U)> Matrix4x4(Matrix<U, 4> m)
    : Matrix<U, 4>(m)
    {
    }

    template<ARITHMETIC(U)> Matrix4x4(U d00, U d01, U d02, U d03, U d10, U d11, U d12, U d13, U d20, U d21, U d22, U d23, U d30, U d31, U d32, U d33)
    {
        this->data[0] = d00;
        this->data[1] = d10;
        this->data[2] = d20;
        this->data[3] = d30;

        this->data[4] = d01;
        this->data[5] = d11;
        this->data[6] = d21;
        this->data[7] = d31;

        this->data[8] = d02;
        this->data[9] = d12;
        this->data[10] = d22;
        this->data[11] = d32;

        this->data[12] = d03;
        this->data[13] = d13;
        this->data[14] = d23;
        this->data[15] = d33;
    }

public:
    Matrix4x4()
    : Matrix<T, 4>()
    {}

    template<ARITHMETIC(U)> static Matrix4x4 fromQuaternion(const Quaternion<U>& q)
    {
        T s = 2.0 / (q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);

        T xs = s * q.x;
        T ys = s * q.y;
        T zs = s * q.z;

        T wx = q.w * xs;
        T wy = q.w * ys;
        T wz = q.w * zs;

        T xx = q.x * xs;
        T xy = q.x * ys;
        T xz = q.x * zs;

        T yy = q.y * ys;
        T yz = q.y * zs;
        T zz = q.z * zs;

        return Matrix4x4(
            1.0 - (yy + zz),
            xy - wz,
            xz + wy,
            0,
            
            xy + wz,
            1.0 - (xx + zz),
            yz - wx,
            0,
            
            xz - wy,
            yz + wx,
            1.0 - (xx + yy),
            0,
            
            0,
            0,
            0,
            1
        );
    }

    static Matrix4x4 identity()
    {
        return Matrix4x4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }

    template<ARITHMETIC(U)> static Matrix4x4 translate(U x, U y, U z)
    {
        return Matrix4x4(
            1, 0, 0, x,
            0, 1, 0, y,
            0, 0, 1, z,
            0, 0, 0, 1
        );
    }

    template<ARITHMETIC(U)> static Matrix4x4 translate(const Vector3<U>& v)
    {
        return Matrix4x4(
            1, 0, 0, v.x,
            0, 1, 0, v.y,
            0, 0, 1, v.z,
            0, 0, 0, 1
        );
    }

    template<ARITHMETIC(U)> static Matrix4x4 rotate(U angle, U x, U y, U z)
    {
        T c = std::cos(angle / 180.0 * 3.14159265358979323846);
        T s = std::sin(angle / 180.0 * 3.14159265358979323846);
        
        T f = std::sqrt(x * x + y * y + z * z);
        
        x /= f;
        y /= f;
        z /= f;

        return Matrix4x4(
            x*x*(1-c)+c, x*y*(1-c)-z*s, x*z*(1-c)+y*s, 0,
            y*x*(1-c)+z*s, y*y*(1-c)+c, y*z*(1-c)-x*s, 0,
            x*z*(1-c)-y*s, y*z*(1-c)+x*s, z*z*(1-c)+c, 0,
            0, 0, 0, 1
        );
    }

    template<ARITHMETIC(U)> static Matrix4x4 scale(U x, U y, U z)
    {
        return Matrix4x4(
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1
        );
    }

    template<ARITHMETIC(U)> static Matrix4x4 frustum(U left, U right, U bottom, U top, U near, U far)
    {
        return Matrix4x4(
            (2 * near) / (right - left), 0, (right + left) / (right - left), 0,
            0, (2 * near) / (top - bottom), (top + bottom) / (top - bottom), 0,
            0, 0, -(far + near) / (far - near), -(2 * far * near) / (far - near),
            0, 0, -1, 0
        );
    }

    template<ARITHMETIC(U)> static Matrix4x4 ortho(U left, U right, U bottom, U top, U near, U far)
    {
        return Matrix4x4(
            2 / (right - left), 0, 0, (right + left) / (right - left),
            0, 2 / (top - bottom), 0, (top + bottom) / (top - bottom),
            0, 0, -2 / (far - near), (far + near) / (far - near),
            0, 0, 0, 1
        );
    }

    template<ARITHMETIC(U)> static Matrix4x4 perspective(U fov_y, U aspect, U near, U far)
    {
        T fW, fH;

        fH = std::tan(fov_y / 360 * pi) * near;
        fW = fH * aspect;

        return Matrix4x4::frustum(-fW, fW, -fH, fH, near, far);
    }

    Matrix4x4 operator*(const Matrix4x4& m) const
    {
        return Matrix4x4(Matrix<T, 4>::operator*(m));
    }

    template<ARITHMETIC(U)> Vector2<T> operator*(const Vector2<U>& v) const
    {
        return Vector2<T>(
            this->data[0] * v.x + this->data[4] * v.y + this->data[12],
            this->data[1] * v.x + this->data[5] * v.y + this->data[13]
        );
    }

    template<ARITHMETIC(U)> Vector3<T> operator*(const Vector3<U>& v) const
    {
        return Vector3<T>(
            this->data[0] * v.x + this->data[4] * v.y + this->data[8] * v.z + this->data[12],
            this->data[1] * v.x + this->data[5] * v.y + this->data[9] * v.z + this->data[13],
            this->data[2] * v.x + this->data[6] * v.y + this->data[10] * v.z + this->data[14]
        );
    }

    template<ARITHMETIC(U)> Vector2<T> applyDirection(const Vector2<U>& v) const
    {
        return Vector2<T>(
            this->data[0] * v.x + this->data[4] * v.y,
            this->data[1] * v.x + this->data[5] * v.y
        );
    }

    template<ARITHMETIC(U)> Vector3<T> applyDirection(const Vector3<U>& v) const
    {
        return Vector3<T>(
            this->data[0] * v.x + this->data[4] * v.y + this->data[8] * v.z,
            this->data[1] * v.x + this->data[5] * v.y + this->data[9] * v.z,
            this->data[2] * v.x + this->data[6] * v.y + this->data[10] * v.z
        );
    }
    
    Matrix4x4 inverse() const
    {
        T inv[16], det;

        inv[0] = this->data[5]  * this->data[10] * this->data[15] - 
                 this->data[5]  * this->data[11] * this->data[14] - 
                 this->data[9]  * this->data[6]  * this->data[15] + 
                 this->data[9]  * this->data[7]  * this->data[14] +
                 this->data[13] * this->data[6]  * this->data[11] - 
                 this->data[13] * this->data[7]  * this->data[10];

        inv[4] = -this->data[4]  * this->data[10] * this->data[15] + 
                  this->data[4]  * this->data[11] * this->data[14] + 
                  this->data[8]  * this->data[6]  * this->data[15] - 
                  this->data[8]  * this->data[7]  * this->data[14] - 
                  this->data[12] * this->data[6]  * this->data[11] + 
                  this->data[12] * this->data[7]  * this->data[10];

        inv[8] = this->data[4]  * this->data[9] * this->data[15] - 
                 this->data[4]  * this->data[11]* this->data[13] - 
                 this->data[8]  * this->data[5] * this->data[15] + 
                 this->data[8]  * this->data[7] * this->data[13] + 
                 this->data[12] * this->data[5] * this->data[11] - 
                 this->data[12] * this->data[7] * this->data[9];

        inv[12] = -this->data[4]  * this->data[9] * this->data[14] + 
                   this->data[4]  * this->data[10] * this->data[13] +
                   this->data[8]  * this->data[5] * this->data[14] - 
                   this->data[8]  * this->data[6] * this->data[13] - 
                   this->data[12] * this->data[5] * this->data[10] + 
                   this->data[12] * this->data[6] * this->data[9];

        inv[1] = -this->data[1]  * this->data[10] * this->data[15] + 
                  this->data[1]  * this->data[11] * this->data[14] + 
                  this->data[9]  * this->data[2] * this->data[15] - 
                  this->data[9]  * this->data[3] * this->data[14] - 
                  this->data[13] * this->data[2] * this->data[11] + 
                  this->data[13] * this->data[3] * this->data[10];

        inv[5] = this->data[0]  * this->data[10] * this->data[15] - 
                 this->data[0]  * this->data[11] * this->data[14] - 
                 this->data[8]  * this->data[2] * this->data[15] + 
                 this->data[8]  * this->data[3] * this->data[14] + 
                 this->data[12] * this->data[2] * this->data[11] - 
                 this->data[12] * this->data[3] * this->data[10];

        inv[9] = -this->data[0]  * this->data[9] * this->data[15] + 
                  this->data[0]  * this->data[11] * this->data[13] + 
                  this->data[8]  * this->data[1] * this->data[15] - 
                  this->data[8]  * this->data[3] * this->data[13] - 
                  this->data[12] * this->data[1] * this->data[11] + 
                  this->data[12] * this->data[3] * this->data[9];

        inv[13] = this->data[0]  * this->data[9] * this->data[14] - 
                  this->data[0]  * this->data[10] * this->data[13] - 
                  this->data[8]  * this->data[1] * this->data[14] + 
                  this->data[8]  * this->data[2] * this->data[13] + 
                  this->data[12] * this->data[1] * this->data[10] - 
                  this->data[12] * this->data[2] * this->data[9];

        inv[2] = this->data[1]  * this->data[6] * this->data[15] - 
                 this->data[1]  * this->data[7] * this->data[14] - 
                 this->data[5]  * this->data[2] * this->data[15] + 
                 this->data[5]  * this->data[3] * this->data[14] + 
                 this->data[13] * this->data[2] * this->data[7] - 
                 this->data[13] * this->data[3] * this->data[6];

        inv[6] = -this->data[0]  * this->data[6] * this->data[15] + 
                  this->data[0]  * this->data[7] * this->data[14] + 
                  this->data[4]  * this->data[2] * this->data[15] - 
                  this->data[4]  * this->data[3] * this->data[14] - 
                  this->data[12] * this->data[2] * this->data[7] + 
                  this->data[12] * this->data[3] * this->data[6];

        inv[10] = this->data[0]  * this->data[5] * this->data[15] - 
                  this->data[0]  * this->data[7] * this->data[13] - 
                  this->data[4]  * this->data[1] * this->data[15] + 
                  this->data[4]  * this->data[3] * this->data[13] + 
                  this->data[12] * this->data[1] * this->data[7] - 
                  this->data[12] * this->data[3] * this->data[5];

        inv[14] = -this->data[0]  * this->data[5] * this->data[14] + 
                   this->data[0]  * this->data[6] * this->data[13] + 
                   this->data[4]  * this->data[1] * this->data[14] - 
                   this->data[4]  * this->data[2] * this->data[13] - 
                   this->data[12] * this->data[1] * this->data[6] + 
                   this->data[12] * this->data[2] * this->data[5];

        inv[3] = -this->data[1] * this->data[6] * this->data[11] + 
                  this->data[1] * this->data[7] * this->data[10] + 
                  this->data[5] * this->data[2] * this->data[11] - 
                  this->data[5] * this->data[3] * this->data[10] - 
                  this->data[9] * this->data[2] * this->data[7] + 
                  this->data[9] * this->data[3] * this->data[6];

        inv[7] = this->data[0] * this->data[6] * this->data[11] - 
                 this->data[0] * this->data[7] * this->data[10] - 
                 this->data[4] * this->data[2] * this->data[11] + 
                 this->data[4] * this->data[3] * this->data[10] + 
                 this->data[8] * this->data[2] * this->data[7] - 
                 this->data[8] * this->data[3] * this->data[6];

        inv[11] = -this->data[0] * this->data[5] * this->data[11] + 
                   this->data[0] * this->data[7] * this->data[9] + 
                   this->data[4] * this->data[1] * this->data[11] - 
                   this->data[4] * this->data[3] * this->data[9] - 
                   this->data[8] * this->data[1] * this->data[7] + 
                   this->data[8] * this->data[3] * this->data[5];

        inv[15] = this->data[0] * this->data[5] * this->data[10] - 
                  this->data[0] * this->data[6] * this->data[9] - 
                  this->data[4] * this->data[1] * this->data[10] + 
                  this->data[4] * this->data[2] * this->data[9] + 
                  this->data[8] * this->data[1] * this->data[6] - 
                  this->data[8] * this->data[2] * this->data[5];

        det = this->data[0] * inv[0] + this->data[1] * inv[4] + this->data[2] * inv[8] + this->data[3] * inv[12];

        if (det == 0)
            return Matrix4x4();

        det = 1.0 / det;

        return Matrix4x4(
            inv[0] * det,
            inv[4] * det,
            inv[8] * det,
            inv[12] * det,
            inv[1] * det,
            inv[5] * det,
            inv[9] * det,
            inv[13] * det,
            inv[2] * det,
            inv[6] * det,
            inv[10] * det,
            inv[14] * det,
            inv[3] * det,
            inv[7] * det,
            inv[11] * det,
            inv[15] * det
        );
    }
};

typedef Matrix4x4<float> Matrix4x4f;
typedef Matrix4x4<double> Matrix4x4d;

template<typename T> static inline std::ostream& operator<<(std::ostream& os, const Matrix4x4<T>& mat)
{
    os << mat.data[0] << ',' << mat.data[1] << ',' << mat.data[2] << ',' << mat.data[3] << '\n';
    os << mat.data[4] << ',' << mat.data[5] << ',' << mat.data[6] << ',' << mat.data[7] << '\n';
    os << mat.data[8] << ',' << mat.data[9] << ',' << mat.data[10] << ',' << mat.data[11] << '\n';
    os << mat.data[12] << ',' << mat.data[13] << ',' << mat.data[14] << ',' << mat.data[15] << '\n';
    return os;
}

};//namespace sp

#endif//MATRIX_4X4_H
