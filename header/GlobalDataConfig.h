#pragma once

#include "stdafx.h"
#include <string>

using namespace std;

namespace com
{

namespace guch
{

namespace assistant
{

namespace data
{

class GlobalData
{
public:
	
	/**
	* ��������
	**/
	static const wstring KIND_LINE;
	static const wstring KIND_BLOCK;

	/**
	* �ܵ�����
	**/
	static const wstring LINE_CATEGORY_SHANGSHUI;
	static const wstring LINE_CATEGORY_XIASHUI;
	static const wstring LINE_CATEGORY_NUANQI;
	static const wstring LINE_CATEGORY_DIANLAN;
	static const wstring LINE_CATEGORY_YUSUI;
	static const wstring LINE_CATEGORY_TONGXIN;

	/**
	* ���������
	**/
	static const wstring BLOCK_CATEGORY_WUSHUI;
	static const wstring BLOCK_CATEGORY_HANGDAO;
	static const wstring BLOCK_CATEGORY_NUANQIGUAN;
	static const wstring BLOCK_CATEGORY_GUODAO;
	static const wstring BLOCK_CATEGORY_CHEZHAN;

	/**
	* ��Խ����
	**/
	static const wstring BLOCK_CATEGORY_ABOVE;
	static const wstring BLOCK_CATEGORY_BELLOW;

	/**
	* �ܵ���״
	**/
	static const wstring LINE_SHAPE_CIRCLE;		//Բ��
	static const wstring LINE_SHAPE_SQUARE;		//����
	static const wstring LINE_SHAPE_GZQPD;		//��ֱǽƽ��
	static const wstring LINE_SHAPE_GZQYG;		//��ֱǽ����
	static const wstring LINE_SHAPE_QQMTX;		//��ǽ(������)

	/**
	* �ܵ���λ
	**/
	static const wstring LINE_UNIT_MM;
	static const wstring LINE_UNIT_CM;
	static const wstring LINE_UNIT_M;

	/**
	* �ַ�����
	**/
	static const UINT ITEM_TEXT_MAX_LENGTH;

	/**
	* �������
	**/
	static const wstring ERROR_DIALOG_CAPTION;

	/**
	* ��������
	**/
	static const wstring CONFIG_LINE_KIND;
	static const wstring CONFIG_SHAPE_KIND;
	static const wstring CONFIG_BLOCK_KIND;

	/**
	* ��ɫ����
	**/
	static const int LINE_COLOR;
	static const int WALLLINE_COLOR;
	static const int SAFELINE_COLOR;
	static const int INTERSET_COLOR;
	static const int INTERSET_WALLLINE_COLOR;
	/**
	* ͸����
	**/
	static const double TRANS_SAFELINE_COLOR;
	
	/**
	* ���ߵĻ��Ƽ���
	**/
	enum LineProirity {
		LINE_NORMAL = 0, 
		LINE_FIRST,
		LINE_SECOND };
};


} // end of data

} // end of assistant

} // end of guch

} // end of com