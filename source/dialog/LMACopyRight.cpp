// LineCutPosDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LMACopyRight.h"

IMPLEMENT_DYNAMIC(LMACopyRight, CAcUiDialog)

LMACopyRight::LMACopyRight(CWnd* pParent /*=NULL*/)
	: CAcUiDialog(LMACopyRight::IDD, pParent)
{
}

LMACopyRight::~LMACopyRight()
{
}

BOOL LMACopyRight::OnInitDialog()
{
	//��ҳ�潻������
	CAcUiDialog::OnInitDialog();

	return TRUE;
}

void LMACopyRight::DoDataExchange(CDataExchange* pDX)
{
	CAcUiDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(LMACopyRight, CAcUiDialog)
	ON_BN_CLICKED(IDOK, &LMACopyRight::OnBnClickedOk)
END_MESSAGE_MAP()

void LMACopyRight::OnBnClickedOk()
{
	//�رնԻ���
	CAcUiDialog::OnOK();
}
