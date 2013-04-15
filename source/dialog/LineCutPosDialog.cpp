// LineCutPosDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LineCutPosDialog.h"

#include "acedads.h"
#include "accmd.h"
#include <adscodes.h>

#include <adsdlg.h>

#include <dbapserv.h>

#include <dbregion.h>

#include <gepnt3d.h>

//symbol table
#include <dbsymtb.h>

#include <acdocman.h>

//3D Object
#include <dbsol3d.h>

// Hatch
#include <dbhatch.h>

// Leader
#include <dblead.h>
#include <dbmleader.h>

#include <ArxWrapper.h>

#include <ArxCustomObject.h>

#include <GlobalDataConfig.h>

#include <LMAUtils.h>

using namespace com::guch::assistant::arx;

// LineCutPosDialog dialog

CString LineCutPosDialog::m_CutLayerName = L"";
AcDbObjectIdArray LineCutPosDialog::m_CutObjects = AcDbObjectIdArray();
wstring LineCutPosDialog::m_CutHatchStyle = L"NET";

IMPLEMENT_DYNAMIC(LineCutPosDialog, CAcUiDialog)

LineCutPosDialog::LineCutPosDialog(CWnd* pParent /*=NULL*/)
	: CAcUiDialog(LineCutPosDialog::IDD, pParent),
	m_Direction(0),
	m_strOffset(0)
{
}

LineCutPosDialog::~LineCutPosDialog()
{
}

BOOL LineCutPosDialog::OnInitDialog()
{
	//��ҳ�潻������
	CAcUiDialog::OnInitDialog();

	//Ĭ��X��ѡ��
	m_DirectionX.SetCheck(BST_CHECKED);
	m_Direction = 1;

	//Ĭ��ƫ��Ϊ0
	m_EditOffset.SetWindowTextW(L"0.00");

	//����ͼƬ
	m_PickCutPosButton.AutoLoad();

	return TRUE;
}

void LineCutPosDialog::DoDataExchange(CDataExchange* pDX)
{
	CAcUiDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_X, m_DirectionX);
	DDX_Control(pDX, IDC_Y, m_DirectionY);
	DDX_Control(pDX, IDC_Z, m_DirectionZ);

	DDX_Control(pDX, IDC_OFFSET, m_EditOffset);

	DDX_Control(pDX, IDC_BUTTON_PICKCUT,m_PickCutPosButton);
}

BEGIN_MESSAGE_MAP(LineCutPosDialog, CAcUiDialog)
	ON_BN_CLICKED(IDC_X, &LineCutPosDialog::OnBnClickedX)
	ON_BN_CLICKED(IDC_Y, &LineCutPosDialog::OnBnClickedY)
	ON_BN_CLICKED(IDC_Z, &LineCutPosDialog::OnBnClickedZ)
	ON_BN_CLICKED(IDC_BUTTON_PICKCUT, &LineCutPosDialog::OnBnPickCutPos)
	ON_BN_CLICKED(IDOK, &LineCutPosDialog::OnBnClickedOk)
END_MESSAGE_MAP()

void LineCutPosDialog::OnBnClickedX()
{
	m_Direction = 1;
}

void LineCutPosDialog::OnBnClickedY()
{
	m_Direction = 2;
}

void LineCutPosDialog::OnBnClickedZ()
{
	m_Direction = 3;
}

void LineCutPosDialog::OnBnPickCutPos()
{
	// Hide the dialog and give control to the editor
	BeginEditorCommand();

	CString temp;
	ads_point pt;

	// Get a point
	if (acedGetPoint(NULL, _T("\nѡȡ�и��: "), pt) == RTNORM) 
	{
		// If the point is good, continue
		CompleteEditorCommand();

		if( m_Direction == 1 )
		{
			temp.Format(_T("%g"), pt[X]);
		}
		else if ( m_Direction == 2 )
		{
			temp.Format(_T("%g"), pt[Y]);
		}
		else if ( m_Direction == 3 )
		{
			temp.Format(_T("%g"), pt[Z]);
		}
	}
	else 
	{
		// otherwise cancel the command (including the dialog)
		CancelEditorCommand();
	}

	m_EditOffset.SetWindowTextW(temp.GetBuffer());
	UpdateData(FALSE);
}

