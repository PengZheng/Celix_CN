---
title: Apache Celix 捆绑包
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

# Apache Celix 捆绑包
一个 Apache Celix 捆绑包包含了一系列的共享库，配置文件以及可选的激活器项，所有这些都打包在 zip 文件中。
可以在 Apache Celix 框架中动态地安装、卸载、启停捆绑包。

## Celix 捆绑包的结构
技术上讲，Apache Celix 捆绑包是一个包含以下内容的 zip 文件：

- META-INF/MANIFEST.MF：必需的捆绑包清单，包含有关捆绑包的信息（名称，激活器库等）
- 捆绑包共享库（so/dylib 文件）：可选，一个捆绑包可能有1个或多个共享库。
  捆绑包清单配置了将加载哪些库（私有库），以及（如果有的话）在激活捆绑包时使用哪个库。
- 捆绑包资源文件：一个捆绑包还可以包含额外的资源文件。
  可能是配置文件，html 文件等。  
  有些捆绑包只有资源文件而没有共享库。
  注意，一个捆绑包可以访问其他捆绑包的资源文件。

如果有 `jar` 命令可用，Celix 的 CMake 命令将使用该命令（而不是 `zip` 命令）来创建捆绑包的
zip 文件，这可以保证 MANIFEST.MF 总是 zip 文件的第一项。

```bash
# 从 cmake 构建文件夹 `cmake-build-debug` 中解压 celix_shell_wui.zip 。
# celix_shell_wui.zip 文件是 Celix Shell Web UI捆绑包，它为 Celix 提供了一个交互式 shell WEB用户接口； 
# 它包含一个清单文件，共享库，和额外的可以被`Celix::http_admin`使用的 WEB 资源。
% unzip cmake-build-debug/bundles/shell/shell_wui/celix_shell_wui.zip -d unpacked_bundle_dir 
% find unpacked_bundle_dir 
unpacked_bundle_dir
unpacked_bundle_dir/resources
unpacked_bundle_dir/resources/index.html
unpacked_bundle_dir/resources/ansi_up.js
unpacked_bundle_dir/resources/script.js
unpacked_bundle_dir/META-INF
unpacked_bundle_dir/META-INF/MANIFEST.MF
unpacked_bundle_dir/libcivetweb_shared.so #or dylib for OSX
unpacked_bundle_dir/libshell_wui.1.so #or dylib for OSX    
```

## 捆绑包生命周期
一个 Apache Celix 捆绑包有其自身的生命周期，包含以下状态：

- 已安装 - 捆绑包已被安装到 Celix 框架中，但尚未解析。对于 Celix 来说，这目前意味着不是所有捆绑包中的库都能被加载或已被加载。
- 已解析 - Bundle 已被安装且其需求已被满足。对于 Celix 来说，这目前意味着所有捆绑包中的库已被加载。
- 启动中 - 启动是一个临时状态，此时捆绑包激活器的 create 和 start 回调函数正在执行。
- 活动 - 捆绑包正在运行。
- 停止中 - 停止是一个临时状态，此时捆绑包激活器的 stop 和 destroy 回调函数正在执行。
- 已卸载 - 捆绑包已从 Celix 框架中移除。

![捆绑包生命周期的状态图](diagrams/bundles_lifecycle.png)

## 捆绑包激活
可以动态地安装和启动捆绑包。当一个捆绑包启动时，它会通过查找捆绑包激活器入口点（使用 `dlsym`）进行激活。入口点签名包括：
- `celix_status_t celix_bundleActivator_create(celix_bundle_context_t *ctx, void **userData)`：
  用于创建捆绑包激活器的调用。
- `celix_status_t celix_bundleActivator_start(void *userData, celix_bundle_context_t *ctx)`：
  用于启动捆绑包的调用。
- `celix_status_t celix_bundleActivator_stop(void *userData, celix_bundle_context_t *ctx)`：
  用于停止捆绑包的调用。
