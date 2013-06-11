#pragma once

#include <resource.h>
#include <dbsymtb.h>
#include <dbapserv.h>
#include <adslib.h>
#include <adui.h>
#include <acui.h>
#include "afxwin.h"
#include <dbents.h>

#include <LineEntryData.h>
#include <LineCutPosDialog.h>

#include <Hlr.h>

using namespace com::guch::assistant::data;

// LineCutPosDialog dialog

class LineShadowCutDialog : public LineCutPosDialog
{
	DECLARE_DYNAMIC(LineShadowCutDialog)

public:

	// Dialog Data
	enum { IDD = IDD_DIALOG_SHADOW_POS };

	LineShadowCutDialog(int dialogId, CWnd* pParent = NULL);   // standard constructor
	virtual ~LineShadowCutDialog();

	BOOL OnInitDialog();

	static afx_msg void Reset();

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
	
	afx_msg void OnBnClickedX();

	afx_msg void OnBnClickedY();

	afx_msg void OnBnClickedZ();

	afx_msg void onBnClickedSame();

	afx_msg void onBnClickedOpposite();

	virtual afx_msg void OnBnClickedOk();

	afx_msg void OnBnPickCutPos();

	//�����ڵ�ͼ���ڵ�ͼ��
	virtual void GenerateCutPlane();

protected:
	
	//ѡȡ�ķ���
	CButton m_DirectionSame;
	CButton m_DirectionOpposite;

	//�۲�ķ���
	int m_ViewDirection;

private:

	//��һ�����߽����ڵ����㣬�õ��и���ʵ��
	virtual void GenerateCutRegion(LineEntity* lineEntry);

	//�Բü�����ʵ�����ͶӰ
	virtual void GenerateShadow();

	//ɾ���ϴ���ͼ�в����Ľ������
	static void CutBack();

	//�õ�ͶӰ����ͼ
	Adesk::Boolean GetViewPoint();

	//����ʵ��ı߽�
	void CalculateBounds( PointEntity* pointEntity );

	//����۲��ߵ�λ�ú�Ŀ��
	void GetViewPosition();

	//��ʾ�ڵ���ͶӰ���ڵ�ͼ��
	void ShowCutRegion();

private:

	//�����������е�ʵ��
	AsdkHlrCollector m_collector;

	//ͶӰ����ͼ
	AcDbViewport m_ShadowViewPort;

	//ʵ��Ŀؼ���Χ�����������ϣ�
	ads_point m_LeftDownCorner;
	ads_point m_RightUpCorner;

	//�ڵ����λ�ú�Ŀ��λ��
	ads_point m_ViewPosition;
	ads_point m_TargetPosition;
};
