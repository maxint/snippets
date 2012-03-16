#pragma once

class CSimpleMFCView : public CView
{
public:
	CSimpleMFCView();
	DECLARE_DYNCREATE(CSimpleMFCView);

protected:
	virtual void OnDraw(CDC* pDC);

protected:
	DECLARE_MESSAGE_MAP()
};