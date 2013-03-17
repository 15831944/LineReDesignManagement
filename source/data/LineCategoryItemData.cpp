#include "stdafx.h"
#include "LineCategoryItemData.h"
#include <LMAUtils.h>

#pragma warning (disable : 4996 )

namespace com
{

namespace guch
{

namespace assistant
{

namespace config
{

LineCategoryItemData::LineCategoryItemData(void)
:mCategory(L""),
mShape(L""),
mRadius(L"0"),
mWidth(L"0"),
mHeight(L"0"),
mWallSize(L"0"),
mSafeSize(L"0"),
mPlaneMark(L""),
mCutMark(L""),
mThroughDirection(L"")
{
}

LineCategoryItemData::LineCategoryItemData( const wstring& rCategory,
											const wstring& rShape,
											const wstring& rRadius,
											const wstring& rWidth,
											const wstring& rHeight,
											const wstring& rWallSize,
											const wstring& rSafeSize,
											const wstring& rPlaneMark,
											const wstring& rCutMark,
											const wstring& rThroughDirection)
:mCategory(rCategory),
mShape(rShape),
mRadius(rRadius),
mWidth(rWidth),
mHeight(rHeight),
mWallSize(rWallSize),
mSafeSize(rSafeSize),
mPlaneMark(rPlaneMark),
mCutMark(rCutMark),
mThroughDirection(rThroughDirection)
{}

LineCategoryItemData::LineCategoryItemData( const LineCategoryItemData& rData)
:mCategory(rData.mCategory),
mWidth(rData.mWidth),
mHeight(rData.mHeight),
mWallSize(rData.mWallSize),
mSafeSize(rData.mSafeSize),
mPlaneMark(rData.mPlaneMark),
mCutMark(rData.mCutMark),
mThroughDirection(rData.mThroughDirection)
{
}

wstring LineCategoryItemData::toString() const
{
	return mCategory + L"\t"
			+ mShape + L"\t"
			+ mRadius + L"\t"
			+ mWidth + L"\t"
			+ mHeight + L"\t"
			+ mWallSize + L"\t"
			+ mSafeSize + L"\t"
			+ mPlaneMark + L"\t"
			+ mCutMark + L"\t"
			+ mThroughDirection;
}

LineCategoryItemData::~LineCategoryItemData(void){}

/*
std::ostream & operator<<(std::ostream &os, const LineCategoryItemData &itemData)
{
	return os << itemData.mID << itemData.mName << itemData.mKind;
}
*/

} // end of data

} // end of assistant

} // end of guch

} // end of com