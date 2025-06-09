#include "pch.h"
#include "RealtimePlotWnd.h"

// �ÓI�����o�[������
const COLORREF CRealtimePlotWnd::s_channelColors[6] = {
    RGB(255, 0, 0),    // Fx: ��
    RGB(0, 150, 0),    // Fy: ��  
    RGB(0, 0, 255),    // Fz: ��
    RGB(255, 128, 0),  // Mx: �I�����W
    RGB(200, 0, 200),  // My: �}�[���^
    RGB(0, 200, 200)   // Mz: �V�A��
};

const TCHAR* CRealtimePlotWnd::s_channelLabels[6] = {
    _T("Fx"), _T("Fy"), _T("Fz"), _T("Mx"), _T("My"), _T("Mz")
};

BEGIN_MESSAGE_MAP(CRealtimePlotWnd, CWnd)
    ON_WM_PAINT()
    ON_WM_CLOSE()
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

CRealtimePlotWnd::CRealtimePlotWnd()
    : m_plotChannelMask(0x3F)  // �S�`�����l���\��
    , m_bAutoScale(true)
    , m_manualMinVal(-100.0)
    , m_manualMaxVal(100.0)
    , m_windowTitle(_T("���A���^�C���Z���T�f�[�^"))  // �ǉ�
    , m_yAxisLabel(_T("��/���[�����g"))              // �ǉ�
    , m_xAxisLabel(_T("���� (�b)"))                  // �ǉ�
    , m_windowOffset(0, 0)
{
}

CRealtimePlotWnd::~CRealtimePlotWnd()
{
}

void CRealtimePlotWnd::SetWindowTitle(const CString& title)
{
    m_windowTitle = title;
    if (IsWindow(GetSafeHwnd())) {
        SetWindowText(title);
    }
}

void CRealtimePlotWnd::SetAxisLabels(const CString& yLabel, const CString& xLabel)
{
    m_yAxisLabel = yLabel;
    m_xAxisLabel = xLabel;
    if (IsWindowVisible()) {
        Invalidate();
    }
}

void CRealtimePlotWnd::OffsetWindowPosition(int offsetX, int offsetY)
{
    m_windowOffset.x = offsetX;
    m_windowOffset.y = offsetY;
}

BOOL CRealtimePlotWnd::CreatePlotWindow(CWnd* pParent)
{
    CString className = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW,
        LoadCursor(NULL, IDC_ARROW),
        (HBRUSH)(COLOR_WINDOW + 1),
        LoadIcon(NULL, IDI_APPLICATION)
    );

    // �E�B���h�E�T�C�Y�ƈʒu�i�I�t�Z�b�g�Ή��j
    CRect rect(100 + m_windowOffset.x, 100 + m_windowOffset.y,
        900 + m_windowOffset.x, 700 + m_windowOffset.y);

    BOOL result = CreateEx(
        WS_EX_TOOLWINDOW,
        className,
        m_windowTitle,  // �ݒ肳�ꂽ�^�C�g�����g�p
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        rect,
        pParent,
        0
    );

    if (result) {
        m_plotData.clear();
        ShowWindow(SW_HIDE);
    }

    return result;
}

void CRealtimePlotWnd::ShowPlotWindow(BOOL bShow)
{
    if (bShow) {
        ShowWindow(SW_SHOW);
        BringWindowToTop();
        SetForegroundWindow();
    }
    else {
        ShowWindow(SW_HIDE);
    }
}

void CRealtimePlotWnd::UpdatePlotData(const std::deque<SensorDataRecord>& data)
{
    m_plotData = data;

    if (IsWindowVisible()) {
        Invalidate(FALSE);
    }
}

void CRealtimePlotWnd::OnPaint()
{
    CPaintDC dc(this);

    CRect clientRect;
    GetClientRect(&clientRect);

    // �_�u���o�b�t�@�����O
    CDC memDC;
    CBitmap memBitmap;
    memDC.CreateCompatibleDC(&dc);
    memBitmap.CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height());
    CBitmap* pOldBitmap = memDC.SelectObject(&memBitmap);

    // �w�i�N���A
    memDC.FillSolidRect(&clientRect, RGB(255, 255, 255));

    // �v���b�g�`��
    if (!m_plotData.empty()) {
        DrawPlot(&memDC, clientRect);
    }
    else {
        // �f�[�^�Ȃ��̕\��
        memDC.SetTextColor(RGB(128, 128, 128));
        memDC.SetBkMode(TRANSPARENT);
        CFont font;
        font.CreatePointFont(120, _T("Arial"));
        CFont* pOldFont = memDC.SelectObject(&font);

        memDC.DrawText(_T("�f�[�^�ҋ@��..."), &clientRect,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        memDC.SelectObject(pOldFont);
    }

    // ������DC�����ʂɓ]��
    dc.BitBlt(0, 0, clientRect.Width(), clientRect.Height(),
        &memDC, 0, 0, SRCCOPY);

    memDC.SelectObject(pOldBitmap);
}

