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

constexpr auto IMAGE_FILES = "imageFiles";
constexpr auto PDF_FILE_PATH = "pdfFilePath";
constexpr auto PAPER_SIZE = "paperSize";
constexpr auto MENU_ITEM_PDF = _T("Image to PDF");
constexpr auto MENU_ITEM_PDF_SPECIAL = _T("Image to PDF (Special)");
constexpr auto PAPER_SIZE_A3 = _T("A3");
constexpr auto PAPER_SIZE_A4 = _T("A4");
constexpr auto PAPER_SIZE_A5 = _T("A5");
constexpr auto PAPER_SIZE_B5 = _T("B5");
constexpr auto IDM_COMMAND = 0;
constexpr auto IDM_COMMAND_PDF = IDM_COMMAND + 0;
constexpr auto IDM_COMMAND_PDF_SPECIAL = IDM_COMMAND + 1;
constexpr auto IDM_COMMAND_PDF_A3 = IDM_COMMAND + 2;
constexpr auto IDM_COMMAND_PDF_A4 = IDM_COMMAND + 3;
constexpr auto IDM_COMMAND_PDF_A5 = IDM_COMMAND + 4;
constexpr auto IDM_COMMAND_PDF_B5 = IDM_COMMAND + 5;
constexpr auto IDM_COMMAND_LAST = IDM_COMMAND_PDF_B5;

auto ITEXTSHARP_WRAPPER = _T("iTextSharpWrapper.dll");
auto ITEXTSHARP_WRAPPER_CLASS = _T("iTextSharpWrapper.implement");
auto ITEXTSHARP_WRAPPER_METHOD = _T("invoke");

