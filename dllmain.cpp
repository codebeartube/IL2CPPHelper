// dllmain.cpp : DLL 애플리케이션의 진입점을 정의합니다.
#include "pch.h"
#include <Windows.h>
#include <cstdio>
#include <MinHook.h>
#include <stdint.h>
#include <chrono>

//{
//    "Address": 8709504,
//        "Name" : "UnityEngine.Input$$GetKey",
//        "Signature" : "bool UnityEngine_Input__GetKey (int32_t key, const MethodInfo* method);",
//        "TypeSignature" : "iii"
//},

//{
//	"Address": 8709440,
//		"Name" : "UnityEngine.Input$$GetKeyDown",
//		"Signature" : "bool UnityEngine_Input__GetKeyDown (int32_t key, const MethodInfo* method);",
//		"TypeSignature" : "iii"
//},

uintptr_t GameAssembly = (uintptr_t)GetModuleHandle("GameAssembly.dll");

bool(__fastcall* InputGetKey)(int32_t key, const DWORD* method);

bool __stdcall InputGetKeyHook(int32_t key, const DWORD* method)
{
	if (key == 0x20)
		return true;

	return InputGetKey(key, method);
}

const unsigned int intervalMs = 1000;
unsigned int lastCallTime = 0;

bool(__fastcall* InputGetKeyDown)(int32_t key, const DWORD* method);

bool __stdcall InputGetKeyDownHook(int32_t key, const DWORD* method)
{
	unsigned int currentTime = static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	if (lastCallTime == 0)
		lastCallTime = currentTime;

	unsigned int duration = currentTime - lastCallTime;
	printf("Duration = %ld\n", duration);

	if (key == 0x20)
	{
		if (duration > intervalMs)
		{
			lastCallTime = currentTime;
			return true;
		}
	}

	return InputGetKeyDown(key, method);
}

void ThreadProc()
{
	AllocConsole();

	FILE* fp = NULL;
	freopen_s(&fp, "CONOUT$", "w", stdout);

	printf("Hi! I'm injected successfully!!! ^_^");

	MH_Initialize();

	MH_CreateHook(reinterpret_cast<LPVOID*>(GameAssembly + 0x84E580), &InputGetKeyHook, reinterpret_cast<LPVOID*>(&InputGetKey));
	MH_EnableHook(reinterpret_cast<LPVOID*>(GameAssembly + 0x84E580));

	MH_CreateHook(reinterpret_cast<LPVOID*>(GameAssembly + 0x84E540), &InputGetKeyDownHook, reinterpret_cast<LPVOID*>(&InputGetKeyDown));
	MH_EnableHook(reinterpret_cast<LPVOID*>(GameAssembly + 0x84E540));
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadProc, 0, 0, 0);
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

