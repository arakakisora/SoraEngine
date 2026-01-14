#define NOMINMAX 

#include "ControlGuide.h"
#include <cassert>
#include <algorithm>
#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG

ControlGuide* ControlGuide::instance_ = nullptr;

ControlGuide* ControlGuide::GetInstance() {
    if (!instance_) instance_ = new ControlGuide();
    return instance_;
}
void ControlGuide::DestroyInstance() {
    delete instance_;
    instance_ = nullptr;
}

ControlGuide::ControlGuide() = default;
ControlGuide::~ControlGuide() = default;

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

        // 便利ボタン
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

void ControlGuide::Initialize(SpriteCommon* spriteCommon) {
    spriteCommon_ = spriteCommon;
    assert(spriteCommon_ && "SpriteCommon must be initialized before ControlGuide");

    // 表示するアイコン（ラベルは不要なので読み込まない）
    struct Setup { const char* icon; Vector2 iconSize; Entry::Side side; float localScale; };
    // 指定: W/A/D と Space を左下、マウスホイール と 上下キー を右下に配置
    const Setup setups[] = {
        // Left bottom group: W (ジャンプ), A (左), D (右), Space (攻撃)
        { "Resources/key_w.jpg",    {64,64}, Entry::Side::Left },
        { "Resources/key_a.jpg",    {64,64}, Entry::Side::Left },
        { "Resources/key_d.jpg",    {64,64}, Entry::Side::Left },
        { "Resources/space.jpg",    {64,64}, Entry::Side::Left },

        // Right bottom group: mouse wheel (エイム上下)、Up/Down keys (エイム)
        { "Resources/mouse_wheel.jpg", {64,64}, Entry::Side::Right },
        { "Resources/arrow_up.jpg",    {64,64}, Entry::Side::Right },
        { "Resources/arrow_down.jpg",  {64,64}, Entry::Side::Right },
    };

    entries_.clear();

    for (auto& s : setups) {
        Entry e;
        e.iconFile  = s.icon;
        e.labelFile = ""; // ラベル不要
        e.size = s.iconSize;
        e.labelSize = {0,0};
        e.side = s.side;
        // 仮位置（UpdateLayoutで上書き）
        e.position = { marginX_, marginY_ };

        // アイコンスプライト作成（ラベルは作らない）
        e.icon = std::make_unique<Sprite>();
        e.icon->Initialize(spriteCommon_, e.iconFile);
        //e.icon->SetSize(e.size);

        entries_.push_back(std::move(e));
    }

    visible_ = true; // リリースではデフォルトで表示にしておく
}

void ControlGuide::Finalize() {
    entries_.clear();
    spriteCommon_ = nullptr;
}

void ControlGuide::Toggle() {
    visible_ = !visible_;
}
void ControlGuide::SetVisible(bool v) { visible_ = v; }
bool ControlGuide::IsVisible() const { return visible_; }

void ControlGuide::UpdateLayout() {
    if (entries_.empty()) return;

    const float screenW = static_cast<float>(WinApp::kClientWindth);
    const float screenH = static_cast<float>(WinApp::kClientHeight);

    std::vector<Entry*> lefts;
    std::vector<Entry*> rights;
    for (auto& e : entries_) {
        (e.side == Entry::Side::Left ? lefts : rights).push_back(&e);
    }

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

    // ---- Left bottom
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

        if (clampToScreen_) {
            float halfW = spritePivotIsCenter_ ? (scaledSize.x * 0.5f) : 0.0f;
            float halfH = spritePivotIsCenter_ ? (scaledSize.y * 0.5f) : 0.0f;
            p->position.x = std::clamp(p->position.x, 0.0f + halfW, screenW - halfW);
            p->position.y = std::clamp(p->position.y, 0.0f + halfH, screenH - halfH);
        }

        y += scaledSize.y + rowSpacing_;
    }

    // ---- Right bottom
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

void ControlGuide::Render() {
    if (!visible_) return;
    if (!spriteCommon_) return;

    // レイアウトを更新（解像度変更や設定変更に対応）
    UpdateLayout();

    // 各エントリを描画（ラベルは描画しない）
    for (auto& e : entries_) {
        Vector2 texSize = e.icon->GetTextureSize(); // 元解像度
        float targetH = e.size.y * scale_;
        float aspect = texSize.x / texSize.y;

        Vector2 iconSizeScaled = {
            targetH * aspect,
            targetH
        };

        e.icon->SetPosition(e.position);
        e.icon->SetSize(iconSizeScaled);
        e.icon->Update();
        e.icon->Draw();
    }
}