// vector-math.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <DirectXMath.h>

using namespace DirectX;

std::ostream& operator<<(std::ostream& out, FXMVECTOR v);

void FloatingPointErrorExample();

int main()
{
    const XMVECTOR n = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    const XMVECTOR u = XMVectorSet(1.0f, 2.0f, 3.0f, 0.0f);
    const XMVECTOR v = XMVectorSet(-2.0f, 1.0f, -3.0f, 0.0f);
    const XMVECTOR w = XMVectorSet(0.707f, 0.707f, 0.0f, 0.0f);

    // Vector addition
    const XMVECTOR a = u + v;

    // Vector subtraction
    const XMVECTOR b = u - v;

    // Scalar multiplication
    const XMVECTOR c = 10.0f * u;

    // Vector length
    const XMVECTOR length = XMVector3Length(u);

    // Vector normalization
    const XMVECTOR normalized = XMVector3Normalize(u);

    // Dot product
    const XMVECTOR s = XMVector3Dot(u, v);

    // Cross product
    const XMVECTOR e = XMVector3Cross(u, v);

    // Orthogonal and perpendicular projections
    XMVECTOR projW;
    XMVECTOR perpW;
    XMVector3ComponentsFromNormal(&projW, &perpW, w, n);

    // Does projW + perpW == w?
    const bool equal = XMVector3Equal(projW + perpW, w) != 0;
    const bool notEqual = XMVector3NotEqual(projW + perpW, w) != 0;

    // The angle between projW and perpW should be 90 degrees
    const XMVECTOR angle = XMVector3AngleBetweenVectors(projW, perpW);
    const float angleRadians = XMVectorGetX(angle);
    const float angleDegrees = XMConvertToDegrees(angleRadians);

    std::cout.setf(std::ios_base::boolalpha);
    std::cout << "***DirectXMath vector operations***\n" << std::endl;
    std::cout << "u = " << u << std::endl;
    std::cout << "v = " << v << std::endl;
    std::cout << "w = " << w << std::endl;
    std::cout << "n = " << n << std::endl;
    std::cout << "a = u + v = " << a << std::endl;
    std::cout << "b = u - v = " << b << std::endl;
    std::cout << "c = 10 * u = " << c << std::endl;
    std::cout << "d = u / ||u|| = " << normalized << std::endl;
    std::cout << "e = u x v = " << e << std::endl;
    std::cout << "L = ||u|| = " << length << std::endl;
    std::cout << "s = u.v = " << s << std::endl;
    std::cout << "projW = " << projW << std::endl;
    std::cout << "perpW = " << perpW << std::endl;
    std::cout << "projW + perpW == w = " << equal << std::endl;
    std::cout << "projW + perpW != w = " << notEqual << std::endl;
    std::cout << "angle = " << angleDegrees << std::endl;

    std::cout << "\n\n\n" << std::endl;
    FloatingPointErrorExample();
}

void FloatingPointErrorExample()
{
    std::cout << "***Floating point error***\n" << std::endl;
    std::cout.precision(8);

    const XMVECTOR u = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
    const XMVECTOR n = XMVector3Normalize(u);
    const float lu = XMVectorGetX(XMVector3Length(n));

    std::cout << "Mathematically, the length should be 1. Is it numerically?" << std::endl;
    std::cout << lu << std::endl;
    std::cout << (lu == 1.0f ? "length 1" : "length not 1") << std::endl;

    std::cout << "Raising 1 to any power should still be 1. Is it?" << std::endl;
    const float powLU = std::powf(lu, 1.0e6f);
    std::cout << "lu^(10^6) = " << powLU << std::endl;
    
}

std::ostream& operator<<(std::ostream& out, FXMVECTOR v)
{
    XMFLOAT3 dest;
    XMStoreFloat3(&dest, v);
    out << "(" << dest.x << "," << dest.y << "," << dest.z << ")";
    return out;
}