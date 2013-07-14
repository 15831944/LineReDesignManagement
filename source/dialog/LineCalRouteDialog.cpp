// LineCalRouteDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LineCalRouteDialog.h"
#include "LineIntersectManage.h"

#include "acedads.h"
#include "accmd.h"
#include <adscodes.h>

#include <adsdlg.h>

#include <dbapserv.h>

#include <dbregion.h>

#include <gepnt3d.h>

//symbol table
#include <dbsymtb.h>

#include <acdocman.h>

//3D Object
#include <dbsol3d.h>

// Hatch
#include <dbhatch.h>

// Leader
#include <dblead.h>
#include <dbmleader.h>

#include <ArxWrapper.h>

#include <ArxCustomObject.h>

#include <GlobalDataConfig.h>

#include <LMAUtils.h>

#pragma warning(disable:4482)

using namespace com::guch::assistant::arx;
using namespace com::guch::assistant::data;
using namespace com::guch::assistant::Intersect;

CString LineCalRouteDialog::m_lineCategory = L"�Զ�·�ɹ���";
CString LineCalRouteDialog::m_CutLayerName = L"";
CString LineCalRouteDialog::m_lineWidth = L"100";

LineCategoryItemData* LineCalRouteDialog::m_lineInfo = NULL;

AcDbObjectIdArray* LineCalRouteDialog::m_CutObjects = NULL;
LineEntity* LineCalRouteDialog::m_RouteLineEntity = NULL;
AcGePoint3dArray* LineCalRouteDialog::m_PointVertices = NULL;

LineEntityFile* LineCalRouteDialog::m_EntryFile = NULL;

// LineCalRouteDialog dialog

IMPLEMENT_DYNAMIC(LineCalRouteDialog, CAcUiDialog)

LineCalRouteDialog::LineCalRouteDialog(CWnd* pParent /*=NULL*/)
: CAcUiDialog(LineCalRouteDialog::IDD, pParent),
	m_startPoint(),
	m_endPoint()
{
	//�õ���ǰ������ĵ�
	m_fileName = curDoc()->fileName();
	acutPrintf(L"\n�ԡ�%s������Ĺ���ʵ��������·��.",m_fileName.c_str());

	//�õ�ʵ�������ļ��е�����
	m_EntryFile = LineEntityFileManager::RegisterEntryFile(m_fileName);
}

LineCalRouteDialog::~LineCalRouteDialog()
{
}

BOOL LineCalRouteDialog::OnInitDialog()
{
	//��ҳ�潻������
	CAcUiDialog::OnInitDialog();

	//Ĭ��ƫ��Ϊ0
	m_StartX.SetWindowTextW(L"0.00");
	m_StartY.SetWindowTextW(L"0.00");
	m_StartZ.SetWindowTextW(L"0.00");

	m_EndX.SetWindowTextW(L"0.00");
	m_EndY.SetWindowTextW(L"0.00");
	m_EndZ.SetWindowTextW(L"0.00");

	//����ͼƬ
	m_PickStart.AutoLoad();
	m_PickEnd.AutoLoad();

	return TRUE;
}

void LineCalRouteDialog::DoDataExchange(CDataExchange* pDX)
{
	CAcUiDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ROUTE_START_X, m_StartX);
	DDX_Control(pDX, IDC_ROUTE_START_Y, m_StartY);
	DDX_Control(pDX, IDC_ROUTE_START_Z, m_StartZ);

	DDX_Control(pDX, IDC_ROUTE_END_X, m_EndX);
	DDX_Control(pDX, IDC_ROUTE_END_Y, m_EndY);
	DDX_Control(pDX, IDC_ROUTE_END_Z, m_EndZ);

	DDX_Control(pDX, IDC_ROUTE_PICK_START,m_PickStart);
	DDX_Control(pDX, IDC_ROUTE_PICK_END,m_PickEnd);
}

BEGIN_MESSAGE_MAP(LineCalRouteDialog, CAcUiDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)

	ON_BN_CLICKED(IDC_ROUTE_PICK_START, &LineCalRouteDialog::OnBnPickStartClicked)
	ON_BN_CLICKED(IDC_ROUTE_PICK_END, &LineCalRouteDialog::OnBnPickEndClicked)