TCHAR module[MAX_PATH] = { 0 };
string_t iTextSharpWrapperDll;
bool isConvertibleImageFileName(string_t);
bool AppendMenuItem(HMENU, LPCWSTR, WORD);
INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HMODULE hInstance = nullptr;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    hInstance = hModule;
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
;           GetModuleFileName(hModule, module, ARRAYSIZE(module) - 1);
            iTextSharpWrapperDll = utils::io::path::combine(utils::io::path::GetDirectoryPath(module), ITEXTSHARP_WRAPPER);
            utils::shell32::init(hModule);
            utils::shell32::OnInitialized = [](std::vector<std::wstring>& files) {
                HRESULT hr = E_INVALIDARG;
                for (auto& file : files) {
                    if (isConvertibleImageFileName(file) || utils::io::directory::exists(file)) {
                        hr = S_OK;
                        break;
                    }
                }
                return hr;
            };
            utils::shell32::OnCommand = [](LPCMINVOKECOMMANDINFO pici, std::vector<std::wstring>& files) {
                auto hr = HRESULT(E_FAIL);
                if (pici && !files.empty()) {
                    auto wCommand = LOWORD(pici->lpVerb);
                    if (utils::io::file::exists(iTextSharpWrapperDll)) {
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
                            json[IMAGE_FILES] = imageFiles;
                            json[PDF_FILE_PATH] = utils::strings::t2utf8(pdfFilePath);
                            switch (wCommand) {
                            case IDM_COMMAND_PDF_A3: json[PAPER_SIZE] = "A3"; break;
                            case IDM_COMMAND_PDF_A4: json[PAPER_SIZE] = "A4"; break;
                            case IDM_COMMAND_PDF_A5: json[PAPER_SIZE] = "A5"; break;
                            case IDM_COMMAND_PDF_B5: json[PAPER_SIZE] = "B5"; break;
                            }
                            utils::dotnet::clr::invoke(
                                iTextSharpWrapperDll,
                                ITEXTSHARP_WRAPPER_CLASS,
                                ITEXTSHARP_WRAPPER_METHOD,
                                utils::strings::t2t(json.toStyledString()));
                        }
                        hr = S_OK;
                    }
                }
                return hr;
            };
            utils::shell32::OnQueryContextMenu = [](HMENU hMenu,
                UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags) {
                auto hr = HRESULT(E_FAIL);
                if (CMF_DEFAULTONLY & uFlags) {
                    hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
                } else {
                    MENUITEMINFOW separator = { sizeof(separator) };
                    separator.fMask = MIIM_TYPE;
                    separator.fType = MFT_SEPARATOR;

                    MENUITEMINFOW mii = { sizeof(mii) };
                    mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
                    mii.wID = idCmdFirst + IDM_COMMAND_PDF;
                    mii.fType = MFT_STRING;
                    mii.dwTypeData = const_cast<LPWSTR>(MENU_ITEM_PDF);
                    mii.fState = MFS_ENABLED;

                    // https://stackoverflow.com/questions/29997600/adding-submenus-to-explorer-context-menu
                    // https://cpp.hotexamples.com/examples/-/-/InsertMenuItem/cpp-insertmenuitem-function-examples.html
                    MENUITEMINFOW miiSpecial = { sizeof(miiSpecial) };
                    miiSpecial.fMask = MIIM_SUBMENU | MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
                    miiSpecial.fType = MFT_STRING;
                    miiSpecial.wID = idCmdFirst + IDM_COMMAND_PDF_SPECIAL;
                    miiSpecial.dwTypeData = const_cast<LPWSTR>(MENU_ITEM_PDF_SPECIAL);
                    miiSpecial.fState = MFS_ENABLED;
                    miiSpecial.hSubMenu = CreatePopupMenu();

                    auto hIcon = LoadIcon(reinterpret_cast<HINSTANCE>(hInstance), MAKEINTRESOURCE(IDI_UNWITTING_LIFE));
                    if (hIcon) {
                        auto bitmap = utils::gdi32::CreateBitmap(hIcon, 24, 24);
                        mii.hbmpItem = bitmap;
                        miiSpecial.hbmpItem = bitmap;
                        utils::gdi32::DeleteObject(hIcon);
                    }
                    if (InsertMenuItem(hMenu, indexMenu++, TRUE, &separator) &&
                        InsertMenuItem(hMenu, indexMenu++, TRUE, &miiSpecial) &&
                        InsertMenuItem(hMenu, indexMenu++, TRUE, &mii) &&
                        InsertMenuItem(hMenu, indexMenu++, TRUE, &separator) &&
                        AppendMenuItem(miiSpecial.hSubMenu, PAPER_SIZE_A3, idCmdFirst + IDM_COMMAND_PDF_A3) &&
                        AppendMenuItem(miiSpecial.hSubMenu, PAPER_SIZE_A4, idCmdFirst + IDM_COMMAND_PDF_A4) &&
                        AppendMenuItem(miiSpecial.hSubMenu, PAPER_SIZE_A5, idCmdFirst + IDM_COMMAND_PDF_A5) &&
                        AppendMenuItem(miiSpecial.hSubMenu, PAPER_SIZE_B5, idCmdFirst + IDM_COMMAND_PDF_B5)) {
                        hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_COMMAND + IDM_COMMAND_LAST + 1));
                    } else {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                }
                return hr;
            };
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

bool isConvertibleImageFileName(string_t fileName) {
    auto extensionName = utils::io::path::GetFileExtensionName(fileName);
    return (utils::strings::equalsIgnoreCase(extensionName, _T(".webp")) ||
        utils::strings::equalsIgnoreCase(extensionName, _T(".jpeg")) ||
        utils::strings::equalsIgnoreCase(extensionName, _T(".bmp")) ||
        utils::strings::equalsIgnoreCase(extensionName, _T(".tiff")) ||
        utils::strings::equalsIgnoreCase(extensionName, _T(".png")));
}

bool AppendMenuItem(HMENU hMenu, LPCWSTR text, WORD wCommand) {
    MENUITEMINFOW mi = { sizeof(mi) };
    mi.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
    mi.wID = wCommand;
    mi.fType = MFT_STRING;
    mi.dwTypeData = const_cast<LPWSTR>(text);
    mi.fState = MFS_ENABLED;
    return InsertMenuItem(hMenu, 0xFFFF, TRUE, &mi);
}