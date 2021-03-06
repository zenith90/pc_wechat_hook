// GetUserLists.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include "commctrl.h"
#include <TlHelp32.h>
#include <stdlib.h>
#include <conio.h>
#include <locale.h>
#include <map>
#include <iostream>
#define HOOK_LEN 5
BYTE backCode[HOOK_LEN] = { 0 };
HWND hDlg = 0;
DWORD hookData = 0;
DWORD WinAdd = 0;
DWORD retCallAdd = 0;
DWORD retAdd = 0;
HWND gHwndList = 0;
wchar_t oldWxid[0x100] = { 0 };
wchar_t newWxid[0x100] = {0};
//获取模块基址
DWORD getWechatWin()
{
	return (DWORD)LoadLibrary("WeChatWin.dll");
}


//开始hook
/**
 * 参数一 hookAdd 想要hook的地址
 * 参数二 jmpAdd hook完回去的地址
 * 参数三 oldBye 备份被hook地址的那段二进制数据 用于恢复hook
**/
VOID StartHook(DWORD hookAdd, LPVOID jmpAdd)
{
	BYTE JmpCode[HOOK_LEN] = { 0 };
	//我们需要组成一段这样的数据
	// E9 11051111(这里是跳转的地方这个地方不是一个代码地址 而是根据hook地址和跳转的代码地址的距离计算出来的)
	JmpCode[0] = 0xE9;
	//计算跳转的距离公式是固定的
	//计算公式为 跳转的地址(也就是我们函数的地址) - hook的地址 - hook的字节长度
	*(DWORD *)&JmpCode[1] = (DWORD)jmpAdd - hookAdd - HOOK_LEN;

	//hook第二步 先备份将要被我们覆盖地址的数据 长度为我们hook的长度 HOOK_LEN 5个字节

	//获取进程句柄
	HANDLE hWHND = OpenProcess(PROCESS_ALL_ACCESS, NULL, GetCurrentProcessId());

	//备份数据
	if (ReadProcessMemory(hWHND, (LPVOID)hookAdd, backCode, HOOK_LEN, NULL) == 0) {
		MessageBox(NULL, "hook地址的数据读取失败", "读取失败", MB_OK);
		return;
	}

	//真正的hook开始了 把我们要替换的函数地址写进去 让他直接跳到我们函数里面去然后我们处理完毕后再放行吧！
	if (WriteProcessMemory(hWHND, (LPVOID)hookAdd, JmpCode, HOOK_LEN, NULL) == 0) {
		MessageBox(NULL, "hook写入失败，函数替换失败", "错误", MB_OK);
		return;
	}

}

CHAR* UnicodeToUTF8(const WCHAR* wideStr)
{
	char* utf8Str = NULL;
	int charLen = -1;
	charLen = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL);
	utf8Str = (char*)malloc(charLen);
	WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, utf8Str, charLen, NULL, NULL);
	return utf8Str;
}

char * UnicodeToANSI(const wchar_t *str)
{
	char * result;
	int textlen = 0;
	textlen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	result = (char *)malloc((textlen + 1) * sizeof(char));
	memset(result, 0, sizeof(char) * (textlen + 1));
	WideCharToMultiByte(CP_ACP, 0, str, -1, result, textlen, NULL, NULL);
	return result;
}



