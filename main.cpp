

#include <iostream>
#include <vector>
#include <map>

#include <chrono>
#include <thread>

#include "main.h"
#include "ping.h"

int main() {
	std::cout << "Starting srcds watchdog" << std::endl;

    std::map<unsigned int, MIB_UDPROW_OWNER_PID> servers;
    std::map<unsigned int, MIB_UDPROW_OWNER_PID> ports;

    while (true) {

        DWORD bufsize = 0;
        std::vector<MIB_UDPROW_OWNER_PID> UDPtables;

        auto ret = GetExtendedUdpTable(0, &bufsize, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
        auto buf = new char[bufsize];
        ret = GetExtendedUdpTable(buf, &bufsize, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
        if (ret == NO_ERROR)
        {
            MIB_UDPTABLE_OWNER_PID* pUDPTable = (MIB_UDPTABLE_OWNER_PID*)buf;
            UDPtables.assign(pUDPTable->table, pUDPTable->table + pUDPTable->dwNumEntries);

            for (DWORD a = 0; a < UDPtables.size(); a++) {
                MIB_UDPROW_OWNER_PID& row = UDPtables[a];

                ports.emplace(
                    ntohs((u_short)row.dwLocalPort), 
                    row
                );
            }
        }


        WTS_PROCESS_INFO* pWPIs = NULL;
        DWORD dwProcCount = 0;
        if (WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, NULL, 1, &pWPIs, &dwProcCount))
        {
            //Go through all processes retrieved
            for (DWORD i = 0; i < dwProcCount; i++)
            {
                // check for srcds.exe
                const std::string name(pWPIs[i].pProcessName);

                if (strcmp(name.c_str(), "srcds.exe") == 0) {
                    const unsigned int pid = pWPIs[i].ProcessId;

                    for (auto& kv : ports) {
                        if (kv.second.dwOwningPid == pid) {
                            servers.emplace(pid, kv.second);
                        }
                    }
                }
            }
        }

        for (auto& kv : servers) {
            auto& server = kv.second;

            std::string port = std::to_string(ntohs((u_short)server.dwLocalPort));
            
            char buf[32];
            ltoa((long)server.dwLocalAddr, buf, 10);

            std::string ip(buf);
            if (ip.size() < 8) {
                ip = "127.0.0.1";
            }

            std::cout << "server: " << ip << "[" << port << "]" << std::endl;

            const int response = Query(ip.c_str(), port.c_str(), 15);
            
            std::cout << "server: " << ip << "[" << port << "] response: " << response << std::endl;
        
        }

        //Free memory
        if (pWPIs)
        {
            WTSFreeMemory(pWPIs);
            pWPIs = NULL;
        }

        return 0;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

	return 0;
}