END_MESSAGE_MAP()

void LineCalRouteDialog::OnBnPickStartClicked()
{
	// Hide the dialog and give control to the editor
	BeginEditorCommand();

	CString temp;
	ads_point pt;

	// Get a point
	if (acedGetPoint(NULL, _T("\nѡȡ��ʼ��: "), pt) == RTNORM) 
	{
		// If the point is good, continue
		CompleteEditorCommand();

		temp.Format(_T("%g"), pt[X]);
		m_StartX.SetWindowTextW(temp.GetBuffer());

		temp.Format(_T("%g"), pt[Y]);
		m_StartY.SetWindowTextW(temp.GetBuffer());

		temp.Format(_T("%g"), pt[Z]);
		m_StartZ.SetWindowTextW(temp.GetBuffer());

		m_startPoint.set(pt[X], pt[Y], pt[Z]);
	}
	else 
	{
		// otherwise cancel the command (including the dialog)
		CancelEditorCommand();
	}

	UpdateData(FALSE);
}

void LineCalRouteDialog::OnBnPickEndClicked()
{
	// Hide the dialog and give control to the editor
	BeginEditorCommand();

	CString temp;
	ads_point pt;

	// Get a point
	if (acedGetPoint(NULL, _T("\nѡȡ��ֹ��: "), pt) == RTNORM) 
	{
		// If the point is good, continue
		CompleteEditorCommand();

		temp.Format(_T("%g"), pt[X]);
		m_EndX.SetWindowTextW(temp.GetBuffer());

		temp.Format(_T("%g"), pt[Y]);
		m_EndY.SetWindowTextW(temp.GetBuffer());

		temp.Format(_T("%g"), pt[Z]);
		m_EndZ.SetWindowTextW(temp.GetBuffer());

		m_endPoint.set(pt[X], pt[Y], pt[Z]);
	}
	else 
	{
		// otherwise cancel the command (including the dialog)
		CancelEditorCommand();
	}

	UpdateData(FALSE);
}

void LineCalRouteDialog::GetStartEndPoint()
{
	CString pointX,pointY,pointZ;

	m_StartX.GetWindowTextW(pointX);
	acdbDisToF(pointX.GetBuffer(), -1, &m_startPoint.x);

	m_StartY.GetWindowTextW(pointY);
	acdbDisToF(pointY.GetBuffer(), -1, &m_startPoint.y);

	m_StartZ.GetWindowTextW(pointZ);
	acdbDisToF(pointZ.GetBuffer(), -1, &m_startPoint.z);

	m_EndX.GetWindowTextW(pointX);
	acdbDisToF(pointX.GetBuffer(), -1, &m_endPoint.x);

	m_EndY.GetWindowTextW(pointY);
	acdbDisToF(pointY.GetBuffer(), -1, &m_endPoint.y);

	m_EndZ.GetWindowTextW(pointZ);
	acdbDisToF(pointZ.GetBuffer(), -1, &m_endPoint.z);

	//��ǰϵͳ���㰴�մ������ϼ���
	//���·�Ϊ��ʼ�㣬�Ϸ�Ϊ��ֹ��
	if( m_startPoint.y > m_endPoint.z )
	{
		AcGePoint3d swap(m_endPoint);

		m_endPoint.set( m_startPoint.x, m_startPoint.y, m_startPoint.z );
		m_startPoint.set( swap.x, swap.y, swap.z );
	}
}

void LineCalRouteDialog::OnBnClickedOk()
{
	//�õ��û����������
	UpdateData(FALSE);

	//�õ���ʼ����ֹ��
	GetStartEndPoint();

	//���Ȼָ���ͼ
	CutBack();

	//����ǰ��������·�����ڵ�ͼ�㡶���ơ���·��������Ĺ��ߣ�
	SetupRouteLineEnv();

	//��������֮���(�ӽ�)���·��;
	CalculateShortestRoute();

	//�������ռ�����Ľ��
	SetupRouteResult();

	//�رնԻ���
	CAcUiDialog::OnOK();
}

