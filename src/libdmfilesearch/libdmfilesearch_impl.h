
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

#ifndef __LIBDMFILESEARCH_IMPL_H_INCLUDE__
#define __LIBDMFILESEARCH_IMPL_H_INCLUDE__
#include "dmfilesearch.h"
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <thread>
#include <atomic>

class DmfilesearchImpl : public Idmfilesearch
{
public:
    virtual ~DmfilesearchImpl() {}
    virtual void DMAPI Release(void) override;
    virtual void DMAPI Test(void) override;
    
    virtual bool DMAPI Initialize() override;
    virtual void DMAPI BuildIndex(const std::string& rootPath) override;
    virtual void DMAPI BuildIndexMultiple(const DMStringList& rootPaths) override;
    
    virtual DMFileList* DMAPI Search(const std::string& pattern) override;
    virtual DMFileList* DMAPI SearchWithOptions(const std::string& pattern, const DMSearchOptions& options) override;
    virtual DMFileList* DMAPI QuickSearch(const std::string& rootPath, const std::string& pattern) override;
    
    virtual void DMAPI ClearIndex() override;
    virtual uint32_t DMAPI GetIndexedFileCount() override;
    virtual bool DMAPI SaveIndex(const std::string& indexFile) override;
    virtual bool DMAPI LoadIndex(const std::string& indexFile) override;
    
    virtual void DMAPI AddIncludeExtension(const std::string& extension) override;
    virtual void DMAPI AddExcludeExtension(const std::string& extension) override;
    virtual void DMAPI AddExcludeDirectory(const std::string& directory) override;
    virtual void DMAPI ClearFilters() override;
    
    virtual void DMAPI SetSearchOptions(const DMSearchOptions& options) override;
    virtual DMSearchOptions DMAPI GetSearchOptions() override;
    
    virtual void DMAPI PrintResults(const DMFileList& results) override;
    virtual void DMAPI SortResults(DMFileList& results, const std::string& sortBy) override;

private:
    // 内部数据结构
    std::vector<DMFileInfo> m_fileIndex;
    std::unordered_map<std::string, std::vector<uint32_t>> m_nameIndex; // 文件名索引
    std::unordered_set<std::string> m_includeExtensions;
    std::unordered_set<std::string> m_excludeExtensions;
    std::unordered_set<std::string> m_excludeDirectories;
    DMSearchOptions m_searchOptions;
    mutable std::mutex m_indexMutex;
    std::atomic<bool> m_indexing{false};
    
    // 内部辅助函数
    void BuildIndexRecursive(const std::string& directory);
    bool ShouldIncludeFile(const std::string& filePath, const std::string& fileName) const;
    bool ShouldIncludeDirectory(const std::string& dirPath) const;
    std::string GetFileExtension(const std::string& fileName) const;
    std::string ToLower(const std::string& str) const;
    bool MatchPattern(const std::string& text, const std::string& pattern, const DMSearchOptions& options) const;
    uint64_t GetFileSize(const std::string& filePath) const;
    uint64_t GetFileModifyTime(const std::string& filePath) const;
    void BuildNameIndex();
    
    // 搜索实现
    void SearchInIndex(const std::string& pattern, const DMSearchOptions& options, DMFileList& results) const;
    void SearchWithWildcard(const std::string& pattern, const DMSearchOptions& options, DMFileList& results) const;
    void SearchWithRegex(const std::string& pattern, const DMSearchOptions& options, DMFileList& results) const;
};

#endif