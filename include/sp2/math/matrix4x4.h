#ifndef SP2_MATRIX_4X4_H
#define SP2_MATRIX_4X4_H

#include <sp2/math/matrix.impl.h>
#include <sp2/math/quaternion.h>

#include <cmath>
#include <cstring>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

namespace sp {

template<typename T> class Matrix4x4 : public Matrix<T, 4>
{
protected:
    Matrix4x4(Matrix<T, 4> m)
    : Matrix<T, 4>(m)
    {
    }

    Matrix4x4(T d00, T d01, T d02, T d03, T d10, T d11, T d12, T d13, T d20, T d21, T d22, T d23, T d30, T d31, T d32, T d33)
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

    static Matrix4x4 fromQuaternion(const Quaternion<T>& q)
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

    static Matrix4x4 translate(T x, T y, T z)
    {
        return Matrix4x4(
            1, 0, 0, x,
            0, 1, 0, y,
            0, 0, 1, z,
            0, 0, 0, 1
        );
    }

    static Matrix4x4 translate(sf::Vector3<T>& v)
    {
        return Matrix4x4(
            1, 0, 0, v.x,
            0, 1, 0, v.y,
            0, 0, 1, v.z,
            0, 0, 0, 1
        );
    }
    
    static Matrix4x4 rotate(T angle, T x, T y, T z)
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
    
    static Matrix4x4 scale(T x, T y, T z)
    {
        return Matrix4x4(
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1
        );
    }
    
    static Matrix4x4 frustum(T left, T right, T bottom, T top, T near, T far)
{
    return Matrix4x4(
        (2 * near) / (right - left), 0, (right + left) / (right - left), 0,
        0, 0, (top + bottom) / (top - bottom), 0,
        0, 0, (far + near) / (far - near), (2 * far * near) / (far - near),
        0, 0, -1, 0
    );
}

    static Matrix4x4 ortho(T left, T right, T bottom, T top, T near, T far)
    {
        return Matrix4x4(
            2 / (right - left), 0, 0, (right + left) / (right - left),
            0, 2 / (top - bottom), 0, (top + bottom) / (top - bottom),
            0, 0, -2 / (far - near), (far + near) / (far - near),
            0, 0, 0, 1
        );
    }

    static Matrix4x4 perspective(T fov_y, T aspect, T near, T far)
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

    sf::Vector2<T> operator*(const sf::Vector2<T>& v) const
    {
        return sf::Vector2<T>(
            this->data[0] * v.x + this->data[4] * v.y + this->data[12],
            this->data[1] * v.x + this->data[5] * v.y + this->data[13]
        );
    }
    
    sf::Vector3<T> operator*(const sf::Vector3<T>& v) const
    {
        return sf::Vector3<T>(
            this->data[0] * v.x + this->data[4] * v.y + this->data[8] * v.z + this->data[12],
            this->data[1] * v.x + this->data[5] * v.y + this->data[9] * v.z + this->data[13],
            this->data[2] * v.x + this->data[6] * v.y + this->data[10] * v.z + this->data[14]
        );
    }
};

typedef Matrix4x4<float> Matrix4x4f;
typedef Matrix4x4<double> Matrix4x4d;

};//!namespace sp

#endif//MATRIX_4X4_H
