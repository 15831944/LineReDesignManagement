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

// LineCutPosDialog dialog

class LineCutPosDialog : public CAcUiDialog
{
	DECLARE_DYNAMIC(LineCutPosDialog)

public:

	// Dialog Data
	enum { IDD = IDD_DIALOG_CUT_POS };

	LineCutPosDialog(int dialogId, CWnd* pParent = NULL);   // standard constructor
	virtual ~LineCutPosDialog();

	virtual BOOL OnInitDialog();

	static afx_msg void Reset();

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:

	afx_msg void OnBnClickedX();

	afx_msg void OnBnClickedY();

	afx_msg void OnBnClickedZ();

	virtual afx_msg void OnBnClickedOk();

	afx_msg void OnBnPickCutPos();

protected:

	//�Ե�ǰ�ļ��е����й��߽�����ͼ
	void GenerateCutRegion();

	//��һ�����߽�����ͼ
	virtual void GenerateCutRegion(LineEntity* lineEntry);

	//��һ�����߶ν�����ͼ
	virtual void GenerateCutRegion(PointEntity* pointEntity, double markOffset);

	//��������ͼ���ڵ�ͼ��
	virtual void GenerateCutPlane();

	//����ת������
	void GenerateTransform();

	//��ʾ����ͼ���ڵ�ͼ�㣬Ҳ����ʾ���е�����
	void ShowCutRegion();

	//�������
	AcDbObjectId CreateHatch(AcDbObjectId entityId);

	//���ע��
	AcDbObjectId CreateMLeader(const AcGePoint3d& center, const wstring& content, double markOffset);

	//ɾ���ϴ���ͼ�в����Ľ������
	static void CutBack();

	//���о���ת����������ͼ����XYƽ��
	void TransformToXY( AcDbObjectIdArray entityIds );

protected:

	static int m_DialogID;

	//ѡȡX��Y��Z��������
	CButton m_DirectionX;
	CButton m_DirectionY;
	CButton m_DirectionZ;

	//ѡȡ��ƫ����
	CAcUiNumericEdit m_EditOffset;

	//ѡȡƫ�����İ�ť
	CAcUiPickButton m_PickCutPosButton;

	//�����ƫ����
	int m_strOffset;

	//����ķ���
	int m_Direction;

	//���ɵ�����
	AcGePlane m_CutPlane;

	//��������ƣ����������µ�ͼ��
	static CString m_CutLayerName;

	//��ͼ�����в������µ����ݿ����
	static AcDbObjectIdArray m_CutObjects;

	//����������ʽ
	static wstring m_CutHatchStyle;

	//����ת���ľ���
	AcGeMatrix3d m_MoveMatrix;

	AcGeMatrix3d m_RotateMatrixFirst;
	AcGeMatrix3d m_RotateMatrixSecond;
};