bool LineCalRouteDialog::SetupRouteLineEnv()
{
	//����ͼ�������
	m_CutLayerName.Format(L"��(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)��(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)�����·��",m_startPoint[X],m_startPoint[Y],m_startPoint[Z],m_endPoint[X],m_endPoint[Y],m_endPoint[Z]);
	acutPrintf(L"\nҪ������ͼ������Ϊ��%s��",m_CutLayerName.GetBuffer());
	m_CutLayerName.Format(L"���·��");

	//������ʼ����ֹ����X�ᴹֱ��ƽ��
	InitializeProjectPlace();

	//��������·�ɵĹ���ʵ��
	InitializeRouteLine();

	return true;
}

void LineCalRouteDialog::InitializeProjectPlace()
{
	acutPrintf(L"\n���·�����ڵ���X�ᴹֱ����");

	//�õ���ʼ�㴹ֱ��X��,��ֱ����10000������
	AcGePoint3d projectPoint( m_startPoint );
	projectPoint.z -= 100;

	m_ProjectPlane = AcGePlane( m_startPoint, m_endPoint, projectPoint);
}

bool LineCalRouteDialog::InitializeRouteLine()
{
	acutPrintf(L"\n��ʼ��·�����ڵĹ�����Ϣ");

	//��������detail��Ϣ
	InitializeRouteLineInfo();

	//�õ�ʵ������
	wstring pipeName(m_CutLayerName.GetBuffer());
	acutPrintf(L"\n�µ�·������ͼ�������Ϊ��%s��", pipeName.c_str());

	//�����µ�·�ɹ���
	acutPrintf(L"\n�����µ�·�ɹ���");
	m_RouteLineEntity = new LineEntity(pipeName,GlobalData::CONFIG_LINE_KIND, m_lineInfo ,NULL);

	//���ɸ����ID
	m_RouteLineEntity->m_LineID = (UINT)GetTickCount();

	//���浽���ݿ�
	m_RouteLineEntity->m_dbId = ArxWrapper::PostToNameObjectsDict(m_RouteLineEntity->m_pDbEntry,LineEntity::LINE_ENTRY_LAYER);

	//������ݿ����ָ�룬��AutoCAD����
	m_RouteLineEntity->m_pDbEntry = NULL;

	//�������ݵ�������
	m_EntryFile->InsertLine(m_RouteLineEntity);

	return true;
}

//��ʼ�������Զ�·�ɵĻ�����Ϣ
bool LineCalRouteDialog::InitializeRouteLineInfo()
{
	acutPrintf(L"\n��ʼ��·�ɵĻ�����Ϣ");

	if( m_lineInfo == NULL )
	{
		CString lineWidth,lineHeight,lineReservedA,lineReservedB,
			lineSafeSize,lineWallSize,lineThroughDirect,
			linePlaneDesc,lineCutDesc;
	
		lineWidth = L"0";
		lineHeight = L"0";
		lineReservedA = L"0";
		lineReservedB = L"0";

		//׼���������ݽṹ��
		LineCategoryItemData* categoryData = new LineCategoryItemData();

		categoryData->mCategory = wstring(LineCalRouteDialog::m_lineCategory.GetBuffer());
		categoryData->mShape = GlobalData::LINE_SHAPE_CIRCLE;

		categoryData->mSize.mRadius = wstring(LineCalRouteDialog::m_lineWidth.GetBuffer());
		categoryData->mSize.mWidth = wstring(lineWidth.GetBuffer());
		categoryData->mSize.mHeight = wstring(lineHeight.GetBuffer());
		categoryData->mSize.mReservedA = wstring(lineReservedA.GetBuffer());
		categoryData->mSize.mReservedB = wstring(lineReservedB.GetBuffer());

		categoryData->mWallSize = wstring(LineCalRouteDialog::m_lineWidth.GetBuffer());
		categoryData->mSafeSize = wstring(LineCalRouteDialog::m_lineWidth.GetBuffer());

		categoryData->mPlaneMark = wstring(linePlaneDesc.GetBuffer());
		categoryData->mCutMark = wstring(lineCutDesc.GetBuffer());

		categoryData->mThroughDirection = wstring(lineThroughDirect.GetBuffer());

		//����������߻�����Ϣ
		m_lineInfo = categoryData;
	}

	return true;
}

