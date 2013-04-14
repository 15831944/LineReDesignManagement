// ------------------------------------------------
//                  LineManagementAssistant
// Copyright 2012-2013, Chengyong Yang & Changhai Gu. 
//               All rights reserved.
// ------------------------------------------------
//	LineEntryData.cpp
//	written by Changhai Gu
// ------------------------------------------------
// $File:\\LineManageAssitant\main\source\data\LineEntryData.h $
// $Author: Changhai Gu $
// $DateTime: 2013/1/2 01:35:46 $
// $Revision: #1 $
// ------------------------------------------------

#include <LineEntryData.h>
#include <LMAUtils.h>
#include <GlobalDataConfig.h>

#include <ArxWrapper.h>
#include <acdocman.h>
#include <acutmem.h>
#include <dbsol3d.h>

#include <LineManageAssitant.h>

using namespace std;
using namespace ::com::guch::assistant::config;
extern wstring gLmaArxLoadPath;

#define NEW_LINE L"\r\n"

namespace com
{

namespace guch
{

namespace assistant
{

namespace data
{

///////////////////////////////////////////////////////////////////////////
// Implementation LineEntityFile

/**
 * ����ʵ���ļ�
 */
LineEntityFile::LineEntityFile(const wstring& fileName, bool import)
	:m_FileName(fileName)
{
	m_LineList = new LineList();
	m_LinePoint = new LinePointMap();

	if( import )
		Import();
}

LineEntityFile::~LineEntityFile()
{
	if( m_LineList )
	{
		for( LineIterator iter = m_LineList->begin();
				iter != m_LineList->end();
				iter++ )
		{
			//TODO no need to do for the database??
		}

		delete m_LineList;
	}

	if( m_LinePoint )
		delete m_LinePoint;
}

void LineEntityFile::Import()
{
	CFile archiveFile;

	//read data from file LMA_CONFIG_FILE
	BOOL result = archiveFile.Open(this->m_FileName.c_str(),CFile::modeRead);
	if( !result )
	{
		acutPrintf(L"\n�򿪹���ʵ�������ļ�ʧ��!");
		return;
	}

	//�õ��ļ����ݳ���
	int length = (int)archiveFile.GetLength()+1;

	//�õ��ļ���խ�ַ�����
	char* content = new char[length];
	memset(content,0,length);
	archiveFile.Read(content,length);

	//����ת��Ϊ���ַ�
	string strCnt(content,length);
	wstring wContent = StringToWString( strCnt );

	//���һس��Ծ�����
	size_t lineFrom(0), linePos(0);

	do
	{
		//����һ���ַ���ʼ������һ����������
		linePos = wContent.find(LineEntity::LINE_DATA_BEGIN,lineFrom + 1);

		//�õ�һ����������
		if( linePos == wstring::npos )
			linePos = wContent.length();

		wstring wLine = wContent.substr(lineFrom, linePos-lineFrom);

		//ɾ�����еĻس�����
		wstring::size_type pos = 0;
		while ( (pos = wLine.find(NEW_LINE, pos)) != wstring::npos ) 
		{
			wLine.replace( pos, wstring(NEW_LINE).length(), L"" );
			pos++;
		}

#ifdef DEBUG
		acutPrintf(L"\n�õ�һ�й���ʵ�����ݡ�%s��.",wLine.c_str());
#endif

		if(wLine.length() == 0)
			break;

		LineEntity *newLine = new LineEntity(wLine);
		m_LineList->push_back( newLine );

		//���浽���ݿ�
		newLine->m_dbId = ArxWrapper::PostToNameObjectsDict(newLine->m_pDbEntry,LineEntity::LINE_ENTRY_LAYER);
		newLine->m_pDbEntry = NULL;

		//������Ӧ������
		newLine->CreateDbObjects();

		//��ס��ǰ��λ��
		lineFrom = linePos;
	}
	while( lineFrom != wContent.length() );

	//�ر��ļ�
	archiveFile.Close();
}

void LineEntityFile::Persistent() const
{
	acutPrintf(L"\n�־û���������.");

	//ExportTo(this->m_FileName);
}

void LineEntityFile::ExportTo(const wstring& filename,const wstring& lineKind) const
{
	acutPrintf(L"\n����ʵ������.");

	CString exportFile;
	exportFile.Format(L"%s",filename.c_str());
	CFile archiveFile(exportFile,CFile::modeCreate|CFile::modeWrite);

	//�������е����Ͷ���
	for( ConstLineIterator iter = m_LineList->begin(); 
			iter != m_LineList->end(); 
			iter++)
	{
		LineEntity* data = *iter;

		if( data 
			&& data->m_LineKind == lineKind )
		{
			//�õ���Ϣ�Ŀ��ַ����л�
			wstring wData = data->toString();

			//תΪխ�ַ�
			string dataStr = WstringToString(wData);

#ifdef DEBUG
			acutPrintf(L"\n����ʵ�����ݡ�%s��.",wData.c_str());
#endif
			//ʹ�� virtual void Write( const void* lpBuf, UINT nCount ); ��խ�ַ�д���ļ�
			archiveFile.Write(dataStr.c_str(),(UINT)dataStr.size());

			//ʹ��WindowsĬ�ϵĻس�������
			archiveFile.Write("\r\n",(UINT)strlen("\r\n"));
		}
	}

	acutPrintf(L"\n����ʵ�����ݵ������.");
	archiveFile.Close();
}

void LineEntityFile::InsertLine(LineEntity* lineEntry)
{
	if( lineEntry )
		m_LineList->push_back(lineEntry);
}

void LineEntityFile::InsertLine( LineList* lineList)
{
	if( lineList == NULL || lineList->size() == 0)
		return;

	for( LineIterator iter = lineList->begin();
			iter != lineList->end();
			iter++)
	{
		m_LineList->push_back((*iter));
	}
}

BOOL LineEntityFile::UpdateLine(LineEntity* lineEntry)
{
	LineIterator iter = FindLinePos(lineEntry->m_LineID);

	if( iter != this->m_LineList->end())
	{
		(*iter)->m_LineName = lineEntry->m_LineName;
		return TRUE;
	}

	return FALSE;
}

BOOL LineEntityFile::DeleteLine( const UINT& lineID )
{
	LineIterator iter = FindLinePos(lineID);

	if( iter != this->m_LineList->end())
	{
		m_LineList->erase(iter);
		return TRUE;
	}
	else
		return FALSE;
}

LineIterator LineEntityFile::FindLinePos( const UINT& lineID ) const
{
	for( LineIterator iter = this->m_LineList->begin();
			iter != this->m_LineList->end();
			iter++)
	{
		if( (*iter)->m_LineID == lineID )
			return iter;
	}

	return m_LineList->end();
}

LineIterator LineEntityFile::FindLinePosByNO( const wstring& lineNO ) const
{
	for( LineIterator iter = this->m_LineList->begin();
			iter != this->m_LineList->end();
			iter++)
	{
		//if( (*iter)->m_LineNO == lineNO )
		//	return iter;
	}

	return m_LineList->end();
}

LineIterator LineEntityFile::FindLinePosByName( const wstring& lineName ) const
{
	for( LineIterator iter = this->m_LineList->begin();
			iter != this->m_LineList->end();
			iter++)
	{
		if( (*iter)->m_LineName == lineName )
			return iter;
	}

	return m_LineList->end();
}

LineEntity* LineEntityFile::FindLine( const UINT& lineID ) const
{
	LineIterator iter = FindLinePos(lineID);

	if( iter != m_LineList->end())
		return (*iter);
	else
		return NULL;
}

LineEntity* LineEntityFile::FindLineByName( const wstring& lineName ) const
{
	LineIterator iter = FindLinePosByName(lineName);

	if( iter != m_LineList->end())
		return (*iter);
	else
		return NULL;
}

LineEntity* LineEntityFile::FindLineByNO( const wstring& lineNO ) const
{
	LineIterator iter = FindLinePosByNO(lineNO);

	if( iter != m_LineList->end())
		return (*iter);
	else
		return NULL;
}

LineEntity* LineEntityFile::HasAnotherLineByNO( const UINT& lineID, const wstring& lineNO  ) const
{
	for( LineIterator iter = this->m_LineList->begin();
			iter != this->m_LineList->end();
			iter++)
	{
		//if( (*iter)->m_LineNO == lineNO && (*iter)->m_LineID != lineID)
		//	return (*iter);
	}

	return NULL;
}

LineEntity* LineEntityFile::HasAnotherLineByByName( const UINT& lineID, const wstring& lineName  ) const
{
	for( LineIterator iter = this->m_LineList->begin();
			iter != this->m_LineList->end();
			iter++)
	{
		if( (*iter)->m_LineName == lineName && (*iter)->m_LineID != lineID)
			return (*iter);
	}

	return NULL;
}

PointList* LineEntityFile::GetTempLine( const UINT& lineID )
{
	LinePointMap::iterator iter = m_LinePoint->find(lineID);

	if( iter == m_LinePoint->end() )
	{
		PointList* newList = new PointList();
		m_LinePoint->insert( std::pair<UINT,PointList*>(lineID,newList));

		return newList;
	}
	else
	{
		return iter->second;
	}
}

PointList* LineEntityFile::TransferTempLine( const UINT& lineID )
{
	LinePointMap::iterator iter = m_LinePoint->find(lineID);

	if( iter == m_LinePoint->end() )
	{
		return NULL;
	}
	else
	{
		PointList* findList = iter->second;
		m_LinePoint->erase(iter);

		return findList;
	}
}

wstring LineEntityFile::GetNewPipeName( const LineCategoryItemData* pipeCategoryData, const wstring& orinalName )
{
	//Find name exist, plus sequence
	int index = 1;
	const wstring& pipeCategory = pipeCategoryData->mCategory;

	while(true)
	{
		CString pipeName;
		
		//����_��״_�ߴ�_���
		CString shape;
		if( pipeCategoryData->mShape == GlobalData::LINE_SHAPE_CIRCLE )
		{
			shape.Format(L"%s_%s",pipeCategoryData->mShape.c_str(),pipeCategoryData->mRadius.c_str());
		}
		else if( pipeCategoryData->mShape == GlobalData::LINE_SHAPE_SQUARE )
		{
			shape.Format(L"%s_%sx%s",pipeCategoryData->mShape.c_str(),
				pipeCategoryData->mWidth.c_str(),pipeCategoryData->mHeight.c_str());
		}
		else
		{
			shape.Format(L"%s",pipeCategoryData->mShape.c_str());
		}

		pipeName.Format(L"%s_%s_%d",pipeCategory.c_str(),shape.GetBuffer(),index);

		//����Ǹ���
		if( orinalName.length() != 0 )
		{
			//��������ޱ仯��ֱ�ӷ���
			if( wstring(pipeName.GetBuffer()) == orinalName )
			{
				return wstring(pipeName.GetBuffer());
			}
		}

		//���û�й�����������֣�������
		if( this->FindLineByName(pipeName.GetBuffer()) == NULL )
		{
			return wstring(pipeName.GetBuffer());
		}
		else
		{
			//���кŵ���
			index++;
		}
	}

	return pipeCategory;
}

LineList LineEntityFile::GetList( const wstring& entityKind ) const
{
	LineList kindList;

	//��ʼ���������������
	for( LineIterator iter = m_LineList->begin();
			iter != m_LineList->end();
			iter++)
	{
		if( (*iter)->m_LineKind == entityKind )
		{
			kindList.push_back((*iter));
		}
	}

	return kindList;
}

/////////////////////////////////////////////////////////////////////////

EntryFileList* LineEntityFileManager::pEntryFileList = NULL;
bool LineEntityFileManager::openingDwg = false;

void LineEntityFileManager::ReadFromCurrentDWG()
{
#ifdef DEBUG
	acutPrintf(L"\n�ӵ�ǰDWG�ļ���ȡ���ݡ�");
#endif

	//ArxWrapper::PullFromNameObjectsDict();
}

void LineEntityFileManager::RemoveEntryFileOnDWGUnLoad()
{
#ifdef DEBUG
		acutPrintf(L"\nDWG�ļ��ر��ˣ�ɾ���������ݡ�");

		if( pEntryFileList )
		{
			for( EntryFileIter iter = pEntryFileList->begin();
					iter != pEntryFileList->end();
					iter++)
			{
				delete (*iter);
			}

			pEntryFileList->clear();
		}
#endif
}



LineEntityFile* LineEntityFileManager::GetLineEntryFile( const wstring& fileName )
{
	if( pEntryFileList == NULL )
	{
		pEntryFileList = new EntryFileList();
#ifdef DEBUG
		acutPrintf(L"\n�ļ�ʵ���������δ����.");
#endif
		return NULL;
	}

	for( EntryFileIter iter = pEntryFileList->begin();
			iter != pEntryFileList->end();
			iter++)
	{
		if( (*iter)->m_FileName == fileName )
			return (*iter);
	}

#ifdef DEBUG
	acutPrintf(L"\nû���ҵ��ļ���%s����Ӧ�Ĺ���ʵ��.",fileName.c_str());
#endif

	return NULL;
}

LineEntityFile* LineEntityFileManager::RegisterEntryFile(const wstring& fileName)
{
	LineEntityFile* entryFile = GetLineEntryFile(fileName);
	if( entryFile == NULL )
	{
		acutPrintf(L"\n������%s����Ӧ�Ĺ���ʵ��.",fileName.c_str());

		entryFile = new LineEntityFile(fileName);
		pEntryFileList->push_back( entryFile );
	}

	return entryFile;
}

LineEntityFile* LineEntityFileManager::SaveFileEntity()
{
	wstring fileName( curDoc()->fileName() );

	acutPrintf(L"\n�ļ�����Ϊ[%s].",fileName.c_str());

	return GetLineEntryFile(fileName);
}

bool LineEntityFileManager::RegisterLineSegment( const wstring& fileName, UINT lineID, UINT sequence, PointEntity*& pStart, PointEntity*& pEnd )
{
	//�ҵ��ļ�������
	LineEntityFile* pFileEntry = RegisterEntryFile(fileName);
	acutPrintf(L"\n����߶�ʱ���ҵ�����ʵ���ļ���������%s��.",fileName.c_str());

	//�ҵ�ʵ����
	LineEntity* lineEntry = pFileEntry->FindLine(lineID);
	PointList* pPointList = NULL;

	if( lineEntry == NULL )
	{
#ifdef DEBUG
		acutPrintf(L"\n���浽��ʱ���߹�������.");
#endif
		pPointList = pFileEntry->GetTempLine( lineID );
	}

	if( sequence == 1 )
	{
#ifdef DEBUG
		acutPrintf(L"\n���к�Ϊ1�����ǵ�һ���߶�.");
#endif
		if( pPointList->size() < 2 )
		{
			pStart = new PointEntity();
			pEnd = new PointEntity();

			pStart->m_PointNO = 0;
			pPointList->push_back( pStart );

			pEnd->m_PointNO = 1;
			pPointList->push_back( pEnd );
		}
		else
		{
			pStart = (*pPointList)[0];
			pEnd = (*pPointList)[1];
		}
	}
	else if ( sequence > 1 )
	{
#ifdef DEBUG
		acutPrintf(L"\n��ͨ�߶�.");
#endif
		if( pPointList->size() <= sequence )
		{
			pEnd = new PointEntity();
			pEnd->m_PointNO = sequence;

			pPointList->push_back( pEnd );
		}
		else
		{
			pEnd = (*pPointList)[sequence];
		}
	}
	else if ( sequence == 0)
	{
		acutPrintf(L"\nʧЧ���߶�.");
	}

	return true;
}

LineEntityFile* LineEntityFileManager::GetCurrentLineEntryFile()
{
	//Get current filename
	wstring fileName = curDoc()->fileName();

	acutPrintf(L"\n���ҡ�%s����Ӧ�ĵĹ���.",fileName.c_str());

	return RegisterEntryFile(fileName);
}

BOOL LineEntityFileManager::ImportLMALineFile( const wstring& lineKind )
{
	//����ѡ��Ի���
	CString szFilter;
	szFilter.Format(L"%s", IsLineEdit(lineKind) ? L"���������ļ� (*.ldt)|*.ldt||" : L"����������ļ� (*.bdt)|*.bdt||");
	CFileDialog dlg(TRUE, IsLineEdit(lineKind) ? L"ldt" : L"bdt", L"", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter.GetBuffer(), 
					CWnd::FromHandle(adsw_acadMainWnd()), 0/*, TRUE*/);

	//If user hasn't export once,use the arx load path
	if( !HasUserSavedFlagData( LMA_VERSION ) )
	{
		acutPrintf(L"\nĬ��Ŀ¼Ϊ�������ϵͳ�ĳ���Ŀ¼.");
		dlg.m_ofn.lpstrInitialDir = gLmaArxLoadPath.c_str();
	}

	if (dlg.DoModal() == IDOK) 
	{
		//�õ���ǰ���ļ�ʵ�������
		LineEntityFile* currentFile = GetCurrentLineEntryFile();

		//�õ������ļ�
        CString impFile = dlg.GetPathName();
		acutPrintf(L"\n��������ļ���%s��.",impFile.GetBuffer());
		LineEntityFile* importFile = new LineEntityFile(impFile.GetBuffer(),true);

		//�������еĹ���
		currentFile->InsertLine( importFile->GetList() );

		//ɾ����ʱ������ļ�ʵ��
		delete importFile;

		//Set the exported flag, then next time use the user's last save/open
		if( !HasUserSavedFlagData( LMA_VERSION ) )
		{
			acutPrintf(L"\n�û������õ���Ŀ¼.");
			PlaceUserSavedFlagData( LMA_VERSION );
		}

		//Ĭ�Ͻ�����ʵ�Ӿ���ʽ
		acedCommand(RTSTR, _T("._VSCURRENT"), RTSTR, L"R", 0);

		//Ĭ�Ͻ���XY��ͼ
		acedCommand(RTSTR, _T("._VIEW"), RTSTR, L"TOP", 0);

        return(TRUE);
    }
    else
        return(FALSE);
}

BOOL LineEntityFileManager::ExportLMALineFile( const wstring& lineKind )
{
	//ȥ�����ļ���׺��dwg��
	wstring fileName( curDoc()->fileName() );
	fileName = fileName.substr(0, fileName.find_first_of(L"."));

	//����ѡ��Ի���
	CString szFilter;
	szFilter.Format(L"%s", IsLineEdit(lineKind) ? L"���������ļ� (*.ldt)|*.ldt||" : L"����������ļ� (*.bdt)|*.bdt||");
	CFileDialog dlg(FALSE, IsLineEdit(lineKind) ? L"ldt" : L"bdt", fileName.c_str(), 
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter.GetBuffer(), 
					CWnd::FromHandle(adsw_acadMainWnd()), 0/*, TRUE*/);

	//If user hasn't export once,use the arx load path
	if( !HasUserSavedFlagData( LMA_VERSION ) )
	{
		acutPrintf(L"\nĬ��Ŀ¼Ϊ�������ϵͳ�ĳ���Ŀ¼.");
		dlg.m_ofn.lpstrInitialDir = gLmaArxLoadPath.c_str();
	}

	if (dlg.DoModal() == IDOK) 
	{
        CString expFile = dlg.GetPathName();
		acutPrintf(L"\n�����������ݵ��ļ���%s��.",expFile.GetBuffer());

		LineEntityFile* exportFile = GetLineEntryFile(wstring(curDoc()->fileName()));

		if( exportFile )
			exportFile->ExportTo(expFile.GetBuffer(),lineKind);

		//Set the exported flag, then next time use the user's last save/open
		if( !HasUserSavedFlagData( LMA_VERSION ) )
		{
			acutPrintf(L"\n�û������õ���Ŀ¼.");
			PlaceUserSavedFlagData( LMA_VERSION );
		}

        return(TRUE);
    }
    else
        return(FALSE);
}

} // end of data

} // end of assistant

} // end of guch

} // end of com