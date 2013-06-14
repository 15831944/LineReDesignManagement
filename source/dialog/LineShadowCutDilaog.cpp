// LineCutPosDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LineShadowCutDialog.h"

#include "acedads.h"
#include "accmd.h"
#include <adscodes.h>
#include "geassign.h"
#include <adsdlg.h>
#include "dbxutil.h"
#include <dbapserv.h>
#include <dbregion.h>
#include <gepnt3d.h>
#include <actrans.h>

//symbol table
#include <dbsymtb.h>

#include <acdocman.h>

//3D Object
#include <dbsol3d.h>
#include <dbmleader.h>

#include <ArxWrapper.h>

#include <ArxCustomObject.h>

#include <GlobalDataConfig.h>

#include <LMAUtils.h>

#pragma warning(disable:4482)

using namespace com::guch::assistant::arx;

// LineShadowCutDialog dialog

IMPLEMENT_DYNAMIC(LineShadowCutDialog, CAcUiDialog)

LineShadowCutDialog::LineShadowCutDialog( int dialogId, CWnd* pParent /*=NULL*/)
	: LineCutPosDialog(dialogId, pParent),
	  m_collector(),
	  m_ShadowViewPort()
{
	memset(m_LeftDownCorner, 0 , 3 * sizeof(double));
	memset(m_RightUpCorner, 0, 3 * sizeof(double));
	memset(m_ViewPosition, 0 , 3 * sizeof(double));
	memset(m_TargetPosition, 0, 3 * sizeof(double));
}

LineShadowCutDialog::~LineShadowCutDialog()
{
}

BOOL LineShadowCutDialog::OnInitDialog()
{
	//��ҳ�潻������
	LineCutPosDialog::OnInitDialog();

	//Ĭ������ķ���ѡ��
	m_DirectionSame.SetCheck(BST_CHECKED);
	m_ViewDirection = 0;

	return TRUE;
}

void LineShadowCutDialog::DoDataExchange(CDataExchange* pDX)
{
	LineCutPosDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_RADIO_DIR_SAME, m_DirectionSame);
	DDX_Control(pDX, IDC_RADIO_DIR_OPPOSITE, m_DirectionOpposite);
}

BEGIN_MESSAGE_MAP(LineShadowCutDialog, CAcUiDialog)
	ON_BN_CLICKED(IDC_X, &LineShadowCutDialog::OnBnClickedX)
	ON_BN_CLICKED(IDC_Y, &LineShadowCutDialog::OnBnClickedY)
	ON_BN_CLICKED(IDC_Z, &LineShadowCutDialog::OnBnClickedZ)
	ON_BN_CLICKED(IDC_RADIO_DIR_SAME, &LineShadowCutDialog::onBnClickedSame)
	ON_BN_CLICKED(IDC_RADIO_DIR_OPPOSITE, &LineShadowCutDialog::onBnClickedOpposite)
	ON_BN_CLICKED(IDC_BUTTON_PICKCUT, &LineShadowCutDialog::OnBnPickCutPos)
	ON_BN_CLICKED(IDOK, &LineShadowCutDialog::OnBnClickedOk)
END_MESSAGE_MAP()

void LineShadowCutDialog::OnBnClickedX()
{
	LineCutPosDialog::OnBnClickedX();
}

void LineShadowCutDialog::OnBnClickedY()
{
	LineCutPosDialog::OnBnClickedY();
}

void LineShadowCutDialog::OnBnClickedZ()
{
	LineCutPosDialog::OnBnClickedZ();
}

void LineShadowCutDialog::onBnClickedSame()
{
	m_ViewDirection = 0;
}

void LineShadowCutDialog::onBnClickedOpposite()
{
	m_ViewDirection = 1;
}

void LineShadowCutDialog::OnBnPickCutPos()
{
	LineCutPosDialog::OnBnPickCutPos();
}

