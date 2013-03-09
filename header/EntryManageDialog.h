#pragma once

#include "afxcmn.h"
#include <resource.h>

#include <dbsymtb.h>
#include <dbapserv.h>
#include <adslib.h>
#include <adui.h>
#include <acui.h>

#include <string>
#include <LineEntryData.h>
#include <ListCtrlEx.h>

using namespace std;

using namespace com::guch::assistant::data;

namespace com
{

namespace guch
{

namespace assistant
{

namespace entry
{

//ʵ�������
class EntryManageDialog : public CDialog
{
	DECLARE_DYNAMIC(EntryManageDialog)

public:

	EntryManageDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~EntryManageDialog();

	// Dialog Data
	enum { IDD = IDD_DIALOG_ENTRY_MANAGE };
	typedef enum { OPER_INIT, OPER_ADD, OPER_UPDATE } OPER_TYPE;

protected:
	
	DECLARE_MESSAGE_MAP()

	//�Ի�������
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	LineCategoryItemData* CreateEntryDetailInfo();
	PointList* CreateEntryPointList();
		
	void FillLineData( LineEntry* lineEntry );
	void ClearLineData();
	void CheckUIData();

	//��ʼ��ҳ������
	BOOL InitEntryListControl();
	BOOL InitEntryDetailControl();
	BOOL InitEntryPointsControl();
	BOOL InitEntryData();
	BOOL InitEntryPointsData(LineEntry* lineEntry);

	//����б���ز���
	HTREEITEM GetKindNode( const wstring& category, bool createOnDemand = false );
	HTREEITEM FindKindNode( const UINT& lineID );
	HTREEITEM InsertTreeNode( const wstring& lineCategory, const wstring& lineName );
	
	//��ť��Ӧ
	afx_msg void OnBnClickedButtonOK();
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonDel();

	//�ؼ����
	virtual afx_msg void OnTreeSelChanged(LPNMHDR pnmhdr, LRESULT *pLResult);
	virtual afx_msg void OnCbnShapeChange();
	virtual afx_msg void OnControlValueChange();

	virtual void ShowControlDynamic();

	//�������
	void SetOperType( OPER_TYPE type ) { m_OperType = type; }

	//���߲���
	LineEntry* GetSelectLine();
	LineEntry* GetLineEntry( const UINT& ID );
	BOOL InsertLine( LineEntry* lineEntry, BOOL bInitialize = FALSE );
	BOOL UpdateLine( LineEntry* lineEntry );

	//�ؼ�����
	void EnableDetailControl(bool enable);

private:

	//�������
	wstring m_EntryKind;

	//ʵ���б�
	CTreeCtrl m_LinesTree;

	//��ϸ��Ϣ
	CComboBox m_LineCategory;
	CComboBox m_LineShape;

	CStatic m_StaticRadius;
	CEdit m_LineRadius;

	CStatic m_StaticWidth;
	CEdit m_LineWidth;

	CStatic m_StaticHeight;
	CEdit m_LineHeight;

	CEdit m_LineWallSize;
	CEdit m_LineSafeSize;

	CEdit m_LinePlaneDesc;
	CEdit m_LineCutDesc;

	CAcUiNumericEdit m_PointEdit;

	//��������
	CListCtrlEx m_LineDetailList;

	//��������
	OPER_TYPE m_OperType;

	//�����Ƿ��иı�
	BOOL m_bDataDirty;

	//ҳ�水ť
	CButton m_ButtonAdd;
	CButton m_ButtonDel;

	CButton m_ButtonOK;

	//������ļ���
	wstring m_fileName;

	//ʵ���ļ�������
	LineEntryFile* m_EntryFile;

	HTREEITEM m_lineRoot;
	HTREEITEM m_blockRoot;
};

} // end of config

} // end of assistant

} // end of guch

} // end of com
