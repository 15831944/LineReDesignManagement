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

#define MAIN_MENU_LINE_CUT_BACK L"�ָ��Ӵ�"

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

