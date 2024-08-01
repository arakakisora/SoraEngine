#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include"MyMath.h"
#include "RenderingPipeline.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"
#include<vector>
#include <numbers>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <wrl.h>
#include "Input.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {

		return true;

	}
	//メッセージに応じて固有の処理を行う
	switch (msg) {

		//ウィンドウが破壊されたら
	case WM_DESTROY:
		//OS対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}

void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());

}


#pragma region CompileShader関数
//CompileShader関数の作成
IDxcBlob* CompileShader(
	//ComilerするSahaderファイルへのパス
	const std::wstring& filePath,
	//compilerに使用するProfile
	const wchar_t* profile,
	//初期化で生成したものを3つ	
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler) {


	//シェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));
	//hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	//読めなかったら止める
	assert(SUCCEEDED(hr));
	//読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTF8の文字コードであることを通知

	LPCWSTR arguments[] = {

		filePath.c_str(),//コンパイル対象のhlslファイル名
		L"-E",L"main",//エントリーpointの指定。基本的にmain以外にはしない
		L"-T",profile,//shaerProfileの設定
		L"-Zi",L"-Qembed_debug",//デバック用の情報を埋め込む
		L"-Od",//最適化を外しておく
		L"-Zpr",//メモリレイアウトは行優先
	};

	//実際にshaerをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,//読み込んだファイル
		arguments,			//コンパイルオプション
		_countof(arguments),//コンパイルオプションの数
		includeHandler,		//includeが含まれた
		IID_PPV_ARGS(&shaderResult)//コンパイル結果

	);
	//コンパイルエラーではなくDXCが起動できない致命的な状況
	assert(SUCCEEDED(hr));

	//警告・エラーが出たらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		assert(false);
	}

	//コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//成功したログを出す
	Log(ConvertString(std::format(L"Complite Succeded,path:{},profile:{}\n", filePath, profile)));
	
	return shaderBlob;
}
#pragma endregion

#pragma region Resource関数
//Resource関数
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes) {

	//VertexResourceを作成
	//頂点リソース用ヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	//頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	//バッファーリソース。テクスチャの場合はまた別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;
	//バッファの場合はこれらには1する決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	return vertexResource;

}
#pragma endregion 

#pragma region DescriptorHeap関数
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
	Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heaptype, UINT numDescriptrs, bool shaderVisible) {

	//ディスクリプタヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heaptype;//レンダーターゲットビュー用
	descriptorHeapDesc.NumDescriptors = numDescriptrs;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//ダブルバッファ用に2つ。多くても別に構わない
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	//ディスクリプトひーぷが作れなかったので起動できない
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}
#pragma endregion 

#pragma region LoadTexture関数
//LoadTexture
DirectX::ScratchImage LoadTexture(const std::string& filePath) {

	//テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	//ミニマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	//ミニマップ着きのデータを返す
	return mipImages;
}
#pragma endregion 

#pragma region CreateTextureResource関数
Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata) {
	//metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resouceDesc{ };
	resouceDesc.Width = UINT(metadata.width);//Textureの幅
	resouceDesc.Height = UINT(metadata.height);//Textureの高さ
	resouceDesc.MipLevels = UINT16(metadata.mipLevels);//mipmapの数
	resouceDesc.DepthOrArraySize = UINT16(metadata.arraySize);//奥行きor配Texturaの配列数
	resouceDesc.Format = metadata.format;//Textureのフォーマット
	resouceDesc.SampleDesc.Count = 1;//サンプリクト。１固定。
	resouceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);//Textureの次元数。普段使っているのは２次元
	//利用するHeapの設定。非常に特殊な運用。
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//細かい設定を行う
	//heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//writeBackポリシーでCPUアクセス可能
	//heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//プロセッサの近くに配置
	//Resouceの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,//Heapの設定
		D3D12_HEAP_FLAG_NONE,//Heapの特殊設定。特になし
		&resouceDesc,//Resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,//Clear最適値。使わないのでnullptr
		IID_PPV_ARGS(&resource));

	assert(SUCCEEDED(hr));
	return resource;

}
#pragma endregion

#pragma region UploadTextureData関数
[[nodiscard]]
Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource>texture, const DirectX::ScratchImage& mipImages,Microsoft::WRL::ComPtr<ID3D12Device> device,
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
{
	std::vector<D3D12_SUBRESOURCE_DATA> subresouces;
	DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresouces);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresouces.size()));

	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(device, intermediateSize);
	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subresouces.size()), subresouces.data());

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediateResource;
}
#pragma endregion 

