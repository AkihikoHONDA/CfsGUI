#pragma once

#include "SensorDataTypes.h"
#include <deque>
#include <vector>

class CRealtimePlotWnd : public CWnd
{
public:
    CRealtimePlotWnd();
    virtual ~CRealtimePlotWnd();

    // �E�B���h�E�쐬
    BOOL CreatePlotWindow(CWnd* pParent);

    // �f�[�^�X�V
    void UpdatePlotData(const std::deque<SensorDataRecord>& data);
    void SetChannelMask(int mask) { m_plotChannelMask = mask; }

    // �E�B���h�E�\��/��\��
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

    // �`��֘A
    void DrawPlot(CDC* pDC, CRect rect);
    void DrawGrid(CDC* pDC, CRect rect, double minVal, double maxVal, double minTime, double maxTime);
    void DrawAxes(CDC* pDC, CRect rect, double minVal, double maxVal, double minTime, double maxTime);
    void DrawLegend(CDC* pDC, CRect rect);

    // �F�ƃ��x����`
    static const COLORREF s_channelColors[6];
    static const TCHAR* s_channelLabels[6];
};
