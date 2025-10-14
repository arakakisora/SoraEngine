#define NOMINMAX 
#include "LightTrail.h"
#include <algorithm>
#include <cassert>
#include "MyMath.h"


using Microsoft::WRL::ComPtr;

static const DXGI_FORMAT kTrailFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

void LightTrail::Initialize(DirectXCommon* dx, SrvManager* srv, GraphicsPipeline* gp,
    uint32_t width, uint32_t height)
{
    dx_ = dx;
    srv_ = srv;
    gp_ = gp;

    // prev_ の初期化
    CreatePrevTexture(width, height);

    // SrvManager のヒープ上に "2連番" を確保（t0=prev / t1=current）
    if (!baseAllocated_) {
        baseSrv_ = srv_->Allocate();
        // 2連番確保したいので連続でもう1個
        (void)srv_->Allocate();
        baseAllocated_ = true;
    }

    // t0 = prev の SRV を作成（初期は真っ黒）
    DirectX::TexMetadata empty{};
    srv_->CreateSRVforTexture2D(baseSrv_, prev_.Get(), kTrailFormat, 1, empty);

    // CB（永続Map）
    const UINT cbSize = Align256(sizeof(TrailParamsCPU));
    cb_ = dx_->CreateBufferResource(cbSize);
    cb_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
    *mapped_ = { 0.f, +1.f, 0.22f, 0.03f, 0.8f, 0.7f, 0.45f, {0.2f, 0.6f}, {0,0} };
}

void LightTrail::Resize(uint32_t width, uint32_t height)
{
    if (width == width_ && height == height_) return;
    CreatePrevTexture(width, height);

    // t0(SRV) を作り直し
    DirectX::TexMetadata empty{};
    srv_->CreateSRVforTexture2D(baseSrv_, prev_.Get(), kTrailFormat, 1, empty);
}

void LightTrail::BeginTransition(float durationSec, int dir)
{
    active_ = true;
    t_ = 0.f;
    dur_ = std::max(0.05f, durationSec);
    if (mapped_) mapped_->direction = static_cast<float>(dir);
}

void LightTrail::Update(float dt)
{
    if (!active_) return;

    t_ = std::min(t_ + dt, dur_);
    float u = (dur_ > 0.f) ? (t_ / dur_) : 1.f;
    // smoothstep
    u = u * u * (3.f - 2.f * u);

    mapped_->t = u;

    if (t_ >= dur_) active_ = false;
}

void LightTrail::Render(uint32_t currentSrvIndex, ID3D12Resource* currentResource)
{
    // currentResource は必須（CopyResourceとSRV作成に使う）
    if (!currentResource) return;

    auto* cmd = dx_->GetCommandList();

    // ---- t1 = current の SRV を「自前の2連番の2個目」に作成し直す ----
    // ※ current は Ofscreen の同一リソースなので、SRV自体は毎フレーム作り直してOK（軽い）
    DirectX::TexMetadata empty{};
    srv_->CreateSRVforTexture2D(baseSrv_ + 1, currentResource, kTrailFormat, 1, empty);

    // ---- PSO / ルートシグネチャ ----
    cmd->SetPipelineState(gp_->GetGraphicsPipelineStateTrail());
    cmd->SetGraphicsRootSignature(gp_->GetRootSignatureTrail());
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // b0 : CBV（Root-Descriptor）
    cmd->SetGraphicsRootConstantBufferView(0, cb_->GetGPUVirtualAddress());

    // t0..t1 : 自前の2連番（baseSrv_）をテーブルとしてセット
    srv_->SetGraficsRootDescriptorTable(1, baseSrv_);

    // フルスクリーントライアングル
    cmd->DrawInstanced(3, 1, 0, 0);

    // ---- 次フレーム用に prev_ を更新（current を prev_ にコピー）----
    // 1) prev_ を COPY_DEST に、current を COPY_SOURCE に
    Transition(prev_.Get(), prevState_, D3D12_RESOURCE_STATE_COPY_DEST);
    // current は Ofscreen 側から PIXEL_SHADER_RESOURCE 状態で呼ばれる想定なので COPY_SOURCE へ
    // ※ 実際の直前状態が異なる可能性もあるが、End() で SRV 状態になっている前提の構成に合わせる
    Transition(currentResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE);

    // 2) 実コピー
    cmd->CopyResource(prev_.Get(), currentResource);

    // 3) 戻す（prev_ を SRV、current も SRV に）
    Transition(prev_.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    Transition(currentResource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    prevState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    // t0(SRV) は prev_ を常に指すので、SRV自体は作り直さなくてOK
}

void LightTrail::CreatePrevTexture(uint32_t width, uint32_t height)
{
    width_ = width; height_ = height;

    // クリア値
    D3D12_CLEAR_VALUE clear{};
    clear.Format = kTrailFormat;
    clear.Color[0] = 0.f; clear.Color[1] = 0.f; clear.Color[2] = 0.f; clear.Color[3] = 0.f;

    // desc
    D3D12_RESOURCE_DESC desc{};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = kTrailFormat;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE; // SRVで読むだけ

    D3D12_HEAP_PROPERTIES heap{};
    heap.Type = D3D12_HEAP_TYPE_DEFAULT;

    prev_.Reset();
    HRESULT hr = dx_->GetDevice()->CreateCommittedResource(
        &heap, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // 初期はSRV状態
        &clear, IID_PPV_ARGS(&prev_));
    assert(SUCCEEDED(hr));

    prevState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}

void LightTrail::Transition(ID3D12Resource* res, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    if (before == after) return;
    D3D12_RESOURCE_BARRIER b{};
    b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    b.Transition.pResource = res;
    b.Transition.StateBefore = before;
    b.Transition.StateAfter = after;
    b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    dx_->GetCommandList()->ResourceBarrier(1, &b);
}