bool LineCalRouteDialog::InitializeStartEndPoints( const AcGePoint3d& startPoint, const AcGePoint3d& endPoint )
{
	acutPrintf(L"\n��3Dģ���л����������ߣ����бȽϼ���");

	//���û�ѡ��Ŀ�ʼ���������ʼ����������
	AppendStartEndPoints( startPoint, endPoint);

	//Ĭ���û�ѡ��Ŀ�ʼ����Ա���������
	SaveRouteLinePoint( startPoint );

	//û�м�������߶�
	m_CheckedEntities.clear();

	return true;
}

bool LineCalRouteDialog::SaveRouteLinePoint( const AcGePoint3d& newPoint )
{
	acutPrintf(L"\n����(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)Ϊһ�����", newPoint[X], newPoint[Y], newPoint[Z]);

	if( m_PointVertices == NULL )
		m_PointVertices = new AcGePoint3dArray();

	m_PointVertices->append( newPoint);

	return true;
}

bool LineCalRouteDialog::AppendStartEndPoints(const AcGePoint3d& startPoint, const AcGePoint3d& endPoint)
{
	PointEntity* point = NULL;

	PointList* newPoints = new PointList();

	point = new PointEntity();
	point->m_PointNO =  0;
	point->m_Point[X] = startPoint[X];
	point->m_Point[Y] = startPoint[Y];
	point->m_Point[Z] = startPoint[Z];

	newPoints->push_back( point );

	point = new PointEntity();
	point->m_PointNO =  1;

	point->m_Point[X] = endPoint[X];
	point->m_Point[Y] = endPoint[Y];
	point->m_Point[Z] = endPoint[Z];

	newPoints->push_back( point );

	//�Դ˿�ʼ�ͽ����㴴���µ�·���߶�
	m_RouteLineEntity->SetPoints(newPoints);

	acutPrintf(L"\n�µ�·���߶ι������");

	return true;
}

/**
 * ����������ʼ����ֹ��֮����߶Σ�Ȼ���ж������й����Ƿ����֡�
 * �õ����Լ�����������ֵĹ��ߡ�
 * �ڴ˹��ߴ����õ���ϳ��߶δ�ֱ�����ϸ߶�Ϊ�뾶�ĵ㣬������ʼ����˵㣬�õ���һ���߶Ρ�
 * Ȼ���ڴ˵�����µ���ʼ��,�ظ�����ļ�����̣��ݹ飩��ֱ�������еĹ��߲������ˣ�������ˡ�
 */
void LineCalRouteDialog::CalculateShortestRoute()
{
	acutPrintf(L"\n��ʼ��������·��");

	//���û�ѡ�����ʵ�㿪ʼ
	m_newStartPoint = m_startPoint;

	int count = 0;

	//�ݹ����
	while( CalculateShortestRoute( m_newStartPoint, m_endPoint ) == false )
	{
		acutPrintf(L"\n���ֵ�ǰ������ϵͳ�е��߶������ֵ����󣬼�������");

		if( count++ >= 1000 )
			break;
	}
}

bool LineCalRouteDialog::CalculateShortestRoute( const AcGePoint3d& start, const AcGePoint3d& end)
{
	acutPrintf(L"\n���ڼ���(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)��(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)�����·��", start[X], start[Y], start[Z], end[X], end[Y], end[Z]);

	//�Կ�ʼ��ͽ�ֹ�㴴��Ĭ�Ͽ�ȵĹ���
	InitializeStartEndPoints(start, end);

	//���ֵĹ���
	AcArray<PointEntity*>* intersectEntities = new AcArray<PointEntity*>();

	//�뵱ǰϵͳ�ڵĹ����ж�
	CheckIntersect(intersectEntities);
	acutPrintf(L"�ཻ�Ĺ����С�%d����",intersectEntities->length());

	if( intersectEntities->length() > 0  )
	{
		//�й������֣��õ������һ��
		PointEntity* nearestLine = GetNearestLineSegement(intersectEntities);

		//�õ������
		AcGePoint3d newPoint = GetProjectPoint3d(nearestLine);

		//�õ�������������֮����߶Σ����뵽ͼ���У�����false����
		m_newStartPoint = newPoint;

		//ɾ���м���
		intersectEntities->removeAll();
		delete intersectEntities;
		intersectEntities = NULL;

		return false;
	}
	else
	{
		//���û�й������֣���ֱ�Ӽ��뵽ͼ���У�����true����
		return true;
	}
}

