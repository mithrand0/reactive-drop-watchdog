#pragma once
#include <string>
#include <windows.h>
#include <wtsapi32.h>
#include <iphlpapi.h>
#include <winsock.h>

#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

int main();
