// ------------------------------------------------
//                  LineManagementAssistant
// Copyright 2012-2013, Chengyong Yang & Changhai Gu. 
//               All rights reserved.
// ------------------------------------------------
//	LineCategoryItemData.h
//	written by Changhai Gu
// ------------------------------------------------
// $File:\\LineManageAssitant\header\LineCategoryItemData.h $
// $Author: Changhai Gu $
// $DateTime: 2013/4/15 19:35:46 $
// $Revision: #1 $
// ------------------------------------------------

#pragma once

#include <string>

#pragma warning(disable:4005) 

using namespace std;

namespace com
{

namespace guch
{

namespace assistant
{

namespace config
{

//ͨ����������
struct CommonConfig
{
	wstring mCategory;
	wstring mName;
	wstring mSubName;
	wstring mReserved;
};

struct LineSizeData
{
	//���Բ
	wstring mRadius;

	//��Ծ���
	wstring mWidth;
	wstring mHeight;

	//��չ����
	wstring mReservedA;
	wstring mReservedB;

	LineSizeData();
	
	LineSizeData( const LineSizeData& rData);

	LineSizeData( const wstring& rRadius,
					const wstring& rWidth,
					const wstring& rHeight,
					const wstring& rReservedA,
					const wstring& rReservedB);

	wstring toString() const;
};

struct LineCategoryItemData
{
	//���õ�����
	wstring mCategory;
	
	//Բ������
	wstring mShape;

	//��С����
	struct LineSizeData mSize;

	//�ں��밲ȫ����
	wstring mWallSize;
	wstring mSafeSize;

	//ƽ���ע�������ע
	wstring mPlaneMark;
	wstring mCutMark;

	//��Խ����
	wstring mThroughDirection;

	LineCategoryItemData(void);
	LineCategoryItemData( const wstring& rCategory,
							const wstring& rShape,
							const LineSizeData& rSize,
							const wstring& rWallSize,
							const wstring& rSafeSize,
							const wstring& rPlaneMark,
							const wstring& rCutMark,
							const wstring& rThroughDirection);

	LineCategoryItemData( const LineCategoryItemData& rData);

	wstring toString() const;
};

} // end of data

} // end of assistant

} // end of guch

} // end of com
