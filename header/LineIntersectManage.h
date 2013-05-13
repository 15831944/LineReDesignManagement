#pragma once

#include <LineEntryData.h>

using namespace std;
using namespace ::com::guch::assistant::data;

// LineCutPosDialog dialog

namespace com
{

namespace guch
{

namespace assistant
{

namespace Intersect
{

class LineIntersectManage
{

public:

	static LineIntersectManage* Instance();

	//�ж�����״̬
	void CheckInteract();
	
	//ɾ���ϴμ�����޽��������3Dģ��״̬
	void Reset();
	
	typedef struct
	{
		PointEntity* intersetcA;
		PointEntity* intersetcB;
		AcDbObjectId intersctcId;
	} IntersectStruct;

protected:

	static LineIntersectManage* mLineIntersectInstance;

	//�жϱ��ļ���Ĺ����������
	void CheckLineInteract();

	//�ж�һ�����߶����������ߵ��������
	void CheckLineInteract( PointEntity* point );

private:

	LineIntersectManage();

	virtual ~LineIntersectManage();

	void Clear();

private:

	AcArray<IntersectStruct*> mIntersectEntities;
	
	//�����ֱȽϵ����߶�
	set<LinePointID> m_CheckedEntities;

	LineEntityFile* m_pCheckLine;
};

} // end of Intersect

} // end of assistant

} // end of guch

} // end of com