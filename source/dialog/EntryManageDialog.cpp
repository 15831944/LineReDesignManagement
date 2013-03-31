// EntryManageDialog.cpp : implementation file
//

#include "stdafx.h"
//#include "afxdialogex.h"

#include <GlobalDataConfig.h>
#include <EntryManageDialog.h>
#include <LMAException.h>
#include <LMAUtils.h>
#include <LineConfigDataManager.h>
#include <ArxWrapper.h>

using namespace com::guch::assistant::data;
using namespace com::guch::assistant::exception;
using namespace com::guch::assistant::config;

namespace com
{

namespace guch
{

namespace assistant
{

namespace entry
{

// EntryManageDialog dialog

IMPLEMENT_DYNAMIC(EntryManageDialog, CDialog)

EntryManageDialog::EntryManageDialog(CWnd* pParent,const wstring& entryKind)
:CDialog( GetDlgID(entryKind), pParent)
,m_EntryKind(entryKind)
{
	//�õ���ǰ������ĵ�
	m_fileName = curDoc()->fileName();
	acutPrintf(L"\n�����Ի������%s��������.",m_fileName.c_str());

	//�õ�ʵ�������ļ��е�����
	m_EntryFile = LineEntryFileManager::RegisterEntryFile(m_fileName);

	//�����ؼ���Ҫ�ص�
	m_LineDetailList.m_Callback = EntryManageDialog::LinePointModified;
	m_LineDetailList.m_ParentDialog = (void*)this;
}

BOOL EntryManageDialog::OnInitDialog()
{
	//��ҳ�潻������
	CDialog::OnInitDialog();

	//��ʼ����߹����б�
	InitEntryListControl();

	//��ʼ��������ϸ�ؼ�
	InitEntryDetailControl();

	//��ʼ�����߶���Ϣ
	InitEntryPointsControl();

	//Ĭ�Ͽؼ�������
	EnableDetailControl(false);

	//ɾ����ȷ�ϰ�ť������
	m_ButtonDel.EnableWindow(false);
	m_ButtonOK.EnableWindow(false);

	//��ʾ��̬Ԫ��
	UpdateData(true);
	ShowDynamicControl();

	return TRUE;
}

BOOL EntryManageDialog::InitEntryListControl()
{
	acutPrintf(L"\n��ʼ������ʵ������.");

#ifdef _DEMO_DATA
	HTREEITEM hKindItem ,hCatogreyItem,kLineItem;

	//�ڸ���������"����"
	hKindItem = m_LinesTree.InsertItem(L"����",TVI_ROOT);

	//�ڡ����ߡ���������������
	hCatogreyItem = m_LinesTree.InsertItem(L"ˮ��",hKindItem);

	//����������
	kLineItem = m_LinesTree.InsertItem(L"ˮ��#1",hCatogreyItem);
	kLineItem = m_LinesTree.InsertItem(L"ˮ��#2",hCatogreyItem,kLineItem);

	//������������
	hCatogreyItem = m_LinesTree.InsertItem(L"ů��",hKindItem,hCatogreyItem);//��Parent1�����һ���ӽ�㣬����Child1_1����
	hCatogreyItem = m_LinesTree.InsertItem(L"����",hKindItem,hCatogreyItem);

	hKindItem = m_LinesTree.InsertItem(L"�����",TVI_ROOT,hKindItem);   

	//�ڡ�����塱��������������
	hCatogreyItem = m_LinesTree.InsertItem(L"���",hKindItem);

	//����������
	kLineItem = m_LinesTree.InsertItem(L"���#1",hCatogreyItem);
	kLineItem = m_LinesTree.InsertItem(L"���#2",hCatogreyItem,kLineItem);

#else

	LineList lineList = m_EntryFile->GetList(m_EntryKind);
	acutPrintf(L"\n��ǰ�ļ��С�%d��������.", lineList.size() );

	//��ʼ���������������
	for( LineIterator iter = lineList.begin();
			iter != lineList.end();
			iter++)
	{
		InsertLine((*iter),TRUE);
	}

	//Ĭ��չ�����ڵ�
	m_LinesTree.Expand(TVI_ROOT, TVE_EXPAND);

#endif
	return TRUE;
}

BOOL EntryManageDialog::InitEntryDetailControl()
{
	//��ʼ�����������б�
	{
		int index = 0;
		wstring configKind = IsLineEdit(m_EntryKind) ? GlobalData::CONFIG_LINE_KIND : GlobalData::CONFIG_BLOCK_KIND;
		LineCommonConfigVector* lineKindConfig = LineConfigDataManager::Instance()->FindConfig( configKind );

		for( ConfigIterator iter = lineKindConfig->begin();
			iter != lineKindConfig->end();
			iter++)
		{
			m_LineCategory.InsertString(index++,(*iter)->mName.c_str());
		}

		delete lineKindConfig;
		m_LineCategory.SetCurSel(0);
	}

	//��ʼ���������������б�
	{
		int index = 0;

		/* TODO From configuration file, the string is different from the hard code in source file
		LineCommonConfigVector* lineShapeKind = LineConfigDataManager::Instance()->FindConfig( GlobalData::CONFIG_SHAPE_KIND);

		for( ConfigIterator iter = lineShapeKind->begin();
			iter != lineShapeKind->end();
			iter++)
		{
			//m_LineShape.InsertString(index++,(*iter)->mName.c_str());
		}

		delete lineShapeKind;
		*/

		m_LineShape.InsertString(index++,GlobalData::LINE_SHAPE_CIRCLE.c_str());
		m_LineShape.InsertString(index++,GlobalData::LINE_SHAPE_SQUARE.c_str());
		m_LineShape.InsertString(index++,GlobalData::LINE_SHAPE_GZQPD.c_str());
		m_LineShape.InsertString(index++,GlobalData::LINE_SHAPE_GZQYG.c_str());
		m_LineShape.InsertString(index++,GlobalData::LINE_SHAPE_QQMTX.c_str());

		
		m_LineShape.SetCurSel(0);
	}

	return TRUE;
}

HTREEITEM EntryManageDialog::GetKindNode(const wstring& category, bool createOnDemand )
{
	TVITEM item;

	HTREEITEM hCurrent = m_LinesTree.GetRootItem();
	while (hCurrent != NULL) 
	{
	   // Get the text for the item. Notice we use TVIF_TEXT because
	   // we want to retrieve only the text, but also specify TVIF_HANDLE
	   // because we're getting the item by its handle.
	   TCHAR szText[1024];
	   item.hItem = hCurrent;
	   item.mask = TVIF_TEXT | TVIF_HANDLE;
	   item.pszText = szText;
	   item.cchTextMax = 1024;

	   BOOL bWorked = m_LinesTree.GetItem(&item);

	   // Try to get the next item
	   hCurrent = m_LinesTree.GetNextItem(hCurrent, TVGN_NEXTVISIBLE);

	   // If we successfuly retrieved an item, and the item's text
	   // is the line kind
	   if (bWorked && wstring(item.pszText) == category)
	   {
#ifdef DEBUG
		   acutPrintf(L"\n�������ҵ����������͡�%s��.",category);
#endif
		  //m_LinesTree.DeleteItem(item.hItem);
		  return item.hItem;
	   }
	}

	if( createOnDemand )
	{
		//not find the kind, creat a new kind
#ifdef DEBUG
		acutPrintf(L"\n������û���ҵ����������͡�%s�����ô���.",category.c_str());
#endif

		hCurrent = m_LinesTree.InsertItem(category.c_str(),hCurrent);
		return hCurrent;
	}
	else
	{
		return NULL;
	}
}

HTREEITEM EntryManageDialog::FindKindNode( const UINT& lineID)
{
	HTREEITEM hCurrent = m_LinesTree.GetRootItem();
	while (hCurrent != NULL) 
	{
	   // Get the ID for the item.
	   UINT currentID = (UINT)m_LinesTree.GetItemData(hCurrent);

	   // Check the ID
	   if( currentID == lineID)
	   {
#ifdef DEBUG
		   acutPrintf(L"\n�������ҵ�������ID��%d��.",lineID);
#endif
		  //m_LinesTree.DeleteItem(item.hItem);
		  return hCurrent;
	   }

	   // Try to get the next visible item
	   hCurrent = m_LinesTree.GetNextItem(hCurrent, TVGN_NEXTVISIBLE);
	}

	return hCurrent;
}

BOOL EntryManageDialog::InsertLine( LineEntry* lineEntry, BOOL bInitialize )
{
	//�ж���Ϸ���
	try
	{
		if( !lineEntry )
		{
			CString errorMsg;
			errorMsg.Format(L"����Ϊ��");

			throw ErrorException(errorMsg.GetBuffer());
		}
	}
	catch(const ErrorException& e)
	{
		MessageBoxW(e.errMsg.c_str(), GlobalData::ERROR_DIALOG_CAPTION.c_str(), MB_OK);
		return FALSE;
	}

	//�õ������Ľڵ�
	HTREEITEM parentNode = GetKindNode(lineEntry->m_LineBasiInfo->mCategory,true);
	
	//�����ʵ������
	HTREEITEM newItem = m_LinesTree.InsertItem(lineEntry->m_LineName.c_str(),parentNode);
		
	//Ĭ�ϲ���֮��Ĭ�ϴ򿪸ýڵ�ĸ��ڵ�
	m_LinesTree.Expand(parentNode, TVE_EXPAND);

	//���ֱ��ڵ�ɼ�
	m_LinesTree.EnsureVisible(newItem);

	//������Ǵ������ļ���ʼ���������û��ֹ�����
	if( !bInitialize )
	{		
		//���浽���ݿ�
		lineEntry->m_dbId = ArxWrapper::PostToNameObjectsDict(lineEntry->m_pDbEntry,LineEntry::LINE_ENTRY_LAYER);

		//������ݿ����ָ�룬��AutoCAD����
		lineEntry->m_pDbEntry = NULL;

		//�������ݵ�������
		m_EntryFile->InsertLine(lineEntry);

		//���ø���Ϊѡ��
		m_LinesTree.Select(newItem, TVGN_CARET);
	}

	//���ø����ID
	m_LinesTree.SetItemData(newItem,(DWORD_PTR)lineEntry->m_LineID);

	return TRUE;
}

BOOL EntryManageDialog::UpdateLine( LineEntry* lineEntry )
{
	//�ж���Ϸ���
	try
	{
		if( !lineEntry )
		{
			CString errorMsg;
			errorMsg.Format(L"����Ϊ��");

			throw ErrorException(errorMsg.GetBuffer());
		}
	}
	catch(const ErrorException& e)
	{
		MessageBoxW(e.errMsg.c_str(), GlobalData::ERROR_DIALOG_CAPTION.c_str(), MB_OK);
		return FALSE;
	}

	//�õ���ڵ�
	HTREEITEM hItem = FindKindNode(lineEntry->m_LineID);

	//�򿪸��ڵ�
	HTREEITEM hParent = m_LinesTree.GetParentItem(hItem);
	if (hParent != NULL)
		m_LinesTree.Expand(hParent, TVE_EXPAND);

	//���ֱ��ڵ�ɼ�
	m_LinesTree.EnsureVisible(hItem);

	//����������
	if( hItem )
	{
		//TODO ������
		m_LinesTree.SetItemText(hItem, lineEntry->m_LineName.c_str());

		//�����Ƴ�ԭ�е����ݿ�������
		ArxWrapper::DeleteFromNameObjectsDict(lineEntry->m_dbId, LineEntry::LINE_ENTRY_LAYER);

		//�½��µ����ݿ�������
		lineEntry->m_pDbEntry = new LineDBEntry( lineEntry );

		//���浽���ݿ�
		lineEntry->m_dbId = ArxWrapper::PostToNameObjectsDict(lineEntry->m_pDbEntry,LineEntry::LINE_ENTRY_LAYER);

		//��������
		m_EntryFile->UpdateLine(lineEntry);

		//���浽�����ļ�(��ʱ������)
		//m_EntryFile->Persistent();
	}

	return TRUE;
}

BOOL EntryManageDialog::InitEntryPointsControl()
{
#ifdef DEBUG
	acutPrintf(L"\n��ʼ�������е����������.");
#endif

	int index = 0;

	//�������
	m_LineDetailList.InsertColumn(0, L"���", LVCFMT_LEFT, 80);
	m_LineDetailList.InsertColumn(1, L"X����", LVCFMT_LEFT, 100);
	m_LineDetailList.InsertColumn(2, L"Y����", LVCFMT_LEFT, 100);
	m_LineDetailList.InsertColumn(3, L"�߳�", LVCFMT_LEFT, 100);

	//�����ֻ��
	m_LineDetailList.SetColumnReadOnly(0);

	//Ĭ�ϱ༭��༭
	m_LineDetailList.SetDefaultEditor(NULL, NULL, &m_PointEdit);

	m_LineDetailList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	return TRUE;
}

BOOL EntryManageDialog::InitEntryPointsData(LineEntry* lineEntry)
{
	m_LineDetailList.DeleteAllItems();

	acutPrintf(L"\n��ʼ��������Ϣ.");

	if( lineEntry && lineEntry->m_PointList )
	{
		acutPrintf(L"\n�������߶Ρ�%d����.",lineEntry->m_PointList->size()-1);
		int index = 0;
		for( PointIter iter = lineEntry->m_PointList->begin();
			iter != lineEntry->m_PointList->end();
			iter++)
		{
			if(*iter)
			{
				CString temp;

				//���
				temp.Format(L"%d",index+1);
				m_LineDetailList.InsertItem(index,temp);

				//x
				temp.Format(L"%0.2f",(*iter)->m_Point[X]);
				m_LineDetailList.SetItemText(index, 1, temp);

				//y
				temp.Format(L"%0.2f",(*iter)->m_Point[Y]);
				m_LineDetailList.SetItemText(index, 2, temp);
				
				//z
				temp.Format(L"%0.2f",(*iter)->m_Point[Z]);
				m_LineDetailList.SetItemText(index, 3, temp);

				index++;
			}
		}
	}

	UpdateData(FALSE);

	return TRUE;
}

EntryManageDialog::~EntryManageDialog()
{
#ifdef DEBUG
	if( m_EntryFile )
		acutPrintf(L"\n�����ļ���%s����ʵ�����Ի���ر���.",m_EntryFile->m_FileName.c_str());
#endif
}

void EntryManageDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//�б�
	DDX_Control(pDX, IDC_TREE_LINES, m_LinesTree);