#pragma region CreateDepthStencilTexturResource関数
Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTexturResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height) {
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;//Textureの幅
	resourceDesc.Height = height;//Textureの高さ
	resourceDesc.MipLevels = 1;//mipmapの数
	resourceDesc.DepthOrArraySize = 1;//奥行きor配列Texturの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DetpthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1;//サンプリング。１固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//DepthSrencilとして使う通知

	//利用するhepの設定
	D3D12_HEAP_PROPERTIES heapProperties{  };
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//VRAM上に作る
	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClerValue{};
	depthClerValue.DepthStencil.Depth = 1.0f;//最大値
	depthClerValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーアット。Resource合わせる
	//Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,//Heapの設定
		D3D12_HEAP_FLAG_NONE,//Heapの特殊設定。特になし
		&resourceDesc,//REesourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度値を書き込む状態のしておく
		&depthClerValue,//Clear最適値
		IID_PPV_ARGS(&resource));//作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));
	return resource;
}
#pragma endregion



#pragma region MaterialData
MaterialData LoadMaterialTemplateFile(const std::string& directorypath,const std::string& filename) {

	MaterialData materialData;//構築するMaterialData
	std::string line;//ファイルから読んだ1行を格納するもの
	std::ifstream file(directorypath + "/" + filename);//ファイルを開く
	assert(file.is_open());//とりあえず開けなっかたら止める
	while (std::getline(file, line)) {
		std::string identifile;
		std::stringstream s(line);
		s >> identifile;

		//identifierの応じた処理
		if (identifile == "map_Kd") {

			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directorypath + "/"+textureFilename;

		}


	}

	return materialData;

}
#pragma endregion


#pragma region LoadObjeFil関数
ModelData LoadObjeFile(const std::string& ditrectoryPath, const std::string& filename) {

	ModelData modelData;//構築するModekData
	std::vector<Vector4>positions;//位置
	std::vector<Vector3>normals;//法線
	std::vector<Vector2>texcoords;//テクスチャ座標
	std::string line;//ファイルから読んだ1行を格納するもの

	//ファイル読み込み
	std::ifstream file(ditrectoryPath + "/" + filename);//faileを開く
	assert(file.is_open());//開けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;//先頭の識別子を読む

		if (identifier == "v") {

			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			position.x *= -1;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1 - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {

			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1;
			normals.push_back(normal);
		}
		else if (identifier == "f") {

			VertexData triangle[3];
			//面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				//頂点の要素へのIndexは「位置・UV・法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');//区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				//要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				//VertexData veretex = { position,texcoord,normal };
				//modelData.vertices.push_back(veretex);
				triangle[faceVertex] = { position,texcoord,normal };

			}
			//頂点を逆順で登録刷ることで、周り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);

		}
		else if (identifier == "mtllib") {

			//materialTemlateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(ditrectoryPath, materialFilename);


		}
	}
	return modelData;
}
#pragma endregion 


struct D3DResourceLeakChecker {
	~D3DResourceLeakChecker() {


#pragma region ResourceCheck 

		//リソースチェック
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		
		}
#pragma endregion 


	}


};

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDesctiptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;

}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDesctiptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;

}
// windowアプリでのエントリ―ポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;
#pragma region Window
	CoInitializeEx(0, COINIT_MULTITHREADED);
	//出力ウィンドウへの文字出力
	OutputDebugStringA("HEllo,DIrectX!\n");

	


	WNDCLASS wc{};
	//ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;
	//ウィンドウクラス名
	wc.lpszClassName = L"CG2WindowClass";
	//インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	//カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//ウィンドウクラス登録する
	RegisterClass(&wc);

	//クライアント領域のサイズ
	const int32_t kClientWindth = 1280;
	const int32_t kClientHeight = 720;

	RECT wrc = { 0,0,kClientWindth ,kClientHeight };

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウ生成
	HWND hwnd = CreateWindow(

		wc.lpszClassName, L"CG2,",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);


#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		//デバッグレイヤーを有効にする
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif
#pragma endregion 

