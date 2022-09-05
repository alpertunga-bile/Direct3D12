#include "../Includes/HelloTriangle.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

HelloTriangle::HelloTriangle(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen) :
	frameIndex(0),
	useWarpDevice(false),
	viewport(0.0f, 0.0f, static_cast<float>(WIDTH), static_cast<float>(HEIGHT)),
	scissorRect(0, 0, static_cast<LONG>(WIDTH), static_cast<LONG>(HEIGHT)),
	rtvDescriptorSize(0)
{
	if (!window.InitializeWindow(hInstance, ShowWnd, width, height, fullscreen))
	{
		MessageBox(0, L"Cannot Initialize Window", L"Error", MB_OK);
		exit(1);
	}
}

void HelloTriangle::OnInit()
{
	LoadPipeline();
	LoadAssets();
}

void HelloTriangle::OnUpdate()
{
}

void HelloTriangle::OnRender()
{
	PopulateCommandLists();

	ID3D12CommandList* pp_commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(pp_commandLists), pp_commandLists);

	CHECK_HRESULT(swapchain->Present(1, 0));

	WaitForPreviousFrame();
}

void HelloTriangle::OnDestroy()
{
	WaitForPreviousFrame();

	CloseHandle(fenceEvent);
}

void HelloTriangle::LoadPipeline()
{
	unsigned int dxgiFactoryFlags = 0;

	// --------------------------------------------------------------------------------------------------------
	// Enable Debug Layer if debug is enabled
#if _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();

		dxgiFactoryFlags != DXGI_CREATE_FACTORY_DEBUG;
	}
#endif

	// --------------------------------------------------------------------------------------------------------
	// Create Device

	Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
	// Creates a DXGI 1.3 factory
	CHECK_HRESULT(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
	GetHardwareAdapter(factory.Get(), &hardwareAdapter, false);

	CHECK_HRESULT(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));

	// --------------------------------------------------------------------------------------------------------
	// Create Command Queue

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	CHECK_HRESULT(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

	// --------------------------------------------------------------------------------------------------------
	// Create Swapchain

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.BufferCount = frameCount;
	swapchainDesc.Width = WIDTH;
	swapchainDesc.Height = HEIGHT;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.SampleDesc.Count = 1;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
	CHECK_HRESULT(factory->CreateSwapChainForHwnd(
		commandQueue.Get(),
		window.GetHWND(),
		&swapchainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	CHECK_HRESULT(factory->MakeWindowAssociation(window.GetHWND(), DXGI_MWA_NO_ALT_ENTER));

	CHECK_HRESULT(swapChain.As(&swapchain));
	frameIndex = swapchain->GetCurrentBackBufferIndex();

	// --------------------------------------------------------------------------------------------------------
	// Create Descriptor Heap

	// render target view descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	CHECK_HRESULT(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

	// shader resource view heap for texture
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	
	CHECK_HRESULT(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap)));

	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// --------------------------------------------------------------------------------------------------------
	// Create Frame Resources

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (unsigned int i = 0; i < frameCount; i++)
	{
		CHECK_HRESULT(swapchain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i])));
		device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, rtvDescriptorSize);
	}

	CHECK_HRESULT(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
}

