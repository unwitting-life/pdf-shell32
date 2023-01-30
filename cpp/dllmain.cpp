// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

#define JSON_SUPPORT
#define SHELL32_SUPPORT
#define DOTNET_SUPPORT
#include "../../cpp-utils/utils.hpp"
#include "resource.h"
#include "pagesize.h"
#include <future>

typedef int(*Count)(const wchar_t* str);
typedef const wchar_t* (*Through)(const wchar_t* str);
typedef const wchar_t* (*GetString)();

constexpr auto IMAGE_FILES = "imageFiles";
constexpr auto PDF_FILE_PATH = "pdfFilePath";
constexpr auto PAPER_SIZE = "paperSize";
constexpr auto MENU_ITEM_CONVERT = _T("转换 \"%s\" 为 PDF 格式...");
constexpr auto MENU_ITEM_CONVERT_SPECIAL = _T("转换 \"%s\" 为 PDF 格式 (指定纸张大小)");
constexpr auto MENU_ITEM_CONVERT_MULTI = _T("合并%d个图像到单个 PDF 文件...");
constexpr auto MENU_ITEM_CONVERT_SPECIAL_MULTI = _T("合并%d个图像到单个 PDF 文件 (指定纸张大小)");
constexpr auto MENU_ITEM_CONVERT_SPECIAL_OTHERS = _T("其他");
constexpr auto IDM_COMMAND = 0;
constexpr auto IDM_COMMAND_PDF = IDM_COMMAND;
constexpr auto IDM_COMMAND_PDF_SPECIAL = IDM_COMMAND + 1;
constexpr auto IDM_COMMAND_11X17 = IDM_COMMAND + 2;
constexpr auto IDM_COMMAND_A0 = IDM_COMMAND + 3;
constexpr auto IDM_COMMAND_A1 = IDM_COMMAND + 4;
constexpr auto IDM_COMMAND_A2 = IDM_COMMAND + 5;
constexpr auto IDM_COMMAND_A3 = IDM_COMMAND + 6;
constexpr auto IDM_COMMAND_A4 = IDM_COMMAND + 7;
constexpr auto IDM_COMMAND_A5 = IDM_COMMAND + 8;
constexpr auto IDM_COMMAND_A6 = IDM_COMMAND + 9;
constexpr auto IDM_COMMAND_A7 = IDM_COMMAND + 10;
constexpr auto IDM_COMMAND_A8 = IDM_COMMAND + 11;
constexpr auto IDM_COMMAND_A9 = IDM_COMMAND + 12;
constexpr auto IDM_COMMAND_A10 = IDM_COMMAND + 13;
constexpr auto IDM_COMMAND_ARCHA = IDM_COMMAND + 14;
constexpr auto IDM_COMMAND_ARCHB = IDM_COMMAND + 15;
constexpr auto IDM_COMMAND_ARCHC = IDM_COMMAND + 16;
constexpr auto IDM_COMMAND_ARCHD = IDM_COMMAND + 17;
constexpr auto IDM_COMMAND_ARCHE = IDM_COMMAND + 18;
constexpr auto IDM_COMMAND_B0 = IDM_COMMAND + 19;
constexpr auto IDM_COMMAND_B1 = IDM_COMMAND + 20;
constexpr auto IDM_COMMAND_B2 = IDM_COMMAND + 21;
constexpr auto IDM_COMMAND_B3 = IDM_COMMAND + 22;
constexpr auto IDM_COMMAND_B4 = IDM_COMMAND + 23;
constexpr auto IDM_COMMAND_B5 = IDM_COMMAND + 24;
constexpr auto IDM_COMMAND_B6 = IDM_COMMAND + 25;
constexpr auto IDM_COMMAND_B7 = IDM_COMMAND + 26;
constexpr auto IDM_COMMAND_B8 = IDM_COMMAND + 27;
constexpr auto IDM_COMMAND_B9 = IDM_COMMAND + 28;
constexpr auto IDM_COMMAND_B10 = IDM_COMMAND + 29;
constexpr auto IDM_COMMAND_CROWNOCTAVO = IDM_COMMAND + 30;
constexpr auto IDM_COMMAND_CROWNQUARTO = IDM_COMMAND + 31;
constexpr auto IDM_COMMAND_DEMYOCTAVO = IDM_COMMAND + 32;
constexpr auto IDM_COMMAND_DEMYQUARTO = IDM_COMMAND + 33;
constexpr auto IDM_COMMAND_EXECUTIVE = IDM_COMMAND + 34;
constexpr auto IDM_COMMAND_FLSA = IDM_COMMAND + 35;
constexpr auto IDM_COMMAND_FLSE = IDM_COMMAND + 36;
constexpr auto IDM_COMMAND_HALFLETTER = IDM_COMMAND + 37;
constexpr auto IDM_COMMAND_ID1 = IDM_COMMAND + 38;
constexpr auto IDM_COMMAND_ID2 = IDM_COMMAND + 39;
constexpr auto IDM_COMMAND_ID3 = IDM_COMMAND + 40;
constexpr auto IDM_COMMAND_LARGECROWNOCTAVO = IDM_COMMAND + 41;
constexpr auto IDM_COMMAND_LARGECROWNQUARTO = IDM_COMMAND + 42;
constexpr auto IDM_COMMAND_LEDGER = IDM_COMMAND + 43;
constexpr auto IDM_COMMAND_LEGAL = IDM_COMMAND + 44;
constexpr auto IDM_COMMAND_LETTER = IDM_COMMAND + 45;
constexpr auto IDM_COMMAND_NOTE = IDM_COMMAND + 46;
constexpr auto IDM_COMMAND_PENGUINLARGEPAPERBACK = IDM_COMMAND + 47;
constexpr auto IDM_COMMAND_PENGUINSMALLPAPERBACK = IDM_COMMAND + 48;
constexpr auto IDM_COMMAND_POSTCARD = IDM_COMMAND + 49;
constexpr auto IDM_COMMAND_ROYALOCTAVO = IDM_COMMAND + 50;
constexpr auto IDM_COMMAND_ROYALQUARTO = IDM_COMMAND + 51;
constexpr auto IDM_COMMAND_SMALLPAPERBACK = IDM_COMMAND + 52;
constexpr auto IDM_COMMAND_TABLOID = IDM_COMMAND + 53;
constexpr auto IDM_COMMAND_LAST = IDM_COMMAND_TABLOID;

