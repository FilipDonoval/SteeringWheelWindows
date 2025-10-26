#include <Windows.h>
#include <ViGEm/Client.h>
#include <iostream>
#include <string>

#include "json.hpp"

using json = nlohmann::json;

/*
json jsonData = {
    {"steering", 0},
    {"object", {
        {"currency", "EURO"},
        {"value", 55.5} 
    }}
};
*/

json jsonData = {
    {"steering", 0},
    {"throttle", 0},
    {"brake", 0},
    {"gearUp", 0},
    {"gearDown", 0}
};



int main()
{


    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);
    /*
    std::cout << jsonData << std::endl;
    std::cout << "1" << std::endl;
    std::cout << jsonData.dump() << std::endl;
    std::string a = jsonData.dump(4);
    std::cout << "2" << std::endl;
    std::cout << a << std::endl;
    Sleep(10000);*/
    // ViGEm
    
    // Allocate and connect ViGEm client
    PVIGEM_CLIENT client = vigem_alloc();
    if (client == NULL)
    {
        std::cerr << "Failed to allocate ViGEm client.\n";
        return -1;
    }

    if (!VIGEM_SUCCESS(vigem_connect(client)))
    {
        std::cerr << "Failed to connect ViGEmBus.\n";
        vigem_free(client);
        return -1;
    }

    // Allocate virtual Xbox 360 controller
    PVIGEM_TARGET pad = vigem_target_x360_alloc();
    if (!VIGEM_SUCCESS(vigem_target_add(client, pad)))
    {
        std::cerr << "Failed to add virtual controller.\n";
        vigem_target_free(pad);
        vigem_free(client);
        return -1;
    }

    XUSB_REPORT report;
    ZeroMemory(&report, sizeof(XUSB_REPORT));

    // 10% left on X axis (-3276)
    report.sThumbLX = static_cast<SHORT>(-32768 * 0.50);
    report.sThumbLY = 0;

    report.wButtons = XUSB_GAMEPAD_X;

    // ViGEm END

    // adb
    void* hRead;
    void* hWrite;

    SECURITY_ATTRIBUTES securityAttributes = { sizeof(SECURITY_ATTRIBUTES), nullptr, true };
    bool createPipe = CreatePipe(&hRead, &hWrite, &securityAttributes, 0);
    if (!createPipe)
    {
        std::cerr << "Failed to create pipe" << std::endl;
        return 1;
    }


    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    PROCESS_INFORMATION processInfo;
    STARTUPINFOA startupInfo = { sizeof(startupInfo) };
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdOutput = hWrite;
    //si.hStdError = hWrite;

    //std::string programPath = "C:\\Users\\Filip\\Desktop\\platform-tools\\adb logcat -s PhoneInput -T 1";
    char programPath[] = "C:\\Users\\Filip\\Desktop\\platform-tools\\adb logcat -s PhoneOutput -T 1";
    //LPSTR

    //bool createProcess = CreateProcessA(nullptr, programPath.data(), nullptr, nullptr, true, 0, nullptr, nullptr, &startupInfo, &processInfo);
    bool createProcess = CreateProcessA(nullptr, programPath, nullptr, nullptr, true, 0, nullptr, nullptr, &startupInfo, &processInfo);
    if (!createProcess)
    {
        std::cerr << "Failed to create process. Error: " << GetLastError() << std::endl;
        CloseHandle(hRead);
        CloseHandle(hWrite);

        return 1;
    }

    CloseHandle(hWrite);

    /*
    char buf[512];
    memset(buf, 0, sizeof(buf));

    DWORD bytesRead;
    while (true)
    {
        if (!ReadFile(hRead, buf, sizeof(buf) - 1, &bytesRead, nullptr) || bytesRead == 0)
        {
            break;
        }

        buf[bytesRead] = '\0';
        std::cout << buf;
    }*/

    // adb END

    
    char buf[4096];
    memset(buf, 0, sizeof(buf));
    DWORD bytesRead;

    std::string data = "0";

    int leftStickPos = 0;
    bool leftStick = false;

    bool throttle = false;

    bool gearup = false;

    int count = 0;
    while (true)
    {
 
        DWORD bytesAvaliable = 0;
        PeekNamedPipe(hRead, nullptr, 0, nullptr, &bytesAvaliable, nullptr);
        if (bytesAvaliable > 0)
        {
            ReadFile(hRead, buf, sizeof(buf) - 1, &bytesRead, nullptr);
       
            std::cout << std::endl << std::endl;
            std::cout << std::endl << std::endl;
            std::cout << std::endl << std::endl;
            std::cout << std::endl << std::endl;
            std::cout << std::endl << std::endl;
            buf[bytesRead] = '\0';
            std::cout << "buffer->" << buf << std::endl;
            
            std::string str(buf);
            str = str.substr(str.find_last_of('\n', str.size() - 2) + 1);
            std::cout << "after->" << str << std::endl;
            str = str.substr(str.find(": ") + 2);
            std::cout << "trim->" << str << std::endl;


            jsonData = json::parse(str);

            std::cout << "json data: -> " << jsonData << std::endl << std::endl;

            /*
            data = str.substr(str.find(": ") + 2);
            if (data.starts_with("<leftStick>"))
            {
                data = str.substr(str.find("<leftStick>") + 11);
                leftStick = true;
            }
            else
            {
                leftStick = false;
            }

            if (data.starts_with("<throttle>"))
            {
                data = str.substr(str.find("<throttle>") + 10);
                int dataInt = std::stoi(data);
                if (dataInt > 200)
                {
                    throttle = true;
                }
                else
                {
                    throttle = false;
                }
                
            }

            if (data.starts_with("<gearup>"))
            {
                gearup = true;
            }
            */

            //std::cout << data << " size of" << data.size() << std::endl;
            /*for (char c : data) {
                //std::cout << "contains -> " << c << ": " << static_cast<int>(c) << std::endl;
                std::cout << "contains -> " << static_cast<int>(c) << std::endl;
            }*/
        }

        int dataInt = jsonData["steering"];
        std::cout << dataInt << std::endl;
        if (dataInt > 0)
        {
            leftStickPos = (dataInt - 9000) * -1;
            leftStickPos = ((leftStickPos - -9000) * (32767 - -32768)) / (9000 - -9000) - 32767;

        }
        report.sThumbLX = static_cast<SHORT>(leftStickPos);

        /*
        if (leftStick)
        {
            //int dataInt = std::stoi(data);
            int dataInt = jsonData["leftStick"];
            std::cout << dataInt << std::endl;
            if (dataInt > 0)
            {
                leftStickPos = (dataInt - 9000) * -1;

            }
        }*/


        report.bRightTrigger = static_cast<BYTE>(jsonData["throttle"]);
        report.bLeftTrigger = static_cast<BYTE>(jsonData["brake"]);
        /*
        if (throttle)
        {
            report.bRightTrigger = static_cast<BYTE>(200);
        }
        else
        {
            report.bRightTrigger = static_cast<BYTE>(0);
        }
        */
        //std::cout << leftStickPos << std::endl;
        
        //std::cout << "a: " << a << std::endl;
        

        if (gearup)
        {
            report.wButtons |= XUSB_GAMEPAD_X;
        }
        

        

        if (GetAsyncKeyState('X'))
        {
            break;
        }
        /*if (GetAsyncKeyState('A'))
        {
            //report.sThumbLX = static_cast<SHORT>(-32768 * 0.10);
            report.wButtons |= XUSB_GAMEPAD_DPAD_LEFT;
        }
        if (GetAsyncKeyState('D'))
        {
            //report.sThumbLX = static_cast<SHORT>(-32768 * 0.70);
        }*/



        if (jsonData["gearUp"] == 1)
        {
            std::cout << "GEAR UP" << std::endl;
            report.wButtons |= XUSB_GAMEPAD_A;
        }
        else
        {
            report.wButtons &= ~XUSB_GAMEPAD_A;
        }

        if (jsonData["gearDown"] == 1)
        {
            std::cout << "GEAR DOWN" << std::endl;
            report.wButtons |= XUSB_GAMEPAD_X;
        }
        else
        {
            report.wButtons &= ~XUSB_GAMEPAD_X;
        }


        /*
        static int aa = 0;
        if (gearup)
        {
            aa = 0;
            report.wButtons |= XUSB_GAMEPAD_A;
        }
        aa++;
        if (aa > 50)
        {
            report.wButtons &= XUSB_GAMEPAD_A;
        }*/
       

        vigem_target_x360_update(client, pad, report);
        std::cout << count++ << std::endl;
        Sleep(1);

    }


    // ViGEm
    // Cleanup
    vigem_target_remove(client, pad);
    vigem_target_free(pad);
    vigem_disconnect(client);
    vigem_free(client);


    // adb
    // Cleanup
    CloseHandle(hRead);
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
    return 0;
}
