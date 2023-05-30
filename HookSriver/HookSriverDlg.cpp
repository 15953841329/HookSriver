
// HookSriverDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "HookSriver.h"
#include "HookSriverDlg.h"
#include "afxdialogex.h"
#include<windows.h>
#include<winioctl.h>
#include<stdio.h>
#define IN_BUFFER_MAXLENGTH  0x10
#define OUT_BUFFER_MAXLENGTH  0x10
//宏定义之获取一个32位的宏控制码  参数:设备类型(鼠标,键盘...Unkonwn);0x000-0x7FF保留,0x800-0xfff随便填一个;数据交互类型(缓冲区,IO,其他);对这个设备的权限
#define OPER1 CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define OPER2 CTL_CODE(FILE_DEVICE_UNKNOWN,0x900,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define OPER3 CTL_CODE(FILE_DEVICE_UNKNOWN,0xA00,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define OPER4 CTL_CODE(FILE_DEVICE_UNKNOWN,0xB00,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define OPER5 CTL_CODE(FILE_DEVICE_UNKNOWN,0xC00,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define OPER6 CTL_CODE(FILE_DEVICE_UNKNOWN,0xD00,METHOD_BUFFERED,FILE_ANY_ACCESS)


#define SYMBOLICLINK_NAME  L"\\\\.\\MyTestDriver"  

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CHookSriverDlg 对话框



CHookSriverDlg::CHookSriverDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HOOKSRIVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHookSriverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT2, ed);
}

BEGIN_MESSAGE_MAP(CHookSriverDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//ON_BN_CLICKED(IDC_RADIO2, &CHookSriverDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_BUTTON1, &CHookSriverDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CHookSriverDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CHookSriverDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CHookSriverDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON2, &CHookSriverDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CHookSriverDlg 消息处理程序

BOOL CHookSriverDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CHookSriverDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CHookSriverDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
	ed.SetWindowTextW(L"输入进程PID");
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。

HCURSOR CHookSriverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



HANDLE g_hDevice;   //全局驱动句柄

//打开驱动服务句柄
//3环链接名:\\\\.\\AABB

BOOL Open(LPCWCHAR pLinkName)
{
	//在3环获取设备句柄
	TCHAR szBuffer[10] = { 0 };
	//CreateFile  打开的是内核的设备对象 
	g_hDevice = CreateFile(pLinkName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (g_hDevice != INVALID_HANDLE_VALUE)
		return TRUE;
	else
		return FALSE;

}



void CHookSriverDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	DWORD dwInBuffer = 0x12345678;
	DWORD szOutBuffer = 0;
	DWORD ByteReturned = 0;
	CString strText;
	GetDlgItemText(IDC_EDIT2, strText);
	// IDC_EDIT_INPUT
	//1.通过符号链接,打开设备
	if (!Open(SYMBOLICLINK_NAME))
	{
		MessageBoxW(NULL, L"未检测到驱动", NULL);
		
	}

	//2.测试通信
	
	DeviceIoControl(g_hDevice, OPER5, &strText, IN_BUFFER_MAXLENGTH, &szOutBuffer, OUT_BUFFER_MAXLENGTH, &ByteReturned, NULL);

	//3.关闭设备
	CloseHandle(g_hDevice);
}


void CHookSriverDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	DWORD dwInBuffer = 0x12345678;
	DWORD szOutBuffer = 0;
	DWORD ByteReturned = 0;

	//1.通过符号链接,打开设备
	if (!Open(SYMBOLICLINK_NAME))
	{
		
		MessageBoxW(NULL,L"未检测到驱动", NULL);

	}

	//2.测试通信

	DeviceIoControl(g_hDevice, OPER1, &dwInBuffer, IN_BUFFER_MAXLENGTH, &szOutBuffer, OUT_BUFFER_MAXLENGTH, &ByteReturned, NULL);

	//3.关闭设备
	CloseHandle(g_hDevice);
}


void CHookSriverDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	DWORD dwInBuffer = 0x12345678;
	DWORD szOutBuffer = 0;
	DWORD ByteReturned = 0;

	//1.通过符号链接,打开设备
	if (!Open(SYMBOLICLINK_NAME))
	{
		MessageBoxW(NULL, L"设备对象打开失败", NULL);
		MessageBoxW(NULL, L"未检测到驱动", NULL);
	

	}

	//2.测试通信

	DeviceIoControl(g_hDevice, OPER3, &dwInBuffer, IN_BUFFER_MAXLENGTH, &szOutBuffer, OUT_BUFFER_MAXLENGTH, &ByteReturned, NULL);

	//3.关闭设备
	CloseHandle(g_hDevice);
}


void CHookSriverDlg::OnBnClickedButton5()
{
	DWORD dwInBuffer = 0x12345678;
	DWORD szOutBuffer = 0;
	DWORD ByteReturned = 0;

	//1.通过符号链接,打开设备
	if (!Open(SYMBOLICLINK_NAME))
	{
		MessageBoxW(NULL, L"设备对象打开失败", NULL);
		MessageBoxW(NULL, L"未检测到驱动", NULL);

	}

	//2.测试通信

	DeviceIoControl(g_hDevice, OPER4, &dwInBuffer, IN_BUFFER_MAXLENGTH, &szOutBuffer, OUT_BUFFER_MAXLENGTH, &ByteReturned, NULL);

	//3.关闭设备
	CloseHandle(g_hDevice);
}


void CHookSriverDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	DWORD dwInBuffer = 0x12345678;
	DWORD szOutBuffer = 0;
	DWORD ByteReturned = 0;

	//1.通过符号链接,打开设备
	if (!Open(SYMBOLICLINK_NAME))
	{
		MessageBoxW(NULL, L"设备对象打开失败", NULL);
		MessageBoxW(NULL, L"未检测到驱动", NULL);


	}

	//2.测试通信

	DeviceIoControl(g_hDevice, OPER6, &dwInBuffer, IN_BUFFER_MAXLENGTH, &szOutBuffer, OUT_BUFFER_MAXLENGTH, &ByteReturned, NULL);

	//3.关闭设备
	CloseHandle(g_hDevice);
}
