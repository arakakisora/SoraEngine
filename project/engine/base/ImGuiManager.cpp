#include "ImGuiManager.h"
#ifdef USE_IMGUI
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#endif // USE_IMGUI

void ImGuiManager::Initialize(DirectXCommon* dxCommon, WinApp* winapp)
{
#ifdef _DEBUG
    dxCommon_ = dxCommon;
    winapp_ = winapp;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    bool win32Ok = ImGui_ImplWin32_Init(winapp_->GetHwnd());
    assert(win32Ok);

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 1;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    HRESULT hr = dxCommon_->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvHeap_));
    assert(SUCCEEDED(hr));

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = dxCommon_->GetDevice();
    init_info.CommandQueue = dxCommon_->GetCommandQueue();
    init_info.NumFramesInFlight = static_cast<int>(dxCommon_->GetBackBufferCount());
    init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
    init_info.SrvDescriptorHeap = srvHeap_.Get();

    // 互換用の単一SRV指定
    init_info.LegacySingleSrvCpuDescriptor = srvHeap_->GetCPUDescriptorHandleForHeapStart();
    init_info.LegacySingleSrvGpuDescriptor = srvHeap_->GetGPUDescriptorHandleForHeapStart();

    bool dx12Ok = ImGui_ImplDX12_Init(&init_info);
    assert(dx12Ok);
#endif // _DEBUG


}

void ImGuiManager::Finalize()
{
#ifdef _DEBUG

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	srvHeap_.Reset();
#endif // _DEBUG


}

void ImGuiManager::Begin()
{
#ifdef _DEBUG

	ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground;

    // 中央ノード透過時は親ウィンドウも背景なし扱い前提
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("MainDockSpaceWindow", nullptr, window_flags);

    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

#endif // _DEBUG



}

void ImGuiManager::End()
{
#ifdef _DEBUG
    ImGui::End();
	ImGui::Render();

#endif // _DEBUG


}

void ImGuiManager::Draw()
{
#ifdef _DEBUG

	ID3D12GraphicsCommandList* commansList = dxCommon_->GetCommandList();

	//デスクリプタヒープの配列をセットする
	ID3D12DescriptorHeap* ppHeaps[] = { srvHeap_.Get() };
	commansList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	//描画コマンドを発行
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commansList);
#endif // _DEBUG


}
