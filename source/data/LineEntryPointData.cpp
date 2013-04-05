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

#include <LineManageAssitant.h>

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

#define POINT_START L"PS"
#define POINT_END L"PE"
#define POINTS_SEP L"&&"

///////////////////////////////////////////////////////////////////////////
// Implementation PointEntry

/**
 * ��������ʵ��
 */

PointEntry::PointEntry()
:m_PointNO(0),
m_LevelKind(L""),
m_Direction(L""),
m_DbEntityCollection()
{
	m_Point[X] = 0;
	m_Point[Y] = 0;
	m_Point[Z] = 0;
}

PointEntry::PointEntry( const UINT& pointNO, const ads_point& point, 
	const wstring& levelKind, const wstring& direction, const AcDbObjectId& entityID)
:m_PointNO(pointNO),
m_LevelKind(levelKind),
m_Direction(direction),
m_DbEntityCollection(entityID)
{
	m_Point[X] = point[X];
	m_Point[Y] = point[Y];
	m_Point[Z] = point[Z];
}

PointEntry::PointEntry( const PointEntry& pointEntry)
{
	this->m_PointNO = pointEntry.m_PointNO;
	this->m_LevelKind = pointEntry.m_LevelKind;
	this->m_Direction = pointEntry.m_Direction;

	this->m_Point[X] = pointEntry.m_Point[X];
	this->m_Point[Y] = pointEntry.m_Point[Y];
	this->m_Point[Z] = pointEntry.m_Point[Z];

	this->m_EntryId = pointEntry.m_EntryId;
}

PointEntry::PointEntry( const wstring& data )
{
	double temp;

	const static size_t start = wcslen(POINT_START);
	size_t end = data.find_first_of(POINT_END);

	int index = 0;

	wstrVector* dataColumn = vectorContructor(data,POINTS_SEP,start,end);
	wstring& column = (*dataColumn)[index++];

	acdbDisToF(column.c_str(), -1, &temp);
	this->m_PointNO = (UINT)temp;

	column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &m_Point[X]);

	column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &m_Point[Y]);

	column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &m_Point[Z]);

	m_LevelKind = (*dataColumn)[index++];

	m_Direction = (*dataColumn)[index++];

	delete dataColumn;
}

wstring PointEntry::toString() const
{
	CString temp;
	temp.Format(L"%s%d%s%0.2f%s%0.2f%s%0.2f%s%s%s%s%s%s",
				POINT_START,
				m_PointNO, POINTS_SEP,
				m_Point[X], POINTS_SEP,
				m_Point[Y], POINTS_SEP,
				m_Point[Z], POINTS_SEP,
				m_LevelKind.c_str(),POINTS_SEP,
				m_Direction.c_str(),POINTS_SEP,
				POINT_END);

	return temp.GetBuffer();
}

void PointEntry::CreateLineFrom(const LineEntry* lineEntity, const ads_point& start )
{
	m_DbEntityCollection
	AcDb3dSolid* pNewLine = DrawCylinder(lineEntry, (*iter)->m_PointNO, *pStart, *pEnd );

	//����ʵ����ObjectID
	(*iter)->m_EntryId = pNewLine->objectId();;
}

/**
 * ������ʼ����У������б������������ض��Ĳ���
 **/
AcDb3dSolid* PointEntry::DrawCylinder(const LineEntry& lineEntity,
										const UINT& sequenceID,
										const AcGePoint3d& start,
										const AcGePoint3d& end ) 
{
#ifdef DEBUG
	acutPrintf(L"\n��������ʵ�������뵽ͼ��ռ�\n");
#endif

	UINT lineID = lineEntity.m_LineID;
	const LineCategoryItemData* categoryData = lineEntity.m_LineBasiInfo;
	const wstring& layerName = lineEntity.m_LineName;

	LMALineDbObject* lmaLineObj = new LMALineDbObject( layerName, *categoryData,
														Adesk::Int32(lineID), Adesk::Int32(sequenceID),
														start,end);
	PostToModelSpace(lmaLineObj,layerName);

	return lmaLineObj;
}

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

void PointDBEntityCollection::DropEntityCollection()
{
	//�õ��߶ε����ݿ����ID
	AcDbObjectId lineObjId = (*iter)->m_EntryId;
	if( lineObjId.isValid() )
	{
#ifdef DEBUG
		acutPrintf(L"\n�߶��յ� ��š�%d�� ���� x:��%lf��y:��%lf��z:��%lf����ɾ��",(*iter)->m_PointNO,(*iter)->m_Point[X],(*iter)->m_Point[Y],(*iter)->m_Point[Z]);
#endif
		//����objectID�����ݿ�õ�ֱ��
		AcDbEntity* pLineObject(NULL);
		Acad::ErrorStatus es = acdbOpenAcDbEntity(pLineObject, lineObjId, AcDb::kForWrite);
		if (es == Acad::eOk)
		{
			LMALineDbObject* pLmaLineObject = dynamic_cast<LMALineDbObject*>(pLineObject);

			if( pLmaLineObject )
			{
				AcDbHandle handleDim = pLmaLineObject->mHandleDim;
				AcDbHandle handleText = pLmaLineObject->mHandleText;

				//�رչ���ʵ��
				pLmaLineObject->close();

				AcDbObjectId dimObjId, txtObjId;
						
				//�õ���ע�����ID
				acdbHostApplicationServices()->workingDatabase()->getAcDbObjectId(
					dimObjId,false,handleDim);
						
				//�õ�����˵��
				acdbHostApplicationServices()->workingDatabase()->getAcDbObjectId(
					txtObjId,false,handleText);

				//ɾ���߶ζ���
				acutPrintf(L"\nɾ�����߶�ʵ��.");
				RemoveDbObject(lineObjId);

				acutPrintf(L"\nɾ����ע����.");
				RemoveDbObject(dimObjId);

				acutPrintf(L"\nɾ������˵��.");
				RemoveDbObject(txtObjId);
			}
		}
		else
		{
			acutPrintf(L"\n��ɾ��������û���ҵ���Ӧ�����ݿ����");
			rxErrorMsg(es);
		}
	}
}

} // end of data

} // end of assistant

} // end of guch

} // end of com