void HelloTriangle::LoadAssets()
{
	// --------------------------------------------------------------------------------------------------------
	// Create Root Signature

	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

	CD3DX12_ROOT_PARAMETER1 rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;

	CHECK_HRESULT(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	CHECK_HRESULT(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

	// --------------------------------------------------------------------------------------------------------
	// Create Pipeline State

	Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;

#if _DEBUG
	unsigned int compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	unsigned int compileFlags = 0;
#endif

	CHECK_HRESULT(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	CHECK_HRESULT(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	CHECK_HRESULT(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));

	// --------------------------------------------------------------------------------------------------------
	// Create Command List

	CHECK_HRESULT(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), pipelineState.Get(), IID_PPV_ARGS(&commandList)));

	// --------------------------------------------------------------------------------------------------------
	// Create Vertex Buffer

	Vertex triangleVertices[] =
	{
		{ {  0.0f,   0.25f * aspect_ratio, 0.0f }, { 0.5f, 0.0f } },
		{ {  0.25f, -0.25f * aspect_ratio, 0.0f }, { 1.0f, 1.0f } },
		{ { -0.25f, -0.25f * aspect_ratio, 0.0f }, { 0.0f, 1.0f } }
	};

	const unsigned int vertexBufferSize = sizeof(triangleVertices);

	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	CHECK_HRESULT(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer)
	));

	UINT8* p_vertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	CHECK_HRESULT(vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&p_vertexDataBegin)));
	memcpy(p_vertexDataBegin, triangleVertices, sizeof(triangleVertices));
	vertexBuffer->Unmap(0, nullptr);

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = vertexBufferSize;

	// --------------------------------------------------------------------------------------------------------
	// Create Texture

	Microsoft::WRL::ComPtr<ID3D12Resource> textureUploadHeap;

	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	CD3DX12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	CHECK_HRESULT(device->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&texture)
	));

	const uint64_t uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

	heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC buffer = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	CHECK_HRESULT(device->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,
		&buffer,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureUploadHeap)
	));

	std::vector<uint8_t> texture_data = GenerateTextureData();

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = &texture_data[0];
	textureData.RowPitch = textureWidth * texturePixelSize;
	textureData.SlicePitch = textureData.RowPitch * textureHeight;

	UpdateSubresources(commandList.Get(), texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		texture.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	
	commandList->ResourceBarrier(1, &barrier);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(texture.Get(), &srvDesc, srvHeap->GetCPUDescriptorHandleForHeapStart());

	CHECK_HRESULT(commandList->Close());
	ID3D12CommandList* pp_commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(pp_commandLists), pp_commandLists);

	// --------------------------------------------------------------------------------------------------------
	// Create Fence

	CHECK_HRESULT(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	fenceValue = 1;

	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
	{
		CHECK_HRESULT(HRESULT_FROM_WIN32(GetLastError()));
	}

	WaitForPreviousFrame();
}

std::vector<uint8_t> HelloTriangle::GenerateTextureData()
{
	const unsigned int rowPitch = textureWidth * texturePixelSize;
	const unsigned int cellPitch = rowPitch >> 3;
	const unsigned int cellHeight = textureWidth >> 3;
	const unsigned int textureSize = rowPitch * textureHeight;

	std::vector<uint8_t> textureData(textureSize);
	uint8_t* p_data = &textureData[0];

	for (unsigned int n = 0; n < textureSize; n += texturePixelSize)
	{
		unsigned int x = n % rowPitch;
		unsigned int y = n / rowPitch;
		unsigned int i = x / cellPitch;
		unsigned int j = y / cellHeight;

		p_data[n]	  = GetIntValue(0, 255);
		p_data[n + 1] = GetIntValue(0, 255);
		p_data[n + 2] = GetIntValue(0, 255);
		p_data[n + 3] = 255;
	}

	return textureData;
}

void HelloTriangle::PopulateCommandLists()
{
	CD3DX12_RESOURCE_BARRIER barrier;

	CHECK_HRESULT(commandAllocator->Reset());

	CHECK_HRESULT(commandList->Reset(commandAllocator.Get(), pipelineState.Get()));

	commandList->SetGraphicsRootSignature(rootSignature.Get());

	ID3D12DescriptorHeap* pp_heaps[] = { srvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(pp_heaps), pp_heaps);

	commandList->SetGraphicsRootDescriptorTable(0, srvHeap->GetGPUDescriptorHandleForHeapStart());
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);

	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->DrawInstanced(3, 1, 0, 0);


	barrier = (CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	commandList->ResourceBarrier(1, &barrier);

	CHECK_HRESULT(commandList->Close());
}

void HelloTriangle::WaitForPreviousFrame()
{
	const UINT64 Fence = fenceValue;
	CHECK_HRESULT(commandQueue->Signal(fence.Get(), Fence));
	fenceValue++;

	if (fence->GetCompletedValue() < Fence)
	{
		CHECK_HRESULT(fence->SetEventOnCompletion(Fence, fenceEvent));
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	frameIndex = swapchain->GetCurrentBackBufferIndex();
}

void HelloTriangle::GetHardwareAdapter(IDXGIFactory1* p_factory, IDXGIAdapter1** pp_adapter, bool isHighPerformance)
{
	pp_adapter = nullptr;

	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
	Microsoft::WRL::ComPtr<IDXGIFactory6> factory;

	if (FAILED(p_factory->QueryInterface(IID_PPV_ARGS(&factory)))) return;

	for (
		unsigned int adapterIndex = 0;
		SUCCEEDED(factory->EnumAdapterByGpuPreference(
			adapterIndex,
			isHighPerformance == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
			IID_PPV_ARGS(&adapter)
		));
		adapterIndex++
		)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	if (adapter.Get() != nullptr) return;

	for (unsigned int adapterIndex = 0; SUCCEEDED(p_factory->EnumAdapters1(adapterIndex, &adapter)); adapterIndex++)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*pp_adapter = adapter.Detach();
}
