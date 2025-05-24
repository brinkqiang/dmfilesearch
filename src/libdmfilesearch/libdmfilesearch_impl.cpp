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

#include "libdmfilesearch_impl.h"
#include "dmformat.h"
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace fs = std::filesystem;


DmfilesearchImpl::DmfilesearchImpl()
{

}


DmfilesearchImpl::~DmfilesearchImpl()
{

}

void DMAPI DmfilesearchImpl::Release(void) {
    delete this;
}

void DMAPI DmfilesearchImpl::Test(void) {
    std::cout << "DmfilesearchImpl::Test() - Everything风格文件搜索引擎" << std::endl;
}

bool DMAPI DmfilesearchImpl::Init() {
    m_fileIndex.clear();
    m_nameIndex.clear();
    m_includeExtensions.clear();
    m_excludeExtensions.clear();
    m_excludeDirectories.clear();
    m_searchOptions = DMSearchOptions();
    return true;
}

void DMAPI DmfilesearchImpl::BuildIndex(const std::string& rootPath) {
    if (m_indexing.load()) {
        std::cout << "索引构建中，请稍候..." << std::endl;
        return;
    }
    
    m_indexing = true;
    
    std::cout << "开始构建索引: " << rootPath << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    m_fileIndex.clear();
    m_nameIndex.clear();
    
    try {
        BuildIndexRecursive(rootPath);
        BuildNameIndex();
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "索引构建完成! 共索引 " << m_fileIndex.size() 
                  << " 个文件/文件夹，耗时 " << duration.count() << "ms" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "构建索引时出错: " << e.what() << std::endl;
    }
    
    m_indexing = false;
}

void DMAPI DmfilesearchImpl::BuildIndexMultiple(const DMStringList& rootPaths) {
    if (m_indexing.load()) {
        std::cout << "索引构建中，请稍候..." << std::endl;
        return;
    }
    
    m_indexing = true;
    
    std::cout << "开始构建多路径索引..." << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    m_fileIndex.clear();
    m_nameIndex.clear();
    
    try {
        for (const auto& rootPath : rootPaths) {
            std::cout << "索引路径: " << rootPath << std::endl;
            BuildIndexRecursive(rootPath);
        }
        
        BuildNameIndex();
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "多路径索引构建完成! 共索引 " << m_fileIndex.size() 
                  << " 个文件/文件夹，耗时 " << duration.count() << "ms" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "构建索引时出错: " << e.what() << std::endl;
    }
    
    m_indexing = false;
}

