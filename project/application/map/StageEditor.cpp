#include "StageEditor.h"
#include "imgui.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

void StageEditor::Run() {
    // 初期化
    if (grid_.empty()) {
        constexpr int kGridWidth = 100;
        constexpr int kGridHeight = 25;
        grid_.resize(kGridHeight, std::vector<GridCell>(kGridWidth));
    }
    RenderUI();
}

void StageEditor::RenderUI() {
    ImGui::Begin("Stage Editor");

 
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
        std::string fullPath = "Resources/Mapdata/" + std::string(fileNameBuffer);
        SaveCSV(fullPath);
    }

    // ステージリスト（ドロップダウン）
    if (!availableStages.empty()) {
        if (ImGui::BeginCombo("Stage List", availableStages[selectedStageIndex].c_str())) {
            for (int i = 0; i < availableStages.size(); ++i) {
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
            ImGui::EndCombo();
        }
    }
    // ステージ一覧の更新
    if (ImGui::Button("Reload Stage List")) {
        availableStages.clear();
        for (const auto& entry : fs::directory_iterator("Resources/Mapdata")) {
            if (entry.path().extension() == ".csv") {
                availableStages.push_back(entry.path().filename().string());
            }
        }
    }

    // ロード（選択中のファイル）
    if (ImGui::Button("Load Selected Stage")) {
        if (!availableStages.empty()) {
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
            constexpr float kCellSize = 12.0f; // ← 10〜16 あたりがオススメ
            if (ImGui::ColorButton(("##" + std::to_string(x) + "_" + std::to_string(y)).c_str(), color, 0, ImVec2(kCellSize, kCellSize))) {
                grid_[y][x].type = selectedType_;
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();
    }

    ImGui::End();
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
    std::ifstream file(filename);
    std::string line;
    int y = 0;
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
