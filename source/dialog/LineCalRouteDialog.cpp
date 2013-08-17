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

typedef map<AcGePoint3dArray*, LineCalRouteDialog::CAL_STATUS>::iterator LineIter;
typedef map<LineEntity*, LineCalRouteDialog::CAL_STATUS>::iterator LineEntityIter;

CString LineCalRouteDialog::m_lineCategory = L"�Զ�·�ɹ���";
CString LineCalRouteDialog::m_CutLayerName = L"";
CString LineCalRouteDialog::m_lineWidth = L"100";

LineCategoryItemData* LineCalRouteDialog::m_lineInfo = NULL;

AcDbObjectIdArray* LineCalRouteDialog::m_CutObjects = NULL;

//All possible line database entities 
list<LineEntity*> LineCalRouteDialog::m_AllPossibleLineEntities = list<LineEntity*>();

//���ܵ�·�������״̬Ϊ0��
map<AcGePoint3dArray*, LineCalRouteDialog::CAL_STATUS> LineCalRouteDialog::m_lPossibleRoutes
	= map<AcGePoint3dArray*, LineCalRouteDialog::CAL_STATUS>();

map<LineEntity*, LineCalRouteDialog::CAL_STATUS> LineCalRouteDialog::m_lPossibleLineEntities
	= map<LineEntity*, LineCalRouteDialog::CAL_STATUS>();

LineEntityFile* LineCalRouteDialog::m_EntryFile = NULL;

// LineCalRouteDialog dialog

IMPLEMENT_DYNAMIC(LineCalRouteDialog, CAcUiDialog)

LineCalRouteDialog::LineCalRouteDialog(CWnd* pParent /*=NULL*/)
: CAcUiDialog(LineCalRouteDialog::IDD, pParent),
	m_startPoint(),
	m_endPoint(),
	m_DrawRealTime(false),
	m_CurrentRouteLineEntity(NULL),
	m_CompareLineSegmentEntity(NULL),
	m_CurrentPointVertices(NULL),
	m_ShortestPointVertices(NULL)
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
	if( m_startPoint.y > m_endPoint.y )
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
	SetupFinalResult();

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

	//��������ǰ·�ɵĹ���ʵ��
	InitializeRouteLine();

	//Initialize the possible route lines;
	InitializePossibleLines();

	//Use the start point user selected to set up the first new possible route.
	AppendInterSegment(m_startPoint);

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

	//������ʱ·�ɹ��ߣ����ڵ�ǰ�ļ���
	acutPrintf(L"\n������ʱ·�ɹ���");
	m_CompareLineSegmentEntity = CreateNewLineEntity();

	return true;
}

LineEntity* LineCalRouteDialog::CreateNewLineEntity()
{
	//�õ�ʵ������
	wstring pipeName(m_CutLayerName.GetBuffer());
	acutPrintf(L"\n�µ�·������ͼ�������Ϊ��%s��", pipeName.c_str());

	LineEntity* lineEntity = new LineEntity(pipeName,GlobalData::CONFIG_LINE_KIND, m_lineInfo , (new PointList()));

	//������ߣ���Ҫ��ʾ
	lineEntity->m_LinePriority = GlobalData::LINE_FIRST;

	//���ɸ����ID
	lineEntity->m_LineID = (UINT)GetTickCount();

	//���浽���ݿ�
	lineEntity->m_dbId = ArxWrapper::PostToNameObjectsDict(lineEntity->m_pDbEntry,LineEntity::LINE_ENTRY_LAYER);

	//������ݿ����ָ�룬��AutoCAD����
	lineEntity->m_pDbEntry = NULL;

	//�������ݵ�������
	m_EntryFile->InsertLine(lineEntity);

	return lineEntity;
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

bool LineCalRouteDialog::InitializePossibleLines()
{
	if( m_DrawRealTime )
	{
		//Create the first possible rout line entity
		m_CurrentRouteLineEntity = CreateNewLineEntity();

		//Start with the first possible line
		m_lPossibleLineEntities.insert(std::pair<LineEntity*,CAL_STATUS>(m_CurrentRouteLineEntity,INIT));
	}
	else
	{
		//Create the first possible route line. 
		m_CurrentPointVertices = new AcGePoint3dArray();

		//Start with the first possible line
		m_lPossibleRoutes.insert(std::pair<AcGePoint3dArray*,CAL_STATUS>(m_CurrentPointVertices,INIT));
	}

	return true;
}

bool LineCalRouteDialog::InitializeCompareSegmentEntity( const AcGePoint3d& startPoint, const AcGePoint3d& endPoint )
{
	acutPrintf(L"\n��3Dģ���л����������ߣ����бȽϼ���");

	//���û�ѡ��Ŀ�ʼ���������ʼ����������
	CreateCompareLineSegement( startPoint, endPoint);

	//Ĭ���û�ѡ��Ŀ�ʼ����Ա���������
	//SaveRouteLinePoint( startPoint );

	//û�м�������߶�
	m_CheckedEntities.clear();

	return true;
}

bool LineCalRouteDialog::SaveRouteLinePoint( const AcGePoint3d& newPoint )
{
	acutPrintf(L"\n����(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)Ϊһ�����", newPoint[X], newPoint[Y], newPoint[Z]);

	AppendInterSegment( newPoint);

	return true;
}

bool LineCalRouteDialog::CreateCompareLineSegement(const AcGePoint3d& startPoint, const AcGePoint3d& endPoint)
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
	m_CompareLineSegmentEntity->SetPoints(newPoints);

	acutPrintf(L"\n�µ�·���߶ι������");

	return true;
}

