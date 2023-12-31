#pragma once

class DXRI
{
public:
	DXRI();

	void Initialize();

	ID3D12DescriptorHeap* CreateDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags);
	ID3D12CommandQueue* CreateCommandQueue();
	ID3D12CommandAllocator* CreateCommandAllocator();
	ID3D12Fence* CreateFence();

	void CreateDevice();
	IDXGISwapChain3* CreateSwapChain(LONG width, LONG height, UINT backbufferCount, HWND windowHandle, ID3D12CommandQueue* commandQueue);
	std::vector<ID3D12Resource*> CreateRenderTargets(ID3D12DescriptorHeap* descHeap, IDXGISwapChain3* swapchain,
	                                                 UINT backbufferCount);
	ID3D12Resource* CreateDepthBuffer(UINT width, UINT height, ID3D12DescriptorHeap* descHeap);
	ID3D12GraphicsCommandList* CreateGraphicsCommandList(ID3D12CommandAllocator* commandAllocator, ID3D12PipelineState* pipelineState);

	ID3D12Resource* CreateRawUploadBuffer(UINT size);

	ID3D12Device* Device = nullptr;
	IDXGIAdapter1* Adapter = nullptr;
    IDXGIFactory4* factory = nullptr;

	//should you set up sync here ?? or internal ??
	//what is the reason behind this code ??

	//notes from 29/11/2023
	//these functions are fun and nice to have but, the shader, texture, pipeline classes are easy to use and not much higher level than those are in my mind?
	//maybe, maybe having basic helper functions like "create buffer -> dx12resource" etc. and easily extendible classes like Texture (hiding some dx12 stuff)
	//together can be useful for the different rendering oriented projects. My reasoning behind that is sometimes you want control, sometimes you don't want to
	//touch any thing other than create material + object + render, this library should serve them together as a rendering helper suite may be the name "dxri" is
	//too powerful :))

};
