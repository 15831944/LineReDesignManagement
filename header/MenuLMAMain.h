#pragma once

#include "afxwin.h"

#include "aced.h"

/*
* ���˵�
*/

#define MAIN_MENU_NAME L"���߸������"

/*
* ���߲˵�
*/

#define MAIN_MENU_LINE_MANAGE L"��������"

#define MAIN_MENU_LINE_IMPORT L"�������ݵ���"

#define MAIN_MENU_LINE_INPUT L"��������¼��"

#define MAIN_MENU_LINE_EXPORT L"�������ݵ���"

/*
* �����˵�
*/

#define MAIN_MENU_BLOCK_MANAGE L"���������"

#define MAIN_MENU_BLOCK_IMPORT L"��������ݵ���"

#define MAIN_MENU_BLOCK_INPUT L"���������¼��"

#define MAIN_MENU_BLOCK_EXPORT L"��������ݵ���"

/*
* ��ͼ�˵�
*/

#define MAIN_MENU_LINE_CUT_MANAGE L"������ͼ"

#define MAIN_MENU_LINE_CUT L"������ͼ"

#define MAIN_MENU_LINE_CUT_BACK L"ɾ����ͼ"

/*
* �ڵ��˵�
*/

#define MAIN_MENU_LINE_SHADOW_MANAGE L"�ڵ���ͼ"

#define MAIN_MENU_LINE_SHADOW L"�����ڵ�"

#define MAIN_MENU_LINE_SHADOW_BACK L"ɾ���ڵ�"

/*
* ���޲˵�
*/

#define MAIN_MENU_LINE_INTERACT_MANAGE L"��������"

#define MAIN_MENU_LINE_INTERACT L"�����ж�"

#define MAIN_MENU_LINE_INTERACT_BACK L"ɾ���ж�"

/*
* ·���ж�
*/

#define MAIN_MENU_LINE_ROUTE L"Ѱ��·��"


/*
* ����
*/

#define MAIN_MENU_LINE_TEST L"���ܲ���"

class MenuLMAMain : public AcEdUIContext
{

public:

	MenuLMAMain(void);

	~MenuLMAMain(void);

    virtual void* getMenuContext(const AcRxClass *pClass, const AcDbObjectIdArray& ids) ;
    virtual void  onCommand(Adesk::UInt32 cmdIndex);
    virtual void  OnUpdateMenu();

	void onAction(const CString& menuName);

private:

	HMENU m_tempHMenu;
    CMenu *mpMenu;
};

