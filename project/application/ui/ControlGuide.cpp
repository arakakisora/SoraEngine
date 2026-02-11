#define NOMINMAX 

#include "ControlGuide.h"
#include <cassert>
#include <algorithm>
#include <memory> // std::unique_ptr, std::make_unique
#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG

// シングルトンの所有権を unique_ptr に移行。
// ヘッダで `static std::unique_ptr<ControlGuide> instance_;` を宣言している前提。
std::unique_ptr<ControlGuide> ControlGuide::instance_ = nullptr;

ControlGuide* ControlGuide::GetInstance() {
    if (!instance_) {
        
        instance_ = std::make_unique<ControlGuide>();
    }
    return instance_.get();
}
void ControlGuide::DestroyInstance() {
    instance_.reset();
}

// デバッグUI（ImGui）表示
void ControlGuide::DebugImGui() {
#ifdef _DEBUG

    if (!debugOpen_) return;

    if (ImGui::Begin("ControlGuide", &debugOpen_)) {
        ImGui::Checkbox("Visible", &visible_);
        ImGui::Checkbox("Clamp To Screen", &clampToScreen_);
        ImGui::Checkbox("Sprite Pivot Is Center", &spritePivotIsCenter_);

        ImGui::Separator();

        ImGui::DragFloat("Scale", &scale_, 0.01f, 0.30f, 2.00f);
        ImGui::DragFloat("Row Spacing", &rowSpacing_, 0.5f, 0.0f, 80.0f);
        ImGui::DragFloat("Margin X", &marginX_, 0.5f, 0.0f, 300.0f);
        ImGui::DragFloat("Margin Y", &marginY_, 0.5f, 0.0f, 300.0f);

        ImGui::Separator();

        ImGui::DragFloat2("Global Offset", &offset_.x, 0.5f);
        ImGui::DragFloat2("Left Offset", &leftOffset_.x, 0.5f);
        ImGui::DragFloat2("Right Offset", &rightOffset_.x, 0.5f);

        ImGui::Separator();

        // プリセットボタン（簡易レイアウト調整）
        if (ImGui::Button("Preset: Small & Safe")) {
            scale_ = 0.78f;
            rowSpacing_ = 10.0f;
            marginX_ = 24.0f;
            marginY_ = 24.0f;
            offset_ = { 0.0f, 0.0f };
            leftOffset_ = { 0.0f, 0.0f };
            rightOffset_ = { 0.0f, 0.0f };
            clampToScreen_ = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Open/Close Debug")) {
            debugOpen_ = !debugOpen_;
        }

        ImGui::Text("Entries: %d", (int)entries_.size());
        ImGui::Text("Tips: If left side is clipped, enable PivotIsCenter and/or increase MarginX.");
    }
    ImGui::End(); 
#endif // _DEBUG

}

// 初期化：SpriteCommon を使ってアイコンスプライトを作成する
void ControlGuide::Initialize(SpriteCommon* spriteCommon) {
    spriteCommon_ = spriteCommon;
    assert(spriteCommon_ && "SpriteCommon must be initialized before ControlGuide");

    // 表示するアイコン（ラベルは不要なので読み込まない）
    struct Setup { const char* icon; Vector2 iconSize; Entry::Side side; float localScale; };
    // 左下グループ： W/A/D/Space、右下グループ：マウスホイール/上下キー
    const Setup setups[] = {
        { "Resources/key_w.dds",    {64,64}, Entry::Side::Left },
        { "Resources/key_a.dds",    {64,64}, Entry::Side::Left },
        { "Resources/key_d.dds",    {64,64}, Entry::Side::Left },
        { "Resources/attackspace.dds",    {64,64}, Entry::Side::Left },

        { "Resources/mouse_wheel.dds", {64,64}, Entry::Side::Right },
        { "Resources/arrow_up.dds",    {64,64}, Entry::Side::Right },
        { "Resources/arrow_down.dds",  {64,64}, Entry::Side::Right },
    };

    // 既存エントリをクリアして再構築
    entries_.clear();

    for (auto& s : setups) {
        Entry e;
        e.iconFile  = s.icon;
        e.labelFile = ""; // ラベル不要
        e.size = s.iconSize;
        e.labelSize = {0,0};
        e.side = s.side;
        // 仮位置（UpdateLayout で上書き）
        e.position = { marginX_, marginY_ };

        // Sprite を unique_ptr で所有（自動破棄される）
        e.icon = std::make_unique<Sprite>();
        e.icon->Initialize(spriteCommon_, e.iconFile);
        // e.icon->SetSize(e.size); // 必要ならサイズを設定

        entries_.push_back(std::move(e));
    }

    visible_ = true; // デフォルト表示
}

// 終了処理：エントリをクリアしてリソース解放（unique_ptr により自動で解放される）
void ControlGuide::Finalize() {
    entries_.clear();
    spriteCommon_ = nullptr;
}

