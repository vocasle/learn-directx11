#include <iostream>
#include <iomanip>

#include <DirectXMath.h>

using namespace DirectX;

std::ostream& operator<<(std::ostream& out, FXMVECTOR v)
{
	XMFLOAT4 dest;
	XMStoreFloat4(&dest, v);
	out << "(" << std::right << std::setw(6)
		<< dest.x << "," << std::setw(6)
		<< dest.y << "," << std::setw(6)
		<< dest.z << "," << std::setw(6)
		<< dest.w << ")";
	return out;
}

std::ostream& operator<<(std::ostream& out, CXMMATRIX mat)
{
	static constexpr size_t mat_size = 4;

	XMFLOAT4X4 dest;
	XMStoreFloat4x4(&dest, mat);
	for (size_t i = 0; i < mat_size; ++i) {
		for (size_t j = 0; j < mat_size; ++j)
			out << std::right << std::setw(6) << dest(i, j) << '\t';
		out << '\n';
	}
	return out;
}

int main()
{
	XMMATRIX A(1.0f, 0.0f, 0.0f, 0.0f,
						0.0f, 2.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 4.0f, 0.0f,
						1.0f, 2.0f, 3.0f, 1.0f);

	XMMATRIX B = XMMatrixIdentity();

	XMMATRIX C = A * B;

	XMMATRIX D = XMMatrixTranspose(A);

	XMVECTOR detA = XMMatrixDeterminant(A);

	XMMATRIX E = XMMatrixInverse(&detA, A);

	XMMATRIX F = A * E;

	std::cout << "A = " << std::endl << A << std::endl;
	std::cout << "B = " << std::endl << B << std::endl;
	std::cout << "C = A*B = " << std::endl << C << std::endl;
	std::cout << "D = transpose(A) = " << std::endl << D << std::endl;
	std::cout << "det = determinant(A) = " << detA << std::endl << std::endl;
	std::cout << "E = inverse(A) = " << std::endl << E << std::endl;
	std::cout << "F = A*E = " << std::endl << F << std::endl;

}
