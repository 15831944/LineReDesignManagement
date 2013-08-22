// ------------------------------------------------
//                  LineManagementAssistant
// Copyright 2012-2013, Chengyong Yang & Changhai Gu. 
//               All rights reserved.
// ------------------------------------------------
//	LineEntryData.cpp
//	written by Changhai Gu
// ------------------------------------------------
// $File:\\LineManageAssitant\main\source\data\LineEntryData.h $
// $Author: Changhai Gu $
// $DateTime: 2013/1/2 01:35:46 $
// $Revision: #1 $
// ------------------------------------------------

#include <LineEntryData.h>
#include <LMAUtils.h>
#include <GlobalDataConfig.h>

#include <ArxWrapper.h>
#include <acdocman.h>
#include <acutmem.h>

#include <ArxCustomObject.h>
#include <LineManageAssitant.h>

using namespace ::com::guch::assistant::arx;
using namespace ::com::guch::assistant::config;
extern wstring gLmaArxLoadPath;

namespace com
{

namespace guch
{

namespace assistant
{

namespace data
{

#define POINT_START L"����:"
#define POINT_END L""
#define POINTS_SEP L"-"

///////////////////////////////////////////////////////////////////////////
// Implementation PointEntity

/**
 * ��������ʵ��
 */

PointEntity::PointEntity()
:m_PointNO(0),
m_LevelKind(L""),
m_Direction(L""),
m_DbEntityCollection()
{
	m_Point[X] = 0;
	m_Point[Y] = 0;
	m_Point[Z] = 0;
}

PointEntity::PointEntity( const UINT& pointNO, const ads_point& point, 
	const wstring& levelKind, const wstring& direction, const AcDbObjectId& entityID)
:m_PointNO(pointNO),
m_LevelKind(levelKind),
m_Direction(direction),
m_DbEntityCollection()
{
	m_Point[X] = point[X];
	m_Point[Y] = point[Y];
	m_Point[Z] = point[Z];
}

PointEntity::PointEntity( const PointEntity& pointEntry)
{
	this->m_PointNO = pointEntry.m_PointNO;
	this->m_LevelKind = pointEntry.m_LevelKind;
	this->m_Direction = pointEntry.m_Direction;

	this->m_Point[X] = pointEntry.m_Point[X];
	this->m_Point[Y] = pointEntry.m_Point[Y];
	this->m_Point[Z] = pointEntry.m_Point[Z];
}

PointEntity::PointEntity( const wstring& data )
{
	double temp;

	const static size_t start = wcslen(POINT_START);
	//size_t end = data.find_first_of(POINT_END);

	int index = 0;

	wstrVector* dataColumn = vectorContructor(data,POINTS_SEP,start,data.length());
	wstring& column = (*dataColumn)[index++];

	acdbDisToF(column.c_str(), -1, &temp);
	this->m_PointNO = (UINT)temp;

	column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &m_Point[X]);

	column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &m_Point[Y]);

	column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &m_Point[Z]);

	//m_LevelKind = (*dataColumn)[index++];
	//m_Direction = (*dataColumn)[index++];

