#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include "dmfilesearch.h"
#include "dmfix_win.h"

Idmfilesearch* g_searchEngine = nullptr;

// 命令行参数结构
struct CmdArgs {
    std::vector<std::string> searchTerms;
    std::vector<std::string> rootPaths;
    std::string indexFile;
    std::string sortBy = "name";
    DMSearchOptions options;
    bool showHelp = false;
    bool buildIndex = true;
    bool saveIndex = false;
    bool loadIndex = false;
    bool quickSearch = false;
    bool clearIndex = false;
    bool showVersion = false;
    std::vector<std::string> includeExtensions;
    std::vector<std::string> excludeExtensions;
    std::vector<std::string> excludeDirectories;
};

void ShowVersion() {
    std::cout << "ES - Everything风格文件搜索工具 v1.0" << std::endl;
    std::cout << "基于高性能文件索引的快速搜索引擎" << std::endl;
}

void ShowHelp() {
    std::cout << "ES - Everything风格文件搜索工具\n" << std::endl;
    std::cout << "用法: es [选项] [搜索模式]" << std::endl;
    std::cout << "\n基本搜索:" << std::endl;
    std::cout << "  es pattern              搜索包含pattern的文件" << std::endl;
    std::cout << "  es *.txt                搜索所有txt文件" << std::endl;
    std::cout << "  es \"hello world\"        搜索包含hello world的文件" << std::endl;
    
    std::cout << "\n索引管理:" << std::endl;
    std::cout << "  -b, --build PATH        构建指定路径的索引" << std::endl;
    std::cout << "  -m, --multiple PATH1,PATH2,... 构建多个路径的索引" << std::endl;
    std::cout << "  --save FILE             保存索引到文件" << std::endl;
    std::cout << "  --load FILE             从文件加载索引" << std::endl;
    std::cout << "  --clear                 清空当前索引" << std::endl;
    
    std::cout << "\n搜索选项:" << std::endl;
    std::cout << "  -c, --case              区分大小写" << std::endl;
    std::cout << "  -w, --whole-word        全词匹配" << std::endl;
    std::cout << "  -r, --regex             使用正则表达式" << std::endl;
    std::cout << "  -p, --path              在完整路径中搜索" << std::endl;
    std::cout << "  -d, --dirs-only         仅搜索目录" << std::endl;
    std::cout << "  -f, --files-only        仅搜索文件" << std::endl;
    std::cout << "  --hidden                包含隐藏文件" << std::endl;
    std::cout << "  --max N                 限制结果数量 (默认1000)" << std::endl;
    
    std::cout << "\n过滤选项:" << std::endl;
    std::cout << "  --ext EXT               仅包含指定扩展名 (如: --ext .txt)" << std::endl;
    std::cout << "  --exclude-ext EXT       排除指定扩展名（多个扩展名用逗号分隔，如：cpp,cc,cxx）" << std::endl;
    std::cout << "  --exclude-dir DIR       排除指定目录" << std::endl;
    
    std::cout << "\n排序选项:" << std::endl;
    std::cout << "  --sort-by name|size|date|path  结果排序方式" << std::endl;
    
    std::cout << "\n快速模式:" << std::endl;
    std::cout << "  -q, --quick PATH PATTERN  不建索引直接搜索" << std::endl;
    
    std::cout << "\n其他选项:" << std::endl;
    std::cout << "  -h, --help              显示此帮助信息" << std::endl;
    std::cout << "  -v, --version           显示版本信息" << std::endl;
    
    std::cout << "\n示例:" << std::endl;
    std::cout << "  es -b /home/user        构建家目录索引" << std::endl;
    std::cout << "  es --save index.dat     保存索引到文件" << std::endl;
    std::cout << "  es --load index.dat     加载索引文件" << std::endl;
    std::cout << "  es -c -w main           区分大小写搜索完整单词main" << std::endl;
    std::cout << "  es -r \".*\\.cpp$\"        用正则表达式搜索cpp文件" << std::endl;
    std::cout << "  es -d config            仅搜索名为config的目录" << std::endl;
    std::cout << "  es --ext .h header      搜索包含header的.h文件" << std::endl;
    std::cout << "  es -q /tmp temp         在/tmp中快速搜索temp" << std::endl;
}

