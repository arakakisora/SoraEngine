#pragma once
#include "MyMath.h"


enum class MapChipType : int {

	Empty = 0,
	Block,
	Enemy1,
	Portal,
	Goal,
	Player,
	UnbreakableBlock,
	damageBlock,


};

#include <unordered_map>
	struct MapChipInfo {
		MapChipType id;// マップチップID
		std::string label;// マップチップラベル
		Vector4 color;// マップチップカラー
		std::string collision;// 衝突情報
		std::string spawn;
		int enemyNumber = -1;// 敵番号
		int hitPoints = 0; // 追加: タイルの耐久値（0 = 壊せない / 1以上 = 壊せる）
	};
class MapChipDatabase
{
public:
	static MapChipDatabase* GetInstance();

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
	const MapChipInfo* GetById(MapChipType id) const;
	/// <summary>
	/// すべてのマップチップ情報を取得します
	/// </summary>
	/// <returns></returns>
	const std::vector<MapChipInfo>& GetAll() const { return chips_; }
private:
	std::vector<MapChipInfo> chips_;// マップチップ情報リスト
	std::unordered_map<MapChipType, MapChipInfo> chipsById_;// IDによるマップチップ情報マップ

};
