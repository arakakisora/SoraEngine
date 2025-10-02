#include "StageEditor.h"
#include "imgui.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include "MapType.h"


namespace fs = std::filesystem;

void StageEditor::Run() {
    // 初期化
    if (grid_.empty()) {
        grid_.resize(range_.Height(), std::vector<GridCell>(range_.Width()));
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

    const auto& infos = GetMapChipInfoList();
    for (size_t i = 0; i < infos.size(); ++i) {
        if (i > 0) ImGui::SameLine();
        ImGui::RadioButton(infos[i].label, &selectedType_, ToInt(infos[i].type));
    }

    ImGui::End();

    ImGui::Begin("Stage");


    for (int iy = 0; iy < range_.Height(); ++iy) {
        for (int ix = 0; ix < range_.Width(); ++ix) {
            GridCell& cell = grid_[iy][ix];

            Vector4 vColor(1, 1, 1, 1);
            for (auto& info : infos) {
                if (ToInt(info.type) == cell.type) {
                    vColor = info.color;
                    break;
                }
            }
            // Vector4 → ImVec4 に変換
            ImVec4 color(vColor.x, vColor.y, vColor.z, vColor.w);

            if (ImGui::ColorButton(("##" + std::to_string(ix) + "_" + std::to_string(iy)).c_str(),
                color, 0, ImVec2(12, 12))) {
                cell.type = selectedType_;
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
