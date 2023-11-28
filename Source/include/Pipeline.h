#pragma once
#include "DynamicRootSignature.h"
#include "Shader.h"

class Pipeline
{
public:
	void Initialize(ID3D12Device* device, VertexShader* vertexShader, PixelShader* pixelShader);

	void SetPipelineState(ID3D12CommandAllocator* commandAllocator, ID3D12GraphicsCommandList* commandList);

	void BindTexture(ID3D12Device* device, std::string name, class Texture* texture);
	void BindTexture(ID3D12Device* device, std::string name, ID3D12Resource* texture);
	void BindConstantBuffer(std::string name, class ConstantBuffer* constantBuffer, ID3D12GraphicsCommandList* commandList);

	void Release();

    ID3D12PipelineState* PipelineState = nullptr;
	ID3D12DescriptorHeap* DescriptorHeap = nullptr;
	bool writeDepth = true;
	bool useAlphaBlend = false;

	VertexShader* VShader;
	PixelShader* PShader;

	DynamicRootSignature* RootSignature;
	D3D12_CULL_MODE CullMode = D3D12_CULL_MODE_NONE;

	std::map<uint32_t, D3D12_GPU_VIRTUAL_ADDRESS> ConstantBufferAddresses;

	std::vector<ID3D12DescriptorHeap*> DescriptorHeaps;
	std::map<std::string, uint32_t> HeapIndexMap;

};
