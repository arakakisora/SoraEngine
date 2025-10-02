
#pragma once
#include <vector>
#include <string>
#include"MapType.h"



inline std::pair<int, int> ToIndex(int x, int y, const StageRange& r) {
	return { x - r.minX, y - r.minY };
}
inline std::pair<int, int> ToCoord(int ix, int iy, const StageRange& r) {
	return { r.minX + ix, r.minY + iy };
}



class StageEditor {
public:
	// ファイル名バッファ
	char fileNameBuffer[64] = "stage1.csv";
	std::vector<std::string> availableStages;
	int selectedStageIndex = 0;

	void Run();
	bool GetReloadRequested() const { return isReloadRequested_; }
	void SetReloadRequested(bool value) { isReloadRequested_ = value; }
	const char* GetFileName() const { return fileNameBuffer; }
	StageRange& GetRange() { return range_; }

	
private:
	void RenderUI();
	void SaveCSV(const std::string& filename);
	void LoadCSV(const std::string& filename);
	

	std::vector<std::vector<GridCell>> grid_;
	int selectedType_ = 1;
	bool isReloadRequested_ = false; // GamePlayScene のメンバに追加
	StageRange range_;
};