	//��ϸ��Ϣ
	DDX_Control(pDX, IDC_COMBO_CATEGORY, m_LineCategory);
	DDX_Control(pDX, IDC_COMBO_SHAPE, m_LineShape);

	DDX_Control(pDX, IDC_STATIC_DYNAMIC_1, m_StaticDynamic_1);
	DDX_Control(pDX, IDC_EDIT_DANAMIC_1, m_EditDynamic_1);

	DDX_Control(pDX, IDC_STATIC_DYNAMIC_2, m_StaticDynamic_2);
	DDX_Control(pDX, IDC_EDIT_DANAMIC_2, m_EditDynamic_2);

	DDX_Control(pDX, IDC_STATIC_DYNAMIC_3, m_StaticDynamic_3);
	DDX_Control(pDX, IDC_EDIT_DANAMIC_3, m_EditDynamic_3);

	DDX_Control(pDX, IDC_STATIC_DYNAMIC_4, m_StaticDynamic_4);
	DDX_Control(pDX, IDC_EDIT_DANAMIC_4, m_EditDynamic_4);

	DDX_Control(pDX, IDC_STATIC_DYNAMIC_5, m_StaticDynamic_5);
	DDX_Control(pDX, IDC_EDIT_DANAMIC_5, m_EditDynamic_5);

