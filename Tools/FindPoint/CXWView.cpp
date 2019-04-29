
// CXWView.cpp : CCXWView ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "CXW.h"
#endif

#include "CXWDoc.h"
#include "CXWView.h"
#include "BasicExcel.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include   <afxpriv.h>

#ifdef PRIVATE_STORESCP_DECLARATIONS
PRIVATE_STORESCP_DECLARATIONS
#else
#define BITTER_APPLICATION "findePoint"
#endif
// CCXWView
//-------------------------------------------------------------------------------
static OFLogger findPointLogger = OFLog::getLogger("bittersweet.findePoint" BITTER_APPLICATION);

IMPLEMENT_DYNCREATE(CCXWView, CView)

BEGIN_MESSAGE_MAP(CCXWView, CView)
    // ��׼��ӡ����
    ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CCXWView::OnFilePrintPreview)
    ON_WM_CONTEXTMENU()
    ON_WM_RBUTTONUP()
    ON_COMMAND(ID_FILE_OPEN, &CCXWView::OnFileOpen)
END_MESSAGE_MAP()

// CCXWView ����/����
void LogMessage(string message)
{
#ifdef _DEBUG
    afxDump << message.c_str() << "\n";
#endif
    OFLOG_INFO(findPointLogger, message.c_str());
}
void LogError(string message)
{
#ifdef _DEBUG
    afxDump << "ERROR:" << message.c_str() << "\n";
#endif
    OFLOG_ERROR(findPointLogger, message.c_str());
}
void LogWarning(string message)
{
#ifdef _DEBUG
    afxDump << "Warning:" << message.c_str() << "\n";
#endif
    OFLOG_WARN(findPointLogger, message.c_str());
}
bool IsExitPoint(double xo, double yo, double x1, double y1, double dis)
{
    double delt = sqrt(pow(xo - x1, 2) + pow(yo - y1, 2));
    if (delt > dis)
    {
        return true;
    }
    else if (delt < dis)
    {
        return false;
    }
    //��ʱ�����Ǹպ��յ�������Ҫ��ľ���
}
//// �õ���ǰִ�е�exe��ȫ·���ļ��������ʧ�ܷ���""
//CString GetApplicationFullName()
//{
//    //HMODULE module = GetModuleHandle(0);
//    //char pFileName[MAX_PATH];
//    //GetModuleFileName(module, pFileName, MAX_PATH);
//
//    //CString csFullPath(pFileName);
//    //int nPos = csFullPath.ReverseFind(_T('\\'));
//    //if (nPos < 0)
//    //    return CString("");
//    //else
//    //{
//    //    CString temp = csFullPath.Left(nPos);
//    //    return temp;
//    //}
//}
//ת��ʵ����������㵽��Ļ�����
void CCXWView::ConvertedPoint()
{
    CRect rc;
    GetClientRect(rc);
    int max_x, max_y;
    max_x = rc.right - 10;
    max_y = rc.bottom - 10;
    double d_max_x = 0, d_min_x = g_Xvalue[0], d_max_y = 0, d_min_y = g_Yvalue[0];
    int point_numbe = g_Xvalue.size();
    g_display_Xvalue.clear();
    g_display_Yvalue.clear();
    g_display_Y_get.clear();
    g_display_X_get.clear();
    for (int i = 0; i < point_numbe; i++)
    {
        if (g_Xvalue[i]>d_max_x)
        {
            d_max_x = g_Xvalue[i];
        }
        if (g_Xvalue[i] < d_min_x)
        {
            d_min_x = g_Xvalue[i];
        }

        if (g_Yvalue[i] > d_max_y)
        {
            d_max_y = g_Yvalue[i];
        }
        if (g_Yvalue[i] < d_min_y)
        {
            d_min_y = g_Yvalue[i];
        }
    }
    int  delt_point_dis = 15;
    double k_x = (d_max_x - d_min_x) / (max_x - 2 * delt_point_dis);
    double k_y = (d_max_y - d_min_y) / (max_y - 2 * delt_point_dis);
    double delt_x = 30 * k_x + d_min_x;
    double delt_y = 20 * k_y + d_min_y;
    double delt_k_x = d_min_x / k_x;
    double delt_k_y = d_min_y / k_y;

    for (int i = 0; i < point_numbe; i++)
    {
        double temp = g_Xvalue[i];
        if (k_x <= 1)
        {
            double x = (temp - d_min_x + delt_point_dis);
            g_display_Xvalue.push_back(x);
        }
        else
        {
            double x = temp / k_x - delt_k_x + delt_point_dis;
            g_display_Xvalue.push_back(x);
        }

        temp = g_Yvalue[i];
        if (k_y <= 1)
        {
            double y = (temp - d_min_y);
            g_display_Yvalue.push_back(max_y - y - delt_point_dis);
        }
        else
        {
            double y = temp / k_y - delt_k_y;
            g_display_Yvalue.push_back(max_y - y - delt_point_dis);
        }
    }
    int get_point_number = g_X_get.size();
    for (int i = 0; i < get_point_number; i++)
    {
        double temp = g_X_get[i];
        if (k_x <= 1)
        {
            double x = (temp - d_min_x + delt_point_dis);
            g_display_X_get.push_back(x);
        }
        else
        {
            double x = temp / k_x - delt_k_x + delt_point_dis;
            g_display_X_get.push_back(x);
        }
        temp = g_Y_get[i];
        if (k_y <= 1)
        {
            double y = (temp - d_min_y);
            g_display_Y_get.push_back(max_y - y - delt_point_dis);
        }
        else
        {
            double y = temp / k_y - delt_k_y;
            g_display_Y_get.push_back(max_y - y - delt_point_dis);
        }
    }
    Invalidate();
}
//�����߶�����Բ��ָ������ĵ�
bool CCXWView::GetIntersection(double xo, double yo, int index_point, double &x_get, double &y_get, bool thesameline)
{
    //��ʼ���㽻��
    //�߶�n��Kn������Բ�����꣨xo,yo��,����g_distance���������߶� 2���˵����꣬���ؽ������ꡣ
    int r_n = index_point;
    int r_n_1 = r_n - 1;
    //double xo = g_Xvalue[r_n_1];
    //double yo = g_Yvalue[r_n_1];
    double K_n = (g_Yvalue[r_n] - g_Yvalue[r_n_1]) / (g_Xvalue[r_n] - g_Xvalue[r_n_1]);
    // 
    double B_n = (g_Xvalue[r_n] * g_Yvalue[r_n_1] - g_Xvalue[r_n_1] * g_Yvalue[r_n]) / (g_Xvalue[r_n] - g_Xvalue[r_n_1]);
    double A_n = 1 + sqrt(K_n);
    double a_n, b_n, c_n;
    //a = 1+k2 , b =2k(B-yo)-2xo , c=xo2+(B-yo)2-R2
    a_n = 1 + pow(K_n, 2);
    //b_n = 2 * K_n*(B_n - yo) - 2 * xo; //�޸�����
    b_n = (2 * (g_Yvalue[r_n] - g_Yvalue[r_n_1])*(B_n - yo)) / (g_Xvalue[r_n] - g_Xvalue[r_n_1]) - 2 * xo;
    c_n = pow(xo, 2) + pow(B_n - yo, 2) - pow(g_distance, 2);
    //X=[-b+sqrt(b2-4*a*c)]/(2*a) X=[-b-sqrt(b2-4*a*c)]/(2*a)
    //��ʼ�⽻������
    double delt_n = pow(b_n, 2) - 4 * a_n*c_n;
    double sqrt_delt_n = sqrt(delt_n);
    double x1 = (sqrt_delt_n - b_n) / (2 * a_n);
    double x2 = (-sqrt_delt_n - b_n) / (2 * a_n);
    //double x_get, y_get;
    //�����ж��߶εķ�����Ϊ��㵽�յ�Ϊ�߶η���
    if (g_Xvalue[r_n_1] < g_Xvalue[r_n])
    {
        double start_x = g_Xvalue[r_n_1];
        double end_x = g_Xvalue[r_n];
        if (thesameline)//���Բ�����߶��ϻ�
        {
            start_x = xo;
        }
        //���ж�x2 �Ƿ����Ҫ���߶� �����յ�����Ϊ���򻰣�x����ֵСЩ������ѡ��
        if (x2 >= start_x && x2 <= end_x)
        {
            x_get = x2;
        }
        else if (x1 >= start_x && x1 <= end_x)
        {
            x_get = x1;
        }
        else
        {
            return false;
        }
    }
    else //����߶�����Ϊ����
    {
        double start_x = g_Xvalue[r_n];
        double end_x = g_Xvalue[r_n_1];
        if (thesameline)
        {
            end_x = xo;
        }
        //���ж�x1 �Ƿ����Ҫ���߶� �����յ�����Ϊ���򻰣�x����ֵ��Щ������ѡ��
        if (x1 >= start_x && x1 <= end_x)
        {
            x_get = x1;
        }
        else if (x2 >= start_x && x2 <= end_x)
        {
            x_get = x2;
        }
        else
        {
            return false;
        }
    }
    y_get = K_n*x_get + B_n;
//#ifdef _DEBUG
//    double dis_test = sqrt(pow(x_get - xo, 2) + pow(y_get - yo, 2));
//    LogMessage("ok!");
//#endif
    return true;
}
//��֤�ļ��Ƿ���ڡ�
bool FileExists(const OFString FileName)
{
    if (FileName == "")
        return false;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    hFind = FindFirstFile(FileName.c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return false;
    FindClose(hFind);
    return true;
}
//��֤Ŀ¼�Ƿ���ڡ�
bool DirectoryExists(const OFString Dir)
{
    if (Dir == "")
        return false;
    int Code = GetFileAttributes(Dir.c_str());
    return (Code != -1) && ((FILE_ATTRIBUTE_DIRECTORY & Code) != 0);
}
//��ȫ·���ļ�������ȡ�ļ�·����
OFString ExtractFileDir(const OFString FileName)
{
    OFString tempstr;
    return OFStandard::getDirNameFromPath(tempstr, FileName);
}
//����һ����Ŀ¼
bool ForceDirectories(const OFString Dir)
{
    OFString path = ExtractFileDir(Dir);
    if (!DirectoryExists(path))
    {
        if (path[path.size() - 1] != ':')
            ForceDirectories(path);
    }
    if (DirectoryExists(Dir) != true)
    {
        if (::CreateDirectory(Dir.c_str(), NULL) == 0)
        {
            OFString msg;
            msg = "CreateDirectory() failed with error %s,(%s)" + GetLastError() + Dir;
            LOG_WARN(findPointLogger, msg);
            return false;
        }
    }
    return true;
}
char * GetAppPath()
{
    char *path = ::GetCommandLine();
    int len = strlen(path);
    if (len > 2)
    {
        if (path[0] == '"')
        {
            path++;
        }
        int len = strlen(path);
        if (len > 2)
        {
            char c = path[len - 2];
            if (path[len - 2] == '"')
            {
                path[len - 2] = '\0';
            }
        }
    }
    return path;
}
CCXWView::CCXWView()
{
    char *path = GetAppPath();
    const char *pattern = "%D{%Y-%m-%d %H:%M:%S.%q} %T %5p: %M %m%n";//https://support.dcmtk.org/docs/classdcmtk_1_1log4cplus_1_1PatternLayout.html
    OFString tempstr;
    OFString log_dir = OFStandard::getDirNameFromPath(tempstr, path) + "\\log";
    tempstr = "Ĭ��������Ϣ";
    ForceDirectories(log_dir);
    OFString logfilename = log_dir + "\\FindPoint.log";// "D:\\code\\C++\\HealthApp\\bin\\win32\\log\\FindPoint.log";//"/home/zyq/code/C++/DicomScuApp/DicomSCU/bin/Debug/dcmtk_storescu";
    OFunique_ptr<dcmtk::log4cplus::Layout> layout(new dcmtk::log4cplus::PatternLayout(pattern));
    dcmtk::log4cplus::SharedAppenderPtr logfile(new dcmtk::log4cplus::FileAppender(logfilename, STD_NAMESPACE ios::app));
    dcmtk::log4cplus::Logger log = dcmtk::log4cplus::Logger::getRoot();
    logfile->setLayout(OFmove(layout));
    log.removeAllAppenders();
    log.addAppender(logfile);

    //LogMessage("FIndPoint start!");
    LOG_INFO(findPointLogger, "orilog info :FIndPoint start!");

}

CCXWView::~CCXWView()
{
    LOG_INFO(findPointLogger, "FIndPoints Exit!");
}

BOOL CCXWView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO:  �ڴ˴�ͨ���޸�
    //  CREATESTRUCT cs ���޸Ĵ��������ʽ

    return CView::PreCreateWindow(cs);
}