	delete dataColumn;
}

wstring PointEntity::toString() const
{
	CString temp;
	temp.Format(L"%s%d%s%0.2f%s%0.2f%s%0.2f%s%s",
				POINT_START, m_PointNO, POINTS_SEP,
				m_Point[X], POINTS_SEP,
				m_Point[Y], POINTS_SEP,
				m_Point[Z], POINTS_SEP,
				POINT_END);

	return temp.GetBuffer();
}

/**
 * ����һ���߶��յ㿪ʼ����������Ĺ���
 **/
void PointEntity::CreateLineFrom(const void* lineEntity, const ads_point& start )
{
	//׼���������߶ε�������Ϣ
	LineEntity* pLineEntity = (LineEntity*)lineEntity;

	//�������Ƽ��ǲ���������˵ÿ�����߶��ڲ�ͬ�Ĳ�
	m_DbEntityCollection.mLayerName = pLineEntity->GetName();
	
	//Ψһ��ʾ���ߵ�ID�����߶ε�����ʵ�嶼�����ֵ���ڹ������߶������
	m_DbEntityCollection.mLineID = pLineEntity->GetLineID();
	
	//���ƹ���ʱ����Ҫ����Ϣ
	m_DbEntityCollection.mCategoryData = const_cast<LineCategoryItemData*>(pLineEntity->GetBasicInfo());
	
	//���߶ε����
	m_DbEntityCollection.mSequenceNO = m_PointNO;

	//���߶ε���ʼ����ֹ��
	m_DbEntityCollection.mStartPoint.set(start[X], start[Y], start[Z]);
	m_DbEntityCollection.mEndPoint.set(m_Point[X], m_Point[Y], m_Point[Z]);

	//�������߶����е����ݿ�ʵ��
	m_DbEntityCollection.DrawEntityCollection(pLineEntity->m_LinePriority);
}

/**
 * ������ʼ����У������б������������ض��Ĳ���
 **/
bool PointDBEntityCollection::DrawEntityCollection(GlobalData::LineProirity proirity) 
{
#ifdef DEBUG
	acutPrintf(L"\n��������ʵ�������뵽ͼ��ռ�");
#endif

	//��������ʵ��
	LMALineDbObject* lmaLineObj = new LMALineDbObject( this );

	//�������ʵ��
	SetLineEntity( ArxWrapper::PostToModelSpace(lmaLineObj, mLayerName) );

	//�ж��Ƿ���Ҫ�����ں�ʵ��
	if( mCategoryData->mWallSize != L"0" 
		&& mCategoryData->mWallSize.length() != 0)
	{
		acutPrintf(L"\n��ǰ�����бں�");

		//��������ʵ��
		LMAWallLineDbObject* lmaWallLineObj = new LMAWallLineDbObject( this );
		if( proirity == GlobalData::LINE_FIRST )
		{
			lmaWallLineObj->setColorIndex(GlobalData::INTERSET_WALLLINE_COLOR );
		}

		//�������ʵ��
		SetWallLineEntity( ArxWrapper::PostToModelSpace(lmaWallLineObj, mLayerName) );
	}

	//�������߰�ȫ��Χʵ��
	LMASafeLineDbObject* lmaSafeLineObj = new LMASafeLineDbObject( this );

	//����������ߣ����Զ�·�ɹ��ߣ�����ʾΪ��ɫ
	if( proirity == GlobalData::LINE_FIRST )
	{
		lmaSafeLineObj->setColorIndex(GlobalData::INTERSET_WALLLINE_COLOR );
	}
	else if(  proirity == GlobalData::LINE_SECOND )
	{
		lmaSafeLineObj->setColorIndex(GlobalData::SAFELINE_COLOR );
	}
	else
	{
		//Ĭ�ϰ�ȫ��Χʵ�岻��ʾ
		lmaSafeLineObj->setVisibility(AcDb::kInvisible);
	}

	//������߰�ȫ��Χʵ��
	SetSafeLineEntity( ArxWrapper::PostToModelSpace(lmaSafeLineObj, mLayerName) );

	return true;
}

bool PointDBEntityCollection::HasEntity( const AcDbObjectId& entityId ) const
{
	if( m_LineEntryId == entityId
		|| m_SafeLineEntityId == entityId
		|| m_DimEntityId == entityId
		|| m_MarkEntityId == entityId )
	{
		return true;
	}
	else
	{
		return false;
	}
}

PointDBEntityCollection::PointDBEntityCollection()
	:mLayerName()
	,mCategoryData(NULL)
	,mLineID(0)
	,mSequenceNO(0)
	,mStartPoint()
	,mEndPoint()
	,m_LineEntryId()
	,m_WallLineEntryId()
	,m_SafeLineEntityId()
	,m_DimEntityId()
	,m_MarkEntityId()
{}

void PointDBEntityCollection::DropEntityCollection()
{
	//�õ��߶ε����ݿ����ID
	if( m_LineEntryId.isValid() )
	{
#ifdef DEBUG
		acutPrintf(L"\n�߶Ρ�%s����š�%d�� ���� x:��%0.2lf��y:��%0.2lf��z:��%0.2lf����ɾ��",
					mLayerName.c_str(), mSequenceNO, mEndPoint[X], mEndPoint[Y], mEndPoint[Z]);
#endif

		//ɾ���߶ζ���
		acutPrintf(L"\nɾ�����߶�ʵ��.");
		ArxWrapper::RemoveDbObject(m_LineEntryId);

		//ɾ���ں�ʵ��
		acutPrintf(L"\nɾ�����߶�ʵ��.");
		ArxWrapper::RemoveDbObject(m_WallLineEntryId);

		//ɾ�����߶ΰ�ȫ��Χʵ��
		acutPrintf(L"\nɾ�����߶�ʵ��.");
		ArxWrapper::RemoveDbObject(m_SafeLineEntityId);
	}
}

void PointDBEntityCollection::SetLineWarning( bool warning )
{
	if( warning )
		acutPrintf(L"\n���ù��������ʵ�����ɫ.");
	else
		acutPrintf(L"\n�ָ����������ʵ�����ɫ.");

	if( m_WallLineEntryId.isValid())
	{
		AcDbEntity* pWallLine = ArxWrapper::GetDbObject( m_WallLineEntryId, true );
		if( pWallLine )
		{
			if( warning )
			{
				acutPrintf(L"\n���ñں�ʵ�����ɫΪ��ɫ.");
				pWallLine->setColorIndex( GlobalData::INTERSET_COLOR );
			}
			else
			{
				acutPrintf(L"\n�ָ��ں�ʵ�����ɫ.");
				pWallLine->setColorIndex( GlobalData::WALLLINE_COLOR );
			}

			pWallLine->close();
		}
	}
	else if( m_LineEntryId.isValid())
	{
		AcDbEntity* pLine = ArxWrapper::GetDbObject( m_LineEntryId, true );
		if( pLine )
		{
			if( warning )
			{
				acutPrintf(L"\n���ù���ʵ�����ɫΪ��ɫ.");
				pLine->setColorIndex( GlobalData::INTERSET_COLOR );
			}
			else
			{
				acutPrintf(L"\n�ָ�����ʵ�����ɫ.");
				pLine->setColorIndex( GlobalData::LINE_COLOR );
			}

			pLine->close();
		}
	}
	else
	{
		acutPrintf(L"\n�ں����ʵ��û��ID.");
	}
}

AcGePlane& PointDBEntityCollection::GetAroundPlane(int direction)
{
	if( !m_LineAroundEntity.bInitialized )
	{
		CalculatePanel();
	}

	switch(direction)
	{
	case 1:
		return m_LineAroundEntity.mFrontPlane;
	case 2:
		return m_LineAroundEntity.mBackPlane;
	case 3:
		return m_LineAroundEntity.mRightPlane;
	case 4:
		return m_LineAroundEntity.mLeftPlane;
	case 5:
		return m_LineAroundEntity.mTopPlane;
	case 6:
		return m_LineAroundEntity.mBottomPlane;
	default:
		return m_LineAroundEntity.mFrontPlane;
	}
}

void PointDBEntityCollection::CalculatePanel()
{
	double xOffset = 0, yOffset = 0, zOffset = 0;
	double radius = 0.0, height = 0.0, width = 0.0,
		wallSize = 0.0, safeSize = 0.0;

	acdbDisToF(mCategoryData->mSize.mRadius.c_str(), -1, &radius);
	radius /= 1000;

	acdbDisToF(mCategoryData->mSize.mHeight.c_str(), -1, &height);
	height /= 1000;

	acdbDisToF(mCategoryData->mSize.mWidth.c_str(), -1, &width);
	width /= 1000;

	acdbDisToF(mCategoryData->mWallSize.c_str(), -1, &wallSize);
	wallSize /= 1000;

	acdbDisToF(mCategoryData->mSafeSize.c_str(), -1, &safeSize);
	safeSize /= 1000;

	//����Բ����
	if( mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		yOffset = xOffset = radius + wallSize + safeSize;
	}
	else //����Բ����
	{
		xOffset = width/2 + wallSize + safeSize;
		yOffset = height/2 + wallSize + safeSize;
	}

	//����Ϊ����ĸ߶�
	zOffset = this->mStartPoint.distanceTo(this->mEndPoint);

	//�õ�8������
	AcGePoint3d topLeftFront(-xOffset,-yOffset,0),
		topRightFront(-xOffset,-yOffset,zOffset),
		bottomLeftFront(xOffset,-yOffset,0),
		bottomRightFront(xOffset,-yOffset,zOffset),
		topLeftBack(-xOffset,yOffset,0),
		topRightBack(-xOffset,yOffset,zOffset),
		bottomLeftBack(xOffset,yOffset,0),
		bottomRightBack(xOffset,yOffset,zOffset);

	//����12����
	//Line Horinzional
	m_LineAroundEntity.mLineFrontTop = AcGeLineSeg3d(topLeftFront,topRightFront);
	m_LineAroundEntity.mLineFrontBottom = AcGeLineSeg3d(bottomLeftFront,bottomRightFront);
	m_LineAroundEntity.mLineBackTop = AcGeLineSeg3d(topLeftBack,topRightBack);
	m_LineAroundEntity.mLineBackBottom = AcGeLineSeg3d(topLeftFront,topRightFront);

	//Line Vertical
	m_LineAroundEntity.mLineLeftFront = AcGeLineSeg3d(topLeftFront,bottomLeftFront);
	m_LineAroundEntity.mLineRightFront = AcGeLineSeg3d(topRightFront,bottomRightFront);
	m_LineAroundEntity.mLineLeftBack = AcGeLineSeg3d(topLeftBack,bottomLeftBack);
	m_LineAroundEntity.mLineRightBack = AcGeLineSeg3d(topRightBack,bottomRightBack);

	//Line Depth
	m_LineAroundEntity.mLineLeftTop = AcGeLineSeg3d(topLeftFront,topLeftBack);
	m_LineAroundEntity.mLineLeftBottom = AcGeLineSeg3d(bottomLeftFront,bottomLeftBack);
	m_LineAroundEntity.mLineRightTop = AcGeLineSeg3d(topRightFront,topRightBack);
	m_LineAroundEntity.mLineRightBottom = AcGeLineSeg3d(bottomRightFront,bottomRightBack);

	AcGePoint3d frontPlanePointStart(xOffset, 0, 10);
	AcGePoint3d frontPlanePointEnd(xOffset, 0, 0);
	AcGePoint3d frontPlanePointEndCopy(xOffset, 10, 0);

	m_LineAroundEntity.mFrontPlane = AcGePlane( frontPlanePointStart, frontPlanePointEnd, frontPlanePointEndCopy);

	AcGeMatrix3d rotateMatrix;
	double angle;

	//�õ�����߶κ�Z�������ƽ�洹ֱ������
	AcGeVector3d line3dVector(mEndPoint.x - mStartPoint.x, mEndPoint.y - mStartPoint.y, mEndPoint.z - mStartPoint.z);
	AcGeVector3d rotateVctor = line3dVector.crossProduct(AcGeVector3d::kZAxis);

	//�õ����ߴ�ֱ������X��ļн�
	angle = rotateVctor.angleTo(AcGeVector3d::kXAxis); // �߶�λ�ڵڶ���������ʱ����ֱ������X��ļн�����߶��ڵ�һ��������ʱ�ļн���ȣ���������
	if( mStartPoint.x < mEndPoint.x )
		angle = -angle;

	//������ת������������б
	rotateMatrix = AcGeMatrix3d::rotation( angle, AcGeVector3d::kZAxis, AcGePoint3d::kOrigin);
	TransformBy(rotateMatrix);

	//�õ����������ת�ĽǶ�
	angle = -line3dVector.angleTo(AcGeVector3d::kZAxis);

	//������ת
	rotateMatrix = AcGeMatrix3d::rotation( angle, rotateVctor, AcGePoint3d::kOrigin);
	TransformBy(rotateMatrix);

	//�õ��߶ε����ĵ�
	AcGePoint3d center( mStartPoint.x + mEndPoint.x, mStartPoint.y + mEndPoint.y, mStartPoint.z + mEndPoint.z); 
	center /= 2;

	//����ƫ��
	AcGeMatrix3d moveMatrix;
	moveMatrix.setToTranslation(AcGeVector3d(center.x,center.y,center.z));

	//���ճ���
	TransformBy(moveMatrix);

	//Set initialized status
	this->m_LineAroundEntity.bInitialized = true;
}

void PointDBEntityCollection::TransformBy( const AcGeMatrix3d& matrix )
{
	//Line Horinzional
	m_LineAroundEntity.mLineFrontTop.transformBy(matrix);
	m_LineAroundEntity.mLineFrontBottom.transformBy(matrix);
	m_LineAroundEntity.mLineBackTop.transformBy(matrix);
	m_LineAroundEntity.mLineBackBottom.transformBy(matrix);

	//Line Vertical
	m_LineAroundEntity.mLineLeftFront.transformBy(matrix);
	m_LineAroundEntity.mLineRightFront.transformBy(matrix);
	m_LineAroundEntity.mLineLeftBack.transformBy(matrix);
	m_LineAroundEntity.mLineRightBack.transformBy(matrix);

	//Line Depth
	m_LineAroundEntity.mLineLeftTop.transformBy(matrix);
	m_LineAroundEntity.mLineLeftBottom.transformBy(matrix);
	m_LineAroundEntity.mLineRightTop.transformBy(matrix);
	m_LineAroundEntity.mLineRightBottom.transformBy(matrix);

	m_LineAroundEntity.mFrontPlane.transformBy(matrix);
}

} // end of data

} // end of assistant

} // end of guch

} // end of com