void LineCutPosDialog::OnBnClickedOk()
{
	//�õ��û����������
	UpdateData(FALSE);

	//���Ȼָ���ͼ
	CutBack();

	//�õ�����
	GenerateCutPlane();

	//�õ�ת������
	GenerateTransform();

	//������ͼ
	GenerateCutRegion();

	//��ʾ��ͼ��
	ShowCutRegion();

	//�رնԻ���
	CAcUiDialog::OnOK();
}

void LineCutPosDialog::GenerateCutPlane()
{
	//��������
	CString offset;
	m_EditOffset.GetWindowTextW(offset);

	if( offset.GetLength())
		m_strOffset = _wtoi(offset);

	if( m_Direction == 1)
	{
		m_CutLayerName.Format(L"��X�ᴹֱƫ����Ϊ��%d��������",m_strOffset);
		m_CutPlane.set(AcGePoint3d(m_strOffset,0,0),AcGeVector3d(1,0,0));
	}
	else if( m_Direction == 2)
	{
		m_CutLayerName.Format(L"��Y�ᴹֱƫ����Ϊ��%d��������",m_strOffset);
		m_CutPlane.set(AcGePoint3d(0,m_strOffset,0),AcGeVector3d(0,1,0));
	}
	else if( m_Direction == 3)
	{
		m_CutLayerName.Format(L"��Z�ᴹֱƫ����Ϊ��%d��������",m_strOffset);
		m_CutPlane.set(AcGePoint3d(0,0,m_strOffset),AcGeVector3d(0,0,1));
	}

#ifdef DEBUG
	acutPrintf(L"\n����Ϊ��%s��",m_CutLayerName.GetBuffer());
#endif
}

void LineCutPosDialog::GenerateTransform()
{
	//������Ӧ��ת��
	if( m_Direction == 1 )
	{
		acutPrintf(L"\n������X�ᴹֱ,��ƫ����YZƽ�棬Ȼ��ת��XZƽ�棬���ת��XYƽ��");

		//ƫ����YZƽ��
		m_MoveMatrix.setToTranslation(AcGeVector3d(-m_strOffset,0,0));

		//���з�ת��XZƽ��
		m_RotateMatrixFirst = AcGeMatrix3d::rotation( -ArxWrapper::kRad90, AcGeVector3d::kZAxis, AcGePoint3d::kOrigin);

		//���з�ת��XYƽ��
		m_RotateMatrixSecond = AcGeMatrix3d::rotation( -ArxWrapper::kRad90, AcGeVector3d::kXAxis, AcGePoint3d::kOrigin);
	}
	else if ( m_Direction == 2 )
	{
		acutPrintf(L"\n������Y�ᴹֱ,��ƫ����XZƽ�棬Ȼ����з�ת��XYƽ��");

		//ƫ����XZƽ��
		m_MoveMatrix.setToTranslation(AcGeVector3d(0,-m_strOffset,0));

		//���з�ת��XYƽ��
		m_RotateMatrixFirst = AcGeMatrix3d::rotation( -ArxWrapper::kRad90, AcGeVector3d::kXAxis, AcGePoint3d::kOrigin);
	} 
	else if ( m_Direction == 3 )
	{
		acutPrintf(L"\n������Z�ᴹֱ��ֱ��ƫ����XYƽ�漴��");
	
		//����ƫ��
		m_MoveMatrix.setToTranslation(AcGeVector3d(0,0,-m_strOffset));
	}
}

void LineCutPosDialog::GenerateCutRegion()
{
	ArxWrapper::LockCurDoc();

	//�õ���ǰ��ʵ���ļ�������
	LineEntityFile* pLineFile = LineEntityFileManager::GetCurrentLineEntryFile();
	if( pLineFile == NULL )
	{
		acutPrintf(L"\nû���ҵ������ļ�������������°ɣ�");
		return;
	}

	//�õ�ʵ���б�
	LineList* lineList = pLineFile->GetList();
	if( lineList == NULL )
	{
		acutPrintf(L"\n��ǰ�ļ���û�й��ߣ����������ͼ��");
		return;
	}

	//����ʵ���ļ��������ÿһ��ʵ�������ͼ
	LineIterator lineIter = lineList->begin();

	for(;lineIter != lineList->end();
		lineIter++)
	{
#ifdef DEBUG
		acutPrintf(L"\n�Թ��ߡ�%s��������ͼ��",(*lineIter)->m_LineName.c_str());
#endif
		if( *lineIter != NULL )
			GenerateCutRegion(*lineIter);
	}

	ArxWrapper::UnLockCurDoc();
}

