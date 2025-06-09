#pragma once

#ifndef __AFXWIN_H__
#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル
#include "SensorDataTypes.h" // 共通データ型定義（追加）
#include "CfsUsb.h"
#include <vector>
#include <deque>

// 前方宣言
class CRealtimePlotWnd;

class CfsGUIDlg : public CDialogEx
{
public:
	CfsGUIDlg(CWnd* pParent = nullptr);
	virtual ~CfsGUIDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CFSGUI_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON m_hIcon;
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThreadFinished(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonPlot();
	afx_msg void OnBnClickedButtonRtPlot();

	HMODULE m_hDll;
	FUNC_Initialize m_pInitialize;
	FUNC_Finalize m_pFinalize;
	FUNC_PortOpen m_pPortOpen;
	FUNC_PortClose m_pPortClose;
	FUNC_SetSerialMode m_pSetSerialMode;
	FUNC_GetSerialData m_pGetSerialData;
	FUNC_GetLatestData m_pGetLatestData;
	FUNC_GetSensorLimit m_pGetSensorLimit;
	FUNC_GetSensorInfo m_pGetSensorInfo;

	int m_portNo;
	double m_Limit[6];

	CWinThread* m_pThread;
	volatile bool m_bIsThreadRunning;
	std::vector<SensorDataRecord> m_recordedData;
	CString m_lastCsvPath;

	void AddLog(CString str);
	static UINT RecordThreadProc(LPVOID pParam);

private:
	// リアルタイムプロット用
	std::deque<SensorDataRecord> m_realtimeBuffer;
	std::deque<SensorDataRecord> m_tempBuffer;
	CRITICAL_SECTION m_bufferCS;

	static const int MAX_REALTIME_POINTS = 500;
	static const int BUFFER_UPDATE_INTERVAL = 50;
	static const int PLOT_UPDATE_INTERVAL = 100;

	bool m_bShowRealTimePlot;
	int m_plotChannelMask;

	UINT_PTR m_bufferUpdateTimer;
	UINT_PTR m_plotUpdateTimer;

	int m_dataDecimation;
	int m_decimationCounter;

	CRealtimePlotWnd* m_pPlotWindow;

public:
	void AddDataToTempBuffer(const SensorDataRecord& record);
	void UpdateRealtimeBuffer();
};