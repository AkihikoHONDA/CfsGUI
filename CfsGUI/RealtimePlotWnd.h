#pragma once

#include "SensorDataTypes.h"
#include <deque>
#include <vector>

class CRealtimePlotWnd : public CWnd
{
public:
    CRealtimePlotWnd();
    virtual ~CRealtimePlotWnd();

    // ウィンドウ作成
    BOOL CreatePlotWindow(CWnd* pParent);

    // データ更新
    void UpdatePlotData(const std::deque<SensorDataRecord>& data);
    void SetChannelMask(int mask) { m_plotChannelMask = mask; }

    // ウィンドウ表示/非表示
    void ShowPlotWindow(BOOL bShow);

    void SetWindowTitle(const CString& title);
    void SetAxisLabels(const CString& yLabel, const CString& xLabel);
    void OffsetWindowPosition(int offsetX, int offsetY);

protected:
    afx_msg void OnPaint();
    afx_msg void OnClose();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

    DECLARE_MESSAGE_MAP()

private:
    std::deque<SensorDataRecord> m_plotData;
    int m_plotChannelMask;
    bool m_bAutoScale;
    double m_manualMinVal, m_manualMaxVal;

    CString m_windowTitle;
    CString m_yAxisLabel;
    CString m_xAxisLabel;
    CPoint m_windowOffset;

    // 描画関連
    void DrawPlot(CDC* pDC, CRect rect);
    void DrawGrid(CDC* pDC, CRect rect, double minVal, double maxVal, double minTime, double maxTime);
    void DrawAxes(CDC* pDC, CRect rect, double minVal, double maxVal, double minTime, double maxTime);
    void DrawLegend(CDC* pDC, CRect rect);

    // 色とラベル定義
    static const COLORREF s_channelColors[6];
    static const TCHAR* s_channelLabels[6];
};