- `celix_status_t celix_bundleActivator_destroy(void *userData, celix_bundle_context_t* ctx)`：
  用来销毁捆绑包激活器（并释放内存）的调用。

在 C 中创建捆绑包激活器的最方便的方式是使用在`celix_bundle_activator.h`中定义的宏 `CELIX_GEN_BUNDLE_ACTIVATOR`。
这个宏需要两个函数（start,stop），这些函数可以被`static`声明，使用类型化的捆绑包激活器结构体而不是`void*`。

对于 C++，必须使用在`celix/BundleActivator.h`中定义的宏`CELIX_GEN_CXX_BUNDLE_ACTIVATOR`来创建捆绑包激活器。
对于 C++，以资源分配即初始化方式来激活捆绑包。
这意味着通过创建一个捆绑包激活器对象来启动一个 C++ 捆绑包，并通过让捆绑包激活器对象离开作用域来停止它。

## 捆绑包和捆绑包上下文

捆绑包可以通过关联的捆绑包执行上下文（简称捆绑包上下文）与 Apache Celix 框架进行交互。
捆绑包上下文提供以下功能/方法：
- 注册和反注册服务。
- 安装、启动、停止或卸载 bundle。
- 跟踪服务的添加和删除。
- 跟踪捆绑包的安装、启动、停止或卸载。
- 跟踪服务追踪器的启停。
- 查找满足给定过滤条件的服务（返回其id）。
- 直接使用服务（无需手动创建服务追踪器）。
- 直接使用捆绑包（无需手动创建捆绑包追踪器）。
- 等待在 Apache Celix 事件线程中处理的事件。
- 获取框架属性值。
- 获取与捆绑包上下文关联的捆绑包对象。

## Hello World 捆绑包示例
Hello World 捆绑包示例是一个简单的示例，当开始/停止Bundle时，它将打印“Hello world”和“Goodbye world”行。

理解示例需要具备C，C++和CMake的知识。

C和C++示例由包含捆绑包激活器的单个源文件和一些用来创建捆绑包和容器的Apache Celix CMake命令组成。

两个容器示例都用到了3个捆绑包： Apache Celix Shell，Apache Celix Shell文本用户界面和Hello World。
Apache Celix Shell提供了一组可交互的shell命令，并且Apache Celix Shell文本用户界面可用于从控制台终端运行这些命令。

当C或C++ Hello World捆绑包示例容器启动后，可以使用以下命令动态启停捆绑包。
```bash
stop 3 #停止Hello World。注意Hello World是第三个包，故其捆绑包id为3。
start 3 #再次启动Hello World。
uninstall 3 #停止并卸载Hello World。
stop 0 #停止Apache Celix框架
```

要查看 Apache Celix shell 中可用的其它命令，请运行 `celix::help` 命令：
```bash
help # 注意也可以用 celix::help 触发（命名空间完整修饰的命令名）。
help celix::start # 获取 celix::start 命令帮助
help celix::lb # 获取 celix::lb 命令帮助
stop 0 #停止 Apache Celix 框架
```

### C 示例
```C
//src/my_bundle_activator.c
#include <stdio.h>
#include <celix_bundle_activator.h>

typedef struct my_bundle_activator_data {
    /* hello world bundle activator 结构体是空的*/
} my_bundle_activator_data_t;

void myBundle_helloWorld(celix_bundle_context_t* ctx) {
    printf("Hello world from bundle with id %li\n", celix_bundleContext_getBundleId(ctx));
}

void myBundle_goodbyeWorld(celix_bundle_context_t* ctx) {
    printf("Goodbye world from bundle with id %li\n", celix_bundleContext_getBundleId(ctx));
}

static celix_status_t myBundle_start(my_bundle_activator_data_t *data CELIX_UNUSED, celix_bundle_context_t *ctx CELIX_UNUSED) {
    myBundle_helloWorld(ctx);
    return CELIX_SUCCESS;
}

static celix_status_t myBundle_stop(my_bundle_activator_data_t *data CELIX_UNUSED, celix_bundle_context_t *ctx CELIX_UNUSED) {
    myBundle_goodbyeWorld(ctx);
    return CELIX_SUCCESS;
}

CELIX_GEN_BUNDLE_ACTIVATOR(my_bundle_activator_data_t, myBundle_start, myBundle_stop)
```

