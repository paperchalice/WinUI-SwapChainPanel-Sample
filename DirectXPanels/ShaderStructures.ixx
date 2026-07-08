module;
#include "pch.h"

export module DX:ShaderStructures;

using namespace DirectX;

export namespace DX
{
struct ModelViewProjectionConstantBuffer
{
    XMFLOAT4X4 model;
    XMFLOAT4X4 view;
    XMFLOAT4X4 projection;
};

// Used to send per-vertex data to the vertex shader.
struct VertexPositionColor
{
    XMFLOAT3 pos;
    XMFLOAT3 color;
};
} // namespace DX