// 表示トグル
void ControlGuide::Toggle() {
    visible_ = !visible_;
}
void ControlGuide::SetVisible(bool v) { visible_ = v; }
bool ControlGuide::IsVisible() const { return visible_; }

// レイアウト計算：画面サイズと設定に基づき各エントリの位置を決定する
void ControlGuide::UpdateLayout() {
    if (entries_.empty()) return;

    const float screenW = static_cast<float>(WinApp::kClientWidth);
    const float screenH = static_cast<float>(WinApp::kClientHeight);

    // 左右グループに分割
    std::vector<Entry*> lefts;
    std::vector<Entry*> rights;
    for (auto& e : entries_) {
        (e.side == Entry::Side::Left ? lefts : rights).push_back(&e);
    }

    // グループの合計高さと最大幅を計算するラムダ
    auto computeGroupMetrics = [&](const std::vector<Entry*>& group) {
        float totalH = 0.0f;
        float maxW = 0.0f;
        for (size_t i = 0; i < group.size(); ++i) {
            const Entry* p = group[i];
            float h = p->size.y * scale_;
            totalH += h;
            if (i + 1 < group.size()) totalH += rowSpacing_;
            float labelW = p->label ? (p->labelSize.x * scale_) : 0.0f;
            float rowW = p->size.x * scale_ + (p->label ? (12.0f * scale_ + labelW) : 0.0f);
            maxW = std::max(maxW, rowW);
        }
        return std::pair<float, float>(totalH, maxW);
    };

    auto [leftH, leftW] = computeGroupMetrics(lefts);
    auto [rightH, rightW] = computeGroupMetrics(rights);

    // ---- 左下グループ配置
    float leftStartX = marginX_ + offset_.x + leftOffset_.x;
    float leftStartY = screenH - marginY_ - leftH + offset_.y + leftOffset_.y;

    float y = leftStartY;
    for (auto p : lefts) {
        Vector2 scaledSize = { p->size.x * scale_, p->size.y * scale_ };

        if (spritePivotIsCenter_) {
            p->position.x = leftStartX + scaledSize.x * 0.5f;
            p->position.y = y + scaledSize.y * 0.5f;
        } else {
            p->position.x = leftStartX;
            p->position.y = y;
        }

        // 画面外に出ないようにクランプ
        if (clampToScreen_) {
            float halfW = spritePivotIsCenter_ ? (scaledSize.x * 0.5f) : 0.0f;
            float halfH = spritePivotIsCenter_ ? (scaledSize.y * 0.5f) : 0.0f;
            p->position.x = std::clamp(p->position.x, 0.0f + halfW, screenW - halfW);
            p->position.y = std::clamp(p->position.y, 0.0f + halfH, screenH - halfH);
        }

        y += scaledSize.y + rowSpacing_;
    }

    // ---- 右下グループ配置
    float rightStartX = screenW - marginX_ - rightW + offset_.x + rightOffset_.x;
    float rightStartY = screenH - marginY_ - rightH + offset_.y + rightOffset_.y;

    y = rightStartY;
    for (auto p : rights) {
        Vector2 scaledSize = { p->size.x * scale_, p->size.y * scale_ };

        if (spritePivotIsCenter_) {
            p->position.x = rightStartX + scaledSize.x * 0.5f;
            p->position.y = y + scaledSize.y * 0.5f;
        } else {
            p->position.x = rightStartX;
            p->position.y = y;
        }

        if (clampToScreen_) {
            float halfW = spritePivotIsCenter_ ? (scaledSize.x * 0.5f) : 0.0f;
            float halfH = spritePivotIsCenter_ ? (scaledSize.y * 0.5f) : 0.0f;
            p->position.x = std::clamp(p->position.x, 0.0f + halfW, screenW - halfW);
            p->position.y = std::clamp(p->position.y, 0.0f + halfH, screenH - halfH);
        }

        y += scaledSize.y + rowSpacing_;
    }
}

// 描画：各エントリのスプライトを適切な位置・サイズで描画する
void ControlGuide::Render() {
    if (!visible_) return;
    if (!spriteCommon_) return;

    // 解像度変更や設定変更に対応してレイアウトを更新
    UpdateLayout();

    // 各エントリを描画（ラベルは今は描画しない）
    for (auto& e : entries_) {
        Vector2 texSize = e.icon->GetTextureSize(); // 元テクスチャ解像度
        float targetH = e.size.y * scale_;
        float aspect = texSize.x / texSize.y;

        Vector2 iconSizeScaled = {
            targetH * aspect,
            targetH
        };

        // スプライトに対する設定と描画
        e.icon->SetPosition(e.position);
        e.icon->SetSize(iconSizeScaled);
        e.icon->Update();
        e.icon->Draw();
    }
}