PointEntity* LineCalRouteDialog::GetNearestLineSegement( AcArray<PointEntity*>* intersectEntities )
{
	acutPrintf(L"\nѰ����X��������ཻ����");

	if( intersectEntities == NULL )
		return NULL;

	if( intersectEntities->length() == 1 )
		return intersectEntities->at(0);

	PointEntity* nearestEntity = NULL;
	AcGePoint3d nearestPoint;
	bool findNearer = false;

	for( int i = 0; i < intersectEntities->length(); i++ )
	{
		PointEntity* pointEntity = intersectEntities->at(i);
		AcGePoint3d start = pointEntity->m_DbEntityCollection.mStartPoint;
		AcGePoint3d end = pointEntity->m_DbEntityCollection.mEndPoint;
		AcGeLine3d intersectLine( start, end ); 

		AcGePoint3d resultPnt;
		if( m_ProjectPlane.intersectWith(intersectLine, resultPnt) )
		{
			acutPrintf(L"\n����Ϊ��X:%0.2lf,Y:%0.2lf,Z:%0.2lf��", resultPnt[X], resultPnt[Y], resultPnt[Z]);
			if( nearestEntity == NULL ||  resultPnt[Y] <nearestPoint[Y] )
			{
				acutPrintf(L"\n�ϴ��������Ϊ��X:%0.2lf,Y:%0.2lf,Z:%0.2lf��", nearestPoint[X], nearestPoint[Y], nearestPoint[Z]);
				nearestEntity = pointEntity;
				nearestPoint = resultPnt;

				acutPrintf(L"\n���ߡ�%s���ĵڡ�%d��������Ϊ�������ཻ��",nearestEntity->m_DbEntityCollection.mLayerName.c_str(), nearestEntity->m_PointNO);
			}				
		}
	}

	//һ��ĳ���߶α�Խ���������ظ�����
	if( nearestEntity )
	{
		m_CheckedEntities.insert( LinePointID(nearestEntity->m_DbEntityCollection.mLineID, nearestEntity->m_DbEntityCollection.mSequenceNO) );
	}

	return nearestEntity;
}

AcGePoint3d LineCalRouteDialog::GetProjectPoint3d(PointEntity* lineSegment)
{
	acutPrintf(L"\n�õ����ߵĴ�ֱ��X��ĵ�");

	AcGePoint3d start = lineSegment->m_DbEntityCollection.mStartPoint;
	AcGePoint3d end = lineSegment->m_DbEntityCollection.mEndPoint;
	AcGeLine3d intersectLine( start, end ); 

	AcGePoint3d resultPnt;
	m_ProjectPlane.intersectWith(intersectLine, resultPnt);

	acutPrintf(L"\n�õ����ߵ��ཻ�㡾X:%0.2lf,Y:%0.2lf,Z:%0.2lf��", resultPnt[X], resultPnt[Y], resultPnt[Z]);

	double wallSize = 0.0;
	acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mWallSize.c_str(), -1, &wallSize);
	wallSize /= 1000;

	double safeSize = 0.0;
	acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSafeSize.c_str(), -1, &safeSize);
	safeSize /= 1000;

	double yOffset = 0.0;
	double zOffset = 0.0;

	if( lineSegment->m_DbEntityCollection.mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		//Բ��Ļ���ȡ�뾶ΪY,Z���ƫ��
		acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSize.mRadius.c_str(), -1, &yOffset);
		yOffset /= 1000;
		yOffset += wallSize;
		yOffset += safeSize;

		zOffset = yOffset;
	}
	else
	{
		//����Ļ���ȡ���ΪY���ƫ��
		acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSize.mWidth.c_str(), -1, &yOffset);
		yOffset /= 1000;
		yOffset += wallSize;
		yOffset += safeSize;

		//ȡ�߶ȵ�һ��ΪZ���ƫ��
		acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSize.mHeight.c_str(), -1, &zOffset);
		zOffset /= 2000;
		zOffset += wallSize;
		zOffset += safeSize;
	}

	AcGePoint3d projectPoint;
	projectPoint.x = resultPnt.x;
	projectPoint.y = resultPnt.y - yOffset;
	projectPoint.z = resultPnt.z + zOffset;

	//������X��Ͻ��ĵ�
	acutPrintf(L"\n�洢�·��ĵ㡾X:%0.2lf,Y:%0.2lf,Z:%0.2lf��Ϊһ�����", projectPoint[X], projectPoint[Y], projectPoint[Z]);
	m_PointVertices->append( projectPoint );

	projectPoint.y = resultPnt.y + yOffset;

	acutPrintf(L"\n�����Ϸ��ĵ㡾X:%0.2lf,Y:%0.2lf,Z:%0.2lf��Ϊ�µļ������", projectPoint[X], projectPoint[Y], projectPoint[Z]);

	return projectPoint;
}