void CRealtimePlotWnd::DrawPlot(CDC* pDC, CRect rect)
{
    if (m_plotData.size() < 2) return;

    // �}�[�W���ݒ�
    CRect plotRect = rect;
    plotRect.DeflateRect(60, 40, 80, 60);

    // �f�[�^�͈͌v�Z
    double minTime = m_plotData.front().timestamp_ms;
    double maxTime = m_plotData.back().timestamp_ms;
    double timeRange = max(maxTime - minTime, 1.0);

    double minVal = 1e10, maxVal = -1e10;

    if (m_bAutoScale) {
        // �����X�P�[�����O�i�\���Ώۃ`�����l���̂݁j
        for (const auto& record : m_plotData) {
            double values[6] = { record.Fx, record.Fy, record.Fz, record.Mx, record.My, record.Mz };
            for (int i = 0; i < 6; ++i) {
                if (m_plotChannelMask & (1 << i)) {
                    minVal = min(minVal, values[i]);
                    maxVal = max(maxVal, values[i]);
                }
            }
        }

        // �͈͂ɗ]�T����������
        if (minVal < 1e9) {  // �L���ȃf�[�^������ꍇ
            double margin = (maxVal - minVal) * 0.1;
            minVal -= margin;
            maxVal += margin;
        }
        else {
            minVal = -1.0;
            maxVal = 1.0;
        }
    }
    else {
        minVal = m_manualMinVal;
        maxVal = m_manualMaxVal;
    }

    double valueRange = max(maxVal - minVal, 1e-10);

    // �O���b�h�`��
    DrawGrid(pDC, plotRect, minVal, maxVal, minTime, maxTime);

    // ���`��
    DrawAxes(pDC, plotRect, minVal, maxVal, minTime, maxTime);

    // �f�[�^���`��i�\���Ώۃ`�����l���̂݁j
    for (int channel = 0; channel < 6; ++channel) {
        if (!(m_plotChannelMask & (1 << channel))) continue;

        CPen channelPen(PS_SOLID, 2, s_channelColors[channel]);
        CPen* pOldPen = pDC->SelectObject(&channelPen);

        std::vector<POINT> points;
        points.reserve(m_plotData.size());

        for (const auto& record : m_plotData) {
            double values[6] = { record.Fx, record.Fy, record.Fz, record.Mx, record.My, record.Mz };

            int x = plotRect.left + (int)((record.timestamp_ms - minTime) / timeRange * plotRect.Width());
            int y = plotRect.bottom - (int)((values[channel] - minVal) / valueRange * plotRect.Height());

            // �N���b�s���O
            x = max(plotRect.left, min(plotRect.right, x));
            y = max(plotRect.top, min(plotRect.bottom, y));

            points.push_back({ x, y });
        }

        if (points.size() > 1) {
            pDC->Polyline(points.data(), points.size());
        }

        pDC->SelectObject(pOldPen);
    }

    // �}��`��
    DrawLegend(pDC, rect);

    // �^�C�g���`��i�J�X�^�}�C�Y�Ή��j
    pDC->SetTextColor(RGB(0, 0, 0));
    pDC->SetBkMode(TRANSPARENT);
    CFont titleFont;
    titleFont.CreatePointFont(140, _T("Arial"));
    CFont* pOldFont = pDC->SelectObject(&titleFont);

    CRect titleRect = rect;
    titleRect.bottom = titleRect.top + 30;
    pDC->DrawText(m_windowTitle, &titleRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pDC->SelectObject(pOldFont);
}


void CRealtimePlotWnd::DrawGrid(CDC* pDC, CRect rect, double minVal, double maxVal, double minTime, double maxTime)
{
    CPen gridPen(PS_DOT, 1, RGB(200, 200, 200));
    CPen* pOldPen = pDC->SelectObject(&gridPen);

    // �����O���b�h��
    int numVerticalLines = 10;
    for (int i = 1; i < numVerticalLines; ++i) {
        int x = rect.left + (rect.Width() * i) / numVerticalLines;
        pDC->MoveTo(x, rect.top);
        pDC->LineTo(x, rect.bottom);
    }

    // �����O���b�h��
    int numHorizontalLines = 8;
    for (int i = 1; i < numHorizontalLines; ++i) {
        int y = rect.top + (rect.Height() * i) / numHorizontalLines;
        pDC->MoveTo(rect.left, y);
        pDC->LineTo(rect.right, y);
    }

    pDC->SelectObject(pOldPen);
}

void CRealtimePlotWnd::DrawAxes(CDC* pDC, CRect rect, double minVal, double maxVal, double minTime, double maxTime)
{
    CPen axisPen(PS_SOLID, 1, RGB(0, 0, 0));
    CPen* pOldPen = pDC->SelectObject(&axisPen);

    // �����`��
    pDC->MoveTo(rect.left, rect.top);
    pDC->LineTo(rect.left, rect.bottom);
    pDC->LineTo(rect.right, rect.bottom);

    pDC->SelectObject(pOldPen);

    // �����x��
    pDC->SetTextColor(RGB(0, 0, 0));
    pDC->SetBkMode(TRANSPARENT);
    CFont axisFont;
    axisFont.CreatePointFont(90, _T("Arial"));
    CFont* pOldFont = pDC->SelectObject(&axisFont);

    // Y�����x���i�l�j
    int numYLabels = 5;
    for (int i = 0; i <= numYLabels; ++i) {
        double value = minVal + (maxVal - minVal) * i / numYLabels;
        int y = rect.bottom - (rect.Height() * i) / numYLabels;

        CString label;
        label.Format(_T("%.2f"), value);

        CRect labelRect(rect.left - 55, y - 8, rect.left - 5, y + 8);
        pDC->DrawText(label, &labelRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    }

    // X�����x���i���ԁj
    int numXLabels = 6;
    for (int i = 0; i <= numXLabels; ++i) {
        double time = minTime + (maxTime - minTime) * i / numXLabels;
        int x = rect.left + (rect.Width() * i) / numXLabels;

        CString label;
        label.Format(_T("%.1f"), time / 1000.0);  // �b�P��

        CRect labelRect(x - 30, rect.bottom + 5, x + 30, rect.bottom + 25);
        pDC->DrawText(label, &labelRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // Y���^�C�g���i�J�X�^�����x���g�p�j
    CFont axisTitleFont;
    axisTitleFont.CreatePointFont(100, _T("Arial"));
    CFont* pOldTitleFont = pDC->SelectObject(&axisTitleFont);

    CRect yAxisTitle(rect.left - 45, rect.top, rect.left - 25, rect.bottom);
    pDC->DrawText(m_yAxisLabel, &yAxisTitle, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // X���^�C�g���i�J�X�^�����x���g�p�j
    CRect xAxisTitle(rect.left, rect.bottom + 30, rect.right, rect.bottom + 50);
    pDC->DrawText(m_xAxisLabel, &xAxisTitle, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    pDC->SelectObject(pOldTitleFont);
    pDC->SelectObject(pOldFont);
}

void CRealtimePlotWnd::DrawLegend(CDC* pDC, CRect rect)
{
    // �\������`�����l�������J�E���g
    int visibleChannels = 0;
    for (int i = 0; i < 6; ++i) {
        if (m_plotChannelMask & (1 << i)) {
            visibleChannels++;
        }
    }

    if (visibleChannels == 0) return;

    // �}��T�C�Y�𓮓I����
    CRect legendRect = rect;
    legendRect.left = legendRect.right - 80;
    legendRect.top += 50;
    legendRect.bottom = legendRect.top + (visibleChannels * 20) + 20;

    // �}��w�i
    CBrush legendBrush(RGB(245, 245, 245));
    CBrush* pOldBrush = pDC->SelectObject(&legendBrush);
    CPen legendPen(PS_SOLID, 1, RGB(128, 128, 128));
    CPen* pOldPen = pDC->SelectObject(&legendPen);

    pDC->Rectangle(&legendRect);

    // �}�ፀ��
    pDC->SetTextColor(RGB(0, 0, 0));
    pDC->SetBkMode(TRANSPARENT);
    CFont legendFont;
    legendFont.CreatePointFont(80, _T("Arial"));
    CFont* pOldFont = pDC->SelectObject(&legendFont);

    int yPos = legendRect.top + 10;
    for (int i = 0; i < 6; ++i) {
        if (m_plotChannelMask & (1 << i)) {
            // �F��
            CPen colorPen(PS_SOLID, 3, s_channelColors[i]);
            pDC->SelectObject(&colorPen);
            pDC->MoveTo(legendRect.left + 5, yPos + 6);
            pDC->LineTo(legendRect.left + 25, yPos + 6);

            // ���x��
            CRect textRect(legendRect.left + 30, yPos, legendRect.right - 5, yPos + 12);
            pDC->DrawText(s_channelLabels[i], &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            yPos += 20;
        }
    }

    pDC->SelectObject(pOldFont);
    pDC->SelectObject(pOldPen);
    pDC->SelectObject(pOldBrush);
}

void CRealtimePlotWnd::OnClose()
{
    ShowWindow(SW_HIDE);
    // CWnd::OnClose(); �͌Ă΂Ȃ�
}

void CRealtimePlotWnd::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    Invalidate();
}

BOOL CRealtimePlotWnd::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

void CRealtimePlotWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_bAutoScale = !m_bAutoScale;
    Invalidate();
    CWnd::OnLButtonDown(nFlags, point);
}

void CRealtimePlotWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
    CMenu menu;
    menu.CreatePopupMenu();

    for (int i = 0; i < 6; ++i) {
        UINT flags = MF_STRING;
        if (m_plotChannelMask & (1 << i)) {
            flags |= MF_CHECKED;
        }
        menu.AppendMenu(flags, 1000 + i, s_channelLabels[i]);
    }

    ClientToScreen(&point);
    UINT cmd = menu.TrackPopupMenu(TPM_RETURNCMD, point.x, point.y, this);

    if (cmd >= 1000 && cmd < 1006) {
        int channel = cmd - 1000;
        m_plotChannelMask ^= (1 << channel);
        Invalidate();
    }

    CWnd::OnRButtonDown(nFlags, point);
}
