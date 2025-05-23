
// Copyright (c) 2018 brinkqiang (brink.qiang@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef __DMFILESEARCH_H_INCLUDE__
#define __DMFILESEARCH_H_INCLUDE__

#include "dmos.h"
#include <string>
#include <vector>
#include <regex>

// 搜索结果结构
struct DMFileInfo {
    std::string fullPath;
    std::string fileName;
    std::string directory;
    uint64_t fileSize;
    uint64_t modifyTime;
    bool isDirectory;
    
    DMFileInfo() : fileSize(0), modifyTime(0), isDirectory(false) {}
};

typedef std::vector<DMFileInfo> DMFileList;
typedef std::vector<std::string> DMStringList;

// 搜索选项
struct DMSearchOptions {
    bool caseSensitive;
    bool wholeWord;
    bool useRegex;
    bool searchInPath;
    bool includeHidden;
    bool dirsOnly;
    bool filesOnly;
    uint32_t maxResults;
    
    DMSearchOptions() : caseSensitive(false), wholeWord(false), useRegex(false), 
                       searchInPath(false), includeHidden(false), dirsOnly(false),
                       filesOnly(false), maxResults(1000) {}
};

class Idmfilesearch
{
public:
    virtual ~Idmfilesearch() {};
    virtual void DMAPI Release(void) = 0;
    virtual void DMAPI Test(void) = 0;
    
    // 初始化搜索引擎
    virtual bool DMAPI Init() = 0;
    
    // 建立文件索引
    virtual void DMAPI BuildIndex(const std::string& rootPath) = 0;
    virtual void DMAPI BuildIndexMultiple(const DMStringList& rootPaths) = 0;
    
    // 搜索功能
    virtual DMFileList* DMAPI Search(const std::string& pattern) = 0;
    virtual DMFileList* DMAPI SearchWithOptions(const std::string& pattern, const DMSearchOptions& options) = 0;
    
    // 实时搜索（边建索引边搜索）
    virtual DMFileList* DMAPI QuickSearch(const std::string& rootPath, const std::string& pattern) = 0;
    
    // 索引管理
    virtual void DMAPI ClearIndex() = 0;
    virtual uint32_t DMAPI GetIndexedFileCount() = 0;
    virtual bool DMAPI SaveIndex(const std::string& indexFile) = 0;
    virtual bool DMAPI LoadIndex(const std::string& indexFile) = 0;
    
    // 过滤器
    virtual void DMAPI AddIncludeExtension(const std::string& extension) = 0;
    virtual void DMAPI AddExcludeExtension(const std::string& extension) = 0;
    virtual void DMAPI AddExcludeDirectory(const std::string& directory) = 0;
    virtual void DMAPI ClearFilters() = 0;
    
    // 搜索选项设置
    virtual void DMAPI SetSearchOptions(const DMSearchOptions& options) = 0;
    virtual DMSearchOptions DMAPI GetSearchOptions() = 0;
    
    // 结果处理
    virtual void DMAPI PrintResults(const DMFileList& results) = 0;
    virtual void DMAPI SortResults(DMFileList& results, const std::string& sortBy) = 0;
};

extern "C" DMEXPORT_DLL Idmfilesearch* DMAPI dmfilesearchGetModule();
typedef Idmfilesearch* (DMAPI* PFN_dmfilesearchGetModule)();

#endif