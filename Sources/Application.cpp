#include "../Includes/Application.h"

namespace ev
{
	Application::Application()
	{
		CreateDebuger();
		CreateDevice();
		CreateCommandVariables();
	}

	void Application::CreateDebuger()
	{
		#ifdef _DEBUG
			CHECK_HRESULT(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController)));
			m_debugController->EnableDebugLayer();
			LOG_INFO("Debug Controller Created");
		#endif
	}

	void Application::CreateDevice()
	{
		unsigned int dxgiFactoryFlag = 0;
		dxgiFactoryFlag != DXGI_CREATE_FACTORY_DEBUG;
		CHECK_HRESULT(CreateDXGIFactory2(dxgiFactoryFlag, IID_PPV_ARGS(&m_dxgiFactory)));

		LOG_INFO("DXGI Factory Created");

		IDXGIAdapter1* adapter = EnumerateAdapters(D3D_FEATURE_LEVEL_11_0);
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));

		if (adapter == nullptr)
		{
			LOG_ERROR("Device Cannot Created");
		}

		LOG_INFO("Device Created");

		CHECK_HRESULT(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

		LOG_INFO("Fence Created");

		m_RTVDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DSVDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_UAVDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		UINT quality = CheckMSAAQuality();
	}

	IDXGIAdapter1* Application::EnumerateAdapters(D3D_FEATURE_LEVEL featureLevel)
	{
		UINT count = 0;
		IDXGIAdapter1* adapter;
		std::vector<IDXGIAdapter1*> adapterList;
		while (m_dxgiFactory->EnumAdapters1(count, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			if (SUCCEEDED(D3D12CreateDevice(adapter, featureLevel, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}

		return adapter;
	}

	UINT Application::CheckMSAAQuality(UINT count)
	{
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MSQualityLevels = {};
		MSQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		MSQualityLevels.SampleCount = count;
		MSQualityLevels.NumQualityLevels = 0;
		MSQualityLevels.Format = m_backBufferFormat;

		CHECK_HRESULT(m_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &MSQualityLevels, sizeof(MSQualityLevels)));
		return MSQualityLevels.NumQualityLevels;
	}

	void Application::CreateCommandVariables()
	{
		// Command Queue
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		CHECK_HRESULT(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

		// Command Allocator
		CHECK_HRESULT(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_directCommandAllocator.GetAddressOf())));

		CHECK_HRESULT(m_device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_directCommandAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(m_commandList.GetAddressOf())
		));

		m_commandList->Close();
	}
}