#pragma region DxgiFactory
	//DXGIファクターの生成
	Microsoft::WRL::ComPtr< IDXGIFactory7> dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; i++) {

		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {

			Log(ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;
	}
	assert(useAdapter != nullptr);
#pragma endregion

#pragma region D3D12Device
	//D3D12Deviceの生成
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	D3D_FEATURE_LEVEL featureLevels[] = {

	 D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0

	};
	const char* fetureLevelstrings[] = { "12.2","12.1","12.0" };
	for (size_t i = 0; i < _countof(featureLevels); ++i) {

		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr)) {

			Log(std::format("FeatureLevel:{}\n", fetureLevelstrings[i]));
			break;
		}
	}
	assert(device != nullptr);
	Log("Complete create D3D12Device!!!\n");

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;

	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラーに止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告の時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		

		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			//windows11でのDXGIでバックレイヤーとDX12デバックレイヤーの相互性によるエラーメッセージ
			//https:stackoverflow.com/quessions/69805245/directx-12application-is-crashing-in-windows-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

	}

#endif
#pragma endregion

#pragma region CommandQueue,CommandAllocator,CommandList
	//コマンドキューを生成する
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//生成がうまくできなかった
	assert(SUCCEEDED(hr));

	//コマンドアロケーターを生成する
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//コマンドアロケーターの生成がうまく行かなった
	assert(SUCCEEDED(hr));

	//コマンドリストを生成する
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr,
		IID_PPV_ARGS(&commandList));
	//コマンドリストの生成がうまく行かなかったので起動できない
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region SwapChain
	//スワップチェーンを生成する
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = kClientWindth;		//画面の幅。ウィンドウのクライアント領域を同じ物にしておく
	swapChainDesc.Height = kClientHeight;		//画面の高さ。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//色の形式	
	swapChainDesc.SampleDesc.Count = 1;//マルチサンプルなし
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタに写したら、中身を破壊
	//コマンドキュー、ウィンドウハンドル、設定渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));


	//ディスクリプタヒープの生成
	//RTV
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	//SRV
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	//DVS用のヒープでディスクリプタの数は1．DSVはShader内で触るものではない
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//SwapChainからResourceをひっぱってくる
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//うまく取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));
#pragma endregion 

#pragma region RTV
	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2dテクスチャとして書き込む
	//ディスクリプトの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStarHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//RVTを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	//まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
	rtvHandles[0] = rtvStarHandle;
	device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
	//２つ目のディスクリプタハンドルを得る（自力で）
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//２つ目を作る
	device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);
#pragma endregion

#pragma region Fence
	//初期値0でFenceを作る
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	//fenceのSignalを待つためのイベントを作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);

#pragma endregion

