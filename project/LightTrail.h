#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "GraphicsPipeline.h"
#include <Vector2.h>
#include <wrl.h>

struct TrailParamsCPU {
    float t;          // 0→1 進行
    float direction;  // +1 L→R, -1 R→L
    float width;
    float softness;
    float headRound;
    float glow;
    float trailLen;
    Vector2 vignette;
    float _pad[2];
};

static inline UINT Align256(UINT s) { return (s + 255u) & ~255u; }

class LightTrail {
public:
    void Initialize(DirectXCommon* dx, SrvManager* srv, GraphicsPipeline* gp,
        uint32_t width, uint32_t height);

    // 画面サイズ変更時に呼ぶ（必要なければ未使用でOK）
    void Resize(uint32_t width, uint32_t height);

    // エフェクト開始
    void BeginTransition(float durationSec, int dir);

    // 時間更新（毎フレーム）
    void Update(float dt);

    bool IsActive() const { return active_; }

    // Ofscreen の SRV インデックス（currentSrvIndex）と、そのテクスチャ本体（currentResource）を渡す
    // ※ currentSrvIndex は Ofscreen の既存SRV。currentResource は CopyResource 用。
    void Render(uint32_t currentSrvIndex, ID3D12Resource* currentResource);

private:
    // prev_ を作る
    void CreatePrevTexture(uint32_t width, uint32_t height);
    // リソース状態遷移ヘルパ
    void Transition(ID3D12Resource* res, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

private:
    DirectXCommon* dx_ = nullptr;
    SrvManager* srv_ = nullptr;
    GraphicsPipeline* gp_ = nullptr;

    // 前フレームを保持する専用テクスチャ（SRGB）
    Microsoft::WRL::ComPtr<ID3D12Resource> prev_;
    D3D12_RESOURCE_STATES prevState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    // SrvManagerのヒープ上に確保する "2連番"
    // baseSrv_   -> t0 = prev_
    // baseSrv_+1 -> t1 = current（毎フレームその場でSRVを作り直す）
    uint32_t baseSrv_ = 0;
    bool baseAllocated_ = false;

    // 定数バッファ（永続Map）
    Microsoft::WRL::ComPtr<ID3D12Resource> cb_;
    TrailParamsCPU* mapped_ = nullptr;

    // 進行管理
    bool  active_ = false;
    float t_ = 0.f;
    float dur_ = 0.55f;

    // 直近の prev_ サイズ
    uint32_t width_ = 0, height_ = 0;
};
