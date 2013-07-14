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

///////////////////////////////////////////////////////////////////////////
// Implementation LineEntity

const wstring LineEntity::LINE_ENTRY_LAYER = L"����ʵ���ֵ�";
const wstring LineEntity::LINE_DATA_BEGIN = L"ʵ�����ݣ�";

/**
 * ����ʵ��
 */

LineEntity::LineEntity()
	:m_LineID(0),
	m_LineName(L""),
	m_LineKind(L""),
	m_LineBasiInfo(new LineCategoryItemData()),
	m_CurrentPointNO(0),
	m_PrePointList(NULL),
	m_PointList(new PointList())
{}

LineEntity::LineEntity(const wstring& rLineName, const wstring& rLineKind,
						LineCategoryItemData* lineInfo, PointList* pointList)
	:m_LineID(0),
	m_LineName(rLineName),
	m_LineKind(rLineKind),
	m_LineBasiInfo(lineInfo),
	m_PointList(pointList),
	m_CurrentPointNO(0),
	m_PrePointList(NULL)
{
	//�������ݿ�������
	m_pDbEntry = new LineDBEntity( this );
}

LineEntity::LineEntity( wstring& data)
{
	m_PointList = new PointList();
	m_PrePointList = NULL;

	double temp;
	int index = 0;

	const static size_t start = wcslen(LINE_DATA_BEGIN.c_str());
	wstrVector* dataColumn = vectorContructor(data,L"\t",start,data.length());

	//�õ��ߵ��������
	wstring& column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &temp);
	this->m_LineID = (UINT)temp;

	m_LineName = (*dataColumn)[index++];
	m_LineKind = (*dataColumn)[index++];

	//�õ���ϸ��Ϣ
	m_LineBasiInfo = new LineCategoryItemData();

	m_LineBasiInfo->mCategory = (*dataColumn)[index++];
	m_LineBasiInfo->mShape = (*dataColumn)[index++];

	m_LineBasiInfo->mSize.mRadius = (*dataColumn)[index++];
	m_LineBasiInfo->mSize.mWidth = (*dataColumn)[index++];
	m_LineBasiInfo->mSize.mHeight = (*dataColumn)[index++];
	m_LineBasiInfo->mSize.mReservedA = (*dataColumn)[index++];
	m_LineBasiInfo->mSize.mReservedB = (*dataColumn)[index++];

	m_LineBasiInfo->mWallSize = (*dataColumn)[index++];
	m_LineBasiInfo->mSafeSize = (*dataColumn)[index++];
	m_LineBasiInfo->mPlaneMark = (*dataColumn)[index++];
	m_LineBasiInfo->mCutMark = (*dataColumn)[index++];
	m_LineBasiInfo->mThroughDirection = (*dataColumn)[index++];

	//����ڵ����ǰ��
	column = (*dataColumn)[index++];
	acdbDisToF(column.c_str(), -1, &temp);
	m_CurrentPointNO = (UINT)temp;

	//�õ�ÿ���������
	int size = (int)dataColumn->size();

	while( index < size )
	{
		column = (*dataColumn)[index++];

		if( column.length() > 3 )
			m_PointList->push_back(new PointEntity(column));
	}

	delete dataColumn;

	//�������ݿ�������
	m_pDbEntry = new LineDBEntity( this );
}

LineEntity::~LineEntity()
{
	//ClearPoints();
}

void LineEntity::ClearPoints()
{
	ClearPoints(this->m_PrePointList);
	ClearPoints(this->m_PointList);
}

void LineEntity::ClearPoints( PointList* pPointList)
{
	if( pPointList )
	{
		for( PointIter iter = pPointList->begin();
				iter != pPointList->end();
				iter++ )
		{
			if(*iter)
				delete *iter;
		}

		delete pPointList;
		pPointList = NULL;
	}
}

PointIter LineEntity::FindPoint( const UINT& PointNO ) const
{
	for( PointIter iter = this->m_PointList->begin();
			iter != this->m_PointList->end();
			iter++)
	{
		if( (*iter)->m_PointNO == PointNO )
			return iter;
	}

	return m_PointList->end();
}

