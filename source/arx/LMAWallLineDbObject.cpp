// ------------------------------------------------
//                  LineManagementAssistant
// Copyright 2012-2013, Chengyong Yang & Changhai Gu. 
//               All rights reserved.
// ------------------------------------------------
//	LMAWallLineDbObject.cpp
//	written by Changhai Gu
// ------------------------------------------------
// $File:\\LineManageAssitant\main\source\ARX\LMAWallLineDbObject.cpp $
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

ACRX_DXF_DEFINE_MEMBERS(LMAWallLineDbObject, AcDb3dSolid, 
AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent, 
0,
    LMAWallLineDbObject, LMA);

/// <summary>
/// Initializes a new instance of the <see cref="LMALineDbObject" /> class.
/// </summary>
LMAWallLineDbObject::LMAWallLineDbObject():
	LMALineDbObject()
{
}

/// <summary>
/// Initializes a new instance of the <see cref="LMALineDbObject" /> class.
/// </summary>
/// <param name="pPointInfo">The p point info.</param>
LMAWallLineDbObject::LMAWallLineDbObject( PointDBEntityCollection* pPointInfo)
: LMALineDbObject( pPointInfo )
{
	Init();
}

 /// <summary>
/// Inits this instance.
/// </summary>
/// <returns></returns>
Acad::ErrorStatus LMAWallLineDbObject::Init()
{
	acutPrintf(L"\n�������߱�ʵ��");

	if( mpPointInfo == NULL ||
		mpPointInfo->mCategoryData == NULL )
	{
		acutPrintf(L"\n������Ϣ���Ϸ�");
		return Acad::eInvalidInput;
	}

	//��ȫ�뾶��ں�
	double wallSize(0);
	acdbDisToF(mpPointInfo->mCategoryData->mWallSize.c_str(), -1, &wallSize);

	//Բ�λ����
	if( mpPointInfo->mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		acdbDisToF(mpPointInfo->mCategoryData->mSize.mRadius.c_str(), -1, &mRadius);

		//ֱ���ĵ�λ�Ǻ��ף�������ĵ�λ����
		mRadius = (mRadius + wallSize) / 1000;

		acutPrintf(L"\n�����ں�%0.2lf���뾶Ϊ��%0.2lf����Բ��", wallSize, mRadius);
	}
	else //if ( mpPointInfo->mCategoryData->mShape == GlobalData::LINE_SHAPE_SQUARE )
	{
		acdbDisToF(mpPointInfo->mCategoryData->mSize.mHeight.c_str(), -1, &mLength);
		acdbDisToF(mpPointInfo->mCategoryData->mSize.mWidth.c_str(), -1, &mWidth);

		//ֱ���ĵ�λ�Ǻ��ף�������ĵ�λ����
		mLength = ( mLength + wallSize )/ 1000;
		mWidth = ( mWidth + wallSize )/ 1000;

		acutPrintf(L"\n�����ں�%0.2lf����Ϊ��%0.2lf����Ϊ��%0.2lf���ķ���", wallSize, mWidth, mLength);
	}

	return CreateDBObject();
}

/// <summary>
/// Creates the pipe.
/// </summary>
/// <returns></returns>
Acad::ErrorStatus LMAWallLineDbObject::CreateDBObject()
{
	//ͬ��Ҳ�ǻ��ƹ���
	//LMALineDbObject::CreateDBObject();

	//acutPrintf(L"\n��ʼ����ʵ��");

	const AcGePoint3d& startPoint = mpPointInfo->mStartPoint;
	const AcGePoint3d& endPoint = mpPointInfo->mEndPoint;

	//�õ��߶εĳ���
	double height = startPoint.distanceTo( endPoint );
	if( height < 0.01 )
	{
		acutPrintf(L"\n�߶�С��1���ף��ݲ�����������ʵ�壡",mRadius,height);
		return Acad::eInvalidInput;
	}

	double wallOffset = height * 0.02;
	height -= wallOffset;

	if( mpPointInfo->mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		//acutPrintf(L"\n���ư뾶Ϊ��%0.2lf����Ϊ��%0.2lf����Բ��",mRadius,height);

		//����Բ����
		this->createFrustum(height,mRadius,mRadius,mRadius);
	}
	else //if (  mpPointInfo->mCategoryData->mShape == GlobalData::LINE_SHAPE_SQUARE )
	{
		//acutPrintf(L"\n���ƿ�%0.2lf���ߡ�%0.2lf������%0.2lf���ķ�����",mLength, mWidth, height);

		//����Բ����
		this->createBox(mLength,mWidth,height);
	}

	//����ƫ��
	AcGeMatrix3d wallUpMatrix;
	double zOffset = wallOffset / 8;
	wallUpMatrix.setToTranslation(AcGeVector3d(0, 0, zOffset));

	//�����ƶ�һ����ƫ������1/4����֤�ڲ�����ʵ��¶��
	transformBy(wallUpMatrix);

	//�õ��߶���Z��Ĵ�ֱ����
	AcGeVector3d line3dVector(endPoint.x - startPoint.x, endPoint.y - startPoint.y, endPoint.z - startPoint.z);
	AcGeVector3d rotateVctor = line3dVector.crossProduct(AcGeVector3d::kZAxis);

	//�õ���ת�ĽǶ�
	double angle = -line3dVector.angleTo(AcGeVector3d::kZAxis);
	//acutPrintf(L"\n�õ���ת�Ƕȡ�%lf��",angle);

	//������ת
	AcGeMatrix3d rotateMatrix = AcGeMatrix3d::rotation( angle, rotateVctor, AcGePoint3d::kOrigin);
	transformBy(rotateMatrix);
	
	//�õ��߶ε����ĵ�
	AcGePoint3d center( startPoint.x + endPoint.x, startPoint.y + endPoint.y, startPoint.z + endPoint.z); 
	center /= 2;
	//acutPrintf(L"\n�õ����ĵ㡾%0.2lf����%0.2lf����%0.2lf��",center.x,center.y,center.z);

	//����ƫ��
	AcGeMatrix3d moveMatrix;
	moveMatrix.setToTranslation(AcGeVector3d(center.x,center.y,center.z));

	//���ճ���
	transformBy(moveMatrix);

	//��עΪ��ɫ����������
	this->setColorIndex(2);

	return Acad::eOk;
}