void LineCutPosDialog::GenerateCutRegion(LineEntity* lineEntry)
{
	PointList* pointList = lineEntry->m_PointList;
	if( pointList == NULL )
	{
		acutPrintf(L"\n�ù���û���߶Σ�������ͼ��");
		return;
	}

	//������ͼ�����ע��
	double markOffset = 0;
	if( lineEntry->m_LineBasiInfo->mShape == GlobalData::LINE_SHAPE_CIRCLE )
	{
		double radius;
		acdbDisToF(lineEntry->m_LineBasiInfo->mSize.mRadius.c_str(), -1, &radius);
		markOffset = ( radius * 1.5 ) / 1000 ;
	}
	else// if ( lineEntry->m_LineBasiInfo->mShape == GlobalData::LINE_SHAPE_SQUARE )
	{
		double width,height;
		acdbDisToF(lineEntry->m_LineBasiInfo->mSize.mWidth.c_str(), -1, &width);
		acdbDisToF(lineEntry->m_LineBasiInfo->mSize.mHeight.c_str(), -1, &height);
		markOffset = ( width / 2 + height / 2 ) / 1000;
	}

	//�����е��߶ν��б���
	PointIter pointIter = pointList->begin();
	for(;pointIter != pointList->end();pointIter++)
	{
		if( pointIter == pointList->begin() )
			continue;

		PointEntity* pointEntity = (*pointIter);

		if( pointEntity == NULL )
		{
			acutPrintf(L"\n���߶β��Ϸ�����Ҫע�⣡");
			continue;
		}

#ifdef DEBUG
		acutPrintf(L"\n�Եڡ�%d�����߶ν�����ͼ��",pointEntity->m_PointNO);
#endif

		GenerateCutRegion( pointEntity, markOffset );
	}
}

void LineCutPosDialog::GenerateCutRegion(PointEntity* pointEntity, double markOffset)
{
	//�õ����Ӧ�Ĺ���ʵ�����
	AcDbObjectId lineEntityId = pointEntity->m_DbEntityCollection.GetWallLineEntity();
	if( !lineEntityId.isValid() )
	{
		acutPrintf(L"\n��ǰ�߶�û�ж�Ӧ�Ĺ��߱�ʵ�壬�����ù�����ͼ��");

		lineEntityId = pointEntity->m_DbEntityCollection.GetLineEntity();
		if( !lineEntityId.isValid() )
		{
			acutPrintf(L"\n��ǰ�߶�û�ж�Ӧ�Ĺ���ʵ�壬������ͼ��");
			return;
		}
	}

	AcDbEntity* pLineObj;
	Acad::ErrorStatus es = acdbOpenAcDbEntity(pLineObj, lineEntityId, AcDb::kForRead);

	if( es == Acad::eOk )
	{
		LMALineDbObject* pLMALine = LMALineDbObject::cast(pLineObj);

		if( pLMALine == NULL )
		{
			acutPrintf(L"\n��ǰ�߶β�����Ч�ĸ���ϵͳ�����ʵ�壬��������ͼ��");
			return;
		}

		//�õ�ʵ�����������еĽ���
		AcDbRegion *pSelectionRegion = NULL;
		pLMALine->getSection(m_CutPlane, pSelectionRegion);

		//�õ�ע�͵����ĵ�
		AcGePoint3d centerPoint = pLMALine->GetCutCenter(m_CutPlane);

		//����ע�͵�����
		CString markContent;

		//����û������˱�ע����,���ñ�ע����#�κ�
		if( pointEntity->m_DbEntityCollection.mCategoryData->mCutMark.length() != 0
			&& pointEntity->m_DbEntityCollection.mCategoryData->mCutMark != L"��" )
		{
			markContent.Format(L"%s#%d",pointEntity->m_DbEntityCollection.mCategoryData->mCutMark.c_str(), pointEntity->m_PointNO);
		}
		else //δ�����ע���ݣ����ù�������#�κ�
		{
			markContent.Format(L"%s#%d",pointEntity->m_DbEntityCollection.mLayerName.c_str(), pointEntity->m_PointNO);
		}

		//�ر�ʵ��
		pLMALine->close();

		if( pSelectionRegion )
		{
			acutPrintf(L"\n������������");

			//�����������ڵ�ͼ��
			if( ArxWrapper::createNewLayer(m_CutLayerName.GetBuffer()) == false )
				return;

			//��������뵽ģ�Ϳռ�
			AcDbObjectId regionId = ArxWrapper::PostToModelSpace(pSelectionRegion,m_CutLayerName.GetBuffer());
			m_CutObjects.append(regionId);

			//�����ý�����������
			AcDbObjectId hatchId = CreateHatch(regionId);
			m_CutObjects.append(hatchId);

			//ת�Ƶ�XYƽ��
			AcDbObjectIdArray transformObjs;
			transformObjs.append(regionId);
			transformObjs.append(hatchId);
			TransformToXY( transformObjs );

			AcDbObjectId mLeaderId = CreateMLeader(centerPoint, markContent.GetBuffer(), markOffset);
			m_CutObjects.append(mLeaderId);
		}
		else
		{
			acutPrintf(L"\n������ù��ߣ�����壩���ཻ����");
		}
	}
	else
	{
		acutPrintf(L"\n�򿪹���ʵ��ʧ�ܣ�");
		rxErrorMsg(es);
	}
}

