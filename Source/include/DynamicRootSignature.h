#pragma once
#include "Shader.h"


class DynamicRootSignature
{
public:
	DynamicRootSignature();
	bool Initialize(ID3D12Device* device, VertexShader* vertexShader, PixelShader* pixelShader);
	
    ID3D12RootSignature* rootSignature;

	ShaderParameters Parameters;
};
