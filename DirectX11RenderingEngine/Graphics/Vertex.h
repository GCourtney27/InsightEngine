#pragma once
//#include <DirectXMath.h>
#include "C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\um\DirectXMath.h"
struct Vertex2D
{
	Vertex2D() {}
	Vertex2D(float x, float y, float z, float u, float v)
		: pos(x, y, z), texCoord(u, v) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 texCoord;
	DirectX::XMFLOAT3 normal;
};

struct Vertex3D
{
	Vertex3D() {}
	Vertex3D(float x, float y, float z, float u, float v, float nx, float ny, float nz)
		: pos(x, y, z), texCoord(u, v), normal(nx, ny, nz) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 texCoord;
	DirectX::XMFLOAT3 normal;
};