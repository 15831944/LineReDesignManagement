#pragma once

#include <resource.h>
#include <dbsymtb.h>
#include <dbapserv.h>
#include <adslib.h>
#include <adui.h>
#include <acui.h>
#include "afxwin.h"

#include <map>
#include <list>

#include <LineEntryData.h>

using namespace std;
using namespace com::guch::assistant::data;

// LineCalRouteDialog dialog

class LineCalRouteDialog : public CAcUiDialog
{
	DECLARE_DYNAMIC(LineCalRouteDialog)

public:

	// Dialog Data
	enum { IDD = IDD_DIALOG_CAL_ROUTE };

	// Pass through direction 000000 -> 111111
	enum PASS_STATUS
		{	PASS_NONE = 0x0,
			PASS_UP = 0x1, 
			PASS_DOWN = ( 0x1 << 1 ),
			PASS_LEFT = (0x1 << 2), 
			PASS_RIGHT = (0x1 << 3), 
			PASS_FRONT = (0x1 << 4), 
			PASS_BACK = (0x1 << 5),
			PASS_ALL = 0X3F };

	// Status for one possible route line
	typedef enum _CALCULATE_STATUS
	{
		INIT = 0,
		DONE = 1
	} CAL_STATUS;

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

	void GetStartEndPoint();

	void CalculateShortestRoute();

	bool CalculateShortestRoute( const AcGePoint3d& start, const AcGePoint3d& end);

	bool SetupRouteLineEnv();

	bool CreateRouteSegment( const AcGePoint3d& start, const AcGePoint3d& end);

	static PASS_STATUS GetPassDirecion( PointEntity *lineSegment);

private:

	bool InitializeRouteLine();

	bool InitializeRouteLineInfo();

	bool InitializePossibleLines();

	bool InitializeStartEndPoints( const AcGePoint3d& startPoint, const AcGePoint3d& endPoint );

	void InitializeProjectPlace();

	bool AppendStartEndPoints(const AcGePoint3d& startPoint, const AcGePoint3d& endPoint);

	bool SaveRouteLinePoint( const AcGePoint3d& newPoint );

	void CheckIntersect(AcArray<PointEntity*>* intersectEntities);

	PointEntity* GetNearestLineSegement( AcArray<PointEntity*>* intersectEntities );

	AcGePoint3d GetProjectPoint3d(PointEntity* lineSegment);

	void SetupLineRouteResult();

	void SetupFinalResult();

	void GetHeightAndStep(PointEntity* lineSegment, double& height, double& step);

	bool GetPossibleStartPoint(AcGePoint3d& startPoint);

	bool SetCurrentPossibleLineDone();

	LineEntity* CreateNewLineEntity();

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
	static LineEntityFile* m_EntryFile;

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

	//���п��ܵĹ���·��
	static list<LineEntity*> m_AllPossibleLineEntities;

	//����ʵ�壬����ǰ���еĹ���
	LineEntity* m_CurrentRouteLineEntity;

	//���п��ܵ�·��,ÿ��·�߶���������߶ε�����
	static map<AcGePoint3dArray*, CAL_STATUS> m_lPossibleRoutes;
	
	//��ǰ���м����·��
	AcGePoint3dArray* m_CurrentPointVertices;

	//��ʼ����ֹ����X�ᴹֱ��ƽ��
	AcGePlane m_ProjectPlane;

	//Խ�����߶β����ظ�����
	set<LinePointID> m_CheckedEntities;
};
