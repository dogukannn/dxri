#pragma once
#include "Shader.h"


class DXRI;

class DynamicRootSignature
{
public:
	DynamicRootSignature();
	bool Initialize(DXRI* dxri, VertexShader* vertexShader, PixelShader* pixelShader);
	
    ID3D12RootSignature* rootSignature;

	ShaderParameters Parameters;
};
