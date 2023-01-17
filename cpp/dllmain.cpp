// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

#define JSON_SUPPORT
#define SHELL32_SUPPORT
#define DOTNET_SUPPORT
#include "../../cpp-utils/utils.hpp"
#include "resource.h"
#include <future>

#define PROP_WNDPROC (_T("WNDPROC"))
#define JSON_KEY_HWND ("hWnd")
#define JSON_KEY_INDEX ("index")
#define JSON_KEY_TOTAL ("total")
#define JSON_KEY_FILE ("file")

typedef int(*Count)(const wchar_t* str);
typedef const wchar_t* (*Through)(const wchar_t* str);
typedef const wchar_t* (*GetString)();

constexpr auto MENU_ITEM_DISPLAY_TEXT = _T("Image to PDF(A4)");

auto ITEXTSHARP_WRAPPER = _T("iTextSharpWrapper.dll");
auto ITEXTSHARP_WRAPPER_CLASS = _T("iTextSharpWrapper.impl");
auto ITEXTSHARP_WRAPPER_METHOD = _T("invoke");

TCHAR module[MAX_PATH] = { 0 };
string_t iTextSharpWrapperDll;
bool isValidFileName(string_t);
INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HMODULE hInstance = nullptr;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    hInstance = hModule;
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
;           GetModuleFileName(hModule, module, ARRAYSIZE(module) - 1);
            iTextSharpWrapperDll = utils::io::path::combine(utils::io::path::GetDirectoryPath(module), ITEXTSHARP_WRAPPER);
            utils::shell32::menuItemDisplayText = MENU_ITEM_DISPLAY_TEXT;
            utils::shell32::init(hModule,
                [&](std::vector<std::wstring>& files) {
                HRESULT hr = E_INVALIDARG;
            for (auto& file : files) {
                if (isValidFileName(file) || utils::io::directory::exists(file)) {
                    hr = S_OK;
                    break;
                }
            }
            return hr;
            },
                [&](std::vector<std::wstring>& files) {
                if (!files.empty()) {
                    auto l = GetLastError();
                    if (utils::io::file::exists(iTextSharpWrapperDll)) {
                        auto imageFiles = Json::Value();
                        auto index = 0;
                        auto directories = std::vector<std::wstring>();
                        auto pdfFileName = std::wstring();
                        for (auto& file : utils::strings::sort(files)) {
                            if (utils::io::directory::exists(file)) {
                                for (auto& e : utils::io::directory::GetFiles(file)) {
                                    if (isValidFileName(e)) {
                                        pdfFileName = e;
                                        imageFiles[index++] = utils::strings::t2utf8(e);
                                    }
                                }
                            } else if (isValidFileName(file)) {
                                pdfFileName = file;
                                imageFiles[index++] = utils::strings::t2utf8(file);
                            }
                        }
                        if (!imageFiles.empty()) {
                            auto imageDirectory = utils::io::path::GetDirectoryPath(pdfFileName);
                            auto fileName = utils::io::path::GetFileName(imageDirectory);
                            auto pdfFilePath = utils::io::path::combine(imageDirectory, fileName) + _T(".pdf");
                            auto json = Json::Value();
                            json["imageFiles"] = imageFiles;
                            json["pdfFilePath"] = utils::strings::t2utf8(pdfFilePath);
                            auto dump = json.toStyledString();
                            DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_PROGRESS), nullptr, DialogProc, reinterpret_cast<LPARAM>(utils::strings::GetBufferA(dump)));
                        }
                    }
                }
            }, IDB_UNWITTING_LIFE);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