// CCXWView ����

void CCXWView::OnDraw(CDC* /*pDC*/)
{
    CCXWDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc)
        return;
    CDC *dc = GetDC();

    int point_number = g_display_Xvalue.size();
    if (point_number > 1)
    {
        for (int i = 0; i < point_number; i++)
        {
            dc->SetPixel(g_display_Xvalue[i] - 1, g_display_Yvalue[i], RGB(255, 0, 0));
            dc->SetPixel(g_display_Xvalue[i], g_display_Yvalue[i] + 1, RGB(255, 0, 0));
            dc->SetPixel(g_display_Xvalue[i] + 1, g_display_Yvalue[i], RGB(255, 0, 0));
            dc->SetPixel(g_display_Xvalue[i], g_display_Yvalue[i] - 1, RGB(255, 0, 0));
            dc->SetPixel(g_display_Xvalue[i], g_display_Yvalue[i], RGB(255, 0, 0));

            dc->SetPixel(g_display_Xvalue[i] - 1, g_display_Yvalue[i] - 1, RGB(255, 0, 0));
            dc->SetPixel(g_display_Xvalue[i] + 1, g_display_Yvalue[i] + 1, RGB(255, 0, 0));
            dc->SetPixel(g_display_Xvalue[i] + 1, g_display_Yvalue[i] - 1, RGB(255, 0, 0));
            dc->SetPixel(g_display_Xvalue[i] - 1, g_display_Yvalue[i] + 1, RGB(255, 0, 0));

            dc->SetPixel(g_display_Xvalue[i] - 2, g_display_Yvalue[i], RGB(255, 0, 0));
            dc->SetPixel(g_display_Xvalue[i], g_display_Yvalue[i] + 2, RGB(255, 0, 0));
            dc->SetPixel(g_display_Xvalue[i] + 2, g_display_Yvalue[i], RGB(255, 0, 0));
            dc->SetPixel(g_display_Xvalue[i], g_display_Yvalue[i] - 2, RGB(255, 0, 0));
        }
        for (int i = 0; i < point_number - 1; i++)
        {
            dc->MoveTo(g_display_Xvalue[i], g_display_Yvalue[i]);
            dc->LineTo(g_display_Xvalue[i + 1], g_display_Yvalue[i + 1]);
        }
        point_number = g_display_X_get.size();
        for (int i = 0; i < point_number; i++)
        {
            dc->SetPixel(g_display_X_get[i] - 1, g_display_Y_get[i], RGB(0, 0, 255));
            dc->SetPixel(g_display_X_get[i], g_display_Y_get[i] + 1, RGB(0, 0, 255));
            dc->SetPixel(g_display_X_get[i] + 1, g_display_Y_get[i], RGB(0, 0, 255));
            dc->SetPixel(g_display_X_get[i], g_display_Y_get[i] - 1, RGB(0, 0, 255));
            dc->SetPixel(g_display_X_get[i], g_display_Y_get[i], RGB(0, 255, 0));

            dc->SetPixel(g_display_X_get[i] - 1, g_display_Y_get[i] - 1, RGB(0, 0, 255));
            dc->SetPixel(g_display_X_get[i] + 1, g_display_Y_get[i] + 1, RGB(0, 0, 255));
            dc->SetPixel(g_display_X_get[i] - 1, g_display_Y_get[i] + 1, RGB(0, 0, 255));
            dc->SetPixel(g_display_X_get[i] + 1, g_display_Y_get[i] - 1, RGB(0, 0, 255));

            dc->SetPixel(g_display_X_get[i] - 2, g_display_Y_get[i], RGB(0, 0, 255));
            dc->SetPixel(g_display_X_get[i], g_display_Y_get[i] + 2, RGB(0, 0, 255));
            dc->SetPixel(g_display_X_get[i] + 2, g_display_Y_get[i], RGB(0, 0, 255));
            dc->SetPixel(g_display_X_get[i], g_display_Y_get[i] - 2, RGB(0, 0, 255));
        }
    }
    else
    {
        CString tip = _T("ʹ��˵�����˵��� '�ļ�(F)'��ѡ�� '��'��ѡ�����ȡ��EXCEL�����ļ���ѡ��󣬺�̨���̿�ʼ���㣬��ɺ�Ի�����ʾ!");
        dc->TextOut(1, 10, tip);
        tip = _T("��ע��EXCEL�ļ����ݸ�ʽ.��һ����񣬵�һ�У���һ���зֱ�ΪX��Y���ڶ��е�����ΪԲ�ĵ�������룬�ڶ��п�ʼʱ��Ӧ������,�������ݼ���ʧ��!");
        dc->TextOut(1, 30, tip);
    }
    //int point_number = g_display_Xvalue.size();
    // TODO:  �ڴ˴�Ϊ����������ӻ��ƴ���
}