// Files data in from a DWG file.
//
Acad::ErrorStatus
LMAWallLineDbObject::dwgInFields(AcDbDwgFiler* pFiler)
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

	//��ʼ�ͽ����˵�
	PointEntity *pStart(NULL), *pEnd(NULL);

	Adesk::UInt32 lineID;
    pFiler->readItem(&lineID);

	Adesk::UInt32 seqNO;
	pFiler->readItem(&seqNO);

	//��ʵ���������ȡ�������߶ε�
	CString filename;
	dbToStr(this->database(),filename);
	
	LineEntity* pLineEntity(NULL);
	if( !LineEntityFileManager::RegisterLineSegment(filename.GetBuffer(),lineID, seqNO, pLineEntity, pStart, pEnd ) )
	{
		acutPrintf(L"\n��Ч�ıں��߶Σ���š�%d����",seqNO);
		return Acad::eAlreadyInDb;
	}

	//�õ�������Ϣ���ƹ�����
	mpPointInfo = &pEnd->m_DbEntityCollection;

	//�������߶ζ������ݿ�ID
	mpPointInfo->SetWallLineEntity(id());

#ifdef DEBUG
	acutPrintf(L"\n��DWG�ļ���%s���õ����߱��߶�ʵ�� ID��%d�����кš�%d��.",
					filename.GetBuffer(),mpPointInfo->mLineID,mpPointInfo->mSequenceNO );
#endif

    return pFiler->filerStatus();
}

// Files data out to a DWG file.
//
Acad::ErrorStatus
LMAWallLineDbObject::dwgOutFields(AcDbDwgFiler* pFiler) const
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

    pFiler->writeItem(Adesk::UInt32(mpPointInfo->mLineID));
	pFiler->writeItem(Adesk::UInt32(mpPointInfo->mSequenceNO));

	CString filename;
	dbToStr(this->database(),filename);

#ifdef DEBUG
	acutPrintf(L"\n������߱��߶�ʵ�� ���кš�%d����DWG�ļ���%s��.",
					mpPointInfo->mSequenceNO,
					filename.GetBuffer());
#endif

    return pFiler->filerStatus();
}

// Files data in from a DXF file.
//
Acad::ErrorStatus
LMAWallLineDbObject::dxfInFields(AcDbDxfFiler* pFiler)
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
    if (!pFiler->atSubclassData(_T("LMASafeLineDbObject"))) {
        return Acad::eBadDxfSequence;
    }

    struct resbuf inbuf;
    while (es == Acad::eOk) {
        if ((es = pFiler->readItem(&inbuf)) == Acad::eOk) {

			switch ( inbuf.restype )
			{
				/*case AcDb::kDxfInt32:
					mLineID = inbuf.resval.rint;
				case AcDb::kDxfInt32 + 1:
					mSequenceNO = inbuf.resval.rint;*/
				default:
					break;
			}
        }
    }

    return pFiler->filerStatus();
}

// Files data out to a DXF file.
//
Acad::ErrorStatus
LMAWallLineDbObject::dxfOutFields(AcDbDxfFiler* pFiler) const
{
    assertReadEnabled();

    AcDb3dSolid::dxfOutFields(pFiler);
    pFiler->writeItem(AcDb::kDxfSubclass, _T("LMASafeLineDbObject"));
    pFiler->writeItem(AcDb::kDxfInt32, mpPointInfo->mLineID);
	pFiler->writeItem(AcDb::kDxfInt32 + 1, mpPointInfo->mSequenceNO);

    return pFiler->filerStatus();
}

} // end of arx

} // end of assistant

} // end of guch

} // end of com
