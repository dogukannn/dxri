#pragma once

class Texture
{
public:
	//Creates resources using directxtk helpers
	void LoadFromFile(ID3D12Device* device, ID3D12CommandQueue* commandQueue, LPCWSTR filename);

	//Creates resource without helpers
	void LoadFromFileManual(ID3D12Device* device, ID3D12CommandQueue* commandQueue,
	                        ID3D12CommandAllocator* commandAllocator, LPCWSTR filename);

	ID3D12Resource* Resource;

	UINT Width;
	UINT Height;
	DXGI_FORMAT Format;
};