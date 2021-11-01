#include "stdafx.h"
#include "BreadCrumb.h"
#include "NetworkView.h"

BreadCrumb::BreadCrumb(CNetworkView* pNetworkView):
	_pView(pNetworkView)
{
}

BreadCrumb::~BreadCrumb()
{
}

void BreadCrumb::OnSelectionChanged(HBREADCRUMBITEM hSelectedItem)
{
	CString selectionText;
	selectionText.Format(L"\nSelection Path is %s.", GetItemPath(hSelectedItem));
	OutputDebugString(selectionText);
}