// CCXWView ��ӡ


void CCXWView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
    AFXPrintPreview(this);
#endif
}

BOOL CCXWView::OnPreparePrinting(CPrintInfo* pInfo)
{
    // Ĭ��׼��
    return DoPreparePrinting(pInfo);
}

void CCXWView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO:  ��Ӷ���Ĵ�ӡǰ���еĳ�ʼ������
}

void CCXWView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO:  ��Ӵ�ӡ����е��������
}

void CCXWView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
    ClientToScreen(&point);
    OnContextMenu(this, point);
}

void CCXWView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
    theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

void CCXWView::OnFileOpen()
{
    CFileDialog filedlg(TRUE, //TRUEΪOPEN�Ի���FALSEΪSAVE AS�Ի���
        NULL,
        NULL,
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        (LPCTSTR)_TEXT("Excel Files (*.xls)|*.xls|All Files (*.*)|*.*||"), NULL);
    if (IDCANCEL == filedlg.DoModal())
        return;
    auto szFilePath = filedlg.GetPathName();
    auto szFileName = filedlg.GetFileName();
    auto p = GetDocument();
    p->OnOpenDocument(szFilePath);
    p->SetTitle(szFileName);
    ReadExcelFie(szFilePath, szFileName);
}

void CCXWView::ReadExcelFie(CString pathfilename, CString filename)
{
    //��һ������ȡ������ļ���Ϣ����ȡ�������������
    string filePathname = ("D:\\CXW\\points.xls");
    if (g_Xvalue.size() > 0)
    {
        g_Xvalue.clear();
        g_Yvalue.clear();
        g_X_get.clear();
        g_Y_get.clear();
    }

    using namespace YExcel;
    BasicExcel e;
    if (pathfilename.GetLength() < 1)
    {
        e.Load(filePathname.c_str(), 1);
    }
    else
    {
#ifdef _UNICODE
        USES_CONVERSION;
        e.Load(T2A(pathfilename), 1);
#else
        e.Load(pathfilename);
#endif
    }

    size_t size = e.GetTotalWorkSheets();
    size = 0;
    BasicExcelWorksheet* sheet1 = e.GetWorksheet("input points");
    if (!sheet1)
    {
        sheet1 = e.GetWorksheet(size);
    }
#ifdef _DEBUG
    afxDump << "---------���Ե�����Ϣ����-----------" << "\n";
    afxDump << "-��ȡ�������ļ���------" << filePathname.c_str() << "\n";
#endif
    if (sheet1)
    {
        size_t maxRows = sheet1->GetTotalRows();
        size_t maxCols = sheet1->GetTotalCols();
        if (maxRows < 3)
        {
            OFString message = "û���㹻���ݣ�����3�У�2�������";
            LogError(message.c_str());
            MessageBox(message.c_str()/*_T("û���㹻���ݣ�����3�У�2�������")*/);
            return;
        }
        if (maxCols < 2)
        {
            OFString message = "û���㹻���ݣ��������㣬������� ��X��Y";
            LogError(message.c_str());
            MessageBox(message.c_str()/*_T("û���㹻���ݣ��������㣬������� ��X��Y")*/);
            return;
        }
        if (maxCols > 2)
        {
            maxCols = 2;
        }
        g_row = maxRows - 1;
        int x, y;
        BasicExcelCell* cell_d = sheet1->Cell(1, 2);
        if (cell_d->Type() == BasicExcelCell::DOUBLE || cell_d->Type() == BasicExcelCell::INT)
        {
            g_distance = cell_d->GetDouble();
        }
        else
        {
            MessageBox(_T("���ݱ�ڶ��е�����û�о������ݣ�������ȷ��Ȼ���ٶ�������!"));
            OFLOG_ERROR(findPointLogger, "��EXCEL����ļ��У��ڶ��е�����û�ж���������ݻ��߾������ݵ�λ��ʽ������");
            return;
        }
#ifdef _DEBUG
        BasicExcelCell* cell_temp = sheet1->Cell(2, 2);
        if (cell_temp->Type() == BasicExcelCell::DOUBLE || cell_temp->Type() == BasicExcelCell::INT)
        {
            double temp = cell_temp->GetDouble();
        }
#endif

        for (size_t r = 1, x = 0, y = 0; r < maxRows; ++r)
        {
            for (size_t c = 0; c < maxCols; ++c)
            {
                BasicExcelCell* cell = sheet1->Cell(r, c);
                switch (cell->Type())
                {
                case BasicExcelCell::UNDEFINED:
                    printf("          ");
                    break;
                case BasicExcelCell::INT:
                {
                    double a = cell->GetDouble();
                    if (c > 0)
                    {
                        g_Yvalue.push_back(a);
                    }
                    else
                    {
                        g_Xvalue.push_back(a);
                    }
                    printf("%10d", cell->GetInteger());
                    break;
                }
                case BasicExcelCell::DOUBLE:
                {
                    double a = cell->GetDouble();
                    if (c > 0)
                    {
                        g_Yvalue.push_back(a);
                    }
                    else
                    {
                        g_Xvalue.push_back(a);
                    }
                    //printf("%10.6lf", cell->GetDouble());
                }
                break;
                case BasicExcelCell::STRING:
                    printf("%10s", cell->GetString());
                    break;
                case BasicExcelCell::WSTRING:
                    wprintf(L"%10s", cell->GetWString());
                    break;
                }
            }
        }
        double x_get(-1), y_get(-1);
        int index_lines = 3;//����0��ʼ����һ���߶������0��1�ĵ���߶�
        double xo = g_Xvalue[index_lines - 1];
        double yo = g_Yvalue[index_lines - 1];
        //�ӵ�һ���߶ο�ʼ�ж�
        bool bstart = true;
        index_lines = 1;//��¼��Ҫ�жϵ��߶ε����
        static bool sameline = true;
        LONG64 times = 0;
        g_X_get.push_back(g_Xvalue[0]);
        g_Y_get.push_back(g_Yvalue[0]);
        while (true)
        {
            if (bstart)
            {
                bstart = false;
                xo = g_Xvalue[index_lines - 1];
                yo = g_Yvalue[index_lines - 1];
                //�ж�Բ�ĺ͵�һ���߶ε��յ�����Ƿ�С�ں͵���distance
                if (IsExitPoint(xo, yo, g_Xvalue[index_lines], g_Yvalue[index_lines], g_distance))
                {
                    bool bexit = GetIntersection(xo, yo, index_lines, x_get, y_get, sameline);
                    if (bexit)
                    {
                        xo = x_get;
                        yo = y_get;
                        g_X_get.push_back(x_get);
                        g_Y_get.push_back(y_get);
                    }
                    if (!IsExitPoint(xo, yo, g_Xvalue[index_lines], g_Yvalue[index_lines], g_distance))
                    {
                        index_lines++;
                        sameline = false;
                    }
                    else
                    {
                        sameline = true;
                    }
                    int stop = 0;
                }
                else
                {
                    index_lines++;
                }
            }
            else
            {
                //�ж�Բ�ĺ͵�һ���߶ε��յ�����Ƿ�С�ں͵���distance
                if (IsExitPoint(xo, yo, g_Xvalue[index_lines], g_Yvalue[index_lines], g_distance))
                {
                    if (index_lines == g_row - 2)
                    {
                        int a = 0;
                    }
                    bool bexit = GetIntersection(xo, yo, index_lines, x_get, y_get, sameline);
                    if (bexit)
                    {
                        xo = x_get;
                        yo = y_get;
                        g_X_get.push_back(x_get);
                        g_Y_get.push_back(y_get);
                    }
                    if (!IsExitPoint(xo, yo, g_Xvalue[index_lines], g_Yvalue[index_lines], g_distance))
                    {
                        index_lines++;
                        sameline = false;
                    }
                    else
                    {
                        sameline = true;
                    }
                    int stop = 0;
                }
                else
                {
                    index_lines++;
                }
            }
            if (index_lines > g_row - 1)
            {
                //�߶��Ѿ�������
                break;
            }
            else
            {
                times++;
                if (times > 10000)
                {
                    OFString message = "������ܳ���ǿ����ֹ����Ŀǰ�����������10000��";
                    OFLOG_ERROR(findPointLogger, message.c_str());
                    MessageBox(message.c_str());
                    break;
                }
            }
        }
        //�Լ���Ľ������
        using namespace YExcel;
        BasicExcel saveExcel;
        saveExcel.New();
        saveExcel.RenameWorksheet("Sheet1", "Result");
        saveExcel.RenameWorksheet("Sheet2", "input points");
        BasicExcelWorksheet* result_sheet = saveExcel.GetWorksheet("Result");
        BasicExcelWorksheet* input_sheet1 = saveExcel.GetWorksheet("input points");
        BasicExcelCell* cell;
        if (result_sheet)
        {
            std::vector<double> dis_list;
            //#ifdef _DEBUG ����ÿһ����������
            LONG64 lsize = g_X_get.size();
            for (LONG64 i = 0; i < lsize - 1; i++)
            {
                double dis = sqrt(pow(g_X_get[i] - g_X_get[i + 1], 2) + pow(g_Y_get[i] - g_Y_get[i + 1], 2));
                dis_list.push_back(dis);
            }
            //#endif
            cell = result_sheet->Cell(0, 0);
            cell->SetString("X'");
            cell = result_sheet->Cell(0, 1);
            cell->SetString("Y'");
            cell = result_sheet->Cell(0, 2);
            cell->SetString("points number");
            cell = result_sheet->Cell(0, 3);
            cell->SetString("distance = point<->point");//
            cell = result_sheet->Cell(0, 4);
            cell->SetString("between two points:X");//
            cell = result_sheet->Cell(0, 5);
            cell->SetString("between two points:Y");//

            LONG64 row = g_X_get.size();
            cell = result_sheet->Cell(1, 2);
            cell->SetInteger(row);
            LONG64 dis_size = dis_list.size();
            for (LONG64 index_dis = 0, r = 1; r < row + 1; r++)
            {
                for (LONG64 c = 0; c < 2; c++)
                {
                    cell = result_sheet->Cell(r, c);
                    if (c > 0)
                    {
                        cell->SetDouble(g_Y_get[r - 1]);
                    }
                    else
                    {
                        cell->SetDouble(g_X_get[r - 1]);
                    }
                }
                if (r > 1 && index_dis < dis_size)
                {
                    cell = result_sheet->Cell(r, 3);
                    cell->SetDouble(dis_list[index_dis++]);
                    cell = result_sheet->Cell(r, 4);
                    cell->SetDouble((g_X_get[r - 2] + g_X_get[r - 1]) / 2);
                    cell = result_sheet->Cell(r, 5);
                    cell->SetDouble((g_Y_get[r - 2] + g_Y_get[r - 1]) / 2);
                }
            }
        }
        if (input_sheet1)
        {
            cell = input_sheet1->Cell(0, 0);
            cell->SetString("input x");
            cell = input_sheet1->Cell(0, 1);
            cell->SetString("input y");
            cell = input_sheet1->Cell(0, 2);
            cell->SetString("distance");
            cell = input_sheet1->Cell(1, 2);
            cell->SetDouble(g_distance);
            cell = input_sheet1->Cell(0, 3);
            cell->SetString("points number");

            LONG64 row = g_Xvalue.size();
            cell = input_sheet1->Cell(1, 3);
            cell->SetInteger(row);
            for (LONG64 r = 1; r < row + 1; r++)
            {
                for (LONG64 c = 0; c < 2; c++)
                {
                    cell = input_sheet1->Cell(r, c);
                    if (c > 0)
                    {
                        cell->SetDouble(g_Yvalue[r - 1]);
                    }
                    else
                    {
                        cell->SetDouble(g_Xvalue[r - 1]);
                    }
                }
            }
        }
        CString newfilename = pathfilename.Left(pathfilename.GetLength() - 4) + CString("_result.xls");
        if (newfilename.GetLength() > 1)
        {
#ifdef _UNICODE
            USES_CONVERSION;
            saveExcel.SaveAs(T2A(newfilename));
#else
            saveExcel.SaveAs(newfilename);
#endif
        }
        CString message = _T("�����Ѿ���ɣ���������") + newfilename;
        //        std::vector<double> dis_list;
        //#ifdef _DEBUG
        //        LONG64 lsize = g_X_get.size();
        //        for (LONG64 i = 0; i < lsize-1; i++)
        //        {
        //            double dis = sqrt(pow(g_X_get[i] - g_X_get[i + 1], 2) + pow(g_Y_get[i] - g_Y_get[i + 1], 2));
        //            dis_list.push_back(dis);
        //        }
        //#endif
        ConvertedPoint();
        MessageBox(message);
        OFLOG_INFO(findPointLogger, message.GetBuffer());
    }
}

