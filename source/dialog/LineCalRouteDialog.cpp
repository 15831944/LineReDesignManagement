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

void LineCalRouteDialog::OnBnClickedOk()
{
	//�õ��û����������
	UpdateData(FALSE);

	//���Ȼָ���ͼ
	CutBack();

	//����ǰ��������·�����ڵ�ͼ�㡶���ơ���·��������Ĺ��ߣ�
	SetupRouteLineEnv();

	//��������֮���(�ӽ�)���·��;
	CalculateShortestRoute();

	//�رնԻ���
	CAcUiDialog::OnOK();
}

bool LineCalRouteDialog::SetupRouteLineEnv()
{
	//����ͼ�������
	m_CutLayerName.Format(L"�ӡ�X:%0.2lf,Y:%0.2lf,Z:%0.2lf������X:%0.2lf,Y:%0,2lf,Z:%0.2lf�������·��",m_startPoint[X],m_startPoint[Y],m_startPoint[Z],m_endPoint[X],m_endPoint[Y],m_endPoint[Z]);

	//��������·�ɵĹ���ʵ��
	InitializeRouteLine();

	return true;
}

bool LineCalRouteDialog::InitializeRouteLine()
{
	//��������detail��Ϣ
	InitializeRouteLineInfo();

	//�õ�ʵ������
	wstring pipeName = m_EntryFile->GetNewPipeName(m_lineInfo, L"");

	//�����µ�·�ɹ���
	LineEntity* newLine = new LineEntity(pipeName,GlobalData::CONFIG_LINE_KIND, m_lineInfo ,NULL);

	m_RouteLineEntity = new LineEntity();

	return true;
}

//��ʼ�������Զ�·�ɵĻ�����Ϣ
bool LineCalRouteDialog::InitializeRouteLineInfo()
{
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

		categoryData->mWallSize = wstring(lineWallSize.GetBuffer());
		categoryData->mSafeSize = wstring(lineSafeSize.GetBuffer());

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
	//���û�ѡ��Ŀ�ʼ���������ʼ����������
	AppendStartEndPoints( startPoint, endPoint);

	//Ĭ���û�ѡ��Ŀ�ʼ����Ա���������
	SaveRouteLinePoint( startPoint );

	return true;
}

bool LineCalRouteDialog::SaveRouteLinePoint( const AcGePoint3d& newPoint )
{
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
	point->m_Point[X] = endPoint[X];
	point->m_Point[X] = endPoint[X];
	point->m_Point[X] = endPoint[X];

	newPoints->push_back( point );

	//�Դ˿�ʼ�ͽ����㴴���µ�·���߶�
	m_RouteLineEntity->SetPoints(newPoints);

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
	//���û�ѡ�����ʵ�㿪ʼ
	m_newStartPoint = m_startPoint;

	//�ݹ����
	while( CalculateShortestRoute( m_newStartPoint, m_endPoint ) == false )
	{
		acutPrintf(L"\n�������һ�����ߵ��߶������ֵ����󣬼�������");
	}
}

bool LineCalRouteDialog::CalculateShortestRoute( const AcGePoint3d& start, const AcGePoint3d& end)
{
	//�Կ�ʼ��ͽ�ֹ�㴴��Ĭ�Ͽ�ȵĹ���
	InitializeStartEndPoints(start, end);

	//���ֵĹ���
	AcArray<PointEntity*>* intersectEntities = new AcArray<PointEntity*>();

	//�뵱ǰϵͳ�ڵĹ����ж�
	if( HasIntersect(intersectEntities) )
	{
		//�й������֣��õ������һ��
		PointEntity* nearestLine = GetNearestLineSegement(intersectEntities);

		//�õ������
		AcGePoint3d newPoint = GetProjectPoint3d(nearestLine);

		//�õ�������������֮����߶Σ����뵽ͼ���У�����false����
		m_newStartPoint = newPoint;

		//ɾ���м���
		delete intersectEntities;

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
	if( intersectEntities->length() == 1 )
		return intersectEntities->at(0);

	return NULL;
}

AcGePoint3d LineCalRouteDialog::GetProjectPoint3d(PointEntity* lineSegment)
{
	AcGePoint3d projectPoint;

	return projectPoint;
}

//�ж�һ�����߶����������ߵ��������
bool LineCalRouteDialog::HasIntersect(AcArray<PointEntity*>* intersectEntities)
{
	PointList* pointList = m_RouteLineEntity->m_PointList;
	if( pointList == NULL 
		|| pointList->size() < 2 )
	{
		acutPrintf(L"\n��ǰ����û�����߶Σ����Բ����м��");
		return true;
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

			pSafeLine->close();
			pCheckSafeLine->close();

			ArxWrapper::UnLockCurDoc();
		}
	}

	return false;
}

bool LineCalRouteDialog::CreateRouteSegment( const AcGePoint3d& start, const AcGePoint3d& end)
{
	return true;
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
	if( m_CutObjects )
	{
		while( m_CutObjects->length() )
		{
			AcDbObjectId objId = m_CutObjects->at(0);

			if( objId.isValid() )
			{
				ArxWrapper::RemoveDbObject(objId);
			}

			m_CutObjects->removeAt(0);
		}
	}

	//ɾ��ǰһ������·��ʱ�Ĺ���ʵ��
	if( m_RouteLineEntity )
		delete m_RouteLineEntity;
}

// LineCalRouteDialog message handlers
