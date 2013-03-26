#include "stdafx.h"

#include <LineConfigDataManager.h>
#include <GlobalDataConfig.h>
#include <LMAUtils.h>

#include <iostream>
#include <fstream>

#include <assert.h>
#include <acutads.h>

using namespace com::guch::assistant::data;

#pragma warning(disable : 4267)
#pragma warning(disable : 4244)

extern wstring gLmaArxLoadPath;
static wstring gConfigLineEnd = L"\r\n";
static wstring gColumnSegment = L"\t";

namespace com
{

namespace guch
{

namespace assistant
{

namespace config
{

LineConfigDataManager* LineConfigDataManager::instance = NULL;
LPCWSTR LineConfigDataManager::LMA_CONFIG_FILE = L"���߸��Ƹ���ϵͳ�����ļ�.txt";

wstring LineConfigDataManager::CONFIG_LINE_NAME = L"��������";
wstring LineConfigDataManager::CONFIG_SHAPE_NAME = L"������״";
wstring LineConfigDataManager::CONFIG_BLOCK_NAME = L"���������";

LineConfigDataManager* LineConfigDataManager::Instance()
{
	if( instance == NULL )
	{
		instance = new LineConfigDataManager();
	}

	return instance;
}

LineConfigDataManager::LineConfigDataManager(void)
{
	mLineConfigData = new LineCommonConfigVector();

#ifdef _DEMO_DATA
	const int MAX_ITEM = 10;

	for( int i = 0; i < MAX_ITEM; i++)
	{
		CString ID;
		ID.Format(L"%d",i);

		CommonConfig* item = new CommonConfig(wstring(ID.GetBuffer()), 
						L"���Թܵ�",
						GlobalData::KIND_LINE,
						GlobalData::LINE_CATEGORY_SHANGSHUI,
						GlobalData::LINE_SHAPE_CIRCLE,
						L"15",
						L"5",
						GlobalData::LINE_UNIT_CM,
						L"��������");

		mLineConfigData->push_back(item);
	}
#else
	CFile archiveFile;

	try
	{
		//read data from file LMA_CONFIG_FILE
		wstring configFile = gLmaArxLoadPath + L"\\" + LMA_CONFIG_FILE;
		acutPrintf(L"\n�����ļ�·���ǡ�%s��",configFile.c_str());

		BOOL result = archiveFile.Open(configFile.c_str(),CFile::modeRead);
		if( !result )
		{
			acutPrintf(L"\n�򿪹������������ļ�ʧ��.");
			return;
		}

		//�õ��ļ����ݳ���
		int length = (ULONGLONG)archiveFile.GetLength()+1;

		//�õ��ļ���խ�ַ�����
		char* content = new char[length];
		memset(content,0,length);
		archiveFile.Read(content,length);

		//����ת��Ϊ���ַ�
		string strCnt(content,length);
		wstring wContent = StringToWString( strCnt );

		//���һس��Ծ�����
		size_t lineFrom = 0;
		size_t linePos = wContent.find_first_of(gConfigLineEnd,lineFrom);

		wstring category;
		while( linePos != wstring::npos )
		{
			//�õ�һ������
			wstring& wLine = wContent.substr(lineFrom, linePos-lineFrom);

			//ע����
			if( wLine.substr(0,2) == L"##" )
			{
				acutPrintf(L"\nע�͡�%s��", wLine.substr(0,wLine.length()).c_str());
			}
			else if (wLine.substr(0,2) == L"**" )
			{
				//�õ�����
				category = wLine.substr(2);
				acutPrintf(L"\n�õ����ࡾ%s��", category.c_str());
			}
			else
			{
				//�����в��
				size_t columnFrom = 0;
				size_t columnPos = wLine.find_first_of(gColumnSegment,columnFrom);

				CommonConfig* newItem = new CommonConfig();
				newItem->mCategory = category;

				int indexCol = 0;
				while( columnPos != wstring::npos )
				{
					//�õ�һ���ֶ�
					wstring& rColumn = wLine.substr(columnFrom,columnPos-columnFrom);

					//����������
					if( indexCol == 0 )
					{
						newItem->mName = rColumn;
					}

					indexCol++;

					//������һ��column
					columnFrom = columnPos + gColumnSegment.length();
					columnPos =  wLine.find_first_of(gColumnSegment,columnFrom);
				}

				wstring& name = wLine.substr(columnFrom);
				if( indexCol == 0 )
				{
					newItem->mName = name;
				}
				else if( indexCol == 1 )
				{
					newItem->mSubName = name;
				}

				mLineConfigData->push_back(newItem);
				acutPrintf(L"\n��ȡ�������� - ���ࡾ%s�����ࡾ%s�����ࡾ%s��", 
												newItem->mCategory.c_str(),
												newItem->mName.c_str(),
												newItem->mSubName.c_str());
			}

			//����һ���ַ���ʼ��������һ��
			lineFrom = linePos + gConfigLineEnd.length();
			linePos = wContent.find_first_of(gConfigLineEnd,lineFrom);
		}

		//�ر��ļ�
		archiveFile.Close();
	}
	catch(exception& e)
	{
		acutPrintf(L"\n�򿪹������������ļ��쳣��%s��",e.what());
	}

#endif
}

LineConfigDataManager::~LineConfigDataManager(void)
{
}

LineCommonConfigVector* LineConfigDataManager::FindConfig( const wstring& category ) const
{
	LineCommonConfigVector* configLig = new LineCommonConfigVector();

	for( ConfigIterator iter = mLineConfigData->begin();
		iter != mLineConfigData->end();
		iter++)
	{
		//���ǰ��������õ�������ң����ǰ��û��½����ߵ����Ͳ���
		if( (*iter)->mCategory.find(category) != wstring::npos )
		{
			configLig->push_back( (*iter) );
		}
	}

	return configLig;
}

wstring LineConfigDataManager::FindDefaultSafeSize( const wstring& category)
{
	wstring lineSafeSize(L"0");

	for( ConfigIterator iter = mLineConfigData->begin();
		iter != mLineConfigData->end();
		iter++)
	{
		if( (*iter)->mName.find(category) != wstring::npos )
		{
			lineSafeSize = (*iter)->mSubName;
		}
	}

	if( lineSafeSize.length() == 0 )
		lineSafeSize = L"0";

	return lineSafeSize;
}

} // end of data

} // end of assistant

} // end of guch

} // end of com