//显示好友列表
VOID insertUserLists(DWORD userData)
{
	/*
	eax + 0x10 wxid 群
	eax + 0x30 wxid 群
	eax + 0x44 微信号
	eax + 0x58 V1数据
	eax + 0x8C 昵称
	eax + 0x11C 小头像
	eax + 0x130 大头像
	eax + 0x144 未知md5数据
	eax + 0x1C8 国籍
	eax + 0x1DC 省份
	eax + 0x1F0 城市
	eax + 0x204 添加来源
	eax + 0x294 朋友圈壁纸
	*/

	DWORD wxidAdd = userData + 0x10;
	DWORD wxuserIDAdd = userData + 0x44;
	DWORD wxidV1Add = userData + 0x58;
	DWORD wxNickAdd = userData + 0x8C;
	DWORD headPicAdd = userData + 0x11C;
	//DWORD wxNickAdd = userData + 0x8C;

	wchar_t wxid[0x100] = {0};
	swprintf_s(wxid,L"%s", *((LPVOID *)wxidAdd));

	wchar_t nick[0x100] = { 0 };
	swprintf_s(nick, L"%s", *((LPVOID *)wxNickAdd));

	wchar_t wxuserID[0x100] = { 0 };
	swprintf_s(wxuserID, L"%s", *((LPVOID *)wxuserIDAdd));

	wchar_t headPic[0x100] = { 0 };
	if ((LPVOID *)headPicAdd != 0x0) {
		swprintf_s(headPic, L"%s", *((LPVOID *)headPicAdd));
	}
	if (oldWxid[0] == 0 && newWxid[0] == 0) {
		swprintf_s(newWxid, L"%s", *((LPVOID *)wxidAdd));
	}

	if (oldWxid[0] == 0 && newWxid[0] != 0) {
		swprintf_s(oldWxid, L"%s", newWxid);
		swprintf_s(newWxid, L"%s", *((LPVOID *)wxidAdd));
	}

	if (oldWxid[0] != 0 && newWxid[0] != 0) {
		swprintf_s(oldWxid, L"%s", newWxid);
		swprintf_s(newWxid, L"%s", *((LPVOID *)wxidAdd));
	}

	if (wcscmp(oldWxid,newWxid) != 0) {

		LVITEM item = { 0 };
		item.mask = LVIF_TEXT;

		item.iSubItem = 0;
		item.pszText = UnicodeToANSI(wxid);
		ListView_InsertItem(gHwndList, &item);

		item.iSubItem = 1;
		item.pszText = UnicodeToANSI(wxuserID);
		ListView_SetItem(gHwndList, &item);

		item.iSubItem = 2;
		item.pszText = UnicodeToANSI(nick);
		ListView_SetItem(gHwndList, &item);
	}
}


DWORD cEax = 0;
DWORD cEcx = 0;
DWORD cEdx = 0;
DWORD cEbx = 0;
DWORD cEsp = 0;
DWORD cEbp = 0;
DWORD cEsi = 0;
DWORD cEdi = 0;
DWORD userData = 0;
//跳转过来的函数 我们自己的
/*
5BBB989D  |.  E8 5EE8FFFF   call WeChatWi.5BBB8100   418100                                 ;  这里也可以获取到数
5BBB98A2  |.  84C0          test al,al                                                ;  edi是数据
51280
*/
VOID __declspec(naked) HookF()
{
	//pushad: 将所有的32位通用寄存器压入堆栈
	//pushfd:然后将32位标志寄存器EFLAGS压入堆栈
	//popad:将所有的32位通用寄存器取出堆栈
	//popfd:将32位标志寄存器EFLAGS取出堆栈
	//先保存寄存器
	// 使用pushad这些来搞还是不太稳定  还是用变量把寄存器的值保存下来 这样可靠点
	
	/*
	mov cEax, eax
		mov cEcx, ecx
		mov cEdx, edx
		mov cEbx, ebx
		mov cEsp, esp
		mov cEbp, ebp
		mov cEsi, esi
		mov cEdi, edi
	*/
	__asm {
		call retCallAdd
		mov cEax, eax
		mov cEcx, ecx
		mov cEdx, edx
		mov cEbx, ebx
		mov cEsp, esp
		mov cEbp, ebp
		mov cEsi, esi
		mov cEdi, edi
	}
	//然后跳转到我们自己的处理函数 想干嘛干嘛
	insertUserLists(cEax);
	//然后在还原他进来之前的所有数据
	/*popad
		popfd  不太可靠恢复不全 所以才有变量的方式保存下来再赋值过去*/
	/*
	mov eax, cEax
		mov ecx, cEcx
		mov edx, cEdx
		mov ebx, cEbx
		mov esp, cEsp
		mov ebp, cEbp
		mov esi, cEsi
		mov edi, cEdi
	*/
	__asm {
		mov eax, cEax
		mov ecx, cEcx
		mov edx, cEdx
		mov ebx, cEbx
		mov esp, cEsp
		mov ebp, cEbp
		mov esi, cEsi
		mov edi, cEdi
		jmp retAdd
	}
}

//41989D
//418154
VOID HookWechatQrcode(HWND hwndDlg, HWND hwndList, DWORD HookAdd)
{
	gHwndList = hwndList;
	WinAdd = getWechatWin();
	hDlg = hwndDlg;
	retCallAdd = WinAdd + 0x528A0;
	retAdd = WinAdd + 0x420F39;
	StartHook(WinAdd + HookAdd, &HookF);
}