std::vector<std::string> SplitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

bool ParseArguments(int argc, char* argv[], CmdArgs& args) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            args.showHelp = true;
        }
        else if (arg == "-v" || arg == "--version") {
            args.showVersion = true;
        }
        else if (arg == "-b" || arg == "--build") {
            if (i + 1 < argc) {
                args.rootPaths.push_back(argv[++i]);
                args.buildIndex = true;
            } else {
                std::cerr << "错误: " << arg << " 需要路径参数" << std::endl;
                return false;
            }
        }
        else if (arg == "-m" || arg == "--multiple") {
            if (i + 1 < argc) {
                args.rootPaths = SplitString(argv[++i], ',');
                args.buildIndex = true;
            } else {
                std::cerr << "错误: " << arg << " 需要路径列表参数" << std::endl;
                return false;
            }
        }
        else if (arg == "--save") {
            if (i + 1 < argc) {
                args.indexFile = argv[++i];
                args.saveIndex = true;
            } else {
                std::cerr << "错误: --save 需要文件名参数" << std::endl;
                return false;
            }
        }
        else if (arg == "--load") {
            if (i + 1 < argc) {
                args.indexFile = argv[++i];
                args.loadIndex = true;
            } else {
                std::cerr << "错误: --load 需要文件名参数" << std::endl;
                return false;
            }
        }
        else if (arg == "--clear") {
            args.clearIndex = true;
        }
        else if (arg == "-c" || arg == "--case") {
            args.options.caseSensitive = true;
        }
        else if (arg == "-w" || arg == "--whole-word") {
            args.options.wholeWord = true;
        }
        else if (arg == "-r" || arg == "--regex") {
            args.options.useRegex = true;
        }
        else if (arg == "-p" || arg == "--path") {
            args.options.searchInPath = true;
        }
        else if (arg == "-d" || arg == "--dirs-only") {
            args.options.dirsOnly = true;
        }
        else if (arg == "-f" || arg == "--files-only") {
            args.options.filesOnly = true;
        }
        else if (arg == "--hidden") {
            args.options.includeHidden = true;
        }
        else if (arg == "--max") {
            if (i + 1 < argc) {
                args.options.maxResults = static_cast<uint32_t>(std::stoi(argv[++i]));
            } else {
                std::cerr << "错误: --max 需要数字参数" << std::endl;
                return false;
            }
        }
        else if (arg == "--ext") {
            if (i + 1 < argc) {
                std::string ext = argv[++i];
                if (!ext.empty() && ext[0] == '.') {
                    ext = ext.substr(1); // 移除点
                }
                args.includeExtensions.push_back(ext);
            } else {
                std::cerr << "错误: --ext 需要扩展名参数" << std::endl;
                return false;
            }
        }
        else if (arg == "--exclude-ext") {
            if (i + 1 < argc) {
                std::string extList = argv[++i];
                std::istringstream iss(extList);
                std::string ext;
                while (std::getline(iss, ext, ',')) {
                    if (!ext.empty()) {
                        if (ext[0] == '.') {
                            ext = ext.substr(1); // 移除点
                        }
                        args.excludeExtensions.push_back(ext);
                    }
                }
            } else {
                std::cerr << "错误: --exclude-ext 需要扩展名参数" << std::endl;
                return false;
            }
        }
        else if (arg == "--exclude-dir") {
            if (i + 1 < argc) {
                args.excludeDirectories.push_back(argv[++i]);
            } else {
                std::cerr << "错误: --exclude-dir 需要目录参数" << std::endl;
                return false;
            }
        }
        else if (arg == "--sort-by") {
            if (i + 1 < argc) {
                args.sortBy = argv[++i];
            } else {
                std::cerr << "错误: --sort-by 需要排序方式参数" << std::endl;
                return false;
            }
        }
        else if (arg == "-q" || arg == "--quick") {
            if (i + 2 < argc) {
                args.rootPaths.push_back(argv[++i]);
                args.searchTerms.push_back(argv[++i]);
                args.quickSearch = true;
            } else {
                std::cerr << "错误: -q/--quick 需要路径和搜索模式参数" << std::endl;
                return false;
            }
        }
        else if (arg[0] != '-') {
            // 搜索模式
            args.searchTerms.push_back(arg);
        }
        else {
            std::cerr << "未知选项: " << arg << std::endl;
            return false;
        }
    }
    
    return true;
}