//�ж�һ�����߶����������ߵ��������
void LineCalRouteDialog::CheckIntersect(AcArray<PointEntity*>* intersectEntities)
{
	acutPrintf(L"\n���������ж�.");

	PointList* pointList = m_RouteLineEntity->m_PointList;
	if( pointList == NULL 
		|| pointList->size() < 2 )
	{
		acutPrintf(L"\n��ǰ����û�����߶Σ����Բ����м��");
		return;
	}

	PointEntity* checkPoint = (*pointList)[1];

	wstring& lineName = checkPoint->m_DbEntityCollection.mLayerName;
	Adesk::Int32& checkLineID = checkPoint->m_DbEntityCollection.mLineID;
	Adesk::Int32& checkSeqNO = checkPoint->m_DbEntityCollection.mSequenceNO;

#ifdef DEBUG
	acutPrintf(L"\n�ԡ�%s���ĵڡ�%d�������������ж�.",lineName.c_str(), checkSeqNO);
#endif

	LineList* lineList = m_EntryFile->GetList();
	for( LineIterator line = lineList->begin();
			line != lineList->end();
			line++ )
	{
		PointList* pointList = (*line)->m_PointList;
		if( pointList == NULL 
			|| pointList->size() == 0 )
		{
			acutPrintf(L"\n��ǰ����û�����߶Σ�����Ҫ������߶ν��������ж�");
			continue;
		}

		for( PointIter point = pointList->begin();
				point != pointList->end();
				point++ )
		{
			Adesk::Int32& lineID = (*point)->m_DbEntityCollection.mLineID;
			Adesk::Int32& seqNO = (*point)->m_DbEntityCollection.mSequenceNO;
			if( seqNO == 0 )
			{
				acutPrintf(L"\n������ʼ�㣬����Ҫ�ж�");
				continue;
			}

			acutPrintf(L"\n�롾%s���ĵڡ�%d�������߶ν����ж�",(*point)->m_DbEntityCollection.mLayerName.c_str(), seqNO );

			if( lineID == checkLineID && abs( seqNO - checkSeqNO ) <= 1 )
			{
				acutPrintf(L"\n�����߶�,�����������ж�");
				continue;
			}

			if( m_CheckedEntities.find(LinePointID(lineID,seqNO)) != m_CheckedEntities.end() )
			{
				acutPrintf(L"\n�����߶��ѱ��ȽϹ�,����");
				continue;
			}

			ArxWrapper::LockCurDoc();

			//�õ������߶ε����ݿⰲȫ��Χ����
			AcDbEntity *pSafeLine = ArxWrapper::GetDbObject( (*point)->m_DbEntityCollection.GetSafeLineEntity(), true );
			if( pSafeLine == NULL )
			{
				acutPrintf(L"\n����������޵Ĺ��ߵİ�ȫ��Χʵ��ʱ����");
				ArxWrapper::UnLockCurDoc();
				continue;
			}

			AcDbEntity *pCheckSafeLine = ArxWrapper::GetDbObject( checkPoint->m_DbEntityCollection.GetSafeLineEntity(), true );
			if( pCheckSafeLine == NULL )
			{
				if( pSafeLine )
					pSafeLine->close();

				acutPrintf(L"\n�õ���������޵Ĺ��ߵİ�ȫ��Χʵ��ʱ����");
				ArxWrapper::UnLockCurDoc();
				continue;
			}

			//�ж�2���Ƿ�����
			AcDb3dSolid* intersetObj = ArxWrapper::GetInterset( pSafeLine, pCheckSafeLine );

			if( intersetObj != NULL )
			{
				acutPrintf(L"\n�롾%s���ĵڡ�%d�������߶����ޣ���Ҫ��¼������",(*point)->m_DbEntityCollection.mLayerName.c_str(), seqNO );

				//��������
				intersectEntities->append(*point);
			}
			else
			{
				acutPrintf(L"\n�롾%s���ĵڡ�%d�������߶�û�����֣�",(*point)->m_DbEntityCollection.mLayerName.c_str(), seqNO );
			}

			pSafeLine->close();
			pCheckSafeLine->close();

			ArxWrapper::UnLockCurDoc();
		}
	}
}

