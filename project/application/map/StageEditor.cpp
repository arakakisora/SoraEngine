#include "StageEditor.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif // USE_IMGUI
#include <fstream>
#include <iostream>
#include <filesystem>
#include "MapChipDatabase.h"

namespace fs = std::filesystem;

void StageEditor::Run() {
    // 初期化
    if (grid_.empty()) {
		// グリッドの初期化
        constexpr int kGridWidth = 100;
        constexpr int kGridHeight = 25;
		// 2Dグリッドを指定サイズで初期化
        grid_.resize(kGridHeight, std::vector<GridCell>(kGridWidth));
    }
    RenderUI();
}

void StageEditor::RenderUI() {
#ifdef USE_IMGUI

    if (fs::exists("Resources/Mapdata") && fs::is_directory("Resources/Mapdata")) {
        std::vector<std::string> newList;
        for (const auto& entry : fs::directory_iterator("Resources/Mapdata")) {
            if (entry.path().extension() == ".csv") {
                newList.push_back(entry.path().filename().string());
            }
        }
        std::sort(newList.begin(), newList.end());
        if (newList != availableStages) {
            availableStages = std::move(newList);
            if (!availableStages.empty()) {
                if (selectedStageIndex >= availableStages.size()) selectedStageIndex = 0;
                strncpy_s(fileNameBuffer, sizeof(fileNameBuffer), availableStages[selectedStageIndex].c_str(), _TRUNCATE);
                fileNameBuffer[sizeof(fileNameBuffer) - 1] = '\0';
            } else {
                selectedStageIndex = 0;
                fileNameBuffer[0] = '\0';
            }
        }
    }

    ImGui::Begin("Stage Editor");
    ImGuiIO& io = ImGui::GetIO();

    // 新規作成
    ImGui::InputText("FileName", fileNameBuffer, IM_ARRAYSIZE(fileNameBuffer));
    if (ImGui::Button("New")) {
        for (auto& row : grid_) {
            for (auto& cell : row) {
                cell.type = 0; // 空白に初期化
            }
        }
    }

    // セーブ
    if (ImGui::Button("Save CSV")) {
		// ファイルパスの生成
        std::string fullPath = "Resources/Mapdata/" + std::string(fileNameBuffer);
        SaveCSV(fullPath);
    }

    // ステージリスト（ドロップダウン） — 常に表示。空なら "No Stages"
    const char* comboPreview = availableStages.empty() ? "No Stages" : availableStages[selectedStageIndex].c_str();
    if (ImGui::BeginCombo("Stage List", comboPreview)) {
        if (availableStages.empty()) {
            ImGui::Selectable("No stages", false, ImGuiSelectableFlags_Disabled);
        } else {
            for (int i = 0; i < static_cast<int>(availableStages.size()); ++i) {
                const bool isSelected = (i == selectedStageIndex);
                if (ImGui::Selectable(availableStages[i].c_str(), isSelected)) {
                    selectedStageIndex = i;
                    strncpy_s(fileNameBuffer, sizeof(fileNameBuffer), availableStages[i].c_str(), _TRUNCATE);
                    fileNameBuffer[sizeof(fileNameBuffer) - 1] = '\0'; // null終端
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
        }
        ImGui::EndCombo();
    }

    // ロード（選択中のファイル）
    if (ImGui::Button("Load Selected Stage")) {
        if (!availableStages.empty()) {
			// 選択中のステージをロード
            std::string selectedPath = "Resources/Mapdata/" + availableStages[selectedStageIndex];
            LoadCSV(selectedPath);
        }
    }

    if (ImGui::Button("Reload Map")) {
        isReloadRequested_ = true;
    }

    for (const auto& info : MapChipDatabase::GetInstance()->GetAll()) {
        ImGui::RadioButton(info.label.c_str(), &selectedType_, info.id);
    }

    ImGui::End();

    ImGui::Begin("Stage");
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    // マップ描画エリア
    // マップ描画エリア
    for (int y = 0; y < static_cast<int>(grid_.size()); ++y) {
        for (int x = 0; x < static_cast<int>(grid_[y].size()); ++x) {

            // セルのタイプ（＝ JSON で定義した id）
            int typeId = grid_[y][x].type;

            // DB から色を取得
            const MapChipInfo* info = MapChipDatabase::GetInstance()->GetById(typeId);

            ImVec4 color(1, 1, 1, 1);
            if (info) {
                color = ImVec4(
                    info->color.x,
                    info->color.y,
                    info->color.z,
                    info->color.w
                );
            }

            constexpr float kCellSize = 12.0f;
            // 普通のクリックでも反応させる（既存動作）
            if (ImGui::ColorButton(
                ("##" + std::to_string(x) + "_" + std::to_string(y)).c_str(),
                color, 0, ImVec2(kCellSize, kCellSize))) {
                grid_[y][x].type = selectedType_;
            }

            // ここから先のドラッグ塗り／消しゴム／スポイトは今のままでOK
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem |
                ImGuiHoveredFlags_RectOnly)) {
                if (io.MouseDown[ImGuiMouseButton_Left]) {
                    grid_[y][x].type = selectedType_;     // 左ドラッグで塗る
                }
                if (io.MouseDown[ImGuiMouseButton_Right]) {
                    grid_[y][x].type = 0;                 // 右ドラッグで消しゴム（Empty=0）
                }
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
                    selectedType_ = grid_[y][x].type;     // 中クリックでスポイト
                }
            }

            ImGui::SameLine();
        }
        ImGui::NewLine();
    }

    ImGui::PopStyleVar();
    ImGui::End();

#endif // USE_IMGUI
}

void StageEditor::SaveCSV(const std::string& filename) {
    fs::create_directories("Resources/Mapdata"); // フォルダがなければ作成
    std::ofstream file(filename);
    for (const auto& row : grid_) {
        for (size_t x = 0; x < row.size(); ++x) {
            file << row[x].type;
            if (x + 1 < row.size()) file << ",";
        }
        file << "\n";
    }
}

void StageEditor::LoadCSV(const std::string& filename) {
	// ファイルを開く
    std::ifstream file(filename);
    std::string line;
    int y = 0;
	// 1行ずつ読み込み
    //ホットリロード
    while (std::getline(file, line)) {
        int x = 0;
        size_t start = 0;
        while (start < line.size()) {
            size_t end = line.find(',', start);
            if (end == std::string::npos) end = line.size();
            int val = std::stoi(line.substr(start, end - start));
            if (y < grid_.size() && x < grid_[y].size()) {
                grid_[y][x].type = val;
            }
            start = end + 1;
            ++x;
        }
        ++y;
    }
}