/**
 * Iterator all the possible route lines, and find the unfinished 
 */
bool LineCalRouteDialog::GetPossibleStartPoint(AcGePoint3d& startPoint)
{
	bool hasStartPoint(false);

	if( m_DrawRealTime )
	{
		acutPrintf(L"\n��ǰ���ܵ�·����Ϊ��%d��",m_lPossibleLineEntities.size());

		for( LineEntityIter iter = m_lPossibleLineEntities.begin(); 
			iter != m_lPossibleLineEntities.end();
			iter++ )
		{
			if( (*iter).second != DONE )
			{
				m_CurrentRouteLineEntity = (*iter).first;

				if( m_CurrentRouteLineEntity )
				{
					int length = static_cast<int>(m_CurrentRouteLineEntity->m_PointList->size());
					ads_point& curStartPoint = m_CurrentRouteLineEntity->m_PointList->at(length-1)->m_Point;

					startPoint.x = curStartPoint[X];
					startPoint.y = curStartPoint[Y];
					startPoint.z = curStartPoint[Z];

					hasStartPoint = true;
					break;
				}
			}
		}
	}
	else
	{
		acutPrintf(L"\n��ǰ���ܵ�·����Ϊ��%d��",m_lPossibleRoutes.size());

		for( LineIter iter = m_lPossibleRoutes.begin(); 
			iter != m_lPossibleRoutes.end();
			iter++ )
		{
			if( (*iter).second != DONE )
			{
				m_CurrentPointVertices = (*iter).first;

				if( m_CurrentPointVertices )
				{
					int length = m_CurrentPointVertices->length();
					startPoint = m_CurrentPointVertices->at(length-1);

					hasStartPoint = true;
					break;
				}
			}
		}
	}

	if( hasStartPoint )
	{
		acutPrintf(L"\n�ҵ�һ��δ��ɵĿ���·�ɡ���ʼ��Ϊx��%0.2lf��y��%0.2lf��z��%0.2lf��",
						startPoint.x,startPoint.y,startPoint.z);
	}

	return hasStartPoint;
}

bool LineCalRouteDialog::SetCurrentPossibleLineDone()
{
	if( m_DrawRealTime )
	{
		for( LineEntityIter iter = m_lPossibleLineEntities.begin(); 
			iter != m_lPossibleLineEntities.end();
			iter++ )
		{
			if( (*iter).second != DONE )
			{
				if( m_CurrentRouteLineEntity == (*iter).first )
				{
					acutPrintf(L"\n���õ�ǰ����·�ɵ�״̬Ϊ�����");
					(*iter).second = DONE;

					return true;
				}
			}
		}
	}
	else
	{
		for( LineIter iter = m_lPossibleRoutes.begin(); 
			iter != m_lPossibleRoutes.end();
			iter++ )
		{
			if( (*iter).second != DONE )
			{
				m_CurrentPointVertices = (*iter).first;

				if( (*iter).first == m_CurrentPointVertices )
				{
					acutPrintf(L"\n���õ�ǰ����·�ɵ�״̬Ϊ�����");
					(*iter).second = DONE;

					return true;
				}
			}
		}
	}

	return false;
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
	while( GetPossibleStartPoint(m_newStartPoint) )
	{
		int count = 0;

		//�ݹ����
		while( CalculateShortestRoute( m_newStartPoint, m_endPoint ) == false )
		{
			acutPrintf(L"\n���ֵ�ǰ������ϵͳ�е��߶������ֵ����󣬼�������");

			if( count++ >= 50 )
				break;
		}

		//current line is done
		SetCurrentPossibleLineDone();

		//One possible line route has been finished
		SetupLineRouteResult();
	}
}