```CMake
#CMakeLists.txt
find_package(Celix REQUIRED)

# 使用 `make all`，`make celix-bundles` 会在以下位置创建此捆绑包：
#  ${CMAKE_CURRENT_BINARY_DIR}/my_bundle.zip.
add_celix_bundle(my_bundle
    VERSION 1.0.0 
    SOURCES src/my_bundle_activator.c
)

#通过`make all`，`make celix-containers`或`make my_container`命令，将在以下位置创建该Apache Celix容器可执行文件：
# ${CMAKE_BINARY_DIR}/deploy/my_container/my_container
add_celix_container(my_container
    C
    BUNDLES
        Celix::shell
        Celix::shell_tui
        my_bundle
)
```

### C++ 示例
```C++
//src/MyBundleActivator.cc
#include <iostream>
#include "celix/BundleActivator.h"

class MyBundleActivator {
public:
    explicit MyBundleActivator(const std::shared_ptr<celix::BundleContext>& ctx) {
        std::cout << "Hello world from bundle with id " << ctx->getBundleId() << std::endl;
    }

    ~MyBundleActivator() noexcept {
        std::cout << "Goodbye world" << std::endl;
    }
};

CELIX_GEN_CXX_BUNDLE_ACTIVATOR(MyBundleActivator)
```

```CMake
#CMakeLists.txt
find_package(Celix REQUIRED)

#使用`make all`，`make celix-bundles`，将会在以下位置创建此捆绑包：
#  ${CMAKE_CURRENT_BINARY_DIR}/MyBundle.zip.
add_celix_bundle(MyBundle
    SOURCES src/MyBundleActivator.cc
)

#使用 `make all`，`make celix-containers` 或者 `make MyContainer` 将会在以下路径生成该 Apache Celix 容器可执行文件:
# ${CMAKE_BINARY_DIR}/deploy/my_container/MyContainer
add_celix_container(MyContainer
    CXX
    BUNDLES
        Celix::ShellCxx
        Celix::shell_tui
        MyBundle
)
```

## 捆绑包之间的交互
根据设计，捆绑包无法直接访问另一个捆绑包的符号。捆绑包之间的交互必须通过Apache Celix服务进行。
这意味着，除非功能是通过Apache Celix服务提供的，否则捆绑包功能是私有的。
在Apache Celix中，通过本地加载捆绑包库（设置了`RTLD_LOCAL`的`dlopen`）来保持符号的私有性。

## 捆绑包符号可见性
由于捆绑包无法直接访问另一个捆绑包的符号，因此捆绑包激活器库的默认符号可见性为隐藏。
要修改这个设定，可以在 `add_celix_bundle` CMake函数调用中提供`DO_NOT_CONFIGURE_SYMBOL_VISIBILITY`选项。

为捆绑包隐藏符号具有几个优点，包括：

- 减小捆绑包库的大小；
- 链接时间和加载时间更短；
- 内存使用降低；
- 增加了优化的可能性。

然而，一个缺点可能是使调试一个捆绑包变得更加困难，特别是在捆绑包不带调试符号的情况下（-g编译器选项）。

需要注意的是，C/C++服务本身的符号不需要导出，也能从另一个捆绑包调用到。
对于C++，只有当所提供的服务基于一个仅包含头文件的C++接口时，才适用这一点；而对于C，由于C服务结构体不会产生任何符号，所以总是如此。

