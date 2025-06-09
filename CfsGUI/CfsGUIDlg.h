#pragma once
#include "CfsUsb.h"
#include <vector>

#define WM_THREAD_FINISHED (WM_APP + 1)

typedef void (CALLBACK* FUNC_Initialize)();
typedef void (CALLBACK* FUNC_Finalize)();
typedef bool (CALLBACK* FUNC_PortOpen)(int);
typedef void (CALLBACK* FUNC_PortClose)(int);
typedef bool (CALLBACK* FUNC_SetSerialMode)(int, bool);
typedef bool (CALLBACK* FUNC_GetSerialData)(int, double*, char*);
typedef bool (CALLBACK* FUNC_GetLatestData)(int, double*, char*);
typedef bool (CALLBACK* FUNC_GetSensorLimit)(int, double*);
typedef bool (CALLBACK* FUNC_GetSensorInfo)(int, char*);

struct SensorDataRecord
{
	double timestamp_ms;
	double Fx, Fy, Fz, Mx, My, Mz;
};

class CfsGUIDlg : public CDialogEx
{
public:
	CfsGUIDlg(CWnd* pParent = nullptr);

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
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonPlot();

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
};