void InitializeSearchEngine() {
    if (!g_searchEngine) {
        g_searchEngine = dmfilesearchGetModule();
        if (!g_searchEngine->Init()) {
            std::cerr << "搜索引擎初始化失败" << std::endl;
            exit(1);
        }
    }
}

void ExecuteCommands(const CmdArgs& args) {
    InitializeSearchEngine();
    
    // 设置搜索选项
    g_searchEngine->SetSearchOptions(args.options);

    // 清空过滤器
    g_searchEngine->ClearFilters();

    // 添加包含扩展名过滤器
    for (const auto& ext : args.includeExtensions) {
        g_searchEngine->AddIncludeExtension(ext);
    }

    // 添加排除扩展名过滤器
    for (const auto& ext : args.excludeExtensions) {
        g_searchEngine->AddExcludeExtension(ext);
    }

    // 添加排除目录过滤器
    for (const auto& dir : args.excludeDirectories) {
        g_searchEngine->AddExcludeDirectory(dir);
    }
    
    // 清空索引
    if (args.clearIndex) {
        g_searchEngine->ClearIndex();
    }
    
    // 加载索引
    if (args.loadIndex) {
        if (!g_searchEngine->LoadIndex(args.indexFile)) {
            std::cerr << "加载索引失败: " << args.indexFile << std::endl;
            return;
        }
    }
    
    // 构建索引
    if (args.buildIndex) {
        if (args.rootPaths.size() == 1) {
            g_searchEngine->BuildIndex(args.rootPaths[0]);
        } else if (args.rootPaths.size() > 1) {
            g_searchEngine->BuildIndexMultiple(args.rootPaths);
        }
        else
        {
            g_searchEngine->BuildIndex(".");
        }
    }
    
    // 保存索引
    if (args.saveIndex) {
        if (!g_searchEngine->SaveIndex(args.indexFile)) {
            std::cerr << "保存索引失败: " << args.indexFile << std::endl;
        }
    }
    
    // 执行搜索
    if (!args.searchTerms.empty()) {
        for (const auto& searchTerm : args.searchTerms) {
            std::unique_ptr<DMFileList> results;
            
            if (args.quickSearch && !args.rootPaths.empty()) {
                results.reset(g_searchEngine->QuickSearch(args.rootPaths[0], searchTerm));
            } else {
                results.reset(g_searchEngine->SearchWithOptions(searchTerm, args.options));
            }
            
            if (results) {
                // 排序结果
                if (!args.sortBy.empty()) {
                    g_searchEngine->SortResults(*results, args.sortBy);
                }
                
                // 显示结果
                g_searchEngine->PrintResults(*results);
            }
        }
    }
    
    // 显示索引统计
    uint32_t indexedCount = g_searchEngine->GetIndexedFileCount();
    if (indexedCount > 0) {
        std::cout << "\n当前索引包含 " << indexedCount << " 个文件/目录" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    CmdArgs args;
    
    // 解析命令行参数
    if (!ParseArguments(argc, argv, args)) {
        return 1;
    }
    
    // 显示帮助或版本信息
    if (args.showHelp) {
        ShowHelp();
        return 0;
    }
    
    if (args.showVersion) {
        ShowVersion();
        return 0;
    }
    
    // 如果没有任何参数，显示帮助
    if (argc == 1) {
        ShowHelp();
        return 0;
    }
    
    try {
        ExecuteCommands(args);
    } catch (const std::exception& e) {
        std::cerr << "执行错误: " << e.what() << std::endl;
        return 1;
    }
 
    if (g_searchEngine)
    {
        g_searchEngine->Release();
    }

    return 0;
}