ContstPointIter LineEntity::FindConstPoint( const UINT& PointNO ) const
{
	for( ContstPointIter iter = this->m_PointList->begin();
			iter != this->m_PointList->end();
			iter++)
	{
		if( (*iter)->m_PointNO == PointNO )
			return iter;
	}

	return m_PointList->end();
}

int LineEntity::InsertPoint( const PointEntity& newPoint )
{
	pPointEntry point = new PointEntity(newPoint);

	point->m_PointNO = m_CurrentPointNO;

	m_PointList->push_back(point);

	m_CurrentPointNO++;

	return (int)m_PointList->size();
}

void LineEntity::UpdatePoint( const PointEntity& updatePoint )
{
	PointIter findPoint = this->FindPoint(updatePoint.m_PointNO);

	if( findPoint != this->m_PointList->end() )
	{
		delete *findPoint;
		*findPoint = new PointEntity(updatePoint);
	}
}

void LineEntity::DeletePoint( const UINT& PointNO )
{
	PointIter findPoint = this->FindPoint(PointNO);

	if( findPoint != this->m_PointList->end() )
	{
		m_PointList->erase(findPoint);
	}
}

void LineEntity::SetBasicInfo( LineCategoryItemData* lineBasiInfo )
{
	if( m_LineBasiInfo )
		delete m_LineBasiInfo;

	m_LineBasiInfo = lineBasiInfo;
}

void LineEntity::SetPoints( PointList* newPoints)
{
	//���浱ǰ�Ľڵ��б�������ɾ����ǰ�Ķ���
	m_PrePointList = m_PointList;

	//�µ��б����ڴ����µ��߶�
	m_PointList = newPoints;

	Redraw();
}

wstring LineEntity::toString()
{
	wstring lineData(LINE_DATA_BEGIN);
	lineData += L"\r\n";

	CString temp;
	temp.Format(L"%d\t%s\t%s\t%s\t%d",
					m_LineID,m_LineName.c_str(),m_LineKind.c_str(),
					m_LineBasiInfo->toString().c_str(),
					(m_PointList != NULL ? m_PointList->size() : 0 ) );

	lineData += wstring(temp.GetBuffer());
	if( m_PointList )
	{
		for( ContstPointIter iter = m_PointList->begin();
				iter != m_PointList->end();
				iter++)
		{
			lineData += L"\t\r\n";
			lineData += (*iter)->toString();
		}
	}

	return lineData;
}

void LineEntity::Redraw()
{
	//ɾ����ǰ���߶�(�����ݿ���)
	EraseDbObjects(true);

	//ɾ����ǰ���߶�(���ڴ���)
	ClearPoints(m_PrePointList);
	m_PrePointList = NULL;

	//�����µ��߶�
	CreateDbObjects();
}

/**
 * ���ݵ����߶����ã��������߶�3D����
 **/
void LineEntity::CreateDbObjects()
{
	if( m_PointList == NULL || m_PointList->size() < 2 )
	{
		acutPrintf(L"\n����ʵ�塾%s�������߶�", m_LineName.c_str());
		return;
	}

	acutPrintf(L"\n��Ҫ���ơ�%d�������߶�",m_PointList->size()-1);

	try
	{
		//���ȴ���ͼ��
		ArxWrapper::createNewLayer(m_LineName);

		//����3Dģ��
		DrawDBEntity();
	}
	catch(const Acad::ErrorStatus es)
	{
		acutPrintf(L"\n�����߶η����쳣��");
		rxErrorMsg(es);
	}
}

/**
 * �������߶�3D����
 **/
void LineEntity::DrawDBEntity()
{
	ads_point *pStart = NULL;

	for( ContstPointIter iter = m_PointList->begin();
		iter != m_PointList->end();
		iter++)
	{
		if( pStart == NULL )
		{
			//���߶εĵ�һ�����
			pStart = &(*iter)->m_Point;
			
			//��ʼ��������Ϣ
			(*iter)->m_DbEntityCollection.mSequenceNO = 0;

			//����Ҫ������3Dģ��
			continue;
		}
		else
		{
			//����3D�������ֱ��
			(*iter)->CreateLineFrom( (void*)this, *pStart );

			//������һ���߶�
			pStart = &(*iter)->m_Point;
		}
	}
}