bool LineCalRouteDialog::CalculateShortestRoute( const AcGePoint3d& start, const AcGePoint3d& end)
{
	acutPrintf(L"\n���ڼ���(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)��(X:%0.2lf,Y:%0.2lf,Z:%0.2lf)�����·��", start[X], start[Y], start[Z], end[X], end[Y], end[Z]);

	//�Կ�ʼ��ͽ�ֹ�㴴��Ĭ�Ͽ�ȵĹ���
	InitializeCompareSegmentEntity(start, end);

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
		//AcGePoint3d newPoint = GetProjectPoint3d(nearestLine);
		//AcGePoint3d newPoint = GetIntersectPoint3d(nearestLine);
		AcGePoint3d newPoint = GetIntersectPoint3d(nearestLine, start, end);
		
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

LineCalRouteDialog::PASS_STATUS LineCalRouteDialog::GetPassDirecion( PointEntity *lineSegment)
{
	const wstring& throughDirection = lineSegment->m_DbEntityCollection.mCategoryData->mThroughDirection;

	int iPassDirection = PASS_NONE;

	//empty means this line is pipe
	if( throughDirection.empty() )
	{
		iPassDirection = PASS_ALL;
	}
	//Not empty means this line is block
	else	
	{
		if( throughDirection.substr(0,1) == L"1" )
		{
			iPassDirection |= PASS_LEFT;
		}

		if( throughDirection.substr(1,1) == L"1" )
		{
			iPassDirection |= PASS_RIGHT;
		}

		if( throughDirection.substr(2,1) == L"1" )
		{
			iPassDirection |= PASS_FRONT;
		}

		if( throughDirection.substr(3,1) == L"1" )
		{
			iPassDirection |= PASS_BACK;
		}

		if( throughDirection.substr(4,1) == L"1" )
		{
			iPassDirection |= PASS_UP;
		}

		if( throughDirection.substr(5,1) == L"1" )
		{
			iPassDirection |= PASS_DOWN;
		}
	}

	return (PASS_STATUS)iPassDirection;
}

AcGePoint3d LineCalRouteDialog::GetIntersectPoint3d(PointEntity* lineSegment)
{
	acutPrintf(L"\n�õ����ߵĴ�ֱ��X��ĵ�");

	AcGePoint3d start = lineSegment->m_DbEntityCollection.mStartPoint;
	AcGePoint3d end = lineSegment->m_DbEntityCollection.mEndPoint;
	AcGeLine3d intersectLine( start, end ); 

	AcGePoint3d resultPnt;
	m_ProjectPlane.intersectWith(intersectLine, resultPnt);
	acutPrintf(L"\n�õ����ߵ��ཻ�㡾X:%0.2lf,Y:%0.2lf,Z:%0.2lf��", resultPnt[X], resultPnt[Y], resultPnt[Z]);

	double heightOffset(0.0), stepOffset(0.0);
	GetHeightAndStep( lineSegment, heightOffset, stepOffset);
	acutPrintf(L"\n�õ����µ�λ�ơ��߶�:%0.2lf, ����:%0.2lf��", heightOffset, stepOffset);

	//�õ���Խ�ķ���
	PASS_STATUS passStatus = GetPassDirecion(lineSegment);

	//������X��Ͻ��ĵ�
	bool branched = false;
		
	AcGePoint3d branchFirstPoint, branchSecondPoint, nextStartPoint;

	AcGePoint3d firstPoint, secondPoint;
	firstPoint = AcGePoint3d(resultPnt.x,resultPnt.y - stepOffset,resultPnt.z);

	do
	{
		//Check the up through status
		if( passStatus & PASS_UP )
		{
			secondPoint = AcGePoint3d(firstPoint.x,firstPoint.y,firstPoint.z + heightOffset);

			//checked then remove the up status
			passStatus = (PASS_STATUS)(passStatus - PASS_UP);
		}
		else if( passStatus & PASS_DOWN )
		{
			secondPoint = AcGePoint3d(firstPoint.x,firstPoint.y,firstPoint.z - heightOffset);

			//checked then remove the down status
			passStatus = (PASS_STATUS)(passStatus - PASS_DOWN);
		} 
		else 
		{
			break;
		}

		if( !branched )
		{
			acutPrintf(L"\n�����ڵ�ǰ���ֵ�ĵ�һ����֧�����������������·�ɼ�����ȥ");
			//Append the first inter segment

			branchFirstPoint = firstPoint;
			acutPrintf(L"\n�洢�����е㡾X:%0.2lf,Y:%0.2lf,Z:%0.2lf��Ϊһ�����", firstPoint[X], firstPoint[Y], firstPoint[Z]);

			//Append the first inter segment
			branchSecondPoint = secondPoint;
			acutPrintf(L"\n��������ĵ㡾X:%0.2lf,Y:%0.2lf,Z:%0.2lf��Ϊ���ߵ�", secondPoint[X], secondPoint[Y], secondPoint[Z]);

			secondPoint.y = branchSecondPoint.y + 2 * stepOffset;
			acutPrintf(L"\n�ƹ�����ĵ㡾X:%0.2lf,Y:%0.2lf,Z:%0.2lf��Ϊ�µļ������", secondPoint[X], secondPoint[Y], secondPoint[Z]);

			//Append the first inter segment
			nextStartPoint = secondPoint;
			branched = true;
		}
		else
		{
			acutPrintf(L"\n�Ѿ������ڵ�ǰ���ֵ��һ����֧�ˣ��ȱ����������Ժ��ټ���");

			if( m_DrawRealTime )
			{
				LineEntity* cloneLineEntity = CreateNewLineEntity();

				//start with current point
				pPointEntry currentPoint = (*m_CurrentRouteLineEntity->m_PointList)[(m_CurrentRouteLineEntity->m_PointList->size() - 1)];
				//cloneLineEntity->InsertPoint(currentPoint,true);

				//inser the inter point
				cloneLineEntity->InsertPoint(&firstPoint,true);

				//inser the second point
				cloneLineEntity->InsertPoint(&secondPoint,true);

				//next start point
				secondPoint.y = secondPoint.y + 2 * stepOffset;
				cloneLineEntity->InsertPoint(&secondPoint,true);

				m_lPossibleLineEntities.insert(std::pair<LineEntity*,CAL_STATUS>(cloneLineEntity,INIT));
			}
			else
			{
				AcGePoint3dArray* clonePoint3dArray = new AcGePoint3dArray(*m_CurrentPointVertices);

				clonePoint3dArray->append(firstPoint);

				clonePoint3dArray->append(secondPoint);

				//next start point
				secondPoint.y = secondPoint.y + 2 * stepOffset;
				clonePoint3dArray->append(secondPoint);

				m_lPossibleRoutes.insert(std::pair<AcGePoint3dArray*,CAL_STATUS>(clonePoint3dArray,INIT));
			}
		}
	}
	while(true);

	if( branched )
	{
		AppendInterSegment( branchFirstPoint );

		//Append the first inter segment
		AppendInterSegment( branchSecondPoint );

		//Append the new startsegment
		AppendInterSegment( nextStartPoint );
	}

	return nextStartPoint;
}

AcGePoint3d LineCalRouteDialog::GetIntersectPoint3d(PointEntity* lineSegment, const AcGePoint3d& throughStart, const AcGePoint3d& throughEnd)
{
	acutPrintf(L"\n�õ����ߵĴ�ֱ��X��ĵ�");

	AcGePoint3d start = lineSegment->m_DbEntityCollection.mStartPoint;
	AcGePoint3d end = lineSegment->m_DbEntityCollection.mEndPoint;
	AcGeLine3d intersectLine( start, end ); 

	AcGePoint3d resultPnt;
	m_ProjectPlane.intersectWith(intersectLine, resultPnt);
	acutPrintf(L"\n�õ����ߵ��ཻ�㡾X:%0.2lf,Y:%0.2lf,Z:%0.2lf��", resultPnt[X], resultPnt[Y], resultPnt[Z]);

	double heightOffset(0.0), stepOffset(0.0);
	GetHeightAndStep( lineSegment, heightOffset, stepOffset);
	acutPrintf(L"\n�õ����µ�λ�ơ��߶�:%0.2lf, ����:%0.2lf��", heightOffset, stepOffset);

	//�õ���Խ�ķ���
	PASS_STATUS passStatus = GetPassDirecion(lineSegment);

	//������X��Ͻ��ĵ�
	bool branched = false;
		
	AcGePoint3d branchFirstPoint, branchSecondPoint, nextStartPoint;

	AcGePoint3d firstPoint, secondPoint;
	firstPoint = AcGePoint3d(resultPnt.x,resultPnt.y - stepOffset,resultPnt.z);

	do
	{
		//Check the up through status
		if( passStatus & PASS_UP )
		{
			secondPoint = AcGePoint3d(firstPoint.x,firstPoint.y,firstPoint.z + heightOffset);

			//checked then remove the up status
			passStatus = (PASS_STATUS)(passStatus - PASS_UP);
		}
		else if( passStatus & PASS_DOWN )
		{
			secondPoint = AcGePoint3d(firstPoint.x,firstPoint.y,firstPoint.z - heightOffset);

			//checked then remove the down status
			passStatus = (PASS_STATUS)(passStatus - PASS_DOWN);
		} 
		else 
		{
			break;
		}

		if( !branched )
		{
			acutPrintf(L"\n�����ڵ�ǰ���ֵ�ĵ�һ����֧�����������������·�ɼ�����ȥ");
			//Append the first inter segment

			branchFirstPoint = firstPoint;
			acutPrintf(L"\n�洢�����е㡾X:%0.2lf,Y:%0.2lf,Z:%0.2lf��Ϊһ�����", firstPoint[X], firstPoint[Y], firstPoint[Z]);

			//Append the first inter segment
			branchSecondPoint = secondPoint;
			acutPrintf(L"\n��������ĵ㡾X:%0.2lf,Y:%0.2lf,Z:%0.2lf��Ϊ���ߵ�", secondPoint[X], secondPoint[Y], secondPoint[Z]);

			secondPoint.y = branchSecondPoint.y + 2 * stepOffset;
			acutPrintf(L"\n�ƹ�����ĵ㡾X:%0.2lf,Y:%0.2lf,Z:%0.2lf��Ϊ�µļ������", secondPoint[X], secondPoint[Y], secondPoint[Z]);

			//Append the first inter segment
			nextStartPoint = secondPoint;
			branched = true;
		}
		else
		{
			acutPrintf(L"\n�Ѿ������ڵ�ǰ���ֵ��һ����֧�ˣ��ȱ����������Ժ��ټ���");

			if( m_DrawRealTime )
			{
				LineEntity* cloneLineEntity = CreateNewLineEntity();

				//start with current point
				pPointEntry currentPoint = (*m_CurrentRouteLineEntity->m_PointList)[(m_CurrentRouteLineEntity->m_PointList->size() - 1)];
				//cloneLineEntity->InsertPoint(currentPoint,true);

				//inser the inter point
				cloneLineEntity->InsertPoint(&firstPoint,true);

				//inser the second point
				cloneLineEntity->InsertPoint(&secondPoint,true);

				//next start point
				secondPoint.y = secondPoint.y + 2 * stepOffset;
				cloneLineEntity->InsertPoint(&secondPoint,true);

				m_lPossibleLineEntities.insert(std::pair<LineEntity*,CAL_STATUS>(cloneLineEntity,INIT));
			}
			else
			{
				AcGePoint3dArray* clonePoint3dArray = new AcGePoint3dArray(*m_CurrentPointVertices);

				clonePoint3dArray->append(firstPoint);

				clonePoint3dArray->append(secondPoint);

				//next start point
				secondPoint.y = secondPoint.y + 2 * stepOffset;
				clonePoint3dArray->append(secondPoint);

				m_lPossibleRoutes.insert(std::pair<AcGePoint3dArray*,CAL_STATUS>(clonePoint3dArray,INIT));
			}
		}
	}
	while(true);

	if( branched )
	{
		AppendInterSegment( branchFirstPoint );

		//Append the first inter segment
		AppendInterSegment( branchSecondPoint );

		//Append the new startsegment
		AppendInterSegment( nextStartPoint );
	}

	return nextStartPoint;
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

	double heightOffset(0.0), stepOffset(0.0);
	GetHeightAndStep( lineSegment, heightOffset, stepOffset);
	acutPrintf(L"\n�õ����µ�λ�ơ��߶�:%0.2lf, ����:%0.2lf��", heightOffset, stepOffset);

	//�õ���Խ�ķ���
	PASS_STATUS passStatus = GetPassDirecion(lineSegment);

	//������X��Ͻ��ĵ�
	bool branched = false;
	AcGePoint3d curInterPoint, nextPoint;

	do
	{
		AcGePoint3d projectPoint;
		//Check the up through status
		if( passStatus & PASS_UP )
		{
			projectPoint = AcGePoint3d(resultPnt.x,resultPnt.y - stepOffset,resultPnt.z + heightOffset);

			//checked then remove the up status
			passStatus = (PASS_STATUS)(passStatus - PASS_UP);
		}
		else if( passStatus & PASS_DOWN )
		{
			projectPoint = AcGePoint3d(resultPnt.x,resultPnt.y - stepOffset,resultPnt.z - heightOffset);

			//checked then remove the down status
			passStatus = (PASS_STATUS)(passStatus - PASS_DOWN);
		} 
		else 
		{
			break;
		}

		if( !branched )
		{
			acutPrintf(L"\n�����ڵ�ǰ���ֵ�ĵ�һ����֧�����������������·�ɼ�����ȥ");
					
			curInterPoint = projectPoint;
			acutPrintf(L"\n�洢�·��ĵ㡾X:%0.2lf,Y:%0.2lf,Z:%0.2lf��Ϊһ�����", projectPoint[X], projectPoint[Y], projectPoint[Z]);

			projectPoint.y = resultPnt.y + stepOffset;
			acutPrintf(L"\n�����Ϸ��ĵ㡾X:%0.2lf,Y:%0.2lf,Z:%0.2lf��Ϊ�µļ������", projectPoint[X], projectPoint[Y], projectPoint[Z]);

			nextPoint = projectPoint;
			branched = true;
		}
		else
		{
			acutPrintf(L"\n�Ѿ������ڵ�ǰ���ֵ��һ����֧�ˣ��ȱ����������Ժ��ټ���");

			if( m_DrawRealTime )
			{
				LineEntity* cloneLineEntity = CreateNewLineEntity();

				//start with current point
				pPointEntry currentPoint = (*m_CurrentRouteLineEntity->m_PointList)[(m_CurrentRouteLineEntity->m_PointList->size() - 1)];
				cloneLineEntity->InsertPoint(currentPoint,true);

				//inser the inter point
				cloneLineEntity->InsertPoint(&projectPoint,true);

				//next start point
				projectPoint.y = resultPnt.y + stepOffset;
				cloneLineEntity->InsertPoint(&projectPoint,true);

				m_lPossibleLineEntities.insert(std::pair<LineEntity*,CAL_STATUS>(cloneLineEntity,INIT));
			}
			else
			{
				AcGePoint3dArray* clonePoint3dArray = new AcGePoint3dArray(*m_CurrentPointVertices);

				clonePoint3dArray->append(projectPoint);

				//next start point
				projectPoint.y = resultPnt.y + stepOffset;
				clonePoint3dArray->append(projectPoint);

				m_lPossibleRoutes.insert(std::pair<AcGePoint3dArray*,CAL_STATUS>(clonePoint3dArray,INIT));
			}
		}
	}
	while(true);

	if( branched )
	{
		//Append the first inter segment
		AppendInterSegment( curInterPoint );

		//Append the first inter segment
		AppendInterSegment( nextPoint );
	}

	return nextPoint;
}

//Create a new line segment, for this point is a new route point
void LineCalRouteDialog::AppendInterSegment(const AcGePoint3d& newPoint)
{
	//Draw realtime means draw the possible line during the process of calculation
	if( m_DrawRealTime )
	{
		PointEntity* point = new PointEntity();

		point->m_Point[X] = newPoint.x;
		point->m_Point[Y] = newPoint.y;
		point->m_Point[Z] = newPoint.z;

		m_CurrentRouteLineEntity->InsertPoint(point, true);
	}
	else
	{
		m_CurrentPointVertices->append( newPoint );
	}
}

void LineCalRouteDialog::GetHeightAndStep(PointEntity* lineSegment, double& height, double& step)
{
	double wallSize = 0.0;
	acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mWallSize.c_str(), -1, &wallSize);
	wallSize /= 1000;

	double safeSize = 0.0;
	acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSafeSize.c_str(), -1, &safeSize);
	safeSize /= 1000;
	
	double temp = 0.0;

	if( lineSegment->m_DbEntityCollection.mCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		//Բ��Ļ���ȡ�뾶ΪY,Z���ƫ��
		acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSize.mRadius.c_str(), -1, &temp);
		temp /= 1000;
		temp += wallSize;
		temp += safeSize;

		height = step = temp;
	}
	else
	{
		//����Ļ���ȡ���ΪY���ƫ��
		acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSize.mWidth.c_str(), -1, &temp);
		temp /= 2000;
		temp += wallSize;
		temp += safeSize;

		step = temp;

		//ȡ�߶ȵ�һ��ΪZ���ƫ��
		acdbDisToF(lineSegment->m_DbEntityCollection.mCategoryData->mSize.mHeight.c_str(), -1, &temp);
		temp /= 2000;
		temp += wallSize;
		temp += safeSize;

		height = temp;
	}
}

