#pragma once

class DXRI;

class ConstantBuffer
{
public:
	ConstantBuffer();
	
	void Initialize(DXRI* dxri, uint64_t size);

	UINT8* Map();
	void Unmap();

	ID3D12Resource* Resource;
	D3D12_CONSTANT_BUFFER_VIEW_DESC ViewDesc = {};
	
	D3D12_RANGE MapRange = {0,0};
};