#pragma region DicCompiler
	//dicCompilerを初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));
	//includeに対する設定
	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region POS
	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//RootParameter作成。複数設定できるので配列。今回結果１つだけなので長さ１配列
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	//rootParameters[0]設定
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを行う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;//レジスタ番号0とバインド
	//rootParameters[1]設定
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを行う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//PixelShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0;//レジスタ番号0とバインド
	//rootParameters[2]設定
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	////rootParameters[3]設定
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;

	descriptionRootSignature.pParameters = rootParameters;//ルートパラメーター配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);//配列の長さ

	//staticSamplers
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0～1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);



	//シリアライズしてバイナリする
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {

		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);

	}
	//バイナリをもとに生成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	//すべての色素要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面（時計回り）を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//shaderをコンパイルする
	IDxcBlob* vertexshaderBlob = CompileShader(L"Resources/Shaders/Object3D.VS.hlsl",
		L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexshaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = CompileShader(L"Resources/Shaders/Object3D.PS.hlsl",
		L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Deothの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


	//PSOを生成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();//RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;//InputLayout
	graphicsPipelineStateDesc.VS = { vertexshaderBlob->GetBufferPointer(),
	vertexshaderBlob->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };//pixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;//Blendstate
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ（形状）のタイプ。三角刑
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定（気にしなくてよい）
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region Resource
	const uint32_t kSubdbivision = 512;
	ModelData modelData = LoadObjeFile("Resources", "bunny.obj");
	ModelData modelDataTeapot = LoadObjeFile("Resources", "teapot.obj");

	//VertexResourceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = CreateBufferResource(device, sizeof(VertexData) * kSubdbivision * kSubdbivision * 6);
	//DepthStencilTextureをウィンドウサイズで作成
	Microsoft::WRL::ComPtr<ID3D12Resource> deptjStenciResource = CreateDepthStencilTexturResource(device, kClientWindth, kClientHeight);
	//Sprite用の頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vetexResourceSprite = CreateBufferResource(device, sizeof(VertexData) * 4);
	//Sprite用のindexResourceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = CreateBufferResource(device, sizeof(uint32_t) * 6);
	//モデル用のVetexResouceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceModel = CreateBufferResource(device, sizeof(VertexData) * modelData.vertices.size());
	//teapotモデル用のVertexResouceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceTeapotModel = CreateBufferResource(device, sizeof(VertexData) * modelData.vertices.size());


	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//Format
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dtexture
	//DSHeapの先頭にDSVを作る
	device->CreateDepthStencilView(deptjStenciResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());



	//vetexResourceModel頂点バッファーを作成する
	D3D12_VERTEX_BUFFER_VIEW VertexBufferViewModel{};
	//リソースの先頭のアドレスから使う
	VertexBufferViewModel.BufferLocation = vertexResourceModel->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点分のサイズ
	VertexBufferViewModel.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	//1頂点当たりのサイズ
	VertexBufferViewModel.StrideInBytes = sizeof(VertexData);
	//頂点リソースにデータを書き込む
	VertexData* vertexDataModel = nullptr;
	//書き込むためのアドレスを取得
	vertexResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataModel));
	std::memcpy(vertexDataModel, modelData.vertices.data(), sizeof(VertexData)* modelData.vertices.size());


	//vetexResourceModel頂点バッファーを作成する
	D3D12_VERTEX_BUFFER_VIEW VertexBufferViewTeapotModel{};
	//リソースの先頭のアドレスから使う
	VertexBufferViewTeapotModel.BufferLocation = vertexResourceTeapotModel->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点分のサイズ
	VertexBufferViewTeapotModel.SizeInBytes = UINT(sizeof(VertexData) * modelDataTeapot.vertices.size());
	//1頂点当たりのサイズ
	VertexBufferViewTeapotModel.StrideInBytes = sizeof(VertexData);
	//頂点リソースにデータを書き込む
	VertexData* vertexDataTeapotModel = nullptr;
	//書き込むためのアドレスを取得
	vertexResourceTeapotModel->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataTeapotModel));
	std::memcpy(vertexDataTeapotModel, modelDataTeapot.vertices.data(), sizeof(VertexData)* modelDataTeapot.vertices.size());
	
	

	////vertexResource頂点バッファーを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{ };
	//リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * kSubdbivision * kSubdbivision * 6;
	//1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	//頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	//経度分割1つ分の経度φd
	const float kLonEvery = 2 * std::numbers::pi_v<float> / (float)kSubdbivision;
	//緯度分割１つ分の緯度Θd
	const float kLatEvery = std::numbers::pi_v<float> / (float)kSubdbivision;
	//緯度方向に分割しながら線を描く
	const float w = 2.0f;
	for (uint32_t latIndex = 0; latIndex < kSubdbivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;//θ
		for (uint32_t lonIndex = 0; lonIndex < kSubdbivision; ++lonIndex) {
			//テクスチャ用のTexcood

			//書き込む最初の場所
			uint32_t start = (latIndex * kSubdbivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;//∮
			//基準点a
			vertexData[start].position.x = std::cosf(lat) * std::cosf(lon);
			vertexData[start].position.y = std::sinf(lat);
			vertexData[start].position.z = std::cosf(lat) * std::sinf(lon);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex) / float(kSubdbivision), 1.0f - float(latIndex) / float(kSubdbivision) };
			//基準点b
			start++;
			vertexData[start].position.x = std::cosf(lat + kLatEvery) * std::cosf(lon);
			vertexData[start].position.y = std::sinf(lat + kLatEvery);
			vertexData[start].position.z = std::cosf(lat + kLatEvery) * std::sinf(lon);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex) / float(kSubdbivision), 1.0f - float(latIndex + 1.0f) / float(kSubdbivision) };
			//基準点c
			start++;
			vertexData[start].position.x = std::cosf(lat) * std::cosf(lon + kLonEvery);
			vertexData[start].position.y = std::sinf(lat);
			vertexData[start].position.z = std::cosf(lat) * std::sinf(lon + kLonEvery);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex + 1.0f) / float(kSubdbivision), 1.0f - float(latIndex) / float(kSubdbivision) };

			//基準点c
			start++;
			vertexData[start].position.x = std::cosf(lat) * std::cosf(lon + kLonEvery);
			vertexData[start].position.y = std::sinf(lat);
			vertexData[start].position.z = std::cosf(lat) * std::sinf(lon + kLonEvery);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex + 1.0f) / float(kSubdbivision), 1.0f - float(latIndex) / float(kSubdbivision) };

			//基準点b
			start++;
			vertexData[start].position.x = std::cosf(lat + kLatEvery) * std::cosf(lon);
			vertexData[start].position.y = std::sinf(lat + kLatEvery);
			vertexData[start].position.z = std::cosf(lat + kLatEvery) * std::sinf(lon);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex) / float(kSubdbivision), 1.0f - float(latIndex + 1.0f) / float(kSubdbivision) };

			//基準点d
			start++;
			vertexData[start].position.x = std::cosf(lat + kLatEvery) * std::cosf(lon + kLonEvery);
			vertexData[start].position.y = std::sinf(lat + kLatEvery);
			vertexData[start].position.z = std::cosf(lat + kLatEvery) * std::sinf(lon + kLonEvery);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex + 1) / float(kSubdbivision), 1.0f - float(latIndex + 1) / float(kSubdbivision) };
		}


	}

	////vetexResourceSprite頂点バッファーを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{ };
	//リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vetexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
	//1頂点当たりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);
	//頂点リソースにデータを書き込む
	VertexData* vertexDataSprite = nullptr;
	//書き込むためのアドレスを取得
	vetexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	//一個目
	vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };//左した
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };//左上
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };//右下
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };
	//二個目
	vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f };//右上
	vertexDataSprite[3].texcoord = { 1.0f,0.0f };
	vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };


	//IndexBufferSprite頂点バッファーを作成する
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	//リソース先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス６つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;
	//インデックスリソースにデータ書き込む
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0; indexDataSprite[1] = 1; indexDataSprite[2] = 2;
	indexDataSprite[3] = 1; indexDataSprite[4] = 3; indexDataSprite[5] = 2;


	
	;

	//ビューポート
	D3D12_VIEWPORT viewport{};
	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = kClientWindth;
	viewport.Height = kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	//シザー矩形
	D3D12_RECT scissorRect{};
	//基本的にビューポートと同じ矩形が構成さるようにする
	scissorRect.left = 0;
	scissorRect.right = kClientWindth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;

	//マテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = CreateBufferResource(device, sizeof(Material));
	//マテリアルにデータを書き込む	
	Material* materialDataSphere = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSphere));
	//色
	materialDataSphere->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	materialDataSphere->enableLighting = true;//有効にするか否か
	materialDataSphere->uvTransform = MekeIdentity4x4();

	//WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* wvpData = nullptr;
	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	//単位行列を書き込む
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();


	//modelマテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceModel = CreateBufferResource(device, sizeof(Material));
	//マテリアルにデータを書き込む	
	Material* materialDataModel = nullptr;
	materialResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&materialDataModel));
	//色
	materialDataModel->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	materialDataModel->enableLighting = true;//有効にするか否か
	materialDataModel->uvTransform = MekeIdentity4x4();

	//ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceModel = CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformaitionMatrixDataModel = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&transformaitionMatrixDataModel));
	//単位行列を書き込む
	transformaitionMatrixDataModel->WVP = MakeIdentity4x4();
	transformaitionMatrixDataModel->World = MakeIdentity4x4();



	//Teapotmodelマテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceTeapotModel = CreateBufferResource(device, sizeof(Material));
	//マテリアルにデータを書き込む	
	Material* materialDataTeapotModel = nullptr;
	materialResourceTeapotModel->Map(0, nullptr, reinterpret_cast<void**>(&materialDataTeapotModel));
	//色
	materialDataTeapotModel->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	materialDataTeapotModel->enableLighting = true;//有効にするか否か
	materialDataTeapotModel->uvTransform = MekeIdentity4x4();

	//TeapotModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceTeapotModel = CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformaitionMatrixDataTeapotModel = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceTeapotModel->Map(0, nullptr, reinterpret_cast<void**>(&transformaitionMatrixDataTeapotModel));
	//単位行列を書き込む
	transformaitionMatrixDataTeapotModel->WVP = MakeIdentity4x4();
	transformaitionMatrixDataTeapotModel->World = MakeIdentity4x4();



	//Sprite用のTransfomationMatrix用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformaitionMatrixDataSprite = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformaitionMatrixDataSprite));
	//単位行列を書き込んでおく
	transformaitionMatrixDataSprite->WVP = MakeIdentity4x4();
	transformaitionMatrixDataSprite->World = MakeIdentity4x4();

	//Sprite用のマテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = CreateBufferResource(device, sizeof(Material));
	//マテリアルにデータを書き込む	
	Material* materialDataSprite = nullptr;
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	//色
	materialDataSprite->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	materialDataSprite->enableLighting = false;
	materialDataSprite->uvTransform = MakeIdentity4x4();

	//平行光源用のResoureceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = CreateBufferResource(device, sizeof(DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,1.0f };
	directionalLightData->intensity = 1.0f;


	

