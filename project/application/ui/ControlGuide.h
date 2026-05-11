#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Sprite.h"
#include "SpriteCommon.h"
#include "Vector2.h"
#include "Input.h"
#include "WinApp.h"

class ControlGuide {
public:
    static ControlGuide* GetInstance();
    static void DestroyInstance();

    // SpriteCommon は既に初期化済みであること
    void Initialize(SpriteCommon* spriteCommon);
    void Finalize();

    // 毎フレーム呼ぶ（レンダーパス内）
    void Render();

    void Toggle();
    void SetVisible(bool v);
    bool IsVisible() const;

    // 配置関連 API
    enum class Anchor {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Center
    };

    void DebugImGui();
    void SetAnchor(Anchor a) { anchor_ = a; }
    void SetOffset(const Vector2& ofs) { offset_ = ofs; }
    void SetScale(float s) { scale_ = s > 0.0f ? s : 1.0f; }
    void SetRowSpacing(float spacing) { rowSpacing_ = spacing; }
    void SetMargins(float marginX, float marginY) { marginX_ = marginX; marginY_ = marginY; }

private:
    enum class StackDirection {
        Vertical,
        Horizontal
    };
 
    struct Entry {
        std::unique_ptr<Sprite> icon;
        std::unique_ptr<Sprite> label; // ラベル画像（任意）
        std::string iconFile;
        std::string labelFile;
        Vector2 size;      // アイコンサイズ（ピクセル）
        Vector2 labelSize; // ラベル画像サイズ（ピクセル）
        Vector2 position;  // 画面内の位置（ピクセル） - レイアウトで上書きされる

        enum class Side {
            Left,
            Right
        } side = Side::Left;
    };

    void UpdateLayout(); // レイアウト計算

private:
   
    StackDirection leftStackDirection_ = StackDirection::Vertical;
    StackDirection rightStackDirection_ = StackDirection::Vertical;

    std::vector<Entry> entries_;
    bool visible_ = true;
    SpriteCommon* spriteCommon_ = nullptr;

    static std::unique_ptr<ControlGuide> instance_; 

    // レイアウト設定
    Anchor anchor_ = Anchor::TopLeft;
    float marginX_ = 87.0f;
    float marginY_ = 8.0f;
    float rowSpacing_ = 11.5f;
    float scale_ = 0.66f;

    Vector2 offset_{ -162.5f, -13.5f };
    Vector2 leftOffset_{ -6.5f,  -7.0f };
    Vector2 rightOffset_{ 0.0f, -10.5f };

    // はみ出し防止
    bool clampToScreen_ = true;

    // デバッグUI表示トグル
    bool debugOpen_ = true;

    // Sprite座標基準（中心/左上）差分吸収
    bool spritePivotIsCenter_ = true;
};