#pragma once

namespace NM
{
	namespace IPRTREE
	{
		class PTree;
	}
}

class CViewTreeCtrl : public CTreeCtrl
{
private:


public:
	CViewTreeCtrl();

protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

public:
	virtual ~CViewTreeCtrl();

protected:
	DECLARE_MESSAGE_MAP()
};
