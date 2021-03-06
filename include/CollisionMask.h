#pragma once
#ifndef WS_COLLISION_MASKS_H
#define WS_COLLISION_MASKS_H
namespace WeiSungEngine {
	enum CollisionCategory {
		CATEGORY_ALL = 0xFFFF,
		CATEGORY_DEFAULT = 0x0001,
		CATEGORY_BUILDING = 0x0002,
		CATEGORY_CAR = 0x0004,
		CATEGORY_AI_CHARACTER = 0x0008,
		CATEGORY_PLAYER = 0x0010,

	};
}
#endif // !WS_COLLISION_MASKS_H