	DDX_Control(pDX, IDC_EDIT_WALL_SIZE,m_LineWallSize);
	DDX_Control(pDX, IDC_EDIT_SAFESIZE,m_LineSafeSize);

	DDX_Control(pDX, IDC_EDIT_PLANE_MARK,m_LinePlaneDesc);
	DDX_Control(pDX, IDC_EDIT_CUT_MARK,m_LineCutDesc);

	//����Ǳ༭�����
	if( !IsLineEdit(m_EntryKind) )
	{
		DDX_Control(pDX, IDC_THROUGH_LEFT,m_ThroughLeft);
		DDX_Control(pDX, IDC_THROUGH_RIGHT,m_ThroughRight);
		DDX_Control(pDX, IDC_THROUGH_RFONT,m_ThroughFront);
		DDX_Control(pDX, IDC_THROUGH_BACK,m_ThroughBack);
		DDX_Control(pDX, IDC_THROUGH_ABOVE,m_ThroughAbove);
		DDX_Control(pDX, IDC_THROUGH_BELLOW,m_ThroughBellow);
	}

	//���߶�
	DDX_Control(pDX, IDC_EDIT_POINT, m_PointEdit);
	DDX_Control(pDX, IDC_LIST_LINE_DETAIL, m_LineDetailList);
	DDX_Control(pDX, IDC_STATIC_DUP_WARNING, m_StaticDuplicateWraning);

	//��ť
	DDX_Control(pDX, IDC_BUTTON_ADD, m_ButtonAdd);
	DDX_Control(pDX, IDC_BUTTON_DEL, m_ButtonDel);

	DDX_Control(pDX, IDOK, m_ButtonOK);
}

LineEntry* EntryManageDialog::GetLineEntry( const UINT& ID )
{
	return m_EntryFile->FindLine(ID);
}

BEGIN_MESSAGE_MAP(EntryManageDialog, CDialog)

	//��ť��Ӧ
	ON_BN_CLICKED(IDOK, OnBnClickedButtonOK)

	ON_BN_CLICKED(IDC_BUTTON_ADD, OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DEL, OnBnClickedButtonDel)
	
	//����ѡ��
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_LINES, OnTreeSelChanged)

	//��������ѡ��
	ON_CBN_SELCHANGE(IDC_COMBO_CATEGORY,	OnCbnCategoryChange)
	ON_CBN_SELCHANGE(IDC_COMBO_CATEGORY,	OnControlValueChange)

	ON_CBN_SELCHANGE(IDC_COMBO_SHAPE,		OnCbnShapeChange)
	ON_CBN_SELCHANGE(IDC_COMBO_SHAPE,		OnControlValueChange)
	
	ON_EN_CHANGE(IDC_EDIT_DANAMIC_1,		OnControlValueChange)
	ON_EN_CHANGE(IDC_EDIT_DANAMIC_2,		OnControlValueChange)
	ON_EN_CHANGE(IDC_EDIT_DANAMIC_3,		OnControlValueChange)

