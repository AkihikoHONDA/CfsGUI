// CfsGUIDlg.cpp をこの内容で完全に置き換える

#include "pch.h"
#include "framework.h"
#include "CfsGUI.h"
#include "CfsGUIDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CfsGUIDlg ダイアログ
CfsGUIDlg::CfsGUIDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CFSGUI_DIALOG, pParent)
	, m_hDll(nullptr)
	, m_pInitialize(nullptr), m_pFinalize(nullptr), m_pPortOpen(nullptr)
	, m_pPortClose(nullptr), m_pSetSerialMode(nullptr), m_pGetSerialData(nullptr)
	, m_pGetLatestData(nullptr), m_pGetSensorLimit(nullptr), m_pGetSensorInfo(nullptr)
	, m_portNo(5)
	, m_pThread(nullptr)
	, m_bIsThreadRunning(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	for (int i = 0; i < 6; ++i) m_Limit[i] = 0.0;
}

void CfsGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CfsGUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_START, &CfsGUIDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CfsGUIDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_PLOT, &CfsGUIDlg::OnBnClickedButtonPlot)
	ON_MESSAGE(WM_THREAD_FINISHED, &CfsGUIDlg::OnThreadFinished)
END_MESSAGE_MAP()


// CfsGUIDlg メッセージ ハンドラー

BOOL CfsGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_PLOT)->EnableWindow(FALSE);
	//AddLog(_T("アプリケーションを開始します。"));

	m_hDll = LoadLibrary(_T("CfsUsb.dll"));
	if (m_hDll == NULL)
	{
		AddLog(_T("エラー: CfsUsb.dll のロードに失敗しました。"));
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
		return TRUE;
	}
	//AddLog(_T("CfsUsb.dll をロードしました。"));

	m_pInitialize = (FUNC_Initialize)GetProcAddress(m_hDll, "Initialize");
	m_pFinalize = (FUNC_Finalize)GetProcAddress(m_hDll, "Finalize");
	m_pPortOpen = (FUNC_PortOpen)GetProcAddress(m_hDll, "PortOpen");
	m_pPortClose = (FUNC_PortClose)GetProcAddress(m_hDll, "PortClose");
	m_pSetSerialMode = (FUNC_SetSerialMode)GetProcAddress(m_hDll, "SetSerialMode");
	m_pGetSerialData = (FUNC_GetSerialData)GetProcAddress(m_hDll, "GetSerialData");
	m_pGetLatestData = (FUNC_GetLatestData)GetProcAddress(m_hDll, "GetLatestData");
	m_pGetSensorLimit = (FUNC_GetSensorLimit)GetProcAddress(m_hDll, "GetSensorLimit");
	m_pGetSensorInfo = (FUNC_GetSensorInfo)GetProcAddress(m_hDll, "GetSensorInfo");

	m_pInitialize();
	//AddLog(_T("DLLを初期化しました。"));

	if (m_pPortOpen(m_portNo) == true)
	{
		//AddLog(_T("ポートをオープンしました。"));
		AddLog(_T("センサーの準備が完了しました。計測を開始できます。"));
		char serialNo_char[9];
		if (m_pGetSensorInfo(m_portNo, serialNo_char) == true)
		{
			CString logStr = _T("シリアルNo: ") + CString(serialNo_char);
			AddLog(logStr);
		}
		if (m_pGetSensorLimit(m_portNo, m_Limit) == true)
		{
			AddLog(_T("センサ定格を取得しました。"));
		}
	}
	else
	{
		AddLog(_T("エラー: ポートのオープンに失敗しました。"));
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	}
	return TRUE;
}

void CfsGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CfsGUIDlg::OnPaint()
{
	if (IsIconic()) { /* ... */ }
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CfsGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CfsGUIDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	if (m_bIsThreadRunning) {
		m_bIsThreadRunning = false;
		if (m_pSetSerialMode != nullptr) m_pSetSerialMode(m_portNo, false);
		WaitForSingleObject(m_pThread->m_hThread, 1000);
	}
	if (m_pPortClose != nullptr) { m_pPortClose(m_portNo); }
	if (m_pFinalize != nullptr) { m_pFinalize(); }
	if (m_hDll != nullptr) { FreeLibrary(m_hDll); }
}

