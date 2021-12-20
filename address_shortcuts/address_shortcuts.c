/*
** OllyDbg Address Shortcuts plugin (for OllyDbg v1.0.10) - address_shortcuts.c
** Authors: Jony
** License: Public domain
*/

#define _CRT_SECURE_NO_DEPRECATE
#define UNICODE
#define _UNICODE
#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "plugin.h"


HWND hwmain;

BOOL WINAPI DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

static uint32_t get_selection(t_table* pt)
{
	t_dump* pd;
	uint32_t address;

	if (pt != NULL
		&& (address = pd->sel0) < pd->sel1)
		return address;

	/* No selection */
	return 0;
}

static const char* get_filename(const char* path)
{
	const char** last_slash = path - 1;

	while (*path != 0) {
		if (*path == '/' || *path == '\\')
			last_slash = path;
		path++;
	}

	return last_slash + 1;
}

enum copy_mode {
	rva,
	rva_pretty,
	file_offset,
	file_offset_pretty
};

enum follow_mode {
	disasm,
	dump,
	stack
};

static void copy_to_clipboard(uint32_t address, enum copy_mode mode)
{
	t_module* m;
	HANDLE hData;
	char* str;
	uint32_t offset;
	const char* name;

	m = _Findmodule(address);
	if (!m) {
		MessageBoxA(NULL, "Address does not belong to any module", "Tips", 0);
		return;
	}

	if (mode == rva || mode == rva_pretty) {
		offset = address - m->base;
		name = m->name;
	}
	else {
		offset = _Findfileoffset(m, address);
		if (offset == 0) {
			MessageBoxA(NULL, "Address has no physical mapping in file", "Tips", 0);
			return;
		}

		name = get_filename(m->path);
	}

	/* Copy to the clipboard. */

	hData = GlobalAlloc(GMEM_MOVEABLE, (strlen(name) + 11 + 1) * sizeof(char));
	if (!hData)
		return;

	str = GlobalLock(hData);
	if (!str)
		goto close_hdata;

	if (mode == rva_pretty || mode == file_offset_pretty)
		sprintf(str, "%s+0x%x", name, offset);
	else
		sprintf(str, "%08X", offset);

	GlobalUnlock(hData);

	if (!OpenClipboard(NULL))
		goto close_hdata;

	if (!EmptyClipboard())
		goto close_clipboard;

	if (!SetClipboardData(CF_TEXT, hData))
		goto close_clipboard;

	CloseClipboard();
	return;

close_clipboard:
	CloseClipboard();
close_hdata:
	GlobalFree(hData);
}

extc void _export __cdecl ODBG_Pluginaction(int origin, int action, void* item) {
	if (origin == PM_MAIN) {
		switch (action)
		{
		case 0:
			MessageBox(hwmain,
				L"Address Shortcuts\r\n"
				L"原作者: Andrew D'Addesio \r\n"
				L"适配修改: Jony \r\n"
				L"URL: https://github.com/caomfan/address_shortcuts\r\n\r\n"
				L"This plugin is public domain software released under the UNLICENSE.",
				L"Address Shortcuts", MB_OK | MB_ICONINFORMATION);
		default:
			break;
		}
	}
	if (origin == PM_DISASM || origin == PM_DUMP || origin == PM_CPUDUMP || origin == PM_CPUSTACK) {
		t_dump* pd = (t_dump*)item;
		switch (action)
		{
		case 0:
			copy_to_clipboard(pd->sel0, rva);
			break;
		case 1:
			copy_to_clipboard(pd->sel0, rva_pretty);
			break;
		case 2:
			copy_to_clipboard(pd->sel0, file_offset);
			break;
		case 3:
			copy_to_clipboard(pd->sel0, file_offset_pretty);
			break;
		default:
			break;
		}
	}
}

extc int _export __cdecl ODBG_Pluginmenu(int origin, CHAR data[4096], VOID* item)
{

	switch (origin)
	{
	case PM_MAIN:
		strcpy(data, "0 &about");
		return 1;
	case PM_DISASM:
	case PM_DUMP:
	case PM_CPUDUMP:
	case PM_CPUSTACK:
		strcpy(data, "Address Shortcuts {0 &Copy RVA,1 &Copy RVA (pretty),2 &Copy file offset,3 &Copy file offset (pretty)}");
		return 1;
	default:
		break;
	}
	return NULL;
};

extc int _export __cdecl ODBG_Plugininit(int ollydbgversion, HWND hw, DWORD* features)
{
	if (ollydbgversion < PLUGIN_VERSION) {
		_Addtolist(0, 0, "提示: 插件版本与OD不匹配!");
		return -1;
	}
	hwmain = hw;
	return 0;
}

extc void _export __cdecl ODBG_Pluginreset(void)
{
}

extc void _export __cdecl ODBG_Plugindestroy(void)
{
}

extc int _export _cdecl ODBG_Plugindata(char shortname[32])
{
	strcpy(shortname, "Address Shortcuts");
	return PLUGIN_VERSION;
};
