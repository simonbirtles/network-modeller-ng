#pragma once
#include <BCGPBreadcrumb.h>

class CNetworkView;

class BreadCrumb : public CBCGPBreadcrumb
{

public:
	virtual void OnSelectionChanged(HBREADCRUMBITEM hSelectedItem);

public:
	BreadCrumb(CNetworkView* pNetworkView);
	virtual ~BreadCrumb();

private:
	CNetworkView* _pView;		// Pointer to Network View Class 

};

