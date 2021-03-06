// TeachDemo.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "TeachDemo.h"
#include "resource1.h"
#include "Inject.h"
#include <string.h>
#include <wchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <direct.h>

//------------------------------------函数申明区-------------------------------------------------
INT_PTR CALLBACK DialogProc( HWND   hwndDlg, UINT   uMsg, WPARAM wParam, LPARAM lParam);
BOOL handleEventMsg(HWND hwndDlg, WPARAM wParam);
VOID setWindowWechat(HWND hwndDlg);
//------------------------------------函数申明区-------------------------------------------------


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	//加载我们的界面
	DialogBox(hInstance, MAKEINTRESOURCE(ID_MAIN), 0, &DialogProc);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT   uMsg, WPARAM wParam, LPARAM lParam)
{
	wchar_t initPath[0x100] = {L"E:\\Program Files (x86)\\Tencent\\WeChat\\WeChat.exe"};
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hwndDlg, WECHAT_PATH, initPath);
		break;
	case WM_COMMAND:
		return handleEventMsg(hwndDlg, wParam);
		break;
	case WM_CLOSE:
		EndDialog(hwndDlg,0);
		break;
	default:
		break;
	}
	return FALSE;
}

//界面事件处理函数
BOOL handleEventMsg(HWND hwndDlg, WPARAM wParam)
{
	//wchar_t processName[0x100] = {0};
	//swprintf_s(processName, L"D://code//c//WechatDll//Debug//WechatDll.dll");
	//CHAR dllName[] = { "D://code//c//WechatDll//Debug//WechatDll.dll" };
	//CHAR dllName[] = { "D://code//c//TeachDemo//Debug//GetWechatInfo.dll" };
	TCHAR paths[0x1000] = { 0 };
	TCHAR *buffer = NULL;
	TCHAR wechat_path[0x200] = {0};
	if ((buffer = _wgetcwd(NULL, 0)) == NULL)
	{
		perror("getcwd error");
	}
	else
	{
		//MessageBox(NULL, buffer, "aa", MB_OK);
		swprintf_s(paths, L"%s\\SendXmlMessage.dll", buffer);
		/*printf("%s\n", buffer);
		free(buffer);*/
	}
	switch (wParam)
	{
	case ID_INJECT:
		GetDlgItemText(hwndDlg,WECHAT_PATH, wechat_path, sizeof(wechat_path));
		runWechat(paths, wechat_path);
		return TRUE;
		break;
	case ID_UN_INJECT:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)setWindowWechat, hwndDlg, 0, NULL);
		return TRUE;
		break;
	default:
		break;
	}
	return FALSE;
}

VOID setWindowWechat(HWND hwndDlg)
{
	while (true)
	{
		setWindow(hwndDlg);
	}
}