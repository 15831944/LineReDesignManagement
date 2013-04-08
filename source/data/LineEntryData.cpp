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
// Implementation LineEntry

const wstring LineEntry::LINE_ENTRY_LAYER = L"����ʵ���ֵ�";

/**
 * ����ʵ��
 */

LineEntry::LineEntry()
	:m_LineID(0),
	m_LineName(L""),
	m_LineKind(L""),
	m_LineBasiInfo(new LineCategoryItemData()),
	m_CurrentPointNO(0),
	m_PrePointList(NULL),
	m_PointList(new PointList())
{}

LineEntry::LineEntry(const wstring& rLineName, const wstring& rLineKind,
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
	m_pDbEntry = new LineDBEntry( this );
}

LineEntry::LineEntry( const wstring& data)
{
	m_PointList = new PointList();
	m_PrePointList = NULL;

	double temp;
	int index = 0;

	wstrVector* dataColumn = vectorContructor(data,L"\t");

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
	m_LineBasiInfo->mRadius = (*dataColumn)[index++];
	m_LineBasiInfo->mWidth = (*dataColumn)[index++];
	m_LineBasiInfo->mHeight = (*dataColumn)[index++];
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
		m_PointList->push_back(new PointEntry(column));
	}

	delete dataColumn;

	//�������ݿ�������
	m_pDbEntry = new LineDBEntry( this );
}

LineEntry::~LineEntry()
{
	//ClearPoints();
}

void LineEntry::ClearPoints()
{
	ClearPoints(this->m_PrePointList);
	ClearPoints(this->m_PointList);
}

void LineEntry::ClearPoints( PointList* pPointList)
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

PointIter LineEntry::FindPoint( const UINT& PointNO ) const
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

ContstPointIter LineEntry::FindConstPoint( const UINT& PointNO ) const
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

int LineEntry::InsertPoint( const PointEntry& newPoint )
{
	pPointEntry point = new PointEntry(newPoint);

	point->m_PointNO = m_CurrentPointNO;

	m_PointList->push_back(point);

	m_CurrentPointNO++;

	return (int)m_PointList->size();
}

void LineEntry::UpdatePoint( const PointEntry& updatePoint )
{
	PointIter findPoint = this->FindPoint(updatePoint.m_PointNO);

	if( findPoint != this->m_PointList->end() )
	{
		delete *findPoint;
		*findPoint = new PointEntry(updatePoint);
	}
}

void LineEntry::DeletePoint( const UINT& PointNO )
{
	PointIter findPoint = this->FindPoint(PointNO);

	if( findPoint != this->m_PointList->end() )
	{
		m_PointList->erase(findPoint);
	}
}

void LineEntry::SetBasicInfo( LineCategoryItemData* lineBasiInfo )
{
	if( m_LineBasiInfo )
		delete m_LineBasiInfo;

	m_LineBasiInfo = lineBasiInfo;
}

void LineEntry::SetPoints( PointList* newPoints)
{
	//���浱ǰ�Ľڵ��б�������ɾ����ǰ�Ķ���
	m_PrePointList = m_PointList;

	//�µ��б����ڴ����µ��߶�
	m_PointList = newPoints;

	Redraw();
}

wstring LineEntry::toString()
{
	wstring lineData;

	CString temp;
	temp.Format(L"%d\t%s\t%s\t%s\t%d",m_LineID,m_LineName.c_str(),m_LineKind.c_str(),
					m_LineBasiInfo->toString().c_str(),
					m_CurrentPointNO);

#ifdef DEBUG
	//acutPrintf(L"\n����ʵ�����л�Ϊ��%s��",temp.GetBuffer());
#endif

	lineData = temp;

	if( this->m_PointList )
	{
		for( ContstPointIter iter = this->m_PointList->begin();
				iter != this->m_PointList->end();
				iter++)
		{
			lineData += L"\t";
			lineData += (*iter)->toString();
		}
	}

	return lineData;
}

void LineEntry::Redraw()
{
	//ɾ����ǰ���߶�(�����ݿ���)
	EraseDbObjects(true);

	//ɾ����ǰ���߶�(���ڴ���)
	ClearPoints(m_PrePointList);

	//�����µ��߶�
	CreateDbObjects();
}

/**
 * ���ݵ����߶����ã��������߶�3D����
 **/
void LineEntry::CreateDbObjects()
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
void LineEntry::DrawDBEntity()
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
void LineEntry::EraseDbObjects( bool old )
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
// Implementation LineDBEntry

ACRX_DXF_DEFINE_MEMBERS(LineDBEntry, AcDbObject, AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent, 0, LineDBEntry, LMA);

LineDBEntry::LineDBEntry()
{
	pImplemention = new LineEntry();
}

LineDBEntry::LineDBEntry( LineEntry* implementation )
{
	pImplemention = implementation;
}

// Files data in from a DWG file.
//
Acad::ErrorStatus
LineDBEntry::dwgInFields(AcDbDwgFiler* pFiler)
{
	if( LineEntryFileManager::openingDwg == false )
	{
		acutPrintf(L"\n���õ�ǰ״̬Ϊ���ڴ��ļ�");
		LineEntryFileManager::openingDwg = true;
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
			pImplemention->m_LineBasiInfo->mRadius = wstring(tmpStr);
			acutDelString(tmpStr);
		
			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mWidth = wstring(tmpStr);
			acutDelString(tmpStr);

			tmpStr = NULL;    // must explicitly set to NULL or readItem() crashes!
			pFiler->readItem(&tmpStr);
			pImplemention->m_LineBasiInfo->mHeight = wstring(tmpStr);
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
		LineEntryFile* entryFile = LineEntryFileManager::RegisterEntryFile(fileName);

		if( pImplemention->m_PointList )
			delete pImplemention->m_PointList;

		this->pImplemention->m_PointList = entryFile->TransferTempLine(pImplemention->m_LineID);

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
LineDBEntry::dwgOutFields(AcDbDwgFiler* pFiler) const
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

	pFiler->writeItem(pImplemention->m_LineBasiInfo->mRadius.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mWidth.c_str());
	pFiler->writeItem(pImplemention->m_LineBasiInfo->mHeight.c_str());

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
LineDBEntry::dxfInFields(AcDbDxfFiler* pFiler)
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
LineDBEntry::dxfOutFields(AcDbDxfFiler* pFiler) const
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