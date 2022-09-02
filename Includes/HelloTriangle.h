#pragma once

#include "window.h"

class HelloTriangle
{
public:
	HelloTriangle(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen);
	
	void OnInit();
	void OnUpdate();
	void OnRender();
	void OnDestroy();

private:
	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandLists();
	void WaitForPreviousFrame();

	void GetHardwareAdapter(IDXGIFactory1* p_factory, IDXGIAdapter1** pp_adapter, bool isHighPerformance);

private:
	Window window;

	static const unsigned int frameCount = 2;
	bool useWarpDevice;

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

	CD3DX12_VIEWPORT viewport;
	CD3DX12_RECT scissorRect;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapchain;
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargets[frameCount];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	unsigned int rtvDescriptorSize;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	unsigned int frameIndex;
	HANDLE fenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	uint64_t fenceValue;
};
