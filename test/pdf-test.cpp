// pdf-test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

#define JSON_SUPPORT
#define SHELL32_SUPPORT
#define DOTNET_SUPPORT
#include "../../cpp-utils/utils.hpp"
#include "resource.h"

#define JSON_HWND ("hWnd")
#define JSON_INDEX ("index")
#define JSON_TOTAL ("total")
#define JSON_FILE ("file")

auto ITEXTSHARP_WRAPPER = _T("iTextSharpWrapper.dll");
auto ITEXTSHARP_WRAPPER_CLASS = _T("iTextSharpWrapper.implement");
auto ITEXTSHARP_WRAPPER_METHOD = _T("invoke");

INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);

TCHAR module[MAX_PATH] = { 0 };
string_t iTextSharpWrapperDll;

int main() {
    auto hBitmap = reinterpret_cast<HBITMAP>(LoadImage(GetModuleHandle(nullptr),
        MAKEINTRESOURCE(IDB_UNWITTING_LIFE), IMAGE_BITMAP, 0, 0,
        LR_DEFAULTSIZE | LR_LOADTRANSPARENT | LR_CREATEDIBSECTION));
    auto transaprent = utils::gdi32::CreateTransparentBitmap(hBitmap);
    auto hDC = GetDC(nullptr);
    auto mem = CreateCompatibleDC(hDC);
    SelectObject(mem, transaprent);
    BitBlt(hDC, 0, 0, 32, 32, mem, 0, 0, SRCCOPY);
 /*   GetModuleFileName(nullptr, module, ARRAYSIZE(module) - 1);
    iTextSharpWrapperDll = utils::io::path::combine(utils::io::path::GetDirectoryPath(module), ITEXTSHARP_WRAPPER);

    auto imageDirectory = _T("D:\\.sources\\github.com\\unwitting-life\\manhuagui.com\\manhuagui.com\\哆啦A梦之解谜侦探团");
    auto imageFiles = Json::Value();
    auto index = 0;
    auto imageFilePath = std::wstring();
    for (auto& imageFile : utils::io::directory::GetFiles(imageDirectory)) {
        auto extensionName = utils::io::path::GetFileExtensionName(imageFile);
        if (utils::strings::equalsIgnoreCase(extensionName, _T(".webp")) ||
            utils::strings::equalsIgnoreCase(extensionName, _T(".jpeg")) ||
            utils::strings::equalsIgnoreCase(extensionName, _T(".png"))) {
            if (imageFilePath.empty()) {
                imageFilePath = imageFile;
            }
            imageFiles[index++] = utils::strings::t2utf8(imageFile);
        }
    }
    auto pdfFilePath = utils::io::path::combine(imageDirectory, utils::io::path::GetFileNameWithouExtension(imageFilePath)) + _T(".pdf");
    auto json = Json::Value();
    json["imageFiles"] = imageFiles;
    json["pdfFilePath"] = utils::strings::t2utf8(pdfFilePath);
    utils::dotnet::clr::invoke(
        iTextSharpWrapperDll,
        ITEXTSHARP_WRAPPER_CLASS,
        ITEXTSHARP_WRAPPER_METHOD,
        utils::strings::t2t(json.toStyledString()));
    _getch();*/
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
                json[JSON_HWND] = __int64(hwndDlg);
                delete[] s;
                s = utils::strings::GetBufferA(json.toStyledString());
                auto dwThreadId = DWORD(0);
                CloseHandle(CreateThread(nullptr, 0, [](LPVOID lParam) -> DWORD {
                    auto dump = reinterpret_cast<char*>(lParam);
                if (dump) {
                    auto json = Json::Value();
                    Json::Reader().parse(dump, json);
                    auto id = GetCurrentThreadId();
                    auto hDlg = reinterpret_cast<HWND>(json[JSON_HWND].asInt64());
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
                auto index = json[JSON_INDEX].asInt();
                auto total = json[JSON_TOTAL].asInt();
                auto file = json[JSON_FILE].asString();
                SendMessage(GetDlgItem(hwndDlg, IDC_PROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, total));
                SendMessage(GetDlgItem(hwndDlg, IDC_PROGRESS), PBM_SETPOS, index, 0);
              //  SetWindowText(GetDlgItem(hwndDlg, IDS_PROGRESS), utils::strings::format(_T("%d%%"), (int)((double)index / total * 100)).c_str());
                SetWindowText(GetDlgItem(hwndDlg, IDS_FILE), utils::strings::t2t(utils::io::path::GetFileNameA(file)).c_str());
            }
            break;
        }
    }
    return retVal;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