//�ж�һ�����߶����������ߵ��������
void LineCalRouteDialog::CheckIntersect(AcArray<PointEntity*>* intersectEntities)
{
	acutPrintf(L"\n���������ж�.");

	PointList* pointList = m_CompareLineSegmentEntity->m_PointList;
	if( pointList == NULL 
		|| pointList->size() < 2 )
	{
		acutPrintf(L"\n��ʱ�ȽϹ�����û�����߶Σ����Բ����м��");
		return;
	}

	PointEntity* checkPoint = (*pointList)[1];

	wstring& lineName = checkPoint->m_DbEntityCollection.mLayerName;
	Adesk::Int32& checkLineID = checkPoint->m_DbEntityCollection.mLineID;
	Adesk::Int32& checkSeqNO = checkPoint->m_DbEntityCollection.mSequenceNO;
	wstring& layerName = wstring(m_CutLayerName.GetBuffer());

#ifdef DEBUG
	acutPrintf(L"\n�ԡ�%s���ĵڡ�%d�������������ж�.",lineName.c_str(), checkSeqNO);
#endif

	LineList* lineList = m_EntryFile->GetList();
	for( LineIterator line = lineList->begin();
			line != lineList->end();
			line++ )
	{
		if( (*line)->m_LineName == layerName )
		{
			acutPrintf(L"\n�˹���Ϊ�Զ�·����ʱ�߶Σ�������Ƚ�");
			continue;
		}

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

void LineCalRouteDialog::SetupLineRouteResult()
{
	//�������һ�ι���
	AppendInterSegment( m_endPoint );

	m_CurrentPointVertices = NULL;
	m_CurrentRouteLineEntity = NULL;

	//����Ƚϵ����������߶�
	m_CompareLineSegmentEntity->SetPoints(NULL);
}

void LineCalRouteDialog::SetupFinalResult()
{
	if( m_DrawRealTime )
	{
		acutPrintf(L"\n��������·�ɽ�����п���·����·��%d����",m_lPossibleLineEntities.size());
	}
	else
	{
		//�õ������·
		GetShortestRoute();

		//����������·��ͬʱ��̵���·���Ϊ��ɫ
		DrawFinalResult();
	}
}

void LineCalRouteDialog::DrawFinalResult()
{
	acutPrintf(L"\n��������·�ɽ����·�ɵ���·�С�%d����",m_lPossibleRoutes.size());

	for( LineIter iter = m_lPossibleRoutes.begin(); 
		iter != m_lPossibleRoutes.end();
		iter++ )
	{
		if( (*iter).second == DONE )
		{
			m_CurrentPointVertices = (*iter).first;
			acutPrintf(L"\n����·�ɣ��������߶Ρ�%d����",m_CurrentPointVertices->length() - 1);

			PointList* newPoints = new PointList();

			for( int i = 0; i < m_CurrentPointVertices->length(); i++ )
			{
				PointEntity* point = new PointEntity();
				point->m_PointNO = i;
		
				point->m_Point[X] = m_CurrentPointVertices->at(i).x;
				point->m_Point[Y] = m_CurrentPointVertices->at(i).y;
				point->m_Point[Z] = m_CurrentPointVertices->at(i).z;

				newPoints->push_back( point );
			}

			m_CurrentRouteLineEntity = CreateNewLineEntity();
			if( m_CurrentRouteLineEntity )
			{
				if( m_CurrentPointVertices == this->m_ShortestPointVertices )
				{
					acutPrintf(L"\n��ǰ����Ϊ�����·�����Ϊ��ɫ");
					m_CurrentRouteLineEntity->m_LinePriority = GlobalData::LINE_SECOND;
				}

				m_CurrentRouteLineEntity->SetPoints( newPoints );
			}

			//Save to possible line list, use to delete all the entities
			m_AllPossibleLineEntities.push_back( m_CurrentRouteLineEntity );

			m_CurrentPointVertices = NULL;
		}
	}
}

double LineCalRouteDialog::GetShortestRoute()
{
	double shortestLength = 0x3FFFFFFF;

	acutPrintf(L"\n�ڡ�%d����������·��Ѱ����̵�·��",m_lPossibleRoutes.size());

	for( LineIter iter = m_lPossibleRoutes.begin(); 
		iter != m_lPossibleRoutes.end();
		iter++ )
	{
		double oneLineLength = 0.0;

		if( (*iter).second == DONE )
		{
			m_CurrentPointVertices = (*iter).first;
				
			acutPrintf(L"\n����һ���µ���·���ȣ����߶Ρ�%d����",m_CurrentPointVertices->length() - 1);

			AcGePoint3d* pLastPoint = NULL;
			for( int i = 0; i < m_CurrentPointVertices->length(); i++ )
			{
				if( pLastPoint == NULL )
				{
					pLastPoint = &m_CurrentPointVertices->at(i);
					continue;
				}

				double segmentLength = pLastPoint->distanceTo(m_CurrentPointVertices->at(i));
				acutPrintf(L"\n�ڡ�%d�����߶γ���Ϊ��%0.2lf��",i, segmentLength);

				oneLineLength += segmentLength;
			}

			acutPrintf(L"\n��ǰ��·�ĳ���Ϊ��%0.2lf������ʱ��̵���·����Ϊ��%0.2lf��",oneLineLength, shortestLength);

			if( oneLineLength < shortestLength )
			{
				acutPrintf(L"\n��ǰ��·����һЩ");
				m_ShortestPointVertices = m_CurrentPointVertices;
				shortestLength = oneLineLength;
			}
		}
	}

	acutPrintf(L"\n��̾���Ϊ��%0.2lf��",shortestLength);
	return shortestLength;
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

		acutPrintf(L"\nɾ���������·�����ڵ�ͼ���ϵ�����ʣ��ʵ��");
		if( ArxWrapper::RemoveFromModelSpace(m_CutLayerName.GetBuffer()) )
		{
			acutPrintf(L"\nɾ��ʣ��ʵ��ɹ�");
		}

		acutPrintf(L"\nɾ���������·�����ڵ�ͼ��");
		if( ArxWrapper::DeleteLayer(m_CutLayerName.GetBuffer(),true) )
		{
			acutPrintf(L"\nɾ��ͼ��ɹ�");
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
	typedef list<LineEntity*>::iterator DbLineIter;

	for( DbLineIter iter = m_AllPossibleLineEntities.begin(); 
		iter != m_AllPossibleLineEntities.end();
		iter++ )
	{
		LineEntity* lineEntity = *iter;

		//ɾ��ǰһ������·��ʱ�Ĺ���ʵ��
		if( lineEntity )
		{
			//�����ݿ�ɾ�����߱���
			ArxWrapper::DeleteFromNameObjectsDict(lineEntity->m_dbId,LineEntity::LINE_ENTRY_LAYER);

			//�����ݿ�ɾ���������е��߶�
			lineEntity->EraseDbObjects();

			//ɾ�����е��ڴ�ڵ�
			lineEntity->SetPoints(NULL);

			//ɾ���߶μ���
			m_EntryFile->DeleteLine(lineEntity->GetLineID());

			delete lineEntity;
			lineEntity = NULL;
		}
	}

	//ɾ��·�ɹ��߼���
	m_AllPossibleLineEntities.clear();

	for( LineIter iter = m_lPossibleRoutes.begin();
		iter != m_lPossibleRoutes.end();
		iter++ )
	{
		if( (*iter).first )
			delete (*iter).first;
	}

	//ɾ�����ߵ㼯��
	m_lPossibleRoutes.clear();
}

// LineCalRouteDialog message handlers
