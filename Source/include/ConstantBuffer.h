#pragma once

class ConstantBuffer
{
public:
	ConstantBuffer();
	
	void Initialize(ID3D12Device* device, uint64_t size);

	UINT8* Map();
	void Unmap();

	ID3D12Resource* Resource;
	D3D12_CONSTANT_BUFFER_VIEW_DESC ViewDesc = {};


	D3D12_RANGE MapRange = {0,0};
};