void CfsGUIDlg::AddLog(CString str)
{
	CTime time = CTime::GetCurrentTime();
	CString timeStr = time.Format(_T("%H:%M:%S - "));
	str += _T("\r\n");
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_LOG);
	if (pEdit && pEdit->GetSafeHwnd())
	{
		int nLength = pEdit->GetWindowTextLength();
		pEdit->SetSel(nLength, nLength);
		pEdit->ReplaceSel(timeStr + str);
	}
}

void CfsGUIDlg::OnBnClickedButtonStart()
{
	//AddLog(_T("計測開始ボタンが押されました。"));
	AddLog(_T("----------------"));
	AddLog(_T("計測を開始しました。"));
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_PLOT)->EnableWindow(FALSE);
	m_recordedData.clear();
	m_recordedData.reserve(80000);
	m_bIsThreadRunning = true;
	m_pThread = AfxBeginThread(RecordThreadProc, this);
}

void CfsGUIDlg::OnBnClickedButtonStop()
{
	AddLog(_T("計測終了を要求しました。"));
	if (m_bIsThreadRunning == false) return;
	m_bIsThreadRunning = false;
	if (m_pSetSerialMode != nullptr) {
		m_pSetSerialMode(m_portNo, false);
	}
}

afx_msg LRESULT CfsGUIDlg::OnThreadFinished(WPARAM wParam, LPARAM lParam)
{
	//AddLog(_T("スレッド終了通知を受信。データ処理を開始します..."));
	WaitForSingleObject(m_pThread->m_hThread, INFINITE);

	if (!m_recordedData.empty())
	{
		try
		{
			std::filesystem::path outputDir = _T(".\\output");
			if (!std::filesystem::exists(outputDir))
			{
				std::filesystem::create_directory(outputDir);
			}

			CTime time = CTime::GetCurrentTime();
			CString timestamp = time.Format(_T("%Y%m%d_%H%M%S"));
			CString timestampedFilename;
			timestampedFilename.Format(_T("sensor_data_%s.csv"), timestamp);

			std::filesystem::path timestampedPath = outputDir / (LPCTSTR)timestampedFilename;
			std::filesystem::path latestPath = outputDir / _T("sensor_data.csv");

			std::ofstream ofs(timestampedPath);
			if (ofs)
			{
				ofs << "Timestamp(ms),Fx,Fy,Fz,Mx,My,Mz" << std::endl;
				ofs << std::fixed << std::setprecision(4);
				for (const auto& rec : m_recordedData) {
					ofs << rec.timestamp_ms << "," << rec.Fx << "," << rec.Fy << "," << rec.Fz << "," << rec.Mx << "," << rec.My << "," << rec.Mz << std::endl;
				}
				ofs.close();
				//AddLog(timestampedFilename + _T(" への書き出しが完了しました。"));

				m_lastCsvPath = timestampedPath.c_str();
				GetDlgItem(IDC_BUTTON_PLOT)->EnableWindow(TRUE);

				std::filesystem::copy(timestampedPath, latestPath, std::filesystem::copy_options::overwrite_existing);
				CString finalMsg;
				finalMsg.Format(_T("計測データを %s に保存しました。"), timestampedFilename);
				AddLog(finalMsg);
				//AddLog(_T("sensor_data.csv を最新のデータに更新しました。"));
			}

			// 統計情報表示
			if (m_recordedData.size() > 1) {
				long totalSamples = m_recordedData.size();
				double totalDuration_ms = m_recordedData.back().timestamp_ms;
				double avgPeriod_ms = totalDuration_ms / (totalSamples - 1);
				double avgFrequency_hz = 1.0 / (avgPeriod_ms / 1000.0);

				CString logStr;
				logStr.Format(_T("--- 計測結果 ---")); AddLog(logStr);
				logStr.Format(_T("総サンプル数: %ld サンプル"), totalSamples); AddLog(logStr);
				logStr.Format(_T("総計測時間: %.4f ms"), totalDuration_ms); AddLog(logStr);
				logStr.Format(_T("平均取得周期: %.4f ms/サンプル"), avgPeriod_ms); AddLog(logStr);
				logStr.Format(_T("平均周波数: %.2f Hz"), avgFrequency_hz); AddLog(logStr);
				logStr.Format(_T("----------------")); AddLog(logStr);
			}
		}
		catch (const std::exception& e) { /* ... */ }
	}

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
	return 0;
}