AcDbObjectId LineCutPosDialog::CreateHatch( AcDbObjectId entityId )
{
	acutPrintf(L"\n�������ͼ��,��ʽΪ��%s��",m_CutHatchStyle.c_str());

	if( !entityId.isValid() )
		return 0;

	AcGeVector3d normal = m_CutPlane.normal();

	AcDbObjectIdArray objIds;
	objIds.append(entityId);

	Acad::ErrorStatus es;
	AcDbHatch *pHatch = new AcDbHatch();

	// �������ƽ��
	pHatch->setNormal(normal);

	// ���ø߶�
	pHatch->setElevation(m_strOffset);

	// ���ù�����
	pHatch->setAssociative(true);

	// �������ͼ��
	pHatch->setPattern(AcDbHatch::kPreDefined, m_CutHatchStyle.c_str());

	// ������߽�
	es = pHatch->appendLoop(AcDbHatch::kExternal, objIds);

	// ��ʾ������
	es = pHatch->evaluateHatch();

	// ��ӵ�ģ�Ϳռ�
	return ArxWrapper::PostToModelSpace(pHatch, m_CutLayerName.GetBuffer());
}

AcDbObjectId LineCutPosDialog::CreateMLeader(const AcGePoint3d& start, const wstring& content, double markOffset)
{
	//��ע�����
	AcGePoint3d startPoint(start.x, start.y, 0);

	//������Ӧ��ת��
	{
		if( m_Direction == 1 )
		{
#ifdef DEBUG
			acutPrintf(L"\n������X�ᴹֱ,��Z��λ��ת��ΪY����Y��λ��ת��ΪX");
#endif
			startPoint.y = start.z;
			startPoint.x = start.y;
		}
		else if ( m_Direction == 2 )
		{
#ifdef DEBUG
			acutPrintf(L"\n������Y�ᴹֱ����Z��λ��ת��ΪY��λ��");
#endif		
			startPoint.y = start.z;
		} 
	}

	//�۵�Ϊ����6����λ����λ�������·�
	AcGePoint3d endPointCorner(startPoint.x + markOffset, startPoint.y - markOffset, 0);
		
	//��֤��ע����������Χ��
	double textHeight = markOffset;
	if( textHeight > 2 )
		textHeight = 2;

	AcGePoint3d endPointText(endPointCorner.x + textHeight, endPointCorner.y, 0);

	AcDbObjectId textId;
	{
		//���ñ�ע������
		AcDbMText* mtext = new AcDbMText;
		mtext->setContents(content.c_str());
		mtext->setAttachment(AcDbMText::AttachmentPoint::kBottomLeft);
		mtext->setLocation(endPointText);

		//���ֵĸ߶�Ϊ�뾶�����߸�/��
		acutPrintf(L"\n��%s���ĸ߶�Ϊ��%0.2lf��",content.c_str(), textHeight/2);
		mtext->setTextHeight(textHeight/2);

		textId = ArxWrapper::PostToModelSpace(mtext, m_CutLayerName.GetBuffer());
		m_CutObjects.append(textId);
	}

	AcDbLeader* leader = new AcDbLeader;

	//���ñ�ע������
	{
		leader->appendVertex(startPoint);
		leader->appendVertex(endPointCorner);
		leader->appendVertex(endPointText);

		leader->attachAnnotation(textId);
		leader->evaluateLeader();
	}

	//��ӵ�ģ�Ϳռ���
	return ArxWrapper::PostToModelSpace(leader, m_CutLayerName.GetBuffer());
}

