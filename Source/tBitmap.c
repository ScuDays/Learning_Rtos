#include "tLib.h"

// 初始化bitmap
void tBitmapInit(tBitmap *bitmap)
{
	bitmap->bitmap = 0;
}
// Bitmap的总位数
uint32_t tBitmapPosCount(void)
{
	return 32;
}
// 设置特定位
void tBitmapSet(tBitmap *bitmap, uint32_t pos)
{
	bitmap->bitmap |= 1 << pos;
}
// 清除特定位
void tBitmapClear(tBitmap *bitmap, uint32_t pos)
{
	bitmap->bitmap &= ~(1 << pos);
}
// bitmap查找函数
uint32_t tBitmapGetFirstSet(tBitmap *bitmap)
{
	// bitmap查找表
	static const uint8_t quickFindTable[] =
		{
			0xff, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};

	if (bitmap->bitmap & 0xFF)
	{
		return quickFindTable[bitmap->bitmap & 0xFF];
	}
	else if (bitmap->bitmap & 0xFF00)
	{
		return quickFindTable[(bitmap->bitmap >> 8) & 0xFF] + 8;
	}
	else if (bitmap->bitmap & 0xFF0000)
	{
		return quickFindTable[(bitmap->bitmap >> 16) & 0xFF] + 16;
	}
	else if (bitmap->bitmap & 0xFF000000)
	{
		return quickFindTable[(bitmap->bitmap >> 24) & 0xFF] + 24;
	}
	else
	{
		return tBitmapPosCount();
	}
}