bool LineCalRouteDialog::CreateRouteSegment( const AcGePoint3d& start, const AcGePoint3d& end)
{
	return true;
}

void LineCalRouteDialog::SetupRouteResult()
{
	m_PointVertices->append( m_endPoint);
	acutPrintf(L"\n�������յ�·�ɽ�������߶Ρ�%d����",m_PointVertices->length()-1);

	PointList* newPoints = new PointList();

	CString temp;
	for( int i = 0; i < m_PointVertices->length(); i++ )
	{
		PointEntity* point = new PointEntity();
		point->m_PointNO = i;
		
		point->m_Point[X] = m_PointVertices->at(i).x;
		point->m_Point[Y] = m_PointVertices->at(i).y;
		point->m_Point[Z] = m_PointVertices->at(i).z;

		newPoints->push_back( point );
	}

	m_RouteLineEntity->SetPoints( newPoints );
}

void LineCalRouteDialog::Reset()
{
	acutPrintf(L"\n���ڵ���ʱͼ��Ϊ��%s����",m_CutLayerName.GetBuffer());

	if( m_CutLayerName.GetLength() > 0 )
	{
		acutPrintf(L"\n�����������ĵ�");
		ArxWrapper::LockCurDoc();

		//acutPrintf(L"\n�ָ�WCS�Ӵ�");
		acedCommand(RTSTR, _T("UCS"), RTSTR, L"W", 0);

		acutPrintf(L"\nɾ������·����صĶ���");
		CutBack();

		acutPrintf(L"\nɾ���������·�����ڵ�ͼ��");
		if( ArxWrapper::DeleteLayer(m_CutLayerName.GetBuffer(),true) )
		{
			acutPrintf(L"\n��ʼ������");
			m_CutLayerName.Format(L"");
		}

		acutPrintf(L"\n��ʾ����ͼ��");
		ArxWrapper::ShowLayer(L"");

		acutPrintf(L"\n����ĵ�����");
		ArxWrapper::UnLockCurDoc();

		acutPrintf(L"\n�л�������");
		ArxWrapper::ChangeView(3);
	}
	else
	{
		acutPrintf(L"\n��ǰϵͳ��û����ͼ��");
	}
}

void LineCalRouteDialog::CutBack()
{
	//ɾ��ǰһ������·��ʱ�Ĺ���ʵ��
	if( m_RouteLineEntity )
	{
		//�����ݿ�ɾ�����߱���
		ArxWrapper::DeleteFromNameObjectsDict(m_RouteLineEntity->m_dbId,LineEntity::LINE_ENTRY_LAYER);

		//�����ݿ�ɾ���������е��߶�
		m_RouteLineEntity->EraseDbObjects();

		//ɾ�����е��ڴ�ڵ�
		m_RouteLineEntity->ClearPoints();

		//ɾ���߶μ���
		m_EntryFile->DeleteLine(m_RouteLineEntity->GetLineID());

		//ɾ�����ߵ㼯��
		m_PointVertices->removeAll();

		delete m_RouteLineEntity;
	}
}

// LineCalRouteDialog message handlers
