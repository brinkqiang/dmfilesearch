# dmfilesearch

Copyright (c) 2013-2018 brinkqiang (brink.qiang@gmail.com)

[![dmfilesearch](https://img.shields.io/badge/brinkqiang-dmfilesearch-blue.svg?style=flat-square)](https://github.com/brinkqiang/dmfilesearch)
[![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](https://github.com/brinkqiang/dmfilesearch/blob/master/LICENSE)
[![blog](https://img.shields.io/badge/Author-Blog-7AD6FD.svg)](https://brinkqiang.github.io/)
[![Open Source Love](https://badges.frapsoft.com/os/v3/open-source.png)](https://github.com/brinkqiang)
[![GitHub stars](https://img.shields.io/github/stars/brinkqiang/dmfilesearch.svg?label=Stars)](https://github.com/brinkqiang/dmfilesearch) 
[![GitHub forks](https://img.shields.io/github/forks/brinkqiang/dmfilesearch.svg?label=Fork)](https://github.com/brinkqiang/dmfilesearch)

## Build status
| [Linux][lin-link] | [Mac][mac-link] | [Windows][win-link] |
| :---------------: | :----------------: | :-----------------: |
| ![lin-badge]      | ![mac-badge]       | ![win-badge]        |

[lin-badge]: https://github.com/brinkqiang/dmfilesearch/workflows/linux/badge.svg "linux build status"
[lin-link]:  https://github.com/brinkqiang/dmfilesearch/actions/workflows/linux.yml "linux build status"
[mac-badge]: https://github.com/brinkqiang/dmfilesearch/workflows/mac/badge.svg "mac build status"
[mac-link]:  https://github.com/brinkqiang/dmfilesearch/actions/workflows/mac.yml "mac build status"
[win-badge]: https://github.com/brinkqiang/dmfilesearch/workflows/win/badge.svg "win build status"
[win-link]:  https://github.com/brinkqiang/dmfilesearch/actions/workflows/win.yml "win build status"

# ES - Everything风格文件搜索工具

一个高性能的C++文件搜索工具，提供类似Everything的快速文件搜索功能。

## 特性

- **高性能索引**: 基于内存索引的快速文件搜索
- **多种搜索模式**: 支持通配符、正则表达式、全词匹配等
- **灵活过滤**: 支持文件扩展名、目录排除等过滤规则
- **索引持久化**: 支持索引的保存和加载
- **跨平台**: 支持Windows和Linux平台
- **命令行友好**: 丰富的命令行选项

## 编译安装

### 依赖要求

- C++17编译器 (GCC 8+, Clang 7+, MSVC 2017+)
- CMake 3.21+
- 文件系统库支持

### 编译步骤

```bash
# 克隆项目
git clone https://github.com/brinkqiang/dmfilesearch
cd dmfilesearch

# 配置和编译
./build.sh

# 安装（可选）
sudo make install
```

### Windows编译

```cmd
git clone https://github.com/brinkqiang/dmfilesearch
cd dmfilesearch

build.bat

```

## 使用方法

### 基本搜索

```bash
# 搜索包含"test"的文件
./es test

# 搜索所有txt文件
./es *.txt

# 搜索包含空格的模式
./es "hello world"
```

### 索引管理

```bash
# 构建当前目录索引
./es -b .

# 构建指定目录索引
./es -b /home/user/documents

# 构建多个目录索引
./es -m "/home/user,/opt,/usr/local"

# 保存索引到文件
./es --save myindex.dat

# 从文件加载索引
./es --load myindex.dat

# 清空当前索引
./es --clear
```

### 高级搜索选项

```bash
# 区分大小写搜索
./es -c MyFile

# 全词匹配
./es -w main

# 正则表达式搜索
./es -r ".*\\.cpp$"

# 在完整路径中搜索
./es -p "/home.*config"

# 仅搜索目录
./es -d config

# 仅搜索文件
./es -f readme

# 包含隐藏文件
./es --hidden .bashrc

# 限制结果数量
./es --max 50 temp
```

### 过滤选项

```bash
# 仅包含特定扩展名
./es --ext .cpp main

# 排除特定扩展名
./es --exclude-ext .o build

# 排除特定目录
./es --exclude-dir .git source
```

### 排序选项

```bash
# 按名称排序
./es --sort-by name *.txt

# 按大小排序
./es --sort-by size *.log

# 按修改时间排序
./es --sort-by date config

# 按路径排序
./es --sort-by path readme
```

### 快速搜索模式

```bash
# 不建索引直接搜索（适合一次性搜索）
./es -q /tmp temp
./es -q /home/user "*.pdf"
```

## 使用场景示例

### 1. 开发者场景

```bash
# 构建项目索引
./es -b /path/to/project

# 查找所有C++源文件
./es --ext .cpp --ext .h

# 查找包含特定函数的文件
./es -r "void.*function.*\("

# 查找配置文件
./es -r "(config|conf)\.(txt|ini|json|yaml)$"
```

### 2. 系统管理员场景

```bash
# 构建系统目录索引
./es -m "/etc,/usr,/opt"

# 查找大文件
./es --sort-by size --max 20 "*"

# 查找配置文件
./es -d config

# 查找日志文件
./es --ext .log --sort-by date
```

### 3. 日常使用场景

```bash
# 构建用户目录索引
./es -b ~

# 查找文档
./es --ext .pdf --ext .doc document

# 查找图片
./es -r "\.(jpg|png|gif|bmp)$" photo

# 查找最近修改的文件
./es --sort-by date --max 10 "*"
```

## 性能优化建议

1. **索引策略**: 
   - 对于经常搜索的目录，建议构建并保存索引
   - 对于一次性搜索，使用快速搜索模式

2. **过滤优化**: 
   - 使用扩展名过滤减少索引大小
   - 排除不需要的目录（如.git, node_modules等）

3. **搜索技巧**: 
   - 使用具体的搜索模式减少结果数量
   - 合理使用正则表达式，避免过于复杂的模式

## 待实现功能

以下功能在当前版本中尚未完全实现或存在已知问题：

- **命令行过滤选项未生效**:
  - `--ext <EXT>` (仅包含指定扩展名)
  - `--exclude-ext <EXT>` (排除指定扩展名)
  - `--exclude-dir <DIR>` (排除指定目录)
  这些选项在命令行中解析后，目前未能正确应用到搜索逻辑中。

- **配置文件支持**:
  - `~/.es.conf` 配置文件中定义的默认选项（如 `case_sensitive`, `max_results`, `exclude_extensions`, `exclude_directories` 等）目前未被程序读取和应用。

## 配置文件

创建 `~/.es.conf` 配置文件可以设置默认选项：

```ini
[search]
case_sensitive=false
max_results=1000
include_hidden=false

[filters]
exclude_extensions=.tmp,.bak,.swp
exclude_directories=.git,node_modules,.vscode

[index]
auto_save=true
index_file=~/.es_index.dat
```

## 常见问题

### Q: 搜索速度慢怎么办？
A: 
1. 确保已构建索引：`./es -b /path/to/search`
2. 使用更具体的搜索模式
3. 使用扩展名过滤减少搜索范围

### Q: 索引占用内存过大？
A: 
1. 排除不需要的目录：`./es --exclude-dir .git`
2. 排除不需要的文件类型：`./es --exclude-ext .tmp`
3. 分别为不同目录构建独立索引

### Q: 如何搜索包含特殊字符的文件？
A: 使用引号包围搜索模式：`./es "file[1].txt"`

### Q: 正则表达式不工作？
A: 确保使用了`-r`选项：`./es -r "pattern"`

## 许可证

MIT License - 详见LICENSE文件

## 贡献

欢迎提交Issue和Pull Request来改进这个项目。

## 更新日志

### v1.0.0
- 初始版本发布
- 支持基本文件搜索功能
- 支持索引的构建、保存和加载
- 支持多种搜索模式和过滤选项
