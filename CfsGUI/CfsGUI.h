
// CfsGUI.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CCfsGUIApp:
// このクラスの実装については、CfsGUI.cpp を参照してください
//

class CCfsGUIApp : public CWinApp
{
public:
	CCfsGUIApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CCfsGUIApp theApp;
