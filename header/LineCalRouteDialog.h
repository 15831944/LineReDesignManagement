#pragma once

#include <resource.h>
#include <dbsymtb.h>
#include <dbapserv.h>
#include <adslib.h>
#include <adui.h>
#include <acui.h>
#include "afxwin.h"

#include <LineEntryData.h>

using namespace com::guch::assistant::data;

// LineCalRouteDialog dialog

class LineCalRouteDialog : public CAcUiDialog
{
	DECLARE_DYNAMIC(LineCalRouteDialog)

public:

	// Dialog Data
	enum { IDD = IDD_DIALOG_CAL_ROUTE };

	LineCalRouteDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~LineCalRouteDialog();

	virtual BOOL OnInitDialog();

	static afx_msg void Reset();

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:

	afx_msg void OnBnPickStartClicked();

	afx_msg void OnBnPickEndClicked();

	virtual afx_msg void OnBnClickedOk();

protected:

	//ɾ���ϴ���ͼ�в����Ľ������
	static void CutBack();

protected:

	//ѡȡ��ʼ����ֹ��
	CAcUiPickButton m_PickStart;
	CAcUiPickButton m_PickEnd;

	//��ʼ��
	CAcUiNumericEdit m_StartX;
	CAcUiNumericEdit m_StartY;
	CAcUiNumericEdit m_StartZ;

	//��ֹ��
	CAcUiNumericEdit m_EndX;
	CAcUiNumericEdit m_EndY;
	CAcUiNumericEdit m_EndZ;

	AcGePoint3d m_startPoint;
	AcGePoint3d m_endPoint;

	//����Ĺ��ߵ����ƣ����������µ�ͼ��
	static CString m_CutLayerName;

	//����·�ɹ����в������µ����ݿ����
	static AcDbObjectIdArray m_CutObjects;
};
