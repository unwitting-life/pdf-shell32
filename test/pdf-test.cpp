// pdf-test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

#define JSON_SUPPORT
#define SHELL32_SUPPORT
#define DOTNET_SUPPORT
#include "../../cpp-utils/utils.hpp"

int main() {
    setlocale(LC_ALL, "Chinese");
    auto imageDirectory = _T("D:\\.sources\\github.com\\unwitting-life\\manhuagui.com\\manhuagui.com\\01";
    auto imageFiles = Json::Value();
    auto index = 0;
    for (auto& imageFile : utils::io::directory::files(imageDirectory))) {
        auto extensionName = utils::io::path::GetFileExtensionName(imageFile);
        if (utils::strings::equalsIgnoreCase(extensionName, _T(".webp")) ||
            utils::strings::equalsIgnoreCase(extensionName, _T(".jpeg")) ||
            utils::strings::equalsIgnoreCase(extensionName, _T(".png"))) {
            imageFiles[index++] = utils::strings::t2s(imageFile);
        }
    }
    auto pdfFilePath = utils::strings::t2s(utils::io::path::GetFileName(imageDirectory) + _T(".pdf"));
    auto json = Json::Value();
    json["imageFiles"] = imageFiles;
    json["pdfFilePath"] = pdfFilePath;
    auto dump = utils::strings::t2t(json.toStyledString());
    utils::dotnet::clr::invoke(
        _T("D:\\.sources\\github.com\\unwitting-life\\pdf-shell32\\.cs\\bin\\Release\\iTextSharpWrapper.dll"),
        _T("iTextSharpWrapper.impl"),
        _T("invoke"),
        dump);
    std::cout << "Hello World!\\n";
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