#pragma endregion

#pragma region Texturを読む
	//Texturを読んで転送する
	DirectX::ScratchImage mipImages = LoadTexture("Resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = CreateTextureResource(device, metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResouce = UploadTextureData(textureResource, mipImages, device, commandList);

	//Textur2を読んで転送する
	DirectX::ScratchImage mipImages2 = LoadTexture("Resources/monsterBall.png");
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = CreateTextureResource(device, metadata2);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResouce2 = UploadTextureData(textureResource2, mipImages2, device, commandList);

	//Textur3を読んで転送する
	DirectX::ScratchImage mipImages3 = LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata3 = mipImages3.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource3 = CreateTextureResource(device, metadata3);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResouce3 = UploadTextureData(textureResource3, mipImages3, device, commandList);

	//Textur4を読んで転送する
	DirectX::ScratchImage mipImages4 = LoadTexture(modelDataTeapot.material.textureFilePath);
	const DirectX::TexMetadata& metadata4 = mipImages4.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource4 = CreateTextureResource(device, metadata4);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResouce4 = UploadTextureData(textureResource4, mipImages4, device, commandList);



#pragma endregion 

#pragma region ShaderResourceView
	//meraDaraを気にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{  };
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	//meraDara2を気にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{  };
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	//meraDara3を気にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc3{  };
	srvDesc3.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc3.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc3.Texture2D.MipLevels = UINT(metadata3.mipLevels);

	//meraDara4を気にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc4{  };
	srvDesc4.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc4.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc4.Texture2D.MipLevels = UINT(metadata4.mipLevels);

	//SRVを作成するDescriptHeap	の場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDesctiptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDesctiptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU3 = GetCPUDesctiptorHandle(srvDescriptorHeap, descriptorSizeSRV, 3);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU3 = GetGPUDesctiptorHandle(srvDescriptorHeap, descriptorSizeSRV, 3);


	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU4 = GetCPUDesctiptorHandle(srvDescriptorHeap, descriptorSizeSRV, 4);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU4 = GetGPUDesctiptorHandle(srvDescriptorHeap, descriptorSizeSRV, 4);
	//先頭はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの設定
	device->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);
	device->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);
	device->CreateShaderResourceView(textureResource3.Get(), &srvDesc3, textureSrvHandleCPU3);
	device->CreateShaderResourceView(textureResource4.Get(), &srvDesc4, textureSrvHandleCPU4);

