// ------------------------------------------------
//                  LineManagementAssistant
// Copyright 2012-2013, Chengyong Yang & Changhai Gu. 
//               All rights reserved.
// ------------------------------------------------
//	ArxWrapper.h
//	written by Changhai Gu
// ------------------------------------------------
// $File:\\LineManageAssitant\main\source\ARX\ArxCustomObject.cpp $
// $Author: Changhai Gu $
// $DateTime: 2013/1/12 06:13:00
// $Revision: #1 $
// ------------------------------------------------

#include <ArxWrapper.h>
#include <ArxCustomObject.h>
#include <LMAUtils.h>
#include <LineEntryData.h>
#include <gelnsg3d.h>
#include <LineConfigDataManager.h>
#include <GlobalDataConfig.h>

using namespace com::guch::assistant::config;
using namespace com::guch::assistant::data;

namespace com
{

namespace guch
{

namespace assistant
{

namespace arx
{

void LMADbObjectManager::RegisterClass()
{
	LMALineDbObject::rxInit();
	LineDBEntry::rxInit();
}

void LMADbObjectManager::UnRegisterClass()
{
	deleteAcRxClass(LMALineDbObject::desc());
	deleteAcRxClass(LineDBEntry::desc());
}

ACRX_DXF_DEFINE_MEMBERS(LMALineDbObject, AcDb3dSolid, 
AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent, 
0,
    LMALineDbObject, LMA);

/// <summary>
/// Initializes a new instance of the <see cref="LMALineDbObject"/> class.
/// </summary>
LMALineDbObject::LMALineDbObject():
	mLayerName()
	,mCategoryData(NULL)
	,mLineID(-1)
	,mSequenceNO(-1)
	,mStartPoint(AcGePoint3d::kOrigin)
	,mEndPoint(AcGePoint3d::kOrigin)
	,mRadius(0)
	,mWidth(0)
	,mLength(0)
{
}

/// <summary>
/// Initializes a new instance of the <see cref="LMALineDbObject"/> class.
/// </summary>
/// <param name="id">The id.</param>
/// <param name="seqNO">The seq NO.</param>
/// <param name="start">The start.</param>
/// <param name="end">The end.</param>
/// <param name="lineEntry">The line entry.</param>
LMALineDbObject::LMALineDbObject(const wstring& layerName,
					const LineCategoryItemData& categoryData,
					const Adesk::Int32& id,
					const Adesk::Int32& seqNO,
					const AcGePoint3d& start,
					const AcGePoint3d& end )
: mLayerName()
, mCategoryData( new LineCategoryItemData(categoryData) )
, mLineID(id)
, mSequenceNO(seqNO)
, mStartPoint(start)
, mEndPoint(end)
, mRadius(0)
, mWidth(0)
, mLength(0)
{
	Init();
}

