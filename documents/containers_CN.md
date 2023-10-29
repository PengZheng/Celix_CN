---
title: Apache Celix 容器
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

# Apache Celix 容器
Apache Celix 容器是可执行文件，它启动一个带有预置属性集合和预置捆绑包集合的 Celix 框架。
虽然也可以在代码中创建并启动一个 Apache Celix 框架，但 Apache Celix 容器的优点是可以使用单个 `add_celix_container` CMake 命令完成这个操作。

`add_celix_container` CMake 命令最终使用具有相同目标名称的CMake `add_executable`构建可执行文件。
因此，可以将 Apache Celix 容器视为正常的CMake可执行文件（例如，使用 `target_link_libraries`），并确保 CLion 将容器检测为可执行文件。

有关`add_celix_container` CMake命令支持的完整选项列表，请参见 [Apache Celix CMake 命令文档](cmake_commands/README_CN.md)

## 生成的主源文件
`add_celix_container` CMake指令的主要目的是生成一个主源文件，其构建而成的二进制使用一组预配置的属性和一组预配置的捆绑包启动Apache Celix框架。

例如，以下（空的）Apache Celix容器：
```CMake
add_celix_container(my_empty_container)
```

将创建以下主源文件（注意：为了显示目的进行了重新格式化）：
```C++
//${CMAKE_BINARY_DIR}/celix/gen/containers/my_empty_container/main.cc
#include <celix_launcher.h>
int main(int argc, char *argv[]) {
    const char * config = "\
CELIX_CONTAINER_NAME=my_empty_container\n\
CELIX_BUNDLES_PATH=bundles\n\
";
    celix_properties_t *embeddedProps = celix_properties_loadFromString(config);
    return celixLauncher_launchAndWaitForShutdown(argc, argv, embeddedProps);
}
```

请注意，因为源文件是C++源文件（.cc扩展名），所以可执行文件将使用C++编译器进行编译。

要创建C Apache Celix容器，请在`add_celix_container`CMake命令中使用`C`选项；这将生成一个`main.c`源文件，而不是`main.cc`源文件:

```CMake
add_celix_container(my_empty_container C)
```

当Apache Celix Container配置有框架属性和/或自启动的捆绑包时，生成的主源文件将会嵌入这些框架属性。

例如以下的`add_celix_container` Apache Celix CMake命令：
```CMake
add_celix_container(my_web_shell_container
    BUNDLES
        Celix::http_admin
        Celix::shell
        Celix::shell_wui
    PROPERTIES
        CELIX_LOGGING_DEFAULT_ACTIVE_LOG_LEVEL=debug
        CELIX_HTTP_ADMIN_LISTENING_PORTS=8888
)
```

将创建以下主源文件（注意：为了显示目的进行了重新格式化）：
```C++
#include <celix_launcher.h>
int main(int argc, char *argv[]) {
    const char * config = "\
CELIX_CONTAINER_NAME=my_web_shell_container\n\
CELIX_BUNDLES_PATH=bundles\n\
CELIX_AUTO_START_3=celix_http_admin-Debug.zip celix_shell-Debug.zip celix_shell_wui-Debug.zip\n\
CELIX_LOGGING_DEFAULT_ACTIVE_LOG_LEVEL=debug\n\
CELIX_HTTP_ADMIN_LISTENING_PORTS=8888";

    celix_properties_t *embeddedProps = celix_properties_loadFromString(config);
    return celixLauncher_launchAndWaitForShutdown(argc, argv, embeddedProps);
}
```

## 安装Celix容器
目前，不支持安装Apache Celix容器（即对Celix容器使用`make install`）。

原因是Apache Celix容器依赖于捆绑包的位置，而目前系统中没有可靠的方式来查找捆绑包。为使其工作，Apache Celix应该支持如下内容：
- 一个类似于`LD_LIBRARY_PATH`的捆绑包搜索路径概念
- 支持在可执行文件中嵌入捆绑包，以便Apache Celix容器可以自包含
- 将捆绑包作为共享库（而非zip），以便可以重用正常的共享库概念（安装，`LD_LIBRARY_PATH`等）

也有例外：若Celix容器只用到了已安装的捆绑包，并且它们是以绝对路径（默认）的形式被添加到容器中，那么被安装的Celix容器也能够正常工作。

## 启动 Apache Celix 容器
Apache Celix 容器的启动方式与普通可执行文件相同，且支持一些命令行选项。

### 命令行选项
Apache Celix 容器支持以下命令行选项：
- `--help`/`-h`：打印帮助消息，包括支持的命令行选项。
- `--props`/`-p`：显示 Celix 容器的内嵌和运行时属性，然后退出。
- `--create-bundle-cache`/`-c`：为 Celix 容器创建捆绑包缓存，然后退出。
- `--embedded_bundles`：显示 Celix 容器的内嵌捆绑包，然后退出。