void CfsGUIDlg::OnBnClickedButtonPlot()
{
	if (m_lastCsvPath.IsEmpty())
	{
		AddLog(_T("エラー: プロット対象のCSVファイルがありません。"));
		return;
	}
	try
	{
		CString plotFilePath = m_lastCsvPath;
		plotFilePath.Replace(_T('\\'), _T('/'));

		// 力(Force)のグラフを生成・表示
		{
			std::filesystem::path scriptPath = _T(".\\output\\plot_forces.plt");
			std::wofstream scriptFile(scriptPath);
			if (scriptFile)
			{
				scriptFile << L"set terminal wxt enhanced size 800, 600 title 'Forces (Fx, Fy, Fz)'\n";
				scriptFile << L"set datafile separator \",\"\n";
				scriptFile << L"set grid\n";
				scriptFile << L"set xlabel \"Time (ms)\"\n";
				scriptFile << L"set ylabel \"Force (N)\"\n";
				scriptFile << L"plot '" << (LPCTSTR)plotFilePath << L"' using 1:2 with lines title 'Fx', \\\n";
				scriptFile << L"     '' using 1:3 with lines title 'Fy', \\\n";
				scriptFile << L"     '' using 1:4 with lines title 'Fz'\n";

				CString arguments;
				arguments.Format(_T("--persist \"%s\""), scriptPath.c_str());
				ShellExecute(NULL, _T("open"), _T("wgnuplot.exe"), arguments, NULL, SW_SHOWNORMAL);
			}
		}

		// モーメント(Moment)のグラフを生成・表示
		{
			std::filesystem::path scriptPath = _T(".\\output\\plot_moments.plt");
			std::wofstream scriptFile(scriptPath);
			if (scriptFile)
			{
				scriptFile << L"set terminal wxt enhanced size 800, 600 title 'Moments (Mx, My, Mz)'\n";
				scriptFile << L"set datafile separator \",\"\n";
				scriptFile << L"set grid\n";
				scriptFile << L"set xlabel \"Time (ms)\"\n";
				scriptFile << L"set ylabel \"Moment (Nm)\"\n";
				scriptFile << L"plot '" << (LPCTSTR)plotFilePath << L"' using 1:5 with lines title 'Mx', \\\n";
				scriptFile << L"     '' using 1:6 with lines title 'My', \\\n";
				scriptFile << L"     '' using 1:7 with lines title 'Mz'\n";

				CString arguments;
				arguments.Format(_T("--persist \"%s\""), scriptPath.c_str());
				ShellExecute(NULL, _T("open"), _T("wgnuplot.exe"), arguments, NULL, SW_SHOWNORMAL);
			}
		}
		AddLog(_T("2つのグラフウィンドウを起動しました。"));
	}
	catch (const std::exception& e) { /* ... */ }
}

UINT CfsGUIDlg::RecordThreadProc(LPVOID pParam)
{
	CfsGUIDlg* pDlg = (CfsGUIDlg*)pParam;

	//pDlg->AddLog(_T("高速記録スレッドを開始しました。"));
	if (pDlg->m_pSetSerialMode(pDlg->m_portNo, true) == false) {
		pDlg->m_bIsThreadRunning = false;
	}
	auto startTime = std::chrono::high_resolution_clock::now();
	while (pDlg->m_bIsThreadRunning) {
		double Data[6];
		char Status;
		if (pDlg->m_pGetSerialData(pDlg->m_portNo, Data, &Status) == true) {
			SensorDataRecord record;
			record.timestamp_ms = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count() / 1000.0;
			record.Fx = pDlg->m_Limit[0] / 10000 * Data[0];
			record.Fy = pDlg->m_Limit[1] / 10000 * Data[1];
			record.Fz = pDlg->m_Limit[2] / 10000 * Data[2];
			record.Mx = pDlg->m_Limit[3] / 10000 * Data[3];
			record.My = pDlg->m_Limit[4] / 10000 * Data[4];
			record.Mz = pDlg->m_Limit[5] / 10000 * Data[5];
			pDlg->m_recordedData.push_back(record);
		}
	}
	if (pDlg->m_pSetSerialMode != nullptr) {
		pDlg->m_pSetSerialMode(pDlg->m_portNo, false);
	}
	//pDlg->AddLog(_T("高速記録スレッドが正常に終了しました。"));
	pDlg->PostMessage(WM_THREAD_FINISHED);
	return 0;
}