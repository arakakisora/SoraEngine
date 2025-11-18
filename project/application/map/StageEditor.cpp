#include "StageEditor.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif // USE_IMGUI
#include <fstream>
#include <iostream>
#include <filesystem>

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

    //// ステージ一覧の手動更新（オプション）
    //if (ImGui::Button("Reload Stage List")) {
    //    // 手動でも即時更新する（自動で毎フレーム更新するため基本的には不要）
    //    availableStages.clear();
    //    if (fs::exists("Resources/Mapdata") && fs::is_directory("Resources/Mapdata")) {
    //        for (const auto& entry : fs::directory_iterator("Resources/Mapdata")) {
    //            if (entry.path().extension() == ".csv") {
    //                availableStages.push_back(entry.path().filename().string());
    //            }
    //        }
    //        std::sort(availableStages.begin(), availableStages.end());
    //        if (!availableStages.empty()) {
    //            if (selectedStageIndex >= availableStages.size()) selectedStageIndex = 0;
    //            strncpy_s(fileNameBuffer, sizeof(fileNameBuffer), availableStages[selectedStageIndex].c_str(), _TRUNCATE);
    //            fileNameBuffer[sizeof(fileNameBuffer) - 1] = '\0';
    //        } else {
    //            selectedStageIndex = 0;
    //            fileNameBuffer[0] = '\0';
    //        }
    //    }
    //}

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

    // 選択タイル
    ImGui::RadioButton("Empty", &selectedType_, 0); // 空白  
    ImGui::RadioButton("Block", &selectedType_, 1); // ブロック
    ImGui::RadioButton("Enemy", &selectedType_, 2); // 敵
    ImGui::RadioButton("Player", &selectedType_, 3);// プレイヤー
	ImGui::RadioButton("Goal", &selectedType_, 4);  // ゴール

    ImGui::End();

    ImGui::Begin("Stage");
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    // マップ描画エリア
    for (int y = 0; y < grid_.size(); ++y) {
        for (int x = 0; x < grid_[y].size(); ++x) {
            ImVec4 color;
            switch (grid_[y][x].type) {
            case 0: color = ImVec4(0, 0, 0, 1); break;
            case 1: color = ImVec4(0, 1, 0, 1); break;
            case 2: color = ImVec4(1, 0, 0, 1); break;
            case 3: color = ImVec4(0, 0, 1, 1); break;
            default: color = ImVec4(1, 1, 1, 1); break;
            }
            constexpr float kCellSize = 12.0f; 
            // 普通のクリックでも反応させる（既存動作）
            if (ImGui::ColorButton(("##" + std::to_string(x) + "_" + std::to_string(y)).c_str(),
                color, 0, ImVec2(kCellSize, kCellSize))) {
                grid_[y][x].type = selectedType_;
            }


            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem |
                ImGuiHoveredFlags_RectOnly)) {
                // 左ドラッグで現在選択タイルを塗る
                if (io.MouseDown[ImGuiMouseButton_Left]) {
                    grid_[y][x].type = selectedType_;
                }
                // 右ドラッグで消しゴム（Empty=0）
                if (io.MouseDown[ImGuiMouseButton_Right]) {
                    grid_[y][x].type = 0;
                }
                // 中クリックでスポイト（そのセルの種類を選択状態に）
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
                    selectedType_ = grid_[y][x].type;
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