auto ITEXTSHARP_WRAPPER = _T("iTextSharpWrapper.dll");
auto ITEXTSHARP_WRAPPER_CLASS = _T("iTextSharpWrapper.implement");
auto ITEXTSHARP_WRAPPER_METHOD = _T("invoke");

HMODULE hInstance = nullptr;
TCHAR module[MAX_PATH] = { 0 };
string_t iTextSharpWrapperDll;
std::unordered_map<int, HBITMAP> m_bitmaps;
bool isConvertibleImageFileName(string_t);
bool AppendMenuItem(HMENU, LPCWSTR, WORD, int);
HBITMAP CreateIconBitmap(int, int, int);
HBITMAP CreateIconBitmap(int);
INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    hInstance = hModule;
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            ; GetModuleFileName(hModule, module, ARRAYSIZE(module) - 1);
            iTextSharpWrapperDll = utils::io::path::combine(utils::io::path::GetDirectoryPath(module), ITEXTSHARP_WRAPPER);
            utils::shell32::init(hModule);
            utils::shell32::OnInitialized = [](std::vector<std::wstring>& files) {
                HRESULT hr = E_INVALIDARG;
                std::vector<std::wstring> imageFiles;
                for (auto& file : files) {
                    if (isConvertibleImageFileName(file)) {
                        imageFiles.push_back(file);
                    } else if (utils::io::directory::exists(file)) {
                        for (auto& e : utils::io::directory::GetFiles(file, false)) {
                            if (isConvertibleImageFileName(e)) {
                                imageFiles.push_back(e);
                            }
                        }
                    }
                }
                files = imageFiles;
                return files.empty() ? E_INVALIDARG : S_OK;
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
                                case IDM_COMMAND_11X17: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_11X17); break;
                                case IDM_COMMAND_A0: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_A0); break;
                                case IDM_COMMAND_A1: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_A1); break;
                                case IDM_COMMAND_A2: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_A2); break;
                                case IDM_COMMAND_A3: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_A3); break;
                                case IDM_COMMAND_A4: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_A4); break;
                                case IDM_COMMAND_A5: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_A5); break;
                                case IDM_COMMAND_A6: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_A6); break;
                                case IDM_COMMAND_A7: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_A7); break;
                                case IDM_COMMAND_A8: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_A8); break;
                                case IDM_COMMAND_A9: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_A9); break;
                                case IDM_COMMAND_A10: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_A10); break;
                                case IDM_COMMAND_ARCHA: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_ARCHA); break;
                                case IDM_COMMAND_ARCHB: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_ARCHB); break;
                                case IDM_COMMAND_ARCHC: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_ARCHC); break;
                                case IDM_COMMAND_ARCHD: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_ARCHD); break;
                                case IDM_COMMAND_ARCHE: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_ARCHE); break;
                                case IDM_COMMAND_B0: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_B0); break;
                                case IDM_COMMAND_B1: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_B1); break;
                                case IDM_COMMAND_B2: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_B2); break;
                                case IDM_COMMAND_B3: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_B3); break;
                                case IDM_COMMAND_B4: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_B4); break;
                                case IDM_COMMAND_B5: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_B5); break;
                                case IDM_COMMAND_B6: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_B6); break;
                                case IDM_COMMAND_B7: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_B7); break;
                                case IDM_COMMAND_B8: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_B8); break;
                                case IDM_COMMAND_B9: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_B9); break;
                                case IDM_COMMAND_B10: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_B10); break;
                                case IDM_COMMAND_CROWNOCTAVO: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_CROWNOCTAVO); break;
                                case IDM_COMMAND_CROWNQUARTO: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_CROWNQUARTO); break;
                                case IDM_COMMAND_DEMYOCTAVO: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_DEMYOCTAVO); break;
                                case IDM_COMMAND_DEMYQUARTO: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_DEMYQUARTO); break;
                                case IDM_COMMAND_EXECUTIVE: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_EXECUTIVE); break;
                                case IDM_COMMAND_FLSA: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_FLSA); break;
                                case IDM_COMMAND_FLSE: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_FLSE); break;
                                case IDM_COMMAND_HALFLETTER: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_HALFLETTER); break;
                                case IDM_COMMAND_ID1: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_ID1); break;
                                case IDM_COMMAND_ID2: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_ID2); break;
                                case IDM_COMMAND_ID3: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_ID3); break;
                                case IDM_COMMAND_LARGECROWNOCTAVO: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_LARGECROWNOCTAVO); break;
                                case IDM_COMMAND_LARGECROWNQUARTO: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_LARGECROWNQUARTO); break;
                                case IDM_COMMAND_LEDGER: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_LEDGER); break;
                                case IDM_COMMAND_LEGAL: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_LEGAL); break;
                                case IDM_COMMAND_LETTER: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_LETTER); break;
                                case IDM_COMMAND_NOTE: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_NOTE); break;
                                case IDM_COMMAND_PENGUINLARGEPAPERBACK: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_PENGUINLARGEPAPERBACK); break;
                                case IDM_COMMAND_PENGUINSMALLPAPERBACK: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_PENGUINSMALLPAPERBACK); break;
                                case IDM_COMMAND_POSTCARD: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_POSTCARD); break;
                                case IDM_COMMAND_ROYALOCTAVO: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_ROYALOCTAVO); break;
                                case IDM_COMMAND_ROYALQUARTO: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_ROYALQUARTO); break;
                                case IDM_COMMAND_SMALLPAPERBACK: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_SMALLPAPERBACK); break;
                                case IDM_COMMAND_TABLOID: json[PAPER_SIZE] = utils::strings::t2s(PAPER_SIZE_TABLOID); break;
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
            utils::shell32::OnQueryContextMenu = [](HMENU hMenu, UINT indexMenu, 
                UINT idCmdFirst, UINT idCmdLast, UINT uFlags, std::vector<std::wstring>& files) {
                    auto hr = HRESULT(E_FAIL);
                    if (CMF_DEFAULTONLY & uFlags) {
                        hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
                    } else {
                        auto itemText = files.size() == 1 ?
                            utils::strings::format(MENU_ITEM_CONVERT, utils::io::path::GetFileName(files[0]).c_str()) :
                            utils::strings::format(MENU_ITEM_CONVERT_MULTI, files.size());
                        auto specialItemText = files.size() == 1 ?
                            utils::strings::format(MENU_ITEM_CONVERT_SPECIAL, utils::io::path::GetFileName(files[0]).c_str()) :
                            utils::strings::format(MENU_ITEM_CONVERT_SPECIAL_MULTI, files.size());

                        MENUITEMINFOW separator = { sizeof(separator) };
                        separator.fMask = MIIM_TYPE;
                        separator.fType = MFT_SEPARATOR;

                        MENUITEMINFOW mii = { sizeof(mii) };
                        mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
                        mii.wID = idCmdFirst + IDM_COMMAND_PDF;
                        mii.fType = MFT_STRING;
                        mii.dwTypeData = const_cast<LPWSTR>(itemText.c_str());
                        mii.fState = MFS_ENABLED;
                        mii.hbmpItem = CreateIconBitmap(IDI_UNWITTING_LIFE);

                        // https://stackoverflow.com/questions/29997600/adding-submenus-to-explorer-context-menu
                        // https://cpp.hotexamples.com/examples/-/-/InsertMenuItem/cpp-insertmenuitem-function-examples.html
                        MENUITEMINFOW special = { sizeof(special) };
                        special.fMask = MIIM_SUBMENU | MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
                        special.fType = MFT_STRING;
                        special.wID = idCmdFirst + IDM_COMMAND_PDF_SPECIAL;
                        special.dwTypeData = const_cast<LPWSTR>(specialItemText.c_str());
                        special.fState = MFS_ENABLED;
                        special.hSubMenu = CreatePopupMenu();
                        special.hbmpItem = CreateIconBitmap(IDI_UNWITTING_LIFE);

                        MENUITEMINFOW others = { sizeof(others) };
                        others.fMask = MIIM_SUBMENU | MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
                        others.fType = MFT_STRING;
                        others.wID = idCmdFirst + IDM_COMMAND_PDF_SPECIAL;
                        others.dwTypeData = const_cast<LPWSTR>(MENU_ITEM_CONVERT_SPECIAL_OTHERS);
                        others.fState = MFS_ENABLED;
                        others.hSubMenu = CreatePopupMenu();

                        if (InsertMenuItem(hMenu, indexMenu++, TRUE, &separator) &&
                            InsertMenuItem(hMenu, indexMenu++, TRUE, &special) &&
                            AppendMenuItem(special.hSubMenu, PAPER_SIZE_A3, idCmdFirst + IDM_COMMAND_A3, IDI_PAPER) &&
                            AppendMenuItem(special.hSubMenu, PAPER_SIZE_A4, idCmdFirst + IDM_COMMAND_A4, IDI_PAPER) &&
                            AppendMenuItem(special.hSubMenu, PAPER_SIZE_B5, idCmdFirst + IDM_COMMAND_B5, IDI_PAPER) &&
                            AppendMenuItem(special.hSubMenu, PAPER_SIZE_LETTER, idCmdFirst + IDM_COMMAND_LETTER, IDI_PAPER) &&
                            InsertMenuItem(special.hSubMenu, 0xFFFF, TRUE, &others) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_11X17, idCmdFirst + IDM_COMMAND_11X17, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_A0, idCmdFirst + IDM_COMMAND_A0, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_A1, idCmdFirst + IDM_COMMAND_A1, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_A2, idCmdFirst + IDM_COMMAND_A2, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_A3, idCmdFirst + IDM_COMMAND_A3, IDI_PAPER) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_A4, idCmdFirst + IDM_COMMAND_A4, IDI_PAPER) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_A5, idCmdFirst + IDM_COMMAND_A5, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_A6, idCmdFirst + IDM_COMMAND_A6, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_A7, idCmdFirst + IDM_COMMAND_A7, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_A8, idCmdFirst + IDM_COMMAND_A8, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_A9, idCmdFirst + IDM_COMMAND_A9, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_A10, idCmdFirst + IDM_COMMAND_A10, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_ARCHA, idCmdFirst + IDM_COMMAND_ARCHA, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_ARCHB, idCmdFirst + IDM_COMMAND_ARCHB, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_ARCHC, idCmdFirst + IDM_COMMAND_ARCHC, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_ARCHD, idCmdFirst + IDM_COMMAND_ARCHD, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_ARCHE, idCmdFirst + IDM_COMMAND_ARCHE, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_B0, idCmdFirst + IDM_COMMAND_B0, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_B1, idCmdFirst + IDM_COMMAND_B1, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_B2, idCmdFirst + IDM_COMMAND_B2, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_B3, idCmdFirst + IDM_COMMAND_B3, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_B4, idCmdFirst + IDM_COMMAND_B4, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_B5, idCmdFirst + IDM_COMMAND_B5, IDI_PAPER) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_B6, idCmdFirst + IDM_COMMAND_B6, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_B7, idCmdFirst + IDM_COMMAND_B7, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_B8, idCmdFirst + IDM_COMMAND_B8, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_B9, idCmdFirst + IDM_COMMAND_B9, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_B10, idCmdFirst + IDM_COMMAND_B10, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_CROWNOCTAVO, idCmdFirst + IDM_COMMAND_CROWNOCTAVO, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_CROWNQUARTO, idCmdFirst + IDM_COMMAND_CROWNQUARTO, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_DEMYOCTAVO, idCmdFirst + IDM_COMMAND_DEMYOCTAVO, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_DEMYQUARTO, idCmdFirst + IDM_COMMAND_DEMYQUARTO, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_EXECUTIVE, idCmdFirst + IDM_COMMAND_EXECUTIVE, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_FLSA, idCmdFirst + IDM_COMMAND_FLSA, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_FLSE, idCmdFirst + IDM_COMMAND_FLSE, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_HALFLETTER, idCmdFirst + IDM_COMMAND_HALFLETTER, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_ID1, idCmdFirst + IDM_COMMAND_ID1, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_ID2, idCmdFirst + IDM_COMMAND_ID2, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_ID3, idCmdFirst + IDM_COMMAND_ID3, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_LARGECROWNOCTAVO, idCmdFirst + IDM_COMMAND_LARGECROWNOCTAVO, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_LARGECROWNQUARTO, idCmdFirst + IDM_COMMAND_LARGECROWNQUARTO, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_LEDGER, idCmdFirst + IDM_COMMAND_LEDGER, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_LEGAL, idCmdFirst + IDM_COMMAND_LEGAL, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_LETTER, idCmdFirst + IDM_COMMAND_LETTER, IDI_PAPER) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_NOTE, idCmdFirst + IDM_COMMAND_NOTE, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_PENGUINLARGEPAPERBACK, idCmdFirst + IDM_COMMAND_PENGUINLARGEPAPERBACK, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_PENGUINSMALLPAPERBACK, idCmdFirst + IDM_COMMAND_PENGUINSMALLPAPERBACK, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_POSTCARD, idCmdFirst + IDM_COMMAND_POSTCARD, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_ROYALOCTAVO, idCmdFirst + IDM_COMMAND_ROYALOCTAVO, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_ROYALQUARTO, idCmdFirst + IDM_COMMAND_ROYALQUARTO, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_SMALLPAPERBACK, idCmdFirst + IDM_COMMAND_SMALLPAPERBACK, NULL) &&
                            AppendMenuItem(others.hSubMenu, PAPER_SIZE_TABLOID, idCmdFirst + IDM_COMMAND_TABLOID, NULL) &&
                            InsertMenuItem(hMenu, indexMenu++, TRUE, &mii) &&
                            InsertMenuItem(hMenu, indexMenu++, TRUE, &separator)) {
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

bool AppendMenuItem(HMENU hMenu, LPCWSTR text, WORD wCommand, int iconId) {
    MENUITEMINFOW mi = { sizeof(mi) };
    mi.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
    mi.wID = wCommand;
    mi.fType = MFT_STRING;
    mi.dwTypeData = const_cast<LPWSTR>(text);
    mi.fState = MFS_ENABLED;
    mi.hbmpItem = CreateIconBitmap(iconId, 16, 16);
    return InsertMenuItem(hMenu, 0xFFFF, TRUE, &mi);
}

HBITMAP CreateIconBitmap(int iconId, int width, int height) {
    auto bitmap = HBITMAP(nullptr);
    if (m_bitmaps.contains(iconId)) {
        bitmap = m_bitmaps[iconId];
    } else {
        bitmap = utils::gdi32::CreateIconBitmap(iconId, width, height);
        m_bitmaps[iconId] = bitmap;
    }
    return bitmap;
}

HBITMAP CreateIconBitmap(int iconId) {
    return CreateIconBitmap(iconId, 24, 24);
}