void LineCutPosDialog::TransformToXY(AcDbObjectIdArray entityIds)
{
	Acad::ErrorStatus es;
	AcDbEntity* pEntity = NULL;

	//������Ӧ��ת��
	for( int i = 0; i < entityIds.length(); i++ )
	{
		AcDbObjectId objId = entityIds[i];

		ArxWrapper::LockCurDoc();

		es = acdbOpenAcDbEntity(pEntity, objId, AcDb::kForWrite);

		if( es != Acad::eOk )
		{
			acutPrintf(L"\n��Ҫ����XYƽ���ʵ��ʧ��");
			rxErrorMsg(es);

			ArxWrapper::UnLockCurDoc();
			continue;
		}

		if( m_Direction == 1 )
		{
			//ƫ����YZƽ��
			pEntity->transformBy(m_MoveMatrix);

			//���з�ת��XZƽ��
			pEntity->transformBy(m_RotateMatrixFirst);

			//���з�ת��XYƽ��
			pEntity->transformBy(m_RotateMatrixSecond);
		}
		else if ( m_Direction == 2 )
		{
			//ƫ����XZƽ��
			pEntity->transformBy(m_MoveMatrix);

			//���з�ת��XYƽ��
			pEntity->transformBy(m_RotateMatrixFirst);
		} 
		else if ( m_Direction == 3 )
		{
			//����ƫ�Ƽ���
			pEntity->transformBy(m_MoveMatrix);
		}

		pEntity->close();

		ArxWrapper::UnLockCurDoc();
	}
}

void LineCutPosDialog::ShowCutRegion()
{
	//ֻ��ʾ����ͼ��
	ArxWrapper::ShowLayer(m_CutLayerName.GetBuffer());

	//�л���ͼ
	//ArxWrapper::ChangeView(m_Direction);

	acedCommand(RTSTR, _T("._-VIEW"), RTSTR, L"TOP", 0);
}

void LineCutPosDialog::Reset()
{
	acutPrintf(L"\n���ڵ���ͼΪ��%s����",m_CutLayerName.GetBuffer());

	if( m_CutLayerName.GetLength() > 0 )
	{
		acutPrintf(L"\n�����������ĵ�");
		ArxWrapper::LockCurDoc();

		//acutPrintf(L"\n�ָ�WCS�Ӵ�");
		acedCommand(RTSTR, _T("UCS"), RTSTR, L"W", 0);

		acutPrintf(L"\nɾ����ͼ��صĶ���");
		CutBack();

		acutPrintf(L"\nɾ����ͼ���ڵ�ͼ��");
		if( ArxWrapper::DeleteLayer(m_CutLayerName.GetBuffer(),true) )
		{
			acutPrintf(L"\n��ʼ������");
			m_CutLayerName.Format(L"");
		}

		acutPrintf(L"\n��ʾ����ͼ��");
		ArxWrapper::ShowLayer(L"");

		acutPrintf(L"\n����ĵ�����");
		ArxWrapper::UnLockCurDoc();
	}
	else
	{
		acutPrintf(L"\n��ǰϵͳ��û����ͼ��");
	}
}

void LineCutPosDialog::CutBack()
{
	while( m_CutObjects.length() )
	{
		AcDbObjectId objId = m_CutObjects.at(0);

		if( objId.isValid() )
		{
			ArxWrapper::RemoveDbObject(objId);
		}

		m_CutObjects.removeAt(0);
	}
}

// LineCutPosDialog message handlers
