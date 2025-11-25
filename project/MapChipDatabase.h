#pragma once
#include "MyMath.h"
#include <unordered_map>
	struct MapChipInfo {
		int id;// マップチップID
		std::string label;// マップチップラベル
		Vector4 color;// マップチップカラー
		std::string collision;// 衝突情報
		std::string spawn;
		int enemyNumber = -1;// 敵番号

	};
class MapChipDatabase
{
public:
	/// <summary>
	/// JSONファイルからマップチップ情報を読み込みます
	/// </summary>
	/// <param name="path"></param>
	void LoadJson(const std::string& path);
	/// <summary>
	/// 指定IDのマップチップ情報を取得します
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	const MapChipInfo* GetById(int id) const;
	/// <summary>
	/// すべてのマップチップ情報を取得します
	/// </summary>
	/// <returns></returns>
	const std::vector<MapChipInfo>& GetAll() const { return chips_; }
private:
	std::vector<MapChipInfo> chips_;// マップチップ情報リスト
	std::unordered_map<int, MapChipInfo> chipsById_;// IDによるマップチップ情報マップ

};
/// <summary>
/// グローバルに 1 個だけ使う前提の DB インスタンス
/// </summary>
extern MapChipDatabase gMapChipDB;