/**
 * ���ݶ��߶ε����ã�ɾ��3D����
 **/
void LineEntity::EraseDbObjects( bool old )
{
	PointList* pPointList = old ? m_PrePointList : m_PointList;
	if( pPointList == NULL || pPointList->size() < 2 )
	{
		acutPrintf(L"\n����û�С�%s�����߶�",(old ? L"ʧЧ" : L"��ǰ"));
		return;
	}

	acutPrintf(L"\nɾ�����ߡ�%s�����С�%s�����߶Σ�����%d����", m_LineName.c_str(),(old ? L"ʧЧ" : L"��ǰ"), (pPointList->size() - 1) );

	//��ֹ��ǰ�ĵ�
	ArxWrapper::LockCurDoc();

	for( ContstPointIter iter = pPointList->begin();
		iter != pPointList->end();
		iter++)
	{
		if( iter == pPointList->begin() )
		{
			continue;
		}
		else
		{
			(*iter)->m_DbEntityCollection.DropEntityCollection();
		}
	}

	//������ǰ�ĵ�
	ArxWrapper::UnLockCurDoc();
}

///////////////////////////////////////////////////////////////////////////
// Implementation LineDBEntity

ACRX_DXF_DEFINE_MEMBERS(LineDBEntity, AcDbObject, AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent, 0, LineDBEntity, LMA);

LineDBEntity::LineDBEntity()
{
	pImplemention = new LineEntity();
}

LineDBEntity::LineDBEntity( LineEntity* implementation )
{
	pImplemention = implementation;
}

// Files data in from a DWG file.
//
Acad::ErrorStatus
LineDBEntity::dwgInFields(AcDbDwgFiler* pFiler)
{
	if( LineEntityFileManager::openingDwg == false )
	{
		acutPrintf(L"\n���õ�ǰ״̬Ϊ���ڴ��ļ�");
		LineEntityFileManager::openingDwg = true;
	}

    assertWriteEnabled();

    AcDbObject::dwgInFields(pFiler);
    // For wblock filing we wrote out our owner as a hard
    // pointer ID so now we need to read it in to keep things
    // in sync.
    //
    if (pFiler->filerType() == AcDb::kWblockCloneFiler) {
        AcDbHardPointerId id;
        pFiler->readItem(&id);
    }

	if( !this->isErased() )
	{
		Adesk::UInt32 lineID;
		pFiler->readItem(&lineID);
		pImplemention->m_LineID = (UINT)lineID;

		TCHAR* tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!

		tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
		pFiler->readItem(&tmpStr);
		pImplemention->m_LineName = wstring(tmpStr);
		acutDelString(tmpStr);

		tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
		pFiler->readItem(&tmpStr);
		pImplemention->m_LineKind = wstring(tmpStr);
		acutDelString(tmpStr);

		{
			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mCategory = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mShape = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mSize.mRadius = wstring(tmpStr);
			acutDelString(tmpStr);
		
			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mSize.mWidth = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mSize.mHeight = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mSize.mReservedA = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mSize.mReservedB = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mWallSize = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mSafeSize = wstring(tmpStr);
			acutDelString(tmpStr);
		
			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mPlaneMark = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mCutMark = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mThroughDirection = wstring(tmpStr);
			acutDelString(tmpStr);
		}

		//�õ���������ݿ�ID
		pImplemention->m_dbId = this->id();

		CString filename;
		dbToStr(this->database(),filename);

#ifdef DEBUG
		acutPrintf(L"\n���ļ���%s����������ʵ�� ID��%d�����ơ�%s�����͡�%s��.",
					filename.GetBuffer(),
					pImplemention->m_LineID,
					pImplemention->m_LineName.c_str(),
					pImplemention->m_LineKind.c_str() );
#endif

		wstring fileName(filename.GetBuffer());
		LineEntityFile* entryFile = LineEntityFileManager::RegisterEntryFile(fileName);

		PointList* tempList = entryFile->TransferTempLine(pImplemention->m_LineID);
		if( tempList )
		{
			for( PointIter iter = tempList->begin();
				 iter != tempList->end();
				 iter++)
			{
				acutPrintf(L"\n׷�����߶Ρ�%s����š�%d��",(*iter)->m_DbEntityCollection.mLayerName.c_str(), (*iter)->m_PointNO);
				this->pImplemention->m_PointList->push_back( *(iter) );

				//���û�����Ϣ
				(*iter)->m_DbEntityCollection.mCategoryData = const_cast<LineCategoryItemData*>(this->pImplemention->GetBasicInfo());
			}

			delete tempList;
		}

#ifdef DEBUG
		acutPrintf(L"\n����ʱ���߹������еõ��߶����ݣ�����Ϊ��%d��", ( pImplemention->m_PointList ? pImplemention->m_PointList->size() : 0 ) );
#endif

		//������ļ��м������line
		entryFile->InsertLine(pImplemention);
	}

    return pFiler->filerStatus();
}