void LineShadowCutDialog::OnBnClickedOk()
{
	//�õ��û����������
	UpdateData(FALSE);

	//���Ȼָ���ͼ
	CutBack();

	//�õ�����
	GenerateCutPlane();

	//�رնԻ���
	CAcUiDialog::OnOK();

	//�õ�Ҫ�����ڵ���ʵ�弯��
	LineCutPosDialog::GenerateCutRegion();

	//�õ�ͶӰ����ͼ
	GetViewPoint();

	//����ͼ����ͶӰ
	GenerateShadow();

	//��ʾ��ͼ��
	ShowCutRegion();
}

void LineShadowCutDialog::GenerateCutPlane()
{
	LineCutPosDialog::GenerateCutPlane();

	CString cutLayerName(m_CutLayerName);
	if( m_ViewDirection == 0)
	{
		m_CutLayerName.Format(L"������-%s",cutLayerName.GetBuffer());
	}
	else if( m_ViewDirection == 1)
	{
		m_CutLayerName.Format(L"������-%s",cutLayerName.GetBuffer());
	}
}

void LineShadowCutDialog::CalculateBounds( PointEntity* pointEntity )
{
	assert(pointEntity);

	//�������½�
	m_LeftDownCorner[0] = std::min<double>( m_LeftDownCorner[0], pointEntity->m_Point[0] );
	m_LeftDownCorner[1] = std::min<double>( m_LeftDownCorner[1], pointEntity->m_Point[1] );
	m_LeftDownCorner[2] = std::min<double>( m_LeftDownCorner[2], pointEntity->m_Point[2] );

	//�������Ͻ�
	m_RightUpCorner[0] = std::max<double>( m_RightUpCorner[0], pointEntity->m_Point[0] );
	m_RightUpCorner[1] = std::max<double>( m_RightUpCorner[1], pointEntity->m_Point[1] );
	m_RightUpCorner[2] = std::max<double>( m_RightUpCorner[2], pointEntity->m_Point[2] );
}

void LineShadowCutDialog::GenerateCutRegion(LineEntity* lineEntry)
{
	PointList* pointList = lineEntry->m_PointList;
	if( pointList == NULL )
	{
		acutPrintf(L"\n�ù���û���߶Σ�������ͼ��");
		return;
	}

	//�����е��߶ν��б���
	PointIter pointIter = pointList->begin();
	for(;pointIter != pointList->end();pointIter++)
	{
		PointEntity* pointEntity = (*pointIter);
		assert(pointEntity);

		//����߽�
		CalculateBounds(pointEntity);

		//��ʼ��û�ж�Ӧ��ʵ��
		if( pointIter == pointList->begin() )
			continue;

		acutPrintf(L"\n�Եڡ�%d�����߶ν����ڵ���ͼ��",pointEntity->m_PointNO);

		AcDbObjectId wallEntityId = pointEntity->m_DbEntityCollection.GetWallLineEntity();
		if( wallEntityId.isValid())
		{
			acutPrintf(L"\nʹ����ǽ�壡");
			m_collector.addEntity(wallEntityId);
		}
		else
		{
			AcDbObjectId lineEntityId = pointEntity->m_DbEntityCollection.GetLineEntity();
			if( lineEntityId.isValid() )
			{
				acutPrintf(L"\nʹ�ù����壡");
				m_collector.addEntity(lineEntityId);
			}
		}
	}
}

