#include "MenuLMAMain.h"

#include "Resource.h"
#include <acdocman.h>
#include <acutads.h>

#include "StdAfx.h"

#include <CommandManager.h>

extern HINSTANCE _hdllInstance = NULL;//ȫ�ֱ���

MenuLMAMain::MenuLMAMain(void)
{
	acutPrintf(L"\n��ʼ�������˵���");
	acDocManagerPtr()->pushResourceHandle(_hdllInstance);

	this->mpMenu = new CMenu();
	this->mpMenu->LoadMenu(IDR_MENU1);// ID_LMA_MAIN_MENU ��Ҫ����Ĳ˵� 

	acDocManager->popResourceHandle();
	acutPrintf(L"\n���˵�������ɡ�");
}

MenuLMAMain::~MenuLMAMain(void)
{
	if (mpMenu) 
		delete mpMenu;
}

void* MenuLMAMain::getMenuContext(const AcRxClass *, const AcDbObjectIdArray&)
{
	acutPrintf(L"\n�õ��Ӳ˵�");
	m_tempHMenu = mpMenu->GetSubMenu(0)->GetSafeHmenu();   
	return &m_tempHMenu;
}

void MenuLMAMain::onCommand(Adesk::UInt32 cmdIndex)
{
	acDocManager->pushResourceHandle(_hdllInstance);

	CString strMenuTitle,strPrompt;
	mpMenu->GetMenuString(cmdIndex,strMenuTitle,MF_BYCOMMAND);

	strPrompt.Format(L"\n���Ѿ�ѡȡ�˲˵�:%s\n",strMenuTitle);
	acutPrintf(strPrompt); // ���ǵ�������ʾ��һ���˵��ѡ��

	onAction(strMenuTitle);

	acedPostCommandPrompt();
	acDocManager->pushResourceHandle(_hdllInstance); // �л���ǰʹ�õ���Դ
}

void MenuLMAMain::OnUpdateMenu()
{
	//mpMenu->EnableMenuItem(ID_LINE_CONFIG,MF_GRAYED); // ʹ�˵����
	//mpMenu->EnableMenuItem(ID_INPUT_POINT,MF_ENABLED); // ʹ�ܲ˵���
	//mpMenu->CheckMenuItem(ID_GENERATE_CUT, MF_BYCOMMAND|MF_CHECKED); // ��ѡ�˵��� 
}

void MenuLMAMain::onAction(const CString& menuName)
{
	if( menuName == MAIN_MENU_LINE_INPUT )
	{
		CommandManager::LineManage();
	}
	else if( menuName == MAIN_MENU_LINE_EXPORT )
	{
		CommandManager::ExportLine();
	}
	else if( menuName == MAIN_MENU_LINE_IMPORT )
	{
		CommandManager::ImportLine();
	}
	if( menuName == MAIN_MENU_BLOCK_INPUT )
	{
		CommandManager::BlockManage();
	}
	else if( menuName == MAIN_MENU_LINE_EXPORT )
	{
		CommandManager::ExportBlock();
	}
	else if( menuName == MAIN_MENU_LINE_IMPORT )
	{
		CommandManager::ImportBlock();
	}
	else if( menuName == MAIN_MENU_LINE_CUT )
	{
		CommandManager::GenerateCut();
	}
	else if( menuName == MAIN_MENU_LINE_CUT_BACK )
	{
		CommandManager::GenerateCutBack();
	}
	else if( menuName == MAIN_MENU_LINE_SHADOW )
	{
		CommandManager::GenerateCut();
	}
	else if( menuName == MAIN_MENU_LINE_CUT_BACK )
	{
		CommandManager::LineShadow();
	}
	else if( menuName == MAIN_MENU_LINE_SHADOW_BACK )
	{
		CommandManager::LineShadowBack();
	}
	else if( menuName == MAIN_MENU_LINE_CALCULATE_ROUTE )
	{
		CommandManager::LineRoute();
	}
	else if( menuName == MAIN_MENU_LINE_DELETE_ROUTE )
	{
		CommandManager::LineRouteBack();
	}
	else
	{
		acutPrintf(L"\nδ����Ĳ˵��%s��",const_cast<CString&>(menuName).GetBuffer());
	}
}