#pragma endregion 


	//ウィンドウのｘボタンが押されるまでループ
	MSG msg{};
	ShowWindow(hwnd, SW_SHOW);

	//ImGui初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(device.Get(),
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap.Get(),
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());


	Input* input = nullptr;
	input = new Input();
	input->Initialize(wc.hInstance,hwnd);
	delete input;


	//wvpData用のTransform変数を作る
	Transform transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,3.0f} };
	//カメラ用のTransformを作る
	Transform cameraTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{ 0.0f,0.0f,-5.0f} };
	//sprite用のtransformSpriteを作る
	Transform transformSprite{ {0.5f,0.5f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	Transform uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	Transform transformModel = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{-2.5f,0.0f,5.0f} };
	Transform transformTeapotModel = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{2.5f,0.0f,5.0f} };

	bool useMonsterBall = true;
	while (msg.message != WM_QUIT) {



		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}
		else {

			//更新
			transform.rotate.y += 0.03f;

			Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrix = Inverse(cameraMatrix);
			Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kClientWindth) / float(kClientHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
			wvpData->WVP = worldViewProjectionMatrix;
			wvpData->World = worldMatrix;


			Matrix4x4 worldMatrixmodel = MakeAffineMatrix(transformModel.scale, transformModel.rotate, transformModel.translate);
			Matrix4x4 worldViewProjectionMatrixModel = Multiply(worldMatrixmodel, Multiply(viewMatrix, projectionMatrix));
			transformaitionMatrixDataModel->WVP = worldViewProjectionMatrixModel;
			transformaitionMatrixDataModel->World = worldMatrixmodel;

			Matrix4x4 worldMatrixTeapotmodel = MakeAffineMatrix(transformTeapotModel.scale, transformTeapotModel.rotate, transformTeapotModel.translate);
			Matrix4x4 worldViewProjectionMatrixTeapotModel = Multiply(worldMatrixTeapotmodel, Multiply(viewMatrix, projectionMatrix));
			transformaitionMatrixDataTeapotModel->WVP = worldViewProjectionMatrixTeapotModel;
			transformaitionMatrixDataTeapotModel->World = worldMatrixTeapotmodel;

			Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
			Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(kClientWindth), float(kClientHeight), 0.0f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
			transformaitionMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
			transformaitionMatrixDataSprite->World = worldMatrix;



			Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
			materialDataSprite->uvTransform = uvTransformMatrix;

			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGui::Begin("Setting");

			//CameraTransform
			if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat3("CameraTranslate", &cameraTransform.translate.x, 0.01f);
				ImGui::DragFloat3("CameraRotate", &cameraTransform.rotate.x, 0.01f);
			}
			ImGui::Checkbox("useMonsterBall", &useMonsterBall);
			// SphereSetColor
			if (ImGui::CollapsingHeader("SetcolorSphere", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::ColorEdit4("*SetColor", &materialDataSphere->color.x);
			}
			// SphereTransform
			if (ImGui::CollapsingHeader("Sphere", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Combo("SphereLighting", &materialDataSphere->enableLighting, "None\0Lambert\0HalfLambert");
				ImGui::DragFloat3("*Scale", &transform.scale.x, 0.01f);
				ImGui::DragFloat3("*Rotate", &transform.rotate.x, 0.01f);
				ImGui::DragFloat3("*Transrate", &transform.translate.x, 0.01f);
			}
			// ModelTransform
			if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Combo("ModelLighting", &materialDataModel->enableLighting, "None\0Lambert\0HalfLambert");
				ImGui::DragFloat3("*ModelScale", &transformModel.scale.x, 0.01f);
				ImGui::DragFloat3("*ModelRotate", &transformModel.rotate.x, 0.01f);
				ImGui::DragFloat3("*ModelTransrate", &transformModel.translate.x, 0.01f);
			}
			// TeapotModelTransform
			if (ImGui::CollapsingHeader("TeapotModel", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Combo("TeapotModelLighting", &materialDataTeapotModel->enableLighting, "None\0Lambert\0HalfLambert");
				ImGui::DragFloat3("*TeapotModelScale", &transformTeapotModel.scale.x, 0.01f);
				ImGui::DragFloat3("*TeapotModelRotate", &transformTeapotModel.rotate.x, 0.01f);
				ImGui::DragFloat3("*TeapotModelTransrate", &transformTeapotModel.translate.x, 0.01f);
			}
			//SpriteTransform
			if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Combo("SpriteLighting", &materialDataSprite->enableLighting, "None\0Lambert\0HalfLambert");
				ImGui::DragFloat3("*ScaleSprite", &transformSprite.scale.x, 0.1f);
				ImGui::DragFloat3("*RotateSprite", &transformSprite.rotate.y, 0.1f);
				ImGui::DragFloat3("*TransrateSprite", &transformSprite.translate.x);
			}
			//uvTransformSprite
			if (ImGui::CollapsingHeader("uvTransformSprite", ImGuiTreeNodeFlags_DefaultOpen))
			{
				
				ImGui::DragFloat2("*UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
				ImGui::DragFloat2("*UVScale", &uvTransformSprite.scale.x, 0.01f, -1.0f, 1.0f);
				ImGui::SliderAngle("*UVRotate", &uvTransformSprite.rotate.z);

			}

			//項目4
			if (ImGui::CollapsingHeader("directionalLight", ImGuiTreeNodeFlags_DefaultOpen))
			{
				
				ImGui::ColorEdit4("*LightSetColor", &directionalLightData->color.x);
				ImGui::DragFloat3("*Lightdirection", &directionalLightData->direction.x, 0.01f, -1.0f, 1.0f);
			}

			



			ImGui::End();
			ImGui::Render();

#pragma region CommandList
			//これから書き込むバックバッファのインデックスを取得する
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
			//TransitionBarrierの設定
			D3D12_RESOURCE_BARRIER barrier{};
			//今回のバリアはTransition
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			//Noneにしておく
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			//バリアを張る対象のリソース。現在のバックバッファに対して行う
			barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
			//遷移前（現在）のResourceState
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			//遷移後のResourceState
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			//Transition Barrierを張る
			commandList->ResourceBarrier(1, &barrier);
			//描画先のRVTを設定する
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
			//指定した色で画面全体をクリアする
			float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };//青っぽい色。RGBAの順
			commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

			//描画先のRTVとDSVを設定する
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			//描画用のDescript
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHepes[] = { srvDescriptorHeap };
			commandList->SetDescriptorHeaps(1, descriptorHepes->GetAddressOf());

			//コマンドリストの内容を確定させる。すべてのコマンドを積んでからCliseすること
			commandList->RSSetViewports(1, &viewport);
			commandList->RSSetScissorRects(1, &scissorRect);
			//RootSignatureを設定。POSに設定しているけどベット設定が必要
			commandList->SetGraphicsRootSignature(rootSignature.Get());
			commandList->SetPipelineState(graphicsPipelineState.Get());

			//Sphere
			commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
			//現状を設定。POSに設定しているものとはまた別。おなじ物を設定すると考えておけばいい
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
			//wvp用のCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
			commandList->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
			commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
			//描画！
			commandList->DrawInstanced(kSubdbivision * kSubdbivision * 6, 1, 0, 0);


			//sprite用の描画
			commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
			commandList->IASetIndexBuffer(&indexBufferViewSprite);
			commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
			//TransFomationMatrixBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
			commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
			//描画！
			//commandList->DrawInstanced(6, 1, 0, 0);
			commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);


			//model用
			commandList->IASetVertexBuffers(0, 1, &VertexBufferViewModel);
			//現状を設定。POSに設定しているものとはまた別。おなじ物を設定すると考えておけばいい
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->SetGraphicsRootConstantBufferView(0, materialResourceModel->GetGPUVirtualAddress());
			//wvp用のCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceModel->GetGPUVirtualAddress());
			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU3);
			commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
			//描画！
			commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);


			//Teapotmodel用
			commandList->IASetVertexBuffers(0, 1, &VertexBufferViewTeapotModel);
			//現状を設定。POSに設定しているものとはまた別。おなじ物を設定すると考えておけばいい
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->SetGraphicsRootConstantBufferView(0, materialResourceTeapotModel->GetGPUVirtualAddress());
			//wvp用のCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceTeapotModel->GetGPUVirtualAddress());
			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU4);
			commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
			//描画！
			commandList->DrawInstanced(UINT(modelDataTeapot.vertices.size()), 1, 0, 0);

			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());


			//画面に描く処理はすべて終わり、画面に映すので、状態遷移
			//今回はRenderTragetからPresentにする
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			//ToransitionBarrierを張る
			commandList->ResourceBarrier(1, &barrier);


			//コマンドリストの内容を確定させる。すべてのコマンドを積んでからCliseすること
			hr = commandList->Close();
			assert(SUCCEEDED(hr));

			//GPUにコマンドリストの実行を行わせる
			ID3D12CommandList* commandLists[] = { commandList.Get()};
			commandQueue->ExecuteCommandLists(1, commandLists);
			//GPUとOSに画面の交換を行う通知する
			swapChain->Present(1, 0);

			//Fenceの値の更新
			fenceValue++;
			//GPUがここまでたどりついた時に、Fenceの値を指定したあたいに代入するようにsignalを送る
			commandQueue->Signal(fence.Get(), fenceValue);

			//Femceの値が指定したSignal値にたどり着いているか確認する
			//GetCompletebValuの初期値はFence作成時に渡した初期値
			if (fence->GetCompletedValue() < fenceValue) {

				//指定したSignalにたどりついていないので、たどり着くまで待つようにイベントを設定する
				fence->SetEventOnCompletion(fenceValue, fenceEvent);
				//イベントを待つ
				WaitForSingleObject(fenceEvent, INFINITE);
			}

			//次のフレーム用のコマンドリストを準備
			hr = commandAllocator->Reset();
			assert(SUCCEEDED(hr));
			hr = commandList->Reset(commandAllocator.Get(), nullptr);
			assert(SUCCEEDED(hr));

		}
#pragma endregion


	}


#pragma region Release

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	
	CloseHandle(fenceEvent);
	
#ifdef _DEBUG
	
#endif // _DEBUG
#pragma endregion

	CoUninitialize();
	CloseWindow(hwnd);


	return 0;

}
