

#include <string>
#include <format>

#include <d3d12.h>
#include <dxgi1_6.h>

#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include"MyMath.h"
#include "RenderingPipeline.h"




#include <numbers>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <wrl.h>
#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
























#pragma region MaterialData
MaterialData LoadMaterialTemplateFile(const std::string& directorypath, const std::string& filename) {

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
			materialData.textureFilePath = directorypath + "/" + textureFilename;

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


// windowアプリでのエントリ―ポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;
#pragma region Window
	CoInitializeEx(0, COINIT_MULTITHREADED);
	//出力ウィンドウへの文字出力
	OutputDebugStringA("HEllo,DIrectX!\n");

	//ポインタ
	WinApp* winApp = nullptr;
	DirectXCommon* dxCommon = nullptr;
	Input* input = nullptr;

	//初期化
	//WindousAPI初期化
	winApp = new WinApp;
	winApp->Initialize();
	//DX初期化
	dxCommon = new DirectXCommon;
	dxCommon->Initialize(winApp);
	//入力初期化
	input = new Input();
	input->Initialize(winApp);



#pragma endregion 






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
	ModelData modelData = LoadObjeFile("Resources", "axis.obj");

	//VertexResourceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = CreateBufferResource(device, sizeof(VertexData) * kSubdbivision * kSubdbivision * 6);
	//DepthStencilTextureをウィンドウサイズで作成

	//Sprite用の頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vetexResourceSprite = CreateBufferResource(device, sizeof(VertexData) * 4);
	//Sprite用のindexResourceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = CreateBufferResource(device, sizeof(uint32_t) * 6);
	//モデル用のVetexResouceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceModel = CreateBufferResource(device, sizeof(VertexData) * modelData.vertices.size());





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
	std::memcpy(vertexDataModel, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());



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
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon-> CreateTextureResource(device, metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResouce = dxCommon->UploadTextureData(textureResource, mipImages);

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

	//SRVを作成するDescriptHeap	の場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU =  dxCommon->GetSRVCPUDescriputorHandole(1);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU =  dxCommon->GetSRVGPUDescriputorHandole(1);

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = dxCommon->GetSRVCPUDescriputorHandole(2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = dxCommon->GetSRVGPUDescriputorHandole(2);

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU3 = dxCommon->GetSRVCPUDescriputorHandole(3);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU3 = dxCommon->GetSRVGPUDescriputorHandole(3);
	//先頭はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの設定
	device->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);
	device->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);
	device->CreateShaderResourceView(textureResource3.Get(), &srvDesc3, textureSrvHandleCPU3);

#pragma endregion 


	//ウィンドウのｘボタンが押されるまでループ









	//wvpData用のTransform変数を作る
	Transform transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	//カメラ用のTransformを作る
	Transform cameraTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{ 0.0f,0.0f,-5.0f} };
	//sprite用のtransformSpriteを作る
	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	Transform uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	Transform transformModel = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	bool useMonsterBall = true;

	while (true) {//ゲームループ

		//Windowsのメッセージ処理
		if (winApp->ProcessMessage()) {
			//ゲームループを抜ける
			break;
		}

		input->Update();

		//更新
		if (input->TriggerKey(DIK_0)) {

			transform.rotate.y += 0.3f;

		}

		Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWindth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		wvpData->WVP = worldViewProjectionMatrix;
		wvpData->World = worldMatrix;

		Matrix4x4 worldMatrixmodel = MakeAffineMatrix(transformModel.scale, transformModel.rotate, transformModel.translate);
		Matrix4x4 worldViewProjectionMatrixModel = Multiply(worldMatrixmodel, Multiply(viewMatrix, projectionMatrix));
		transformaitionMatrixDataModel->WVP = worldViewProjectionMatrixModel;
		transformaitionMatrixDataModel->World = worldMatrixmodel;

		Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
		Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWindth), float(WinApp::kClientHeight), 0.0f, 100.0f);
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
			ImGui::DragFloat3("*Scale", &transform.scale.x, 0.01f);
			ImGui::DragFloat3("*Rotate", &transform.rotate.x, 0.01f);
			ImGui::DragFloat3("*Transrate", &transform.translate.x, 0.01f);
		}
		// ModelTransform
		if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::DragFloat3("*ModelScale", &transformModel.scale.x, 0.01f);
			ImGui::DragFloat3("*ModelRotate", &transformModel.rotate.x, 0.01f);
			ImGui::DragFloat3("*ModelTransrate", &transformModel.translate.x, 0.01f);
		}
		//SpriteTransform
		if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
		{
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

		dxCommon->Begin();







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
		//commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);


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

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());






		dxCommon->End();





#pragma endregion


	}


#pragma region Release

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	/*CloseHandle(fenceEvent);*/



#ifdef _DEBUG

#endif // _DEBUG
#pragma endregion

	//WindowsAPI終了処理
	winApp->Finalize();
	//WindowsAPI解放
	delete winApp;
	delete dxCommon;
	delete input;

	return 0;

}
