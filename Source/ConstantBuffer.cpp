#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer()
{

}

void ConstantBuffer::Initialize(ID3D12Device* device, uint64_t size)
{
	D3D12_HEAP_PROPERTIES cbHeapProperties;
	cbHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	cbHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	cbHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	cbHeapProperties.CreationNodeMask = 1;
	cbHeapProperties.VisibleNodeMask = 1;


	D3D12_RESOURCE_DESC cbResourceDesc;
	cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbResourceDesc.Alignment = 0;
	cbResourceDesc.Width = (size + 255) & ~255;
	cbResourceDesc.Height = 1;
	cbResourceDesc.DepthOrArraySize = 1;
	cbResourceDesc.MipLevels = 1;
	cbResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	cbResourceDesc.SampleDesc.Count = 1;
	cbResourceDesc.SampleDesc.Quality = 0;
	cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	cbResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(device->CreateCommittedResource(
		&cbHeapProperties, D3D12_HEAP_FLAG_NONE, &cbResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&Resource)));

	ViewDesc.BufferLocation = Resource->GetGPUVirtualAddress();
	ViewDesc.SizeInBytes = (size + 255) & ~255; // CB size is required to be 256-byte aligned.
}

UINT8* ConstantBuffer::Map()
{
	UINT8* mappedBuffer;
	ThrowIfFailed(Resource->Map(
		0, &MapRange, reinterpret_cast<void**>(&mappedBuffer)));
	return mappedBuffer;
}

void ConstantBuffer::Unmap()
{
	Resource->Unmap(0, &MapRange);
}
