#include "ConstantBuffer.h"
#include "dxri.h"

ConstantBuffer::ConstantBuffer()
{

}

void ConstantBuffer::Initialize(DXRI* dxri, uint64_t size)
{
	Resource = dxri->CreateRawUploadBuffer((size + 255) & ~255);

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