// Files data out to a DWG file.
//
Acad::ErrorStatus
LineDBEntity::dwgOutFields(AcDbDwgFiler* pFiler) const
{
    assertReadEnabled();

    AcDbObject::dwgOutFields(pFiler);
    // Since objects of this class will be in the Named
    // Objects Dictionary tree and may be hard referenced
    // by some other object, to support wblock we need to
    // file out our owner as a hard pointer ID so that it
    // will be added to the list of objects to be wblocked.
    //

    if (pFiler->filerType() == AcDb::kWblockCloneFiler)
        pFiler->writeHardPointerId((AcDbHardPointerId)ownerId());

#ifdef DEBUG
	acutPrintf(L"\n�������ʵ�嵽���ݿ� ID��%d�����ơ�%s�����͡�%s��.",
				pImplemention->m_LineID,
				pImplemention->m_LineName.c_str(),
				pImplemention->m_LineKind.c_str());
#endif

    pFiler->writeItem(Adesk::UInt32(pImplemention->m_LineID));
	pFiler->writeItem(pImplemention->m_LineName.c_str());
	pFiler->writeItem(pImplemention->m_LineKind.c_str());

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mCategory.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mShape.c_str());

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mSize.mRadius.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mSize.mWidth.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mSize.mHeight.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mSize.mReservedA.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mSize.mReservedB.c_str());

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mWallSize.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mSafeSize.c_str());

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mPlaneMark.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mCutMark.c_str());

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mThroughDirection.c_str());

    return pFiler->filerStatus();
}

// Files data in from a DXF file.
//
Acad::ErrorStatus
LineDBEntity::dxfInFields(AcDbDxfFiler* pFiler)
{
    assertWriteEnabled();

    Acad::ErrorStatus es;
    if ((es = AcDbObject::dxfInFields(pFiler))
        != Acad::eOk)
    {
        return es;
    }

    // Check if we're at the right subclass getLineID marker.
    //
    if (!pFiler->atSubclassData(_T("LineEntryData"))) {
        return Acad::eBadDxfSequence;
    }

    struct resbuf inbuf;
    while (es == Acad::eOk) {
        if ((es = pFiler->readItem(&inbuf)) == Acad::eOk) {

			/*
			switch ( inbuf.restype )
			{
				//case AcDb::kDxfInt16:
				//	m_LineID = inbuf.resval.rint;
				//case AcDb::kDxfInt16 + 1:
					//mSequenceNO = inbuf.resval.rint;
			}
			*/
        }
    }

    return pFiler->filerStatus();
}

// Files data out to a DXF file.
//
Acad::ErrorStatus
LineDBEntity::dxfOutFields(AcDbDxfFiler* pFiler) const
{
    assertReadEnabled();

    AcDbObject::dxfOutFields(pFiler);
    pFiler->writeItem(AcDb::kDxfSubclass, _T("LineEntryData"));
    //pFiler->writeItem(AcDb::kDxfInt16, mLineID);
	//pFiler->writeItem(AcDb::kDxfInt16 + 1, mSequenceNO);

    return pFiler->filerStatus();
}

} // end of data

} // end of assistant

} // end of guch

} // end of com