void DmfilesearchImpl::BuildIndexRecursive(const std::string& directory) {
    try {
        if (!ShouldIncludeDirectory(directory)) {
            return;
        }
        
        for (const auto& entry : fs::recursive_directory_iterator(
            directory, fs::directory_options::skip_permission_denied)) {
            
            const auto& path = entry.path();
            std::string pathStr = path.string();
            std::string fileName = path.filename().string();
            
            // 跳过隐藏文件（除非设置包含）
            if (!m_searchOptions.includeHidden && fileName[0] == '.') {
                continue;
            }
            
            if (entry.is_directory()) {
                if (!ShouldIncludeDirectory(pathStr)) {
                    continue;
                }
            } else {
                if (!ShouldIncludeFile(pathStr, fileName)) {
                    continue;
                }
            }
            
            DMFileInfo fileInfo;
            fileInfo.fullPath = pathStr;
            fileInfo.fileName = fileName;
            fileInfo.directory = path.parent_path().string();
            fileInfo.isDirectory = entry.is_directory();
            
            if (!fileInfo.isDirectory) {
                fileInfo.fileSize = GetFileSize(pathStr);
            }
            fileInfo.modifyTime = GetFileModifyTime(pathStr);
            
            m_fileIndex.push_back(fileInfo);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "访问目录出错 " << directory << ": " << e.what() << std::endl;
    }
}

void DMAPI DmfilesearchImpl::BuildNameIndex() {
    m_nameIndex.clear();
    for (size_t i = 0; i < m_fileIndex.size(); ++i) {
        const auto& fileInfo = m_fileIndex[i];
        std::string key = m_searchOptions.caseSensitive ? fileInfo.fileName : ToLower(fileInfo.fileName);
        m_nameIndex[key].push_back(static_cast<uint32_t>(i));
    }
}

DMFileList* DMAPI DmfilesearchImpl::Search(const std::string& pattern) {
    return SearchWithOptions(pattern, m_searchOptions);
}

DMFileList* DMAPI DmfilesearchImpl::SearchWithOptions(const std::string& pattern, const DMSearchOptions& options) {
    if (m_fileIndex.empty()) {
        std::cout << "索引为空，请先构建索引" << std::endl;
        return new DMFileList();
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    DMFileList* results = new DMFileList();
    
    try {
        SearchInIndex(pattern, options, *results);
        
        // 限制结果数量
        if (results->size() > options.maxResults) {
            results->resize(options.maxResults);
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        
        std::cout << "搜索完成，找到 " << results->size() 
                  << " 个结果，耗时 " << duration.count() << "μs" << std::endl;
                  
    } catch (const std::exception& e) {
        std::cerr << "搜索时出错: " << e.what() << std::endl;
    }
    
    return results;
}

DMFileList* DMAPI DmfilesearchImpl::QuickSearch(const std::string& rootPath, const std::string& pattern) {
    std::cout << "快速搜索模式: " << rootPath << " -> \"" << pattern << "\"" << std::endl;
    
    DMFileList* results = new DMFileList();
    DMSearchOptions options = m_searchOptions;
    
    try {
        for (const auto& entry : fs::recursive_directory_iterator(
            rootPath, fs::directory_options::skip_permission_denied)) {
            
            const auto& path = entry.path();
            std::string pathStr = path.string();
            std::string fileName = path.filename().string();
            
            // 应用过滤规则
            if (entry.is_directory()) {
                if (!ShouldIncludeDirectory(pathStr)) continue;
                if (options.filesOnly) continue;
            } else {
                if (!ShouldIncludeFile(pathStr, fileName)) continue;
                if (options.dirsOnly) continue;
            }
            
            // 匹配模式
            std::string searchText = options.searchInPath ? pathStr : fileName;
            if (MatchPattern(searchText, pattern, options)) {
                DMFileInfo fileInfo;
                fileInfo.fullPath = pathStr;
                fileInfo.fileName = fileName;
                fileInfo.directory = path.parent_path().string();
                fileInfo.isDirectory = entry.is_directory();
                fileInfo.fileSize = entry.is_directory() ? 0 : GetFileSize(pathStr);
                fileInfo.modifyTime = GetFileModifyTime(pathStr);
                
                results->push_back(fileInfo);
                
                if (results->size() >= options.maxResults) {
                    break;
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "快速搜索出错: " << e.what() << std::endl;
    }
    
    std::cout << "快速搜索完成，找到 " << results->size() << " 个结果" << std::endl;
    return results;
}

void DmfilesearchImpl::SearchInIndex(const std::string& pattern, const DMSearchOptions& options, DMFileList& results) const {
    if (options.useRegex) {
        SearchWithRegex(pattern, options, results);
    } else {
        SearchWithWildcard(pattern, options, results);
    }
}

void DmfilesearchImpl::SearchWithWildcard(const std::string& pattern, const DMSearchOptions& options, DMFileList& results) const {
    for (const auto& fileInfo : m_fileIndex) {
        // 应用文件类型过滤
        if (options.dirsOnly && !fileInfo.isDirectory) continue;
        if (options.filesOnly && fileInfo.isDirectory) continue;
        
        std::string searchText = options.searchInPath ? fileInfo.fullPath : fileInfo.fileName;
        
        if (MatchPattern(searchText, pattern, options)) {
            results.push_back(fileInfo);
        }
    }
}

void DmfilesearchImpl::SearchWithRegex(const std::string& pattern, const DMSearchOptions& options, DMFileList& results) const {
    try {
        std::regex_constants::syntax_option_type regexFlags = std::regex_constants::ECMAScript;
        if (!options.caseSensitive) {
            regexFlags |= std::regex_constants::icase;
        }
        
        std::regex regexPattern(pattern, regexFlags);
        
        for (const auto& fileInfo : m_fileIndex) {
            if (options.dirsOnly && !fileInfo.isDirectory) continue;
            if (options.filesOnly && fileInfo.isDirectory) continue;
            
            std::string searchText = options.searchInPath ? fileInfo.fullPath : fileInfo.fileName;
            
            if (std::regex_search(searchText, regexPattern)) {
                results.push_back(fileInfo);
            }
        }
    } catch (const std::regex_error& e) {
        std::cerr << "正则表达式错误: " << e.what() << std::endl;
    }
}

bool DMAPI DmfilesearchImpl::MatchPattern(const std::string& text, const std::string& pattern, const DMSearchOptions& options) const {
    std::string searchText = options.caseSensitive ? text : ToLower(text);
    std::string searchPattern = options.caseSensitive ? pattern : ToLower(pattern);
    
    if (options.wholeWord) {
        return searchText == searchPattern;
    }
    
    // 简单通配符支持
    if (searchPattern.find('*') != std::string::npos || searchPattern.find('?') != std::string::npos) {
        // 转换为正则表达式
        std::string regexPattern = searchPattern;
        std::replace(regexPattern.begin(), regexPattern.end(), '*', '.');
        regexPattern += "*";
        std::replace(regexPattern.begin(), regexPattern.end(), '?', '.');
        
        try {
            std::regex regex(regexPattern, options.caseSensitive ? 
                std::regex_constants::ECMAScript : 
                std::regex_constants::ECMAScript | std::regex_constants::icase);
            return std::regex_search(searchText, regex);
        } catch (const std::regex_error&) {
            // 如果正则表达式无效，回退到简单匹配
            return searchText.find(searchPattern) != std::string::npos;
        }
    }
    
    return searchText.find(searchPattern) != std::string::npos;
}

void DMAPI DmfilesearchImpl::ClearIndex() {
    m_fileIndex.clear();
    m_nameIndex.clear();
    std::cout << "索引已清空" << std::endl;
}

uint32_t DMAPI DmfilesearchImpl::GetIndexedFileCount() {
    return static_cast<uint32_t>(m_fileIndex.size());
}

bool DMAPI DmfilesearchImpl::SaveIndex(const std::string& indexFile) {
    try {
        std::ofstream ofs(indexFile, std::ios::binary);
        if (!ofs) return false;
        
        // 写入文件数量
        uint32_t count = static_cast<uint32_t>(m_fileIndex.size());
        ofs.write(reinterpret_cast<const char*>(&count), sizeof(count));
        
        // 写入文件信息
        for (const auto& fileInfo : m_fileIndex) {
            uint32_t pathLen = static_cast<uint32_t>(fileInfo.fullPath.length());
            uint32_t nameLen = static_cast<uint32_t>(fileInfo.fileName.length());
            uint32_t dirLen = static_cast<uint32_t>(fileInfo.directory.length());
            
            ofs.write(reinterpret_cast<const char*>(&pathLen), sizeof(pathLen));
            ofs.write(fileInfo.fullPath.c_str(), pathLen);
            
            ofs.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
            ofs.write(fileInfo.fileName.c_str(), nameLen);
            
            ofs.write(reinterpret_cast<const char*>(&dirLen), sizeof(dirLen));
            ofs.write(fileInfo.directory.c_str(), dirLen);
            
            ofs.write(reinterpret_cast<const char*>(&fileInfo.fileSize), sizeof(fileInfo.fileSize));
            ofs.write(reinterpret_cast<const char*>(&fileInfo.modifyTime), sizeof(fileInfo.modifyTime));
            ofs.write(reinterpret_cast<const char*>(&fileInfo.isDirectory), sizeof(fileInfo.isDirectory));
        }
        
        std::cout << "索引已保存到: " << indexFile << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "保存索引失败: " << e.what() << std::endl;
        return false;
    }
}

bool DMAPI DmfilesearchImpl::LoadIndex(const std::string& indexFile) {
    try {
        std::ifstream ifs(indexFile, std::ios::binary);
        if (!ifs) return false;
        
        m_fileIndex.clear();
        
        // 读取文件数量
        uint32_t count;
        ifs.read(reinterpret_cast<char*>(&count), sizeof(count));
        
        m_fileIndex.reserve(count);
        
        // 读取文件信息
        for (uint32_t i = 0; i < count; ++i) {
            DMFileInfo fileInfo;
            
            uint32_t pathLen, nameLen, dirLen;
            
            ifs.read(reinterpret_cast<char*>(&pathLen), sizeof(pathLen));
            fileInfo.fullPath.resize(pathLen);
            ifs.read(&fileInfo.fullPath[0], pathLen);
            
            ifs.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
            fileInfo.fileName.resize(nameLen);
            ifs.read(&fileInfo.fileName[0], nameLen);
            
            ifs.read(reinterpret_cast<char*>(&dirLen), sizeof(dirLen));
            fileInfo.directory.resize(dirLen);
            ifs.read(&fileInfo.directory[0], dirLen);
            
            ifs.read(reinterpret_cast<char*>(&fileInfo.fileSize), sizeof(fileInfo.fileSize));
            ifs.read(reinterpret_cast<char*>(&fileInfo.modifyTime), sizeof(fileInfo.modifyTime));
            ifs.read(reinterpret_cast<char*>(&fileInfo.isDirectory), sizeof(fileInfo.isDirectory));
            
            m_fileIndex.push_back(fileInfo);
        }
        
        BuildNameIndex();
        
        std::cout << "索引已从文件加载: " << indexFile << " (共" << count << "项)" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "加载索引失败: " << e.what() << std::endl;
        return false;
    }
}

void DMAPI DmfilesearchImpl::AddIncludeExtension(const std::string& extension) {
    m_includeExtensions.insert(ToLower(extension));
}

void DMAPI DmfilesearchImpl::AddExcludeExtension(const std::string& extension) {
    m_excludeExtensions.insert(ToLower(extension));
}

void DMAPI DmfilesearchImpl::AddExcludeDirectory(const std::string& directory) {
    m_excludeDirectories.insert(directory);
}

void DMAPI DmfilesearchImpl::ClearFilters() {
    m_includeExtensions.clear();
    m_excludeExtensions.clear();
    m_excludeDirectories.clear();
}

void DMAPI DmfilesearchImpl::SetSearchOptions(const DMSearchOptions& options) {
    m_searchOptions = options;
}

DMSearchOptions DMAPI DmfilesearchImpl::GetSearchOptions() {
    return m_searchOptions;
}

void DMAPI DmfilesearchImpl::PrintResults(const DMFileList& results) {
    if (results.empty()) {
        std::cout << "未找到匹配的文件" << std::endl;
        return;
    }
    
    std::cout << "\n搜索结果 (共 " << results.size() << " 项):" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    for (const auto& fileInfo : results) {
        std::cout << (fileInfo.isDirectory ? "[DIR] " : "[FILE]") << fileInfo.fullPath;
        
        if (!fileInfo.isDirectory) {
            std::cout << " (" << fileInfo.fileSize << " bytes)";
        }
        std::cout << std::endl;
    }
}

void DMAPI DmfilesearchImpl::SortResults(DMFileList& results, const std::string& sortBy) {
    if (sortBy == "name") {
        std::sort(results.begin(), results.end(), 
            [](const DMFileInfo& a, const DMFileInfo& b) {
                return a.fileName < b.fileName;
            });
    } else if (sortBy == "size") {
        std::sort(results.begin(), results.end(), 
            [](const DMFileInfo& a, const DMFileInfo& b) {
                return a.fileSize > b.fileSize;
            });
    } else if (sortBy == "date") {
        std::sort(results.begin(), results.end(), 
            [](const DMFileInfo& a, const DMFileInfo& b) {
                return a.modifyTime > b.modifyTime;
            });
    } else if (sortBy == "path") {
        std::sort(results.begin(), results.end(), 
            [](const DMFileInfo& a, const DMFileInfo& b) {
                return a.fullPath < b.fullPath;
            });
    }
}

// 辅助函数实现
bool DmfilesearchImpl::ShouldIncludeFile(const std::string& filePath, const std::string& fileName) const {
    std::string ext = GetFileExtension(fileName);
    
    // 检查排除扩展名
    if (!m_excludeExtensions.empty() && m_excludeExtensions.count(ext)) {
        return false;
    }
    
    // 检查包含扩展名
    if (!m_includeExtensions.empty() && !m_includeExtensions.count(ext)) {
        return false;
    }
    
    return true;
}

bool DmfilesearchImpl::ShouldIncludeDirectory(const std::string& dirPath) const {
    for (const auto& excludeDir : m_excludeDirectories) {
        if (dirPath.find(excludeDir) != std::string::npos) {
            return false;
        }
    }
    return true;
}

std::string DmfilesearchImpl::GetFileExtension(const std::string& fileName) const {
    size_t pos = fileName.find_last_of('.');
    if (pos == std::string::npos) return "";
    return ToLower(fileName.substr(pos));
}

std::string DmfilesearchImpl::ToLower(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

uint64_t DmfilesearchImpl::GetFileSize(const std::string& filePath) const {
    try {
        return static_cast<uint64_t>(fs::file_size(filePath));
    } catch (const fs::filesystem_error&) {
        return 0;
    }
}

uint64_t DmfilesearchImpl::GetFileModifyTime(const std::string& filePath) const {
    try {
        auto ftime = fs::last_write_time(filePath);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
        return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(
            sctp.time_since_epoch()).count());
    } catch (const fs::filesystem_error&) {
        return 0;
    }
}

extern "C" DMEXPORT_DLL Idmfilesearch* DMAPI dmfilesearchGetModule() {
    return new DmfilesearchImpl();
}