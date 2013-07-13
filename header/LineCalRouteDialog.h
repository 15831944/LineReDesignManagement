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

	void CalculateShortestRoute();

	bool CalculateShortestRoute( const AcGePoint3d& start, const AcGePoint3d& end);

	bool SetupRouteLineEnv();

	bool CreateRouteSegment( const AcGePoint3d& start, const AcGePoint3d& end);

private:

	bool InitializeRouteLine();

	bool InitializeRouteLineInfo();

	bool InitializeStartEndPoints( const AcGePoint3d& startPoint, const AcGePoint3d& endPoint );

	bool AppendStartEndPoints(const AcGePoint3d& startPoint, const AcGePoint3d& endPoint);

	bool SaveRouteLinePoint( const AcGePoint3d& newPoint );

	bool HasIntersect(AcArray<PointEntity*>* intersectEntities);

	PointEntity* GetNearestLineSegement( AcArray<PointEntity*>* intersectEntities );

	AcGePoint3d GetProjectPoint3d(PointEntity* lineSegment);

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

	AcGePoint3d m_newStartPoint;

	//������ļ���
	wstring m_fileName;

	//ʵ���ļ�������
	LineEntityFile* m_EntryFile;

	//Ĭ�ϵĹ�������
	static CString m_lineCategory;

	//����Ĺ��ߵ����ƣ����������µ�ͼ��
	static CString m_CutLayerName;

	//Ĭ�ϵĹ��߿��
	static CString m_lineWidth;

	//Ĭ�ϵĹ�����Ϣ
	static LineCategoryItemData* m_lineInfo;

	//����·�ɹ����в������µ����ݿ����
	static AcDbObjectIdArray* m_CutObjects;

	//����ʵ�壬����ǰ����
	static LineEntity* m_RouteLineEntity;

	//���ս��,��������߶ε�����
	static AcGePoint3dArray* m_PointVertices;
};
