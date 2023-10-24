---
title: 构建和安装Apache Celix
---

<!--
Licensed to the Apache Software Foundation (ASF) under one or more
contributor license agreements.  See the NOTICE file distributed with
this work for additional information regarding copyright ownership.
The ASF licenses this file to You under the Apache License, Version 2.0
(the "License"); you may not use this file except in compliance with
the License.  You may obtain a copy of the License at
   
    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-->

# 构建和安装 Apache Celix
Apache Celix 旨在支持各种 UNIX 平台。

目前，[持续集成构建服务器]为 Apache Celix 进行构建和测试:

* Ubuntu Focal Fossa (20.04)
  * GCC
  * CLang
* Ubuntu Jammy Jellyfish (22.04)
  * GCC 
  * CLang 
* OSX
  * CLang

### 下载 Apache Celix 源码
您首先需要下载 Apache Celix 源代码。这可以通过克隆 Apache Celix git仓库来完成：

```bash
#克隆仓库
git clone --single-branch --branch master https://github.com/apache/celix.git
```

## 构建和安装
Apache Celix 可以用 [Conan](https://conan.io) 作为包管理器/构建系统进行构建，或者直接使用 [CMake](https://cmake.org)。

### 使用 Conan 构建 Apache Celix
需要在您的系统上安装以下包 （库+头文件）：

* 开发环境
  * build-essentials（gcc/g++ 或 clang/clang++）
  * java 或 zip (用于打包捆绑包)
  * make (3.14 或更高版本)
  * git
  * cmake (3.19 或更高版本)
  * Conan (2 或更高版本)

对于Ubuntu 22.04，使用以下命令：
```bash
sudo apt-get install -yq --no-install-recommends \
    build-essential \
    cmake \
    git \
    default-jdk \
    python3 \
    python3-pip \
    ninja-build
        
#安装Conan
pip3 install -U conan   
```

使用系统的自动检测来生成 conan 的默认配置文件：
```bash
conan profile detect
```

在 Conan 缓存中创建 Apache Celix包 - 并构建依赖项：
```bash
cd <celix_source_dir>
conan create . --build missing -o build_all=True   
# 可选地，使用 CMake->Ninja 进行构建，而不是 CMake->Make。请注意，这包括使用 Ninja 构建依赖项。
conan create . --build missing -o build_all=True  -c tools.cmake.cmaketoolchain:generator=Ninja 
```

注意，使用 Conan 构建时无需额外进行安装，因为 Conan 会在本地缓存中安装好可通过 Conan 使用的Apache Celix 包。

设置 `build_all=True` 选项将构建 Apache Celix 的全部功能。也可以只打开所需功能的构建选项来进行编译裁剪。
例如，仅构建 Apache Celix 框架库，可使用以下命令：
```bash
conan create . --build missing -o build_framework=True
```

以上命令将构建框架库及其所依赖的工具库。仅构建 Apache Celix 工具库，可使用以下命令：
```bash
conan create . --build missing -o build_utils=True
```

查看可用构建选项的完整概述，请使用以下命令：
```bash
conan inspect . | grep build_
```
#### CMake 私有链接的解决方法

当通过 Conan 使用 Celix 时，你可能会碰到一个[问题](https://github.com/apache/celix/issues/642)，即链接器找不到 libzip.so。
这是 Conan 的一个[已知问题](https://github.com/conan-io/conan/issues/7192)。

我们在 Celix 中采用的解决方法是，在 conanfile.py 中添加以下内容:

```python
def generate(self):
    deps = CMakeDeps(self)
    deps.generate()
    tc = CMakeToolchain(self)
    # the following is workaround for https://github.com/conan-io/conan/issues/7192
    if self.settings.os == "Linux":
        tc.cache_variables["CMAKE_EXE_LINKER_FLAGS"] = "-Wl,--unresolved-symbols=ignore-in-shared-libs"
    elif self.settings.os == "Macos":
        tc.cache_variables["CMAKE_EXE_LINKER_FLAGS"] = "-Wl,-undefined -Wl,dynamic_lookup"
    tc.generate()
```

### 使用 CMake 直接构建 Apache Celix
需要在您的系统上安装以下包 （库+头文件）：

* 开发环境
  * build-essentials（gcc/g++ 或 clang/clang++）
  * java 或 zip （用于打包捆绑包）
  * make（3.19或更高版本）
  * git
* Apache Celix依赖
  * libzip
  * uuid
  * zlib
  * curl (仅在 Celix 框架中初始化，可通过 FRAMEWORK_CURLINIT 裁剪)
  * jansson (用于 libdfi 中的序列化)
  * libffi (用于 libdfi)
  * libxml2 (用于远程服务和 bonjour shell)
  * rapidjson (用于 C++ 远程服务发现)
  * libczmq (用于 PubSubAdmin ZMQ)

对于Ubuntu 22.04，请使用以下命令： 
```bash
#### get dependencies
sudo apt-get install -yq --no-install-recommends \
    build-essential \
    cmake \
    git \
    curl \
    uuid-dev \
    libjansson-dev \
    libcurl4-openssl-dev \
    default-jdk \
    libffi-dev \
    libzip-dev \
    libxml2-dev \
    libczmq-dev \
    libcpputest-dev \
    rapidjson-dev
```

对于已安装 brew 的 OSX 系统，使用以下命令：
```bash
brew update && \
brew install lcov libffi libzip czmq rapidjson libxml2 cmake jansson && \
brew link --force libffi
``` 

使用 CMake 和 make 来构建 Apache Celix
```bash
cd celix
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .. 
make -j
```

## 编辑构建选项
通过编辑 Apache Celix 的 CMake 构建选项，用户能够配置安装位置并选择所需构建的捆绑包/库。
要编辑这些选项，请使用 ccmake 或 cmake-gui。 对于 cmake-gui，可能需要额外安装程序包（Fedora：`dnf install cmake-gui`）。

```bash
cd celix/build
ccmake .
# 修改选项，例如，启用 BUILD_REMOTE_SHELL 来构建远程(telnet) shell
# 编辑 CMAKE_INSTALL_PREFIX 配置以设置安装位置
```

对于本指南，我们假定 CMAKE_INSTALL_PREFIX 是 `/usr/local`。

## 安装 Apache Celix

```bash
cd celix/build
make -j
sudo make install
```

## 运行 Apache Celix
如果 Apache Celix 安装成功，可以运行
```bash
celix
```
应给出以下输出：
"错误：无效或不存在的配置文件：'config.properties'。没有此文件或目录。"

关于如何构建您自己的项目和/或运行 Apache Celix 示例的更多信息，请查看 [Celix 简介](../README_CN.md)。

# 独立构建 etcdlib 库
```bash
#bash
git clone git@github.com:apache/celix.git
mkdir celix/build
cd celix/build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ../libs/etcdlib
make -j
sudo make install
```

# 独立建立Celix Promises库
```bash
#bash
git clone git@github.com:apache/celix.git
mkdir celix/build
cd celix/build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ../libs/promises
make -j
sudo make install
```

# 独立构建 Celix Push Streams 库
```bash
#bash
git clone git@github.com:apache/celix.git
mkdir celix/build
cd celix/build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ../libs/pushstreams
make -j
sudo make install
```