	ON_BN_CLICKED(IDC_THROUGH_LEFT,			OnControlValueChange)
	ON_BN_CLICKED(IDC_THROUGH_RIGHT,		OnControlValueChange)
	ON_BN_CLICKED(IDC_THROUGH_RFONT,		OnControlValueChange)
	ON_BN_CLICKED(IDC_THROUGH_BACK,			OnControlValueChange)
	ON_BN_CLICKED(IDC_THROUGH_ABOVE,		OnControlValueChange)
	ON_BN_CLICKED(IDC_THROUGH_BELLOW,		OnControlValueChange)

	ON_EN_CHANGE(IDC_EDIT_WALL_SIZE,		OnControlValueChange)
	ON_EN_CHANGE(IDC_EDIT_SAFESIZE,			OnControlValueChange)
	
	ON_EN_CHANGE(IDC_EDIT_PLANE_MARK,		OnControlValueChange)
	ON_EN_CHANGE(IDC_EDIT_CUT_MARK,			OnControlValueChange)

	ON_BN_DOUBLECLICKED(IDC_LIST_LINE_DETAIL ,	OnControlValueChange)

END_MESSAGE_MAP()

void EntryManageDialog::OnBnClickedButtonOK()
{
	//ȷ�ϰ�ť������
	m_ButtonOK.EnableWindow(false);

	//�õ�������ϸ��Ϣ
	LineCategoryItemData* detailInfo = CreateEntryDetailInfo();
	if( detailInfo == NULL )
	{
		acutPrintf(L"\n������Ϣ¼�벻����.");
		return;
	}

	//�õ����߶�����
	PointList* pointList = CreateEntryPointList();
	if( pointList == NULL )
	{
		acutPrintf(L"\n���߶���Ϣ¼�벻����.");
		return;
	}

	if( m_OperType == OPER_ADD )
	{
		//�õ�ʵ������
		wstring pipeName = m_EntryFile->GetNewPipeName(detailInfo,L"");
		acutPrintf(L"\n�������ߡ�%s��,���߶Ρ�%d����.",pipeName.c_str(),pointList->size());

		//�����µĹ���
		LineEntry* newLine = new LineEntry(pipeName,m_EntryKind,detailInfo,NULL);

		//���ɸ����ID
		newLine->m_LineID = (UINT)GetTickCount();

		//�������߶�(�ڴ˽����ػ�)
		newLine->SetPoints( pointList );

		//����µĹ��ߣ�ͬʱ��ӵ����ݿ⣩
		InsertLine(newLine);

		//���ò�������Ϊ����
		SetOperType( OPER_UPDATE );
	}
	else /*if( m_OperType == OPER_UPDATE )*/
	{
		//�õ���ǰ�༭��ֱ��
		LineEntry* selectLine = GetSelectLine();

		//�����µ�����
		if( selectLine )
		{
			acutPrintf(L"\n���¹��ߡ�%s��,���߶Ρ�%d����.",
				selectLine->m_LineName.c_str(),
				pointList->size());

			//�õ��µ�ʵ������
			selectLine->m_LineName = m_EntryFile->GetNewPipeName(detailInfo,selectLine->m_LineName);

			//���û�����Ϣ
			selectLine->SetBasicInfo( detailInfo );

			//�������߶���Ϣ
			selectLine->SetPoints( pointList );

			//���¹���
			UpdateLine(selectLine);
		}
	}

	//Ĭ�Ͻ�����ʵ�Ӿ���ʽ
	acedCommand(RTSTR, _T("._VSCURRENT"), RTSTR, L"R", 0);

	//Ĭ�Ͻ���XY��ͼ
	acedCommand(RTSTR, _T("._-VIEW"), RTSTR, L"TOP", 0);

	//���浽��ʱ�ļ�(������Ҫ�˹���)
	//m_EntryFile->Persistent();
}

LineEntry* EntryManageDialog::GetSelectLine()
{
	HTREEITEM selectedItem = m_LinesTree.GetSelectedItem();

	if( selectedItem == NULL)
		 return NULL;

	UINT selectedID = (UINT)m_LinesTree.GetItemData(selectedItem);

	return GetLineEntry(selectedID);
}

void EntryManageDialog::OnBnClickedButtonAdd()
{
	//�жϵ�ǰ�����Ƿ���Ҫ����
	CheckUIData();

	//���пռ����
	EnableDetailControl(true);

	//������е�ҳ������
	ClearLineData();

	//���ò�������
	SetOperType(OPER_ADD);

	//Ĭ�Ͽ�ʼ����뾶
	m_EditDynamic_1.GetFocus();
}

void EntryManageDialog::OnBnClickedButtonDel()
{
	//ѡ�����
	HTREEITEM selectedItem = m_LinesTree.GetSelectedItem();

	if( selectedItem == NULL)
		 return;

	UINT selectedID = (UINT)m_LinesTree.GetItemData(selectedItem);

	//�õ�ѡ�������
	LineEntry* pEntry = m_EntryFile->FindLine(selectedID);

	if( pEntry )
	{
		// Initializes the variables to pass to the MessageBox::Show method.
		CString message;
		message.Format(L"ȷʵҪɾ��[%s]��?",pEntry->m_LineName.c_str());

		LPCTSTR caption = L"ɾ��ʵ��";

		// Displays the MessageBox.
		int result = MessageBoxW(message, caption, MB_OKCANCEL);
		if ( result == IDOK )
		{
			//�����ݿ�ɾ�����߱���
			ArxWrapper::DeleteFromNameObjectsDict(pEntry->m_dbId,LineEntry::LINE_ENTRY_LAYER);

			//�����ݿ�ɾ���������е��߶�
			ArxWrapper::eraseLMALine(*pEntry);

			//ɾ�����е��ڴ�ڵ�
			pEntry->ClearPoints();

			//ɾ���߶μ���
			m_EntryFile->DeleteLine(selectedID);

			//���û�и����͵Ĺ��ߣ���ɾ������ڵ�
			RemoveLineFromTree(selectedItem);
		}
	}
}

BOOL EntryManageDialog::InitEntryData()
{
	//ѡ�����
	UINT selectedID = GetSelectLineID();

	//�õ�ѡ�������
	LineEntry* pEntry = m_EntryFile->FindLine(selectedID);

	//�������
	FillLineData(pEntry);

	return TRUE;
}

UINT EntryManageDialog::GetSelectLineID() const
{
	//ѡ�����
	HTREEITEM selectedItem = m_LinesTree.GetSelectedItem();
		
	UINT selectedID = (UINT)m_LinesTree.GetItemData(selectedItem);

	return selectedID;
}

void EntryManageDialog::OnTreeSelChanged(LPNMHDR pnmhdr, LRESULT *pLResult)
{
	//�����ĳһ�ѡ��
	if( pnmhdr->code == TVN_SELCHANGED )
	{
		//ѡ�������ڵ�ʱ�����ж�
		CheckUIData();

		//ѡ�����
		UINT selectedID = GetSelectLineID();

		if( selectedID != 0 )
		{
			//���ѡ�е�����
			InitEntryData();

			//����ȷ�ϰ�ť������
			m_ButtonOK.EnableWindow(false);
			m_ButtonDel.EnableWindow(true);

			//���ؼ�����
			EnableDetailControl(true);

			//�����޸�����
			m_LineCategory.EnableWindow(false);

			//��ʾ����
			UpdateData(false);

			//��̬��ʾ���
			ShowDynamicControl();
		}
		else
		{	
			//���пռ����
			EnableDetailControl(false);

			//������е�ҳ������
			ClearLineData();
		}
	}
}

void EntryManageDialog::FillLineData( LineEntry* lineEntry )
{
	if( lineEntry )
	{
		//������ϸ����
		FillComobBox(m_LineCategory, lineEntry->m_LineBasiInfo->mCategory.c_str());
		FillComobBox(m_LineShape, lineEntry->m_LineBasiInfo->mShape);

		//�������
		if( lineEntry->m_LineBasiInfo->mShape == GlobalData::LINE_SHAPE_CIRCLE )
		{
			m_EditDynamic_1.SetWindowText(lineEntry->m_LineBasiInfo->mRadius.c_str());
		}
		else if ( lineEntry->m_LineBasiInfo->mShape == GlobalData::LINE_SHAPE_SQUARE )
		{
			m_EditDynamic_1.SetWindowText(lineEntry->m_LineBasiInfo->mWidth.c_str());
			m_EditDynamic_2.SetWindowText(lineEntry->m_LineBasiInfo->mHeight.c_str());
		}

		m_EditDynamic_3.SetWindowText(L"0");
		m_EditDynamic_4.SetWindowText(L"0");
		m_EditDynamic_5.SetWindowText(L"0");

		m_LineWallSize.SetWindowText(lineEntry->m_LineBasiInfo->mWallSize.c_str());
		m_LineSafeSize.SetWindowText(lineEntry->m_LineBasiInfo->mSafeSize.c_str());

		m_LinePlaneDesc.SetWindowText(lineEntry->m_LineBasiInfo->mPlaneMark.c_str());
		m_LineCutDesc.SetWindowText(lineEntry->m_LineBasiInfo->mCutMark.c_str());

		if( !IsLineEdit(m_EntryKind) )
		{
			FillLineThroughDirection(lineEntry->m_LineBasiInfo->mThroughDirection);
		}

		//����������Ϣ
		InitEntryPointsData(lineEntry);
	}

	UpdateData(FALSE);
}

void EntryManageDialog::FillComobBox(CComboBox& comboBox, const wstring& value)
{
	for( int i = 0; i < comboBox.GetCount(); i++ )
	{
		CString entity;
		comboBox.GetLBText(i,entity);

		if( wstring(entity.GetBuffer()) == value )
		{
			//acutPrintf(L"\nѡ���������еĵڡ�%d����");
			comboBox.SetCurSel(i);
		}
	}
}

LineCategoryItemData* EntryManageDialog::CreateEntryDetailInfo()
{
	UpdateData(TRUE);

	CString lineCategory,lineShape,lineRadius,lineWidth,lineHeight,
			lineSafeSize,lineWallSize,lineThroughDirect,
			linePlaneDesc,lineCutDesc;

	//�õ�������������Ϣ
	m_LineCategory.GetWindowTextW(lineCategory);
	m_LineShape.GetWindowTextW(lineShape);
	
	lineRadius = L"0";
	lineWidth = L"0";
	lineHeight = L"0";

	if( wstring(lineShape.GetBuffer()) == GlobalData::LINE_SHAPE_CIRCLE )
	{
		m_EditDynamic_1.GetWindowTextW(lineRadius);

		if( lineRadius == L"0" )
		{
			MessageBoxW(L"�뾶���������", GlobalData::ERROR_DIALOG_CAPTION.c_str(), MB_OK);
			return NULL;
		}
	}
	else if ( wstring(lineShape.GetBuffer()) == GlobalData::LINE_SHAPE_SQUARE )
	{
		m_EditDynamic_1.GetWindowTextW(lineWidth);
		m_EditDynamic_2.GetWindowTextW(lineHeight);

		if( lineWidth == L"0" || lineHeight == L"0" )
		{
			MessageBoxW(L"���߱��붼������", GlobalData::ERROR_DIALOG_CAPTION.c_str(), MB_OK);
			return NULL;
		}
	}

	m_LineWallSize.GetWindowTextW(lineWallSize);
	m_LineSafeSize.GetWindowTextW(lineSafeSize);

	m_LinePlaneDesc.GetWindowTextW(linePlaneDesc);
	m_LineCutDesc.GetWindowTextW(lineCutDesc);
	
	if( !IsLineEdit(m_EntryKind) )
	{
		lineThroughDirect = GetLineThrough();
	}

	//׼���������ݽṹ��
	LineCategoryItemData* categoryData = new LineCategoryItemData();

	categoryData->mCategory = wstring(lineCategory.GetBuffer());
	categoryData->mShape = wstring(lineShape.GetBuffer());

	categoryData->mRadius = wstring(lineRadius.GetBuffer());
	categoryData->mWidth = wstring(lineWidth.GetBuffer());
	categoryData->mHeight = wstring(lineHeight.GetBuffer());

	categoryData->mWallSize = wstring(lineWallSize.GetBuffer());
	categoryData->mSafeSize = wstring(lineSafeSize.GetBuffer());

	categoryData->mPlaneMark = wstring(linePlaneDesc.GetBuffer());
	categoryData->mCutMark = wstring(lineCutDesc.GetBuffer());

	categoryData->mThroughDirection = wstring(lineThroughDirect.GetBuffer());

	return categoryData;
}

PointList* EntryManageDialog::CreateEntryPointList()
{
	UpdateData(TRUE);

	PointList* newPoints = new PointList();

	CString temp;
	for( int i = 0; i < m_LineDetailList.GetItemCount(); i++ )
	{
		PointEntry* point = new PointEntry();

		//�õ���ǰ��ţ��������б��е����кţ�
		point->m_PointNO = (UINT)i;

		temp = m_LineDetailList.GetItemText(i,1);
		acdbDisToF(temp.GetBuffer(), -1, &((point->m_Point)[X]));

		temp = m_LineDetailList.GetItemText(i,2);
		acdbDisToF(temp.GetBuffer(), -1, &((point->m_Point)[Y]));

		temp = m_LineDetailList.GetItemText(i,3);
		acdbDisToF(temp.GetBuffer(), -1, &((point->m_Point)[Z]));

		//���뵽������
		newPoints->push_back(point);
	}

	//Check whether there are endpoint(s) at same position
	wstring duplicateMsg;
	if( HasDuplicatePoint( newPoints, duplicateMsg ) )
	{
		acutPrintf(L"\n��%s��",duplicateMsg.c_str() );
		MessageBoxW(duplicateMsg.c_str(), L"����", MB_OK);

		for( PointIter iter = newPoints->begin();
			iter != newPoints->end();
			iter++ )
		{
			delete (*iter);
		}

		delete newPoints;
		newPoints = NULL;
	}

	return newPoints;
}

void EntryManageDialog::ClearLineData()
{
	//�����ϸ����
	m_LineCategory.SetCurSel(0);
	CString category;
	m_LineCategory.GetWindowTextW(category);
	wstring lineCategory(category.GetBuffer());

	m_LineShape.SetCurSel(0);
	ShowDynamicControl();

	m_EditDynamic_1.SetWindowText(L"0");
	m_EditDynamic_2.SetWindowText(L"0");
	m_EditDynamic_3.SetWindowText(L"0");
	m_EditDynamic_4.SetWindowText(L"0");
	m_EditDynamic_5.SetWindowText(L"0");

	m_LineWallSize.SetWindowText(L"0");

	wstring defaultSafeSize = LineConfigDataManager::Instance()->FindDefaultSafeSize(lineCategory);
	m_LineSafeSize.SetWindowText(defaultSafeSize.c_str());

	m_LinePlaneDesc.SetWindowText(L"��");
	m_LineCutDesc.SetWindowText(L"��");

	if( !IsLineEdit(m_EntryKind) )
	{
		ClearLineThroughDirection();
	}

	//������ߵ�����
	m_LineDetailList.DeleteAllItems();

	//����ɾ��������
	m_ButtonDel.EnableWindow(false);
	m_ButtonOK.EnableWindow(false);

	UpdateData(FALSE);
}

void EntryManageDialog::EnableDetailControl(bool enable)
{
	m_LineCategory.EnableWindow(enable);
	m_LineShape.EnableWindow(enable);

	m_EditDynamic_1.EnableWindow(enable);
	m_EditDynamic_2.EnableWindow(enable);
	m_EditDynamic_3.EnableWindow(enable);
	m_EditDynamic_4.EnableWindow(enable);
	m_EditDynamic_5.EnableWindow(enable);

	m_LineWallSize.EnableWindow(enable);
	m_LineSafeSize.EnableWindow(enable);

	m_LinePlaneDesc.EnableWindow(enable);
	m_LineCutDesc.EnableWindow(enable);

	m_LineDetailList.EnableWindow(enable);

	if( !IsLineEdit(m_EntryKind) )
	{
		m_ThroughLeft.EnableWindow(enable);
		m_ThroughRight.EnableWindow(enable);
		m_ThroughFront.EnableWindow(enable);
		m_ThroughBack.EnableWindow(enable);
		m_ThroughAbove.EnableWindow(enable);
		m_ThroughBellow.EnableWindow(enable);
	}
}

void EntryManageDialog::OnCbnCategoryChange()
{
	if( m_OperType == OPER_ADD )
	{
		UpdateData(true);

		CString category;
		m_LineCategory.GetWindowTextW(category);
		wstring lineCategory(category.GetBuffer());

		wstring defaultSafeSize = LineConfigDataManager::Instance()->FindDefaultSafeSize(lineCategory);
		m_LineSafeSize.SetWindowText(defaultSafeSize.c_str());
	}
}

void EntryManageDialog::OnCbnShapeChange()
{
	ShowDynamicControl();
}

void EntryManageDialog::OnControlValueChange()
{
	//ȷ�ϰ�ť����
	m_ButtonOK.EnableWindow(true);

	UpdateData(false);
}

void EntryManageDialog::ShowDynamicControl()
{
	UpdateData(true);

	//�������������еĶ�̬�ؼ�
	HideDynamicControl();

	//�������ʾ��̬�ؼ�
	CString shapeSelected;
	m_LineShape.GetWindowTextW(shapeSelected);
	wstring shape(shapeSelected.GetBuffer());

	int index = m_LineShape.GetCurSel();

	if( shape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		m_StaticDynamic_1.SetWindowTextW(L"�ھ� (mm)");

		m_StaticDynamic_1.ShowWindow( true );
		m_EditDynamic_1.ShowWindow( true );
	}
	else if( shape == GlobalData::LINE_SHAPE_SQUARE )
	{
		m_StaticDynamic_1.SetWindowTextW(L"���� (mm)");
		m_StaticDynamic_2.SetWindowTextW(L"���� (mm)");

		m_StaticDynamic_1.ShowWindow( true );
		m_EditDynamic_1.ShowWindow( true );

		m_StaticDynamic_2.ShowWindow( true );
		m_EditDynamic_2.ShowWindow( true );
	}
	else if( shape == GlobalData::LINE_SHAPE_GZQPD )
	{
		m_StaticDynamic_1.SetWindowTextW(L"���� (mm)");
		m_StaticDynamic_2.SetWindowTextW(L"ʸ�� (mm)");
		m_StaticDynamic_3.SetWindowTextW(L"ǽ�� (mm)");

		m_StaticDynamic_1.ShowWindow( true );
		m_EditDynamic_1.ShowWindow( true );

		m_StaticDynamic_2.ShowWindow( true );
		m_EditDynamic_2.ShowWindow( true );

		m_StaticDynamic_3.ShowWindow( true );
		m_EditDynamic_3.ShowWindow( true );
	}
	else if( shape == GlobalData::LINE_SHAPE_GZQYG )
	{
		m_StaticDynamic_1.SetWindowTextW(L"���� (mm)");
		m_StaticDynamic_2.SetWindowTextW(L"��ʸ�� (mm)");
		m_StaticDynamic_3.SetWindowTextW(L"��ʸ�� (mm)");
		m_StaticDynamic_4.SetWindowTextW(L"ǽ�� (mm)");

		m_StaticDynamic_1.ShowWindow( true );
		m_EditDynamic_1.ShowWindow( true );

		m_StaticDynamic_2.ShowWindow( true );
		m_EditDynamic_2.ShowWindow( true );

		m_StaticDynamic_3.ShowWindow( true );
		m_EditDynamic_3.ShowWindow( true );

		m_StaticDynamic_4.ShowWindow( true );
		m_EditDynamic_4.ShowWindow( true );
	}
	else if( shape == GlobalData::LINE_SHAPE_QQMTX )
	{
		m_StaticDynamic_1.SetWindowTextW(L"��ʸ�� (mm)");
		m_StaticDynamic_2.SetWindowTextW(L"��ʸ�� (mm)");
		m_StaticDynamic_3.SetWindowTextW(L"��ʸ�� (mm)");
		m_StaticDynamic_4.SetWindowTextW(L"��ʸ�� (mm)");
		m_StaticDynamic_5.SetWindowTextW(L"ǽ�� (mm)");

		m_StaticDynamic_1.ShowWindow( true );
		m_EditDynamic_1.ShowWindow( true );

		m_StaticDynamic_2.ShowWindow( true );
		m_EditDynamic_2.ShowWindow( true );

		m_StaticDynamic_3.ShowWindow( true );
		m_EditDynamic_3.ShowWindow( true );

		m_StaticDynamic_4.ShowWindow( true );
		m_EditDynamic_4.ShowWindow( true );

		m_StaticDynamic_5.ShowWindow( true );
		m_EditDynamic_5.ShowWindow( true );
	}

	UpdateData(false);
}

void EntryManageDialog::HideDynamicControl()
{
	m_StaticDynamic_1.ShowWindow( false );
	m_EditDynamic_1.ShowWindow( false );

	m_StaticDynamic_2.ShowWindow( false );
	m_EditDynamic_2.ShowWindow( false );

	m_StaticDynamic_3.ShowWindow( false );
	m_EditDynamic_3.ShowWindow( false );

	m_StaticDynamic_4.ShowWindow( false );
	m_EditDynamic_4.ShowWindow( false );

	m_StaticDynamic_5.ShowWindow( false );
	m_EditDynamic_5.ShowWindow( false );
}

void EntryManageDialog::CheckUIData()
{
	if( m_ButtonOK.IsWindowEnabled() )
	{
		int result = MessageBoxW(L"�����Ѹ��ģ��Ƿ񱣴�", L"����", MB_OKCANCEL);

		if( result == IDOK )
		{
			OnBnClickedButtonOK();
		}
	}
}

/// <summary>
/// Lines the point modified.
/// </summary>
/// <param name="dialog">The dialog.</param>
/// <param name="row">The row.</param>
void EntryManageDialog::LinePointModified(void* dialog, int row)
{
	acutPrintf(L"\n���߹������ص�����������");
	EntryManageDialog* entryDlg(NULL);

	if( entryDlg = static_cast<EntryManageDialog*>(dialog) )
	{
		acutPrintf(L"\nĬ��Ϊ��ֵ�����˱仯");
		
		//�ؼ�������ֵ�仯
		entryDlg->OnControlValueChange();
	
		//check whether the is the duplicate point(s) for the editing row

		//current status is a row is created
		if( row == -1 )
		{
			//To check second last row instead
			row = entryDlg->m_LineDetailList.GetItemCount() - 2;

			//Check only when there have more than three row
			if( row >= 1 )
			{
				entryDlg->CheckDuplicateValue( row, true );
			}
		}
		else //normal edit status 
		{
			entryDlg->CheckDuplicateValue( row, false );
		}
	}
}

void EntryManageDialog::CheckDuplicateValue( int row, BOOL excludeLast )
{
	UpdateData(TRUE);

	//get the editing value firstly
	CString editX = m_LineDetailList.GetItemText(row, 1);
	CString editY = m_LineDetailList.GetItemText(row, 2);
	CString editZ = m_LineDetailList.GetItemText(row, 3);

	CString duplicatPoint;

	//check other rows 
	CString compX, compY, compZ;
	for( int i = 0; i < m_LineDetailList.GetItemCount(); i++ )
	{
		//exclude current line
		if( i == row )
			continue;

		//for the last row is duplicated from second last, for exclude this when the row is new created
		if( excludeLast && 
				i == m_LineDetailList.GetItemCount() - 1 )
			continue;

		compX = m_LineDetailList.GetItemText(i,1);
		compY = m_LineDetailList.GetItemText(i,2);
		compZ = m_LineDetailList.GetItemText(i,3);

		//X,Y,Z are all same
		if( editX.Compare( compX ) == 0 
			&& editY.Compare( compY ) == 0
			&& editZ.Compare( compZ ) == 0 )
		{
			//���뵽������
			acutPrintf(L"\n�ڡ�%d����ڡ�%d����������غ�", row+1, i+1);

			if( duplicatPoint.IsEmpty() )
			{
				duplicatPoint.Format(L"%d",i+1);
			}
			else
			{
				CString temp(duplicatPoint);
				duplicatPoint.Format(L"%s, %d",temp.GetBuffer(),i+1);
			}
		}
	}

	//display the warning message
	CString duplicateWarningMsg;
	if( !duplicatPoint.IsEmpty() )
	{
		duplicateWarningMsg.Format(L"ע�⣺\n\n�ڡ�%s�����롾%d����\n������ظ�", duplicatPoint.GetBuffer(), row+1);
		m_StaticDuplicateWraning.SetWindowText( duplicateWarningMsg );

		m_StaticDuplicateWraning.ShowWindow(TRUE);
	}
	else
	{
		duplicateWarningMsg.Empty();
		m_StaticDuplicateWraning.SetWindowText(duplicateWarningMsg);

		m_StaticDuplicateWraning.ShowWindow(FALSE);
	}

	//Display
	UpdateData(FALSE);
}

bool EntryManageDialog::HasDuplicatePoint( PointList* pointList, wstring& duplicateMsg )
{
	bool hasDuplicate = false;

	typedef vector<wstring> DuplcatePoints;
	DuplcatePoints duplicatePoint;

	UINT count =  pointList->size();

	if( count <= 1 )
		return hasDuplicate;

	bool* pointIgnore = new bool[count];
	memset( pointIgnore, 0, sizeof(bool) * count );

	for( int i = 0; i < count; i++ )
	{
		//��������������������
		if( pointIgnore[i] )
			continue;

		const ads_point& pointOutter = pointList->at(i)->m_Point;
		CString duplicate;

		for( int j = i + 1; j < count; j++ )
		{
			const ads_point& pointIntter = pointList->at(j)->m_Point;

			if( abs(pointOutter[X] - pointIntter[X]) <= 0.001 
				&& abs(pointOutter[X] - pointIntter[X]) <= 0.001 
				&& abs(pointOutter[X] - pointIntter[X]) <= 0.001 )
			{
				pointIgnore[j] = true;

				if( duplicate.IsEmpty() )
				{
					duplicate.Format(L"%d", j+1 );
				}
				else
				{
					CString temp(duplicate);
					duplicate.Format(L"%s, %d", temp.GetBuffer(), j+1 );
				}
			}
		}

		if( !duplicate.IsEmpty() )
		{
			CString oneDuplicatePoints;
			oneDuplicatePoints.Format(L"�ڡ�%d����ڡ�%s��������غ�", i+1, duplicate.GetBuffer() );
			duplicatePoint.push_back( wstring(oneDuplicatePoints.GetBuffer()) );
		}
	}

	delete []pointIgnore;

	if( duplicatePoint.size() > 0 )
	{
		hasDuplicate = true;

		for( int i = 0; i < duplicatePoint.size(); i++ )
		{
			duplicateMsg += duplicatePoint[i];

			if( i != duplicatePoint.size() - 1 )
				duplicateMsg += L"\n";
		}
	}

	return hasDuplicate;
}

UINT EntryManageDialog::GetDlgID( const wstring& entryKind )
{
	if( entryKind == GlobalData::KIND_LINE )
	{
		return IDD_DIALOG_ENTRY_MANAGE;
	}
	else
	{
		return IDD_DIALOG_BLOCK_MANAGE;
	}
}
	
void EntryManageDialog::FillLineThroughDirection( const wstring& throughDirection)
{
	m_ThroughLeft.SetCheck( throughDirection.substr(0,1) == L"1" ? BST_CHECKED : BST_UNCHECKED );
	m_ThroughRight.SetCheck( throughDirection.substr(1,1) == L"1" ? BST_CHECKED : BST_UNCHECKED );
	m_ThroughFront.SetCheck( throughDirection.substr(2,1) == L"1" ? BST_CHECKED : BST_UNCHECKED );
	m_ThroughBack.SetCheck( throughDirection.substr(3,1) == L"1" ? BST_CHECKED : BST_UNCHECKED );
	m_ThroughAbove.SetCheck( throughDirection.substr(4,1) == L"1" ? BST_CHECKED : BST_UNCHECKED );
	m_ThroughBellow.SetCheck( throughDirection.substr(5,1) == L"1" ? BST_CHECKED : BST_UNCHECKED );
}

void EntryManageDialog::ClearLineThroughDirection()
{
	m_ThroughLeft.SetCheck(BST_UNCHECKED);
	m_ThroughRight.SetCheck(BST_UNCHECKED);
	m_ThroughFront.SetCheck(BST_UNCHECKED);
	m_ThroughBack.SetCheck(BST_UNCHECKED);
	m_ThroughAbove.SetCheck(BST_UNCHECKED);
	m_ThroughBellow.SetCheck(BST_UNCHECKED);
}

CString EntryManageDialog::GetLineThrough()
{
	CString lineThrough;

	lineThrough.Format(L"%s%s%s%s%s%s",
		m_ThroughLeft.GetCheck() == BST_CHECKED ? L"1" : L"0",
		m_ThroughRight.GetCheck() == BST_CHECKED ? L"1" : L"0",
		m_ThroughFront.GetCheck() == BST_CHECKED ? L"1" : L"0",
		m_ThroughBack.GetCheck() == BST_CHECKED ? L"1" : L"0",
		m_ThroughAbove.GetCheck() == BST_CHECKED ? L"1" : L"0",
		m_ThroughBellow.GetCheck() == BST_CHECKED ? L"1" : L"0");

	return lineThrough;
}

/// <summary>
/// Removes the empty parent.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void EntryManageDialog::RemoveLineFromTree( HTREEITEM& lineTreeNode )
{
	BOOL shouldRemoveParent = TRUE;
	HTREEITEM hSiblingItem = NULL;

	//Has previous sibling item
	hSiblingItem = m_LinesTree.GetNextItem(lineTreeNode, TVGN_PREVIOUS);
	if( hSiblingItem != NULL )
	{
		acutPrintf(L"\n���滹���ڸ����͵Ĺ���");
		shouldRemoveParent = FALSE;
	}

	//Has next sibling item previous
	hSiblingItem = m_LinesTree.GetNextItem(lineTreeNode, TVGN_NEXT);
	if( hSiblingItem != NULL )
	{
		acutPrintf(L"\n���滹���ڸ����͵Ĺ���");
		shouldRemoveParent = FALSE;
	}

	//Store the parent item
	HTREEITEM hParent = m_LinesTree.GetNextItem(lineTreeNode, TVGN_PARENT);
	
	//ɾ����ǰ�ڵ�
	m_LinesTree.DeleteItem(lineTreeNode);

	//�����ڵ�
	if( shouldRemoveParent )
	{
		acutPrintf(L"\nӦ��ɾ������ڵ�");
		if(hParent != NULL)
		{
			acutPrintf(L"\nɾ������ڵ�");
			m_LinesTree.DeleteItem(hParent);
		}
	}
}

// EntryManageDialog message handlers

} // end of entry

} // end of assistant

} // end of guch

} // end of com
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               