捆绑包激活器符号（创建、开始、停止和销毁）必须被导出，因为它们需要被Apache Celix框架调用。
因此，将`celix_bundle_activator.h`中声明的捆绑包激活器接口标记为导出。

### 禁用捆绑包符号隐藏的例子
```CMake
add_celix_bundle(my_bundle_do_not_hide_symbols
    VERSION 1.0.0
    SOURCES src/my_bundle_activator.c
    DO_NOT_CONFIGURE_SYMBOL_VISIBILITY
)
```

## 安装捆绑包
可以使用 Apache Celix 的 CMake 命令 `install_celix_bundle` 在系统上安装 Apache Celix捆绑包。
捆绑包将作为 zip 文件被安装在软件包（软件包名默认为 CMAKE_PROJECT_NAME）共享目录中
（例如 `/use/share/celix/bundles`）。

也可将捆绑包作为 CMake 导入目标来使用，但这需要更复杂的 CMake 安装设置。

## 安装Apache Celix CMake 目标
`install_celix_targets`可以用来生成带有捆绑包 CMake 导入目标的CMake文件，一般与CMake Config文件配合使用。
这样下游使用CMake的`find_package`机制就能找到捆绑包的CMake目标。

示例：
```CMake
#工程设置
project(ExamplePackage C CXX)
find_package(Celix REQUIRED)

#创建捆绑包
add_celix_bundle(ExampleBundleA ...)
add_celix_bundle(ExampleBundleB ...)

#安装捆绑包zip
install_celix_bundle(ExampleBundleA EXPORT MyExport)
install_celix_bundle(ExampleBundleB EXPORT MyExport)
#安装导出的 Apache Celix CMake 目标
install_celix_targets(MyExport NAMESPACE ExamplePackage:: DESTINATION share/ExamplePackage/cmake FILE CelixTargets)

#安装软件包 CMake configuration
file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/ExamplePackageConfig.cmake CONTENT "
  # relative install dir from lib/CMake/ExamplePackage.
  get_filename_component(EXAMPLE_PACKAGE_REL_INSTALL_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
  get_filename_component(EXAMPLE_PACKAGE_REL_INSTALL_DIR "${EXAMPLE_PACKAGE_REL_INSTALL_DIR}" PATH)
  get_filename_component(EXAMPLE_PACKAGE_REL_INSTALL_DIR "${EXAMPLE_PACKAGE_REL_INSTALL_DIR}" PATH)
  get_filename_component(EXAMPLE_PACKAGE_REL_INSTALL_DIR "${EXAMPLE_PACKAGE_REL_INSTALL_DIR}" PATH)
  include(${EXAMPLE_PACKAGE_REL_INSTALL_DIR}/share/celix/cmake/CelixTargets.cmake)
")

install(FILES
  ${CMAKE_BINARY_DIR}/ExamplePackageConfig.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ExamplePackage)
```

下游使用示例：
```CMake 
project(UsageExample C CXX)
find_package(Celix REQUIRED)
find_package(ExamplePackage REQUIRED)
add_celix_container(test_container BUNDLES
  Celix::shell
  Celix::shell_tui
  ExamplePackage::ExampleBundleA
  ExamplePackage::ExampleBundleB
)
```

参见 [Apache Celix CMake 命令](cmake_commands/README_CN.md) 以获取更详细的信息。

# `celix::lb` shell 命令
要交互式查看已安装的捆绑包，可以使用 `celix::lb` （list bundles）shell 命令。

支持的`lb`命令行用法示例：
- `celix::lb` - 显示已安装捆绑包概览，包括捆绑包id、捆绑包状态、捆绑包名以及捆绑包组。
- `lb` - 同 `celix::lb`（只要没有其他命名冲突的 `lb` 命令）。
- `lb -s` - 同 `celix::lb`，但显示捆绑包符号名而非捆绑包名。
- `lb -u` - 同 `celix::lb`，但显示捆绑包更新位置而非捆绑包名。