void LineShadowCutDialog::Reset()
{
	acutPrintf(L"\n���ڵ���ͼΪ��%s����",m_CutLayerName.GetBuffer());

	if( m_CutLayerName.GetLength() > 0 )
	{
		acutPrintf(L"\n�����������ĵ�");
		ArxWrapper::LockCurDoc();

		acutPrintf(L"\n�ָ�WCS�Ӵ�");
		acedCommand(RTSTR, _T("UCS"), RTSTR, L"W", 0);

		acutPrintf(L"\nɾ����ͼ��صĶ���");
		CutBack();

		acutPrintf(L"\nɾ����ͼ���ڵ�ͼ��");
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

void LineShadowCutDialog::GetViewPosition()
{
	int nPosIndex = 0;
	int nTargetIndex = 0;

	//�õ���ֱ��ķ���
	if( m_Direction == 1 )
	{
		nPosIndex = 0;
	} 
	else if ( m_Direction == 2 )
	{
		nPosIndex = 1;
	} 
	else if( m_Direction == 3 )
	{
		nPosIndex = 2;
	}

	//�õ��۲�ķ���
	ads_point nPointPos;
	if( m_ViewDirection == 0 )
	{
		memcpy(nPointPos, m_RightUpCorner, 3 * sizeof(double));
	}
	else
	{
		memcpy(nPointPos, m_LeftDownCorner, 3 * sizeof(double));
	}

	m_ViewPosition[nPosIndex] = m_strOffset;
	m_TargetPosition[nPosIndex] = nPointPos[nPosIndex];
}

Adesk::Boolean LineShadowCutDialog::GetViewPoint()
{
	//�õ��۲���Ŀ���
	GetViewPosition();

	acdbUcs2Wcs(m_ViewPosition, m_ViewPosition, Adesk::kFalse ) ;
    acdbUcs2Wcs(m_TargetPosition, m_TargetPosition, Adesk::kFalse ) ;

    m_ShadowViewPort.setViewTarget(asPnt3d (m_TargetPosition)) ;
    m_ShadowViewPort.setViewDirection(asPnt3d (m_ViewPosition) - asPnt3d (m_TargetPosition)) ;

    m_ShadowViewPort.setFrontClipDistance(asPnt3d (m_ViewPosition).distanceTo (asPnt3d (m_TargetPosition))) ;
    m_ShadowViewPort.setBackClipDistance(0) ;

    m_ShadowViewPort.setFrontClipOn() ;    
    return (Adesk::kTrue) ;
}

void LineShadowCutDialog::GenerateShadow()
{
	//����ͶӰ
	int control = kProject | kEntity | kBlock | kHonorInternals;
	AsdkHlrEngine hlr(&m_ShadowViewPort, control) ;
	Acad::ErrorStatus es = hlr.run(m_collector) ;

	if( es != Acad::eOk )
	{
		acutPrintf(L"\n����ͶӰ���ʧ����!");
		rxErrorMsg(es);
	}

	actrTransactionManager->startTransaction () ;

	//����ͶӰ���
	int nOutput =m_collector.mOutputData.logicalLength () ;
    acutPrintf (ACRX_T("\nͶӰ������С�%d��������"), nOutput) ;

	//����ͶӰ���ڵ�ͼ��
	if( ArxWrapper::createNewLayer(m_CutLayerName.GetBuffer()) == false )
		return;

    for ( int j =0 ; j < nOutput ; j++ ) {
        AsdkHlrData *pResult = m_collector.mOutputData[j] ;
        AcDbEntity *pResultEntity =pResult->getResultEntity () ;

        AcDbObjectId id =  ArxWrapper::PostToModelSpace( pResultEntity, m_CutLayerName.GetBuffer());
		m_CutObjects.append(id);
    }

    actrTransactionManager->endTransaction();

	acutPrintf(L"\n�۲������Ϊx��%02.lf��y��%02.lf��z��%02.lf��",m_ViewPosition[0],m_ViewPosition[1],m_ViewPosition[2]);
	acutPrintf(L"\nĿ�������Ϊx��%02.lf��y��%02.lf��z��%02.lf��",m_TargetPosition[0],m_TargetPosition[1],m_TargetPosition[2]);
}

void LineShadowCutDialog::ShowCutRegion()
{
	//ֻ��ʾ����ͼ��
	ArxWrapper::ShowLayer(m_CutLayerName.GetBuffer());

	int direction = m_Direction;
	if( m_ViewDirection == 1 )
	{
		direction +=3;
	}

	//�л���ͼ
	ArxWrapper::ChangeView(direction);
}

void LineShadowCutDialog::CutBack()
{
	while( m_CutObjects.length() )
	{
		AcDbObjectId objId = m_CutObjects.at(0);

		if( objId.isValid() )
		{
			ArxWrapper::RemoveDbObject(objId);
		}

		m_CutObjects.removeAt(0);
	}
}

// LineCutPosDialog message handlers
