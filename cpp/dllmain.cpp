// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

#define JSON_SUPPORT
#define SHELL32_SUPPORT
#define DOTNET_SUPPORT
#include "../../cpp-utils/utils.hpp"
#include "resource.h"
#include <future>

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

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
;           GetModuleFileName(hModule, module, ARRAYSIZE(module) - 1);
            iTextSharpWrapperDll = utils::io::path::combine(utils::io::path::GetDirectoryPath(module), ITEXTSHARP_WRAPPER);
            utils::shell32::menuItemDisplayText = MENU_ITEM_DISPLAY_TEXT;
            utils::shell32::init(hModule,
                [&](std::vector<std::wstring>& files) {
                    HRESULT hr = E_INVALIDARG;
                    for (auto& file : files) {
                        if (isValidFileName(file)) {
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
                        for (auto& file : files) {
                            if (isValidFileName(file)) {
                                imageFiles[index++] = utils::strings::t2utf8(file);
                            }
                        }
                        auto imageDirectory = utils::io::path::GetDirectoryPath(files[0]);
                        auto fileName = utils::io::path::GetFileNameWithouExtension(files[0]);
                        auto pdfFilePath = utils::io::path::combine(imageDirectory, fileName) + _T(".pdf");
                        auto json = Json::Value();
                        json["imageFiles"] = imageFiles;
                        json["pdfFilePath"] = utils::strings::t2utf8(pdfFilePath);
                        auto dump = json.toStyledString();
                        auto _ = std::async(std::launch::async, &utils::dotnet::clr::invoke,
                            iTextSharpWrapperDll, ITEXTSHARP_WRAPPER_CLASS, ITEXTSHARP_WRAPPER_METHOD,
                            utils::strings::GetBuffer(utils::strings::t2t(dump)));
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