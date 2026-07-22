#pragma once

enum Corner {
	kRightBottom,
	kLeftBottom,
	kRightTop,
	kLeftTop,
	kNumCorner
};

enum class CollisionType {
	Top,
	Bottom,
	Left,
	Right
};