 /// <summary>
/// Inits this instance.
/// </summary>
/// <returns></returns>
Acad::ErrorStatus LMALineDbObject::Init()
{
	if( mCategoryData == NULL )
	{
		acutPrintf(L"\n������Ϣ���Ϸ�");
		return Acad::eInvalidInput;
	}

	//Բ�λ����
	if( mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		acdbDisToF(mCategoryData->mRadius.c_str(), -1, &mRadius);

		//ֱ���ĵ�λ�Ǻ��ף�������ĵ�λ����
		mRadius = mRadius / 1000;
	}
	else if ( mCategoryData->mShape == GlobalData::LINE_SHAPE_SQUARE )
	{
		acdbDisToF(mCategoryData->mHeight.c_str(), -1, &mLength);
		acdbDisToF(mCategoryData->mWidth.c_str(), -1, &mWidth);

		//ֱ���ĵ�λ�Ǻ��ף�������ĵ�λ����
		mLength = mLength / 1000;
		mWidth = mWidth / 1000;
	}

	return CreatePipe();
}

/// <summary>
/// Creates the pipe.
/// </summary>
/// <returns></returns>
Acad::ErrorStatus LMALineDbObject::CreatePipe()
{
	acutPrintf(L"\n��ʼ���ƹ���");

	//�õ��߶εĳ���
	double height = mStartPoint.distanceTo(mEndPoint);
	if( height < 0.1 )
		return Acad::eInvalidInput;

	if( mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		acutPrintf(L"\n���ư뾶Ϊ��%0.2lf����Ϊ��%0.2lf����Բ��",mRadius,height);

		//����Բ����
		this->createFrustum(height,mRadius,mRadius,mRadius);
	}
	else if ( mCategoryData->mShape == GlobalData::LINE_SHAPE_SQUARE )
	{
		acutPrintf(L"\n���Ƴ���%0.2lf����%0.2lf���ߡ�%0.2lf���ĳ�����",mLength, mWidth, height);

		//����Բ����
		this->createBox(mLength,mWidth,height);
	}

	//�õ��߶���Z��Ĵ�ֱ����
	AcGeVector3d line3dVector(mEndPoint.x - mStartPoint.x,mEndPoint.y - mStartPoint.y, mEndPoint.z-mStartPoint.z);
	AcGeVector3d rotateVctor = line3dVector.crossProduct(AcGeVector3d::kZAxis);

	//�õ���ת�ĽǶ�
	double angle = -line3dVector.angleTo(AcGeVector3d::kZAxis);
	acutPrintf(L"\n�õ���ת�Ƕȡ�%lf��",angle);

	//������ת
	AcGeMatrix3d rotateMatrix = AcGeMatrix3d::rotation( angle, rotateVctor, AcGePoint3d::kOrigin);
	transformBy(rotateMatrix);
	
	//�õ��߶ε����ĵ�
	AcGePoint3d center(mStartPoint.x + mEndPoint.x, mStartPoint.y + mEndPoint.y, mStartPoint.z + mEndPoint.z); 
	center /= 2;
	acutPrintf(L"\n�õ����ĵ㡾%lf����%lf����%lf��",center.x,center.y,center.z);

	//����ƫ��
	AcGeMatrix3d moveMatrix;
	moveMatrix.setToTranslation(AcGeVector3d(center.x,center.y,center.z));

	transformBy(moveMatrix);

	//������ע
	CreateDimensions();

	return Acad::eOk;
}

/// <summary>
/// Creates the dimensions.
/// </summary>
/// <returns></returns>
Acad::ErrorStatus LMALineDbObject::CreateDimensions()
{
	//������ע��
    AcDbAlignedDimension* mAlignedDim = new AcDbAlignedDimension;

	//���ñ�ע������
	AcDbText* mLineDim = new AcDbText;
	{
		CString textSeq;
		textSeq.Format(L"��%d��",mSequenceNO);
		mLineDim->setTextString(textSeq);
	}

	//�õ��߶γ���
	double length = mStartPoint.distanceTo(mEndPoint);
	if( length < 0.1 )
		return Acad::eInvalidInput;

	//���ֵ�ƫ��Ϊֱ������
	double dimAlignTextOff = 0;
	if ( mLineShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		dimAlignTextOff = mRadius*2;
	}
	else if ( mLineShape == GlobalData::LINE_SHAPE_SQUARE )
	{
		dimAlignTextOff = mLength + mWidth;
	}

	//������ԭ�㴦����X�᷽���ע
	mAlignedDim->setXLine1Point(AcGePoint3d::kOrigin);
    mAlignedDim->setXLine2Point(AcGePoint3d(length,0,0));
	mAlignedDim->setTextPosition(AcGePoint3d(length/2,dimAlignTextOff,0));

	//���֡��κš�
	double dimTextOff = 0,dimTextHeight = 0;

	//�������ֵĸ߶ȣ���Y��λ��
	if ( mLineShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		dimTextHeight = mRadius;
		dimTextOff = mRadius/2;
	}
	else if ( mLineShape == GlobalData::LINE_SHAPE_SQUARE )
	{
		dimTextHeight = mLength;
		dimTextOff = mLength/2;
	}

	mLineDim->setHeight(dimTextHeight); 
	mLineDim->setPosition(AcGePoint3d(length/2, dimTextOff,0));

#ifdef DEBUG
	acutPrintf(L"\n��ע����Ϊ��%0.2lf��������˵����λ��X��%0.2lf��Y��%0.2lf��,�κ�˵��λ�ø߶ȡ�%0.2lf��Y��%0.2lf��",
					length, length/2, dimAlignTextOff,dimTextHeight,dimTextOff);
#endif

	//���ñ�ע�ֵ�����
    mAlignedDim->useSetTextPosition();    // make text go where user picked
    mAlignedDim->setDatabaseDefaults();

	//�����߶ε�����
	AcGeVector3d line3dVector(mEndPoint.x - mStartPoint.x,mEndPoint.y - mStartPoint.y, mEndPoint.z-mStartPoint.z);

	{
		//Ȼ����ת��Z�ᴦ
		AcGeMatrix3d zMatrix = AcGeMatrix3d::rotation( -ArxWrapper::kRad90, AcGeVector3d::kYAxis, AcGePoint3d::kOrigin);
		mAlignedDim->transformBy(zMatrix);
		mLineDim->transformBy(zMatrix);

		//�õ���ֱ����XZƽ�棩�Ƕ�
		double xAngle = AcGeVector3d(line3dVector.x,line3dVector.y,0).angleTo(AcGeVector3d::kXAxis);
		acutPrintf(L"\n�õ�Z�����ת�Ƕȡ�%lf��",xAngle);

		//��Z����ת
		AcGeMatrix3d rotateZMatrix = AcGeMatrix3d::rotation( xAngle, AcGeVector3d::kZAxis, AcGePoint3d::kOrigin);
		mAlignedDim->transformBy(rotateZMatrix);
		mLineDim->transformBy(rotateZMatrix);
	}

	{
		//�õ���ת�ĽǶ�
		double angle = -line3dVector.angleTo(AcGeVector3d::kZAxis);
		acutPrintf(L"\n�õ���ת�Ƕȡ�%lf��",angle);

		//�õ��߶���Z����ɵ�ƽ��Ĵ�ֱ����
		AcGeVector3d rotateVctor = line3dVector.crossProduct(AcGeVector3d::kZAxis);

		//������ת
		AcGeMatrix3d rotateMatrix = AcGeMatrix3d::rotation( angle, rotateVctor, AcGePoint3d::kOrigin);
		mAlignedDim->transformBy(rotateMatrix);

		mLineDim->transformBy(rotateMatrix);
	}

	{
		//����ƫ��
		AcGeMatrix3d moveMatrix;
		moveMatrix.setToTranslation(AcGeVector3d(mStartPoint.x,mStartPoint.y,mStartPoint.z));

		mAlignedDim->transformBy(moveMatrix);

		mLineDim->transformBy(moveMatrix);
	}

	//���������ע
	/*{
		CString textSeq;
		textSeq.Format(L"%d",mSequenceNO);

		mAlignedDim->setDimensionText(textSeq);
	}*/

	//��ӵ�ģ�Ϳռ�
	if( mLineEntry )
	{
		acutPrintf(L"\n�����ע�����ݿ�");

		//mAlignedDim->setLinetype( acdbHostApplicationServices()->workingDatabase()->byLayerLinetype(), true );
		ArxWrapper::PostToModelSpace(mAlignedDim,mLineEntry->m_LineName.c_str());
		mAlignedDim->getAcDbHandle(mHandleDim);

		acutPrintf(L"\n��������˵�������ݿ�");
		
		//mLineDim->setLinetype( acdbHostApplicationServices()->workingDatabase()->byLayerLinetype(), true );
		ArxWrapper::PostToModelSpace(mLineDim,mLineEntry->m_LineName.c_str());
		mLineDim->getAcDbHandle(mHandleText);
	}
	else
	{
		acutPrintf(L"\n���߶ι�����ֱ�߲����ڣ����ܳ����ˣ�");
	}

	return Acad::eOk;
}

AcGePoint3d LMALineDbObject::GetCutCenter( const AcGePlane& cutPlane )
{
	//�õ�������յ��γɵ��߶�
	AcGeLineSeg3d lineSeg(this->mStartPoint,this->mEndPoint);

	//�߶���ֱ�����еĽ���
	AcGePoint3d center;
	Adesk::Boolean hasIntersect = cutPlane.intersectWith(lineSeg, center);

	return center;
}


// Files data in from a DWG file.
//
Acad::ErrorStatus
LMALineDbObject::dwgInFields(AcDbDwgFiler* pFiler)
{
    assertWriteEnabled();

    AcDb3dSolid::dwgInFields(pFiler);

    // For wblock filing we wrote out our owner as a hard
    // pointer ID so now we need to read it in to keep things
    // in sync.
    //
    if (pFiler->filerType() == AcDb::kWblockCloneFiler) {
        AcDbHardPointerId id;
        pFiler->readItem(&id);
    }

	Adesk::UInt32 lineID;
    pFiler->readItem(&lineID);
	mLineID = (UINT)lineID;

	Adesk::UInt32 seqNO;
	pFiler->readItem(&seqNO);
	mSequenceNO = (UINT)seqNO;

	pFiler->readPoint3d(&mStartPoint);
	pFiler->readPoint3d(&mEndPoint);
	
	pFiler->readAcDbHandle(&mHandleDim);
	pFiler->readAcDbHandle(&mHandleText);

	CString filename;
	dbToStr(this->database(),filename);

#ifdef DEBUG
	acutPrintf(L"\n��DWG�ļ���%s���õ������߶�ʵ�� ID��%d�����кš�%d�� ��� X:��%lf��Y:��%lf��Z:��%lf�� �յ� X:��%lf��Y:��%lf��Z:��%lf��.",
					filename.GetBuffer(),mLineID,mSequenceNO,
					mStartPoint.x,mStartPoint.y,mStartPoint.z,
					mEndPoint.x,mEndPoint.y,mEndPoint.z);
#endif

	LineEntryFileManager::RegisterLineSegment(filename.GetBuffer(),this,mLineID,mSequenceNO,
												mStartPoint,mEndPoint);

    return pFiler->filerStatus();
}

// Files data out to a DWG file.
//
Acad::ErrorStatus
LMALineDbObject::dwgOutFields(AcDbDwgFiler* pFiler) const
{
    assertReadEnabled();

    AcDb3dSolid::dwgOutFields(pFiler);

    // Since objects of this class will be in the Named
    // Objects Dictionary tree and may be hard referenced
    // by some other object, to support wblock we need to
    // file out our owner as a hard pointer ID so that it
    // will be added to the list of objects to be wblocked.
    //
    if (pFiler->filerType() == AcDb::kWblockCloneFiler)
        pFiler->writeHardPointerId((AcDbHardPointerId)ownerId());

    pFiler->writeItem(Adesk::UInt32(mLineID));
	pFiler->writeItem(Adesk::UInt32(mSequenceNO));

	pFiler->writeItem(mStartPoint);
	pFiler->writeItem(mEndPoint);
	
	pFiler->writeAcDbHandle(mHandleDim);
	pFiler->writeAcDbHandle(mHandleText);

	CString filename;
	dbToStr(this->database(),filename);

#ifdef DEBUG
	acutPrintf(L"\n��������߶�ʵ�� ���кš�%d�� ��� X:��%0.2lf��Y:��%0.2lf��Z:��%0.2lf�� �յ� X:��%0.2lf��Y:��%0.2lf��Z:��%0.2lf����DWG�ļ���%s��.",
					mSequenceNO,mStartPoint.x,mStartPoint.y,mStartPoint.z,
					mEndPoint.x,mEndPoint.y,mEndPoint.z,
					filename.GetBuffer());
#endif

    return pFiler->filerStatus();
}

// Files data in from a DXF file.
//
Acad::ErrorStatus
LMALineDbObject::dxfInFields(AcDbDxfFiler* pFiler)
{
    assertWriteEnabled();

    Acad::ErrorStatus es;
    if ((es = AcDb3dSolid::dxfInFields(pFiler))
        != Acad::eOk)
    {
        return es;
    }

    // Check if we're at the right subclass getLineID marker.
    //
    if (!pFiler->atSubclassData(_T("LMALineDbObject"))) {
        return Acad::eBadDxfSequence;
    }

    struct resbuf inbuf;
    while (es == Acad::eOk) {
        if ((es = pFiler->readItem(&inbuf)) == Acad::eOk) {

			switch ( inbuf.restype )
			{
				case AcDb::kDxfInt32:
					mLineID = inbuf.resval.rint;
				case AcDb::kDxfInt32 + 1:
					mSequenceNO = inbuf.resval.rint;
			}
        }
    }

    return pFiler->filerStatus();
}

// Files data out to a DXF file.
//
Acad::ErrorStatus
LMALineDbObject::dxfOutFields(AcDbDxfFiler* pFiler) const
{
    assertReadEnabled();

    AcDb3dSolid::dxfOutFields(pFiler);
    pFiler->writeItem(AcDb::kDxfSubclass, _T("LMALineDbObject"));
    pFiler->writeItem(AcDb::kDxfInt32, mLineID);
	pFiler->writeItem(AcDb::kDxfInt32 + 1, mSequenceNO);

    return pFiler->filerStatus();
}

} // end of arx

} // end of assistant

} // end of guch

} // end of com
