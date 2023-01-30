// pdf-test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#ifdef _HAS_STD_BYTE
#undef _HAS_STD_BYTE
#endif
#define _HAS_STD_BYTE 0

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

bool isConvertibleImageFileName(string_t);
INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);

TCHAR module[MAX_PATH] = { 0 };
string_t iTextSharpWrapperDll;

int main() {
#if 0
    auto hBitmap = utils::gdi32::CreateBitmap(LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_UNWITTING_LIFE)), 22, 22);
    auto hDC = GetDC(nullptr);
    auto mem = CreateCompatibleDC(hDC);
    SelectObject(mem, hBitmap);
    BitBlt(hDC, 0, 0, 32, 32, mem, 0, 0, SRCCOPY);
#endif
    GetModuleFileName(nullptr, module, ARRAYSIZE(module) - 1);
    iTextSharpWrapperDll = utils::io::path::combine(utils::io::path::GetDirectoryPath(module), ITEXTSHARP_WRAPPER);

    auto files = std::vector<string_t>();
    files.push_back(_T("E:\\Downloads\\cropped-Nocchi-icon-32x32.bmp"));
    files.push_back(_T("D:\\.sources\\github.com\\unwitting-life\\manhuagui.com\\manhuagui.com\\哆啦A梦：超棒球外传"));

    auto imageFiles = Json::Value();
    auto index = 0;
    auto directories = std::vector<std::wstring>();
    auto imageFilePath = std::wstring();
    for (auto& file : utils::strings::sort(files)) {
        if (utils::io::directory::exists(file)) {
            for (auto& e : utils::io::directory::GetFiles(file)) {
                if (isConvertibleImageFileName(e)) {
                    if (imageFilePath.empty()) {
                        imageFilePath = e;
                    }
                    imageFiles[index++] = utils::strings::t2utf8(e);
                }
            }
        } else if (isConvertibleImageFileName(file)) {
            if (imageFilePath.empty()) {
                imageFilePath = file;
            }
            imageFiles[index++] = utils::strings::t2utf8(file);
        }
    }
    if (!imageFiles.empty()) {
        auto imageDirectory = utils::io::path::GetDirectoryPath(imageFilePath);
        auto pdfFilePath = utils::io::path::combine(imageDirectory,
            imageFiles.size() == 1 ?
            utils::io::path::GetFileNameWithouExtension(imageFilePath) :
            utils::io::path::GetFileName(imageDirectory)) + _T(".pdf");
        auto json = Json::Value();
        json["imageFiles"] = imageFiles;
        json["pdfFilePath"] = utils::strings::t2utf8(pdfFilePath);
        json["paperSize"] = "A5";
        utils::dotnet::clr::invoke(
            iTextSharpWrapperDll,
            ITEXTSHARP_WRAPPER_CLASS,
            ITEXTSHARP_WRAPPER_METHOD,
            utils::strings::t2t(json.toStyledString()));
    }
    _getch();
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

bool isConvertibleImageFileName(string_t fileName) {
    auto extensionName = utils::io::path::GetFileExtensionName(fileName);
    return (utils::strings::equalsIgnoreCase(extensionName, _T(".webp")) ||
        utils::strings::equalsIgnoreCase(extensionName, _T(".jpeg")) ||
        utils::strings::equalsIgnoreCase(extensionName, _T(".bmp")) ||
        utils::strings::equalsIgnoreCase(extensionName, _T(".tiff")) ||
        utils::strings::equalsIgnoreCase(extensionName, _T(".png")));
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