bool isValidFileName(string_t fileName) {
    auto extensionName = utils::io::path::GetFileExtensionName(fileName);
    return (utils::strings::equalsIgnoreCase(extensionName, _T(".webp")) ||
        utils::strings::equalsIgnoreCase(extensionName, _T(".jpeg")) ||
        utils::strings::equalsIgnoreCase(extensionName, _T(".bmp")) ||
        utils::strings::equalsIgnoreCase(extensionName, _T(".tiff")) ||
        utils::strings::equalsIgnoreCase(extensionName, _T(".png")));
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    auto retVal = FALSE;
    switch (uMsg) {
        case WM_INITDIALOG: {
            RECT rt = { 0 };
            if (GetWindowRect(hwndDlg, &rt)) {
                auto width = rt.right - rt.left;
                auto height = rt.bottom - rt.top;
                auto srcWidth = GetSystemMetrics(SM_CXSCREEN);
                auto srcHeight = GetSystemMetrics(SM_CYSCREEN);
                MoveWindow(hwndDlg, (srcWidth - width) / 2, (srcHeight - height) / 2, width, height, TRUE);
            }
            auto s = reinterpret_cast<char*>(lParam);
            if (s) {
                auto json = Json::Value();
                Json::Reader().parse(s, json);
                json[JSON_KEY_HWND] = __int64(hwndDlg);
                delete[] s;
                s = utils::strings::GetBufferA(json.toStyledString());
                auto dwThreadId = DWORD(0);
                CloseHandle(CreateThread(nullptr, 0, [](LPVOID lParam) -> DWORD {
                    auto dump = reinterpret_cast<char*>(lParam);
                if (dump) {
                    auto json = Json::Value();
                    Json::Reader().parse(dump, json);
                    auto id = GetCurrentThreadId();
                    auto hDlg = reinterpret_cast<HWND>(json[JSON_KEY_HWND].asInt64());
                    if (hDlg && utils::io::file::exists(iTextSharpWrapperDll)) {
                        utils::dotnet::clr::invoke(
                            iTextSharpWrapperDll,
                            ITEXTSHARP_WRAPPER_CLASS,
                            ITEXTSHARP_WRAPPER_METHOD,
                            utils::strings::t2t(dump));
                        EndDialog(hDlg, 0);
                    }
                    delete[] dump;
                }
                return 0;
                }, reinterpret_cast<LPVOID>(s), 0, &dwThreadId));
                auto progress = GetDlgItem(hwndDlg, IDS_PROGRESS);
                auto sytle = SetWindowLongPtr(progress, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));
                SetProp(progress, PROP_WNDPROC, reinterpret_cast<HANDLE>(sytle));
                retVal = TRUE;
            }
            break;
        }

        case WM_MOUSEMOVE: {
            if (wParam & MK_LBUTTON) {
                PostMessage(hwndDlg, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            }
            break;
        }

        case WM_USER + 1: {
            auto s = reinterpret_cast<char*>(wParam);
            if (s) {
                auto json = Json::Value();
                Json::Reader().parse(s, json);
                auto index = json[JSON_KEY_INDEX].asInt();
                auto total = json[JSON_KEY_TOTAL].asInt();
                auto file = json[JSON_KEY_FILE].asString();
                SendMessage(GetDlgItem(hwndDlg, IDC_PROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, total));
                SendMessage(GetDlgItem(hwndDlg, IDC_PROGRESS), PBM_SETPOS, index, 0);
                SetWindowText(GetDlgItem(hwndDlg, IDS_PROGRESS), utils::strings::format(_T("%d%%"), (int)((double)index / total * 100)).c_str());
                SetWindowText(GetDlgItem(hwndDlg, IDS_FILE), utils::strings::t2t(utils::io::path::GetFileNameA(file)).c_str());
            }
            break;
        }
    }
    return retVal;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wparam, LPARAM lparam) {
    if (Message == WM_PAINT) {
        TCHAR s[1000] = { 0 };
        GetWindowText(hwnd, s, ARRAYSIZE(s) - 1);

        RECT rc = { 0 };
        PAINTSTRUCT ps = { 0 };
        HDC hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rc);
        SetBkMode(hdc, TRANSPARENT);
        //DrawText(hdc, s, _tcslen(s), &rc, DT_CENTER | DT_VCENTER);
        EndPaint(hwnd, &ps);
        return 0;
    }
    return CallWindowProc(reinterpret_cast<WNDPROC>(GetProp(hwnd, PROP_WNDPROC)), hwnd, Message, wparam, lparam);
}