// CCXWView ���

#ifdef _DEBUG
void CCXWView::AssertValid() const
{
    CView::AssertValid();
}

void CCXWView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CCXWDoc* CCXWView::GetDocument() const // �ǵ��԰汾��������
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCXWDoc)));
    return (CCXWDoc*)m_pDocument;
}
#endif //_DEBUG


// CCXWView ��Ϣ�������

/*
////��ʼ���㽻��
////�߶�n��Kn������Բ�����꣨xo,yo��,����g_distance���������߶� 2���˵����꣬���ؽ������ꡣ
//int r_n = 3;
//int r_n_1 = r_n - 1;
//double xo = g_Xvalue[r_n_1];
//double yo = g_Yvalue[r_n_1];
//double K_n = (g_Yvalue[r_n] - g_Yvalue[r_n_1]) / (g_Xvalue[r_n] - g_Xvalue[r_n_1]);
////
//double B_n = (g_Xvalue[r_n] * g_Yvalue[r_n_1] - g_Xvalue[r_n_1] * g_Yvalue[r_n]) / (g_Xvalue[r_n] - g_Xvalue[r_n_1]);
//double A_n = 1 + sqrt(K_n);
//double a_n, b_n, c_n;
////a = 1+k2 , b =2k(B-yo)-2xo , c=xo2+(B-yo)2-R2
//a_n = 1 + pow(K_n, 2);
//b_n = 2 * K_n*(B_n - yo) - 2 * xo;
//c_n = pow(xo, 2) + pow(B_n - yo, 2) - pow(g_distance, 2);
////��ʼ�⽻������
//double delt_n = pow(b_n,2)-4*a_n*c_n;
//double sqrt_delt_n = sqrt(delt_n);
//double x1 = (sqrt_delt_n - b_n) / (2 * a_n);
//double x2 = (-sqrt_delt_n - b_n) / (2 * a_n);
//double x_get, y_get;
//if (x1 >= g_Xvalue[r_n_1] && x1 <= g_Xvalue[r_n])
//{
//    x_get = x1;
//}
//else if(x2 >= g_Xvalue[r_n_1] && x2 <= g_Xvalue[r_n])
//{
//    x_get = x2;
//}
//y_get = K_n*x_get + B_n;
//double dis_test =sqrt( pow(x_get - xo, 2) + pow(y_get - yo, 2));
//LogMessage("ok!");
*/
