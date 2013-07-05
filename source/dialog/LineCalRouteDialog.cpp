// LineCalRouteDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LineCalRouteDialog.h"

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

CString LineCalRouteDialog::m_CutLayerName = L"";
AcDbObjectIdArray LineCalRouteDialog::m_CutObjects = AcDbObjectIdArray();

// LineCalRouteDialog dialog

IMPLEMENT_DYNAMIC(LineCalRouteDialog, CAcUiDialog)

LineCalRouteDialog::LineCalRouteDialog(CWnd* pParent /*=NULL*/)
: CAcUiDialog(LineCalRouteDialog::IDD, pParent),
	m_startPoint(),
	m_endPoint()
{
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

	//��������֮���(�ӽ�)���·��;

	//�رնԻ���
	CAcUiDialog::OnOK();
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

// LineCalRouteDialog message handlers
