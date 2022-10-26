#pragma once

#include "common.h"

namespace ev
{
	class Application
	{
		// public methods
	public:
		Application();
		~Application() = default;

		// private methods
	private:
		void CreateDebuger();
		void CreateDevice();
		IDXGIAdapter1* EnumerateAdapters(D3D_FEATURE_LEVEL featureLevel);
		UINT CheckMSAAQuality(UINT count = 4);
		void CreateCommandVariables();

		// public variables
	public:

		// private variables
	private: 
		UINT m_RTVDescriptorSize;
		UINT m_DSVDescriptorSize;
		UINT m_UAVDescriptorSize;
		DXGI_FORMAT m_backBufferFormat;

		// Device & Debug
		Microsoft::WRL::ComPtr<ID3D12Debug> m_debugController;
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgiFactory;

		// Command
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_directCommandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

		// Fence
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	};
}