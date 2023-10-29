---
title: Apache Celix 框架
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

# Apache Celix 框架

Apache Celix 框架是 Apache Celix 应用的核心，支持部署被称为捆绑包的动态可扩展模块。

通过 Celix 框架工厂或通过使用 `add_celix_container` CMake命令配置 [Apache Celix 容器](containers_CN.md)，可以创建框架实例。

## 框架工厂

可以使用C/C++函数/方法创建Apache Celix框架实例：

- `celix_frameworkFactory_createFramework`
- `celix::createFramework`

当一个Apache Celix框架实例被销毁时，它会自动停止和卸载所有正在运行的捆绑包。
对于C，需要使用`celix_frameworkFactory_destroyFramework`调用来销毁框架实例；对于C++，在超出范围时框架实例自动销毁。

### 示例：使用C创建一个Apache Celix框架

```C
//src/main.c
#include
<celix_bundle_activator.h>
int main() {
// 创建框架属性
celix_properties_t* properties = properties_create();
properties_set(properties, "CELIX_LOGGING_DEFAULT_ACTIVE_LOG_LEVEL", "debug");

// 创建框架
celix_framework_t* fw = celix_frameworkFactory_createFramework(properties);

// 获取框架捆绑包上下文并打印日志
celix_bundle_context_t* fwContext = celix_framework_getFrameworkContext(fw);
celix_bundleContext_log(fwContext, CELIX_LOG_LEVEL_INFO, "Hello from framework bundle context");

// 销毁框架
celix_frameworkFactory_destroyFramework(fw);
}
```

```cmake
#CMakeLists.txt
find_package(Celix REQUIRED)
add_executable(create_framework_in_c src/main.c)
target_link_libraries(create_framework_in_c PRIVATE Celix::framework)
```

### 示例：使用C++创建一个Apache Celix框架

```C++
//src/main.cc
#include
<celix/FrameworkFactory.h>
int main() {
// 创建框架属性
celix::Properties properties{
};
properties.set("CELIX_LOGGING_DEFAULT_ACTIVE_LOG_LEVEL", "debug");

// 创建框架
std::shared_ptr<celix::Framework> fw = celix::createFramework(properties);

// 获取框架捆绑包上下文并打印日志
std::shared_ptr<celix::BundleContext> ctx = fw->getFrameworkBundleContext();
ctx->logInfo("Hello from framework bundle context");
}
```

```cmake
#CMakeLists.txt
find_package(Celix REQUIRED)
add_executable(create_framework_in_cxx src/main.cc)
target_link_libraries(create_framework_in_cxx PRIVATE Celix::framework)
```

## Apache Celix 启动器

如果 Apache Celix 框架是主应用程序，Apache Celix 启动器可以用于创建该框架并等待框架关闭。

Apache Celix 启动器还做了一些额外的工作：

- 处理命令参数（主要是为了打印框架的嵌入属性和运行时属性）。
- 尝试从当前工作目录读取"config.properties"文件，并将其与可选的提供给 Apache Celix 启动器的嵌入属性进行组合。
- 接收到`SIGINT`和`SIGTERM`信号时关闭框架。
- 忽略`SIGUSR1`和`SIGUSR2`信号。
- 若配置了`framework_curlinit`Conan选项或`FRAMEWORK_CURLINIT`CMake选项，则调用 `curl_global_init` 来初始化可能使用的libcurl。
  注意 `curl_global_init` 不是线程安全的，也没有受到类似 pthread_once 等的保护。
- 关闭并销毁 Apache Celix 框架。

### 示例：使用Apache Celix启动器创建Apache Celix框架

```C
//src/launcher.c
#include
<celix_bundle_activator.h>
int main(int argc, char** argv) {
return celixLauncher_launchAndWaitForShutdown(argc, argv, NULL);
}
```

```cmake
#CMakeLists.txt
find_package(Celix REQUIRED)
add_executable(create_framework_with_celix_launcher src/launcher.c)
target_link_libraries(create_framework_with_celix_launcher PRIVATE Celix::framework)
```

## 在Apache Celix框架中安装并启动捆绑包

可以使用Apache Celix框架的捆绑包上下文（捆绑包ID为0）或启动器按照"config.properties"文件中的配置属性来安装并启动捆绑包。

可以直接使用捆绑包zip文件的路径来安装捆绑包。如果路径是相对路径，那么框架将在框架属性`CELIX_BUNDLES_PATH`配置的目录中搜索捆绑包。
`CELIX_BUNDLES_PATH`的默认值为 "bundles"。

也可以使用框架属性来配置需要安装并启动的捆绑包。
这可以通过配置从CELIX_AUTO_START_0到CELIX_AUTO_START_6的框架属性来实现。
注意在CELIX_AUTO_START_0中配置的捆绑包会首先被安装并启动。
关于框架属性的更完整概述，请参见`celix_constants.h`

若使用Apache Celix启动器，则可以利用Java属性文件格式的"config.properties"文件来提供框架属性。

用来配置捆绑包安装和启动的框架属性：

- CELIX_AUTO_START_0
- CELIX_AUTO_START_1
- CELIX_AUTO_START_2
- CELIX_AUTO_START_3
- CELIX_AUTO_START_4
- CELIX_AUTO_START_5
- CELIX_AUTO_START_6

### 示例：在C语言中安装并启动捆绑包

```C
//src/main.c
#include
<celix_bundle_activator.h>
int main() {
// 创建框架属性
celix_properties_t* properties = properties_create();
properties_set(properties, "CELIX_BUNDLES_PATH", "bundles;/opt/alternative/bundles");

// 创建框架
celix_framework_t* fw = celix_frameworkFactory_createFramework(NULL);

// 获取框架捆绑包上下文并安装捆绑包
celix_bundle_context_t* fwContext = celix_framework_getFrameworkContext(fw);
celix_bundleContext_installBundle(fwContext, "FooBundle.zip", true);

// 销毁框架
celix_frameworkFactory_destroyFramework(fw);
}
```

### 示例：在C++中安装并启动捆绑包

```C++
//src/main.cc
#include
<celix/FrameworkFactory.h>
int main() {
// 创建框架属性
celix::Properties properties{
};
properties.set("CELIX_BUNDLES_PATH", "bundles;/opt/alternative/bundles");

// 创建框架
std::shared_ptr<celix::Framework> fw = celix::createFramework(properties);

// 获取框架捆绑包上下文并安装捆绑包
std::shared_ptr<celix::BundleContext> ctx = fw->getFrameworkBundleContext();
ctx->installBundle("FooBundle.zip");
}
```

### 示例：使用Apache Celix启动器安装并启动捆绑包

```C
//src/launcher.c
#include
<celix_bundle_activator.h>
int main(int argc, char** argv) {
return celixLauncher_launchAndWaitForShutdown(argc, argv, NULL);
}
```

```cmake
#CMakeLists.txt
find_package(Celix REQUIRED)
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/config.properties CONTENT "
CELIX_BUNDLES_PATH=bundles;/opt/alternative/bundles
CELIX_AUTO_START_3=FooBundle.zip
")
add_executable(create_framework_with_celix_launcher src/launcher.c)
target_link_libraries(create_framework_with_celix_launcher PRIVATE Celix::framework)
```

## 框架条件服务

在一个动态框架中，例如 Apache Celix，有时很难确定框架或一个动态服务型应用的某些部分何时就绪。
为了解决这个问题，Apache Celix 提供了名为`celix_condition`的条件服务。

一个 `celix_condition` 服务是一个带有 "condition.id" 服务属性的标记接口。此服务可用表明 "condition.id" 所标识的条件已经得到满足。

`celix_condition`
服务是[OSGi 8 Condition Service Specification](https://docs.osgi.org/specification/osgi.core/8.0.0/service.condition.html)
的 Celix 适配版本。

Apache Celix 框架将为以下各状态提供对应的 `celix_condition` 服务：

- Celix条件"true"，这个服务永远可用。
- Celix条件"framework.ready"。这个服务会在框架成功并完全启动时被注册。
  完全启动指的是安装和启动所有配置的捆绑包和服务，并处理完期间产生的所有事件（事件队列为空）。
  注意 "framework.ready" 条件并没有包含在 OSGi 条件规范中。
- Celix condition "framework.error"。
  这个服务会在框架未成功启动时被注册。这可能是由于任何配置的捆绑包启动或安装失败。注意 "framework.error" 条件并没有包含在
  OSGi 条件规范中。

与OSGi规范不同，Apache Celix 框架并未提供API来添加、移除框架监听器。
用户应当使用框架条件服务来替代监听器。这样做的好处是可以确保不会错过任何框架状态变更，因为指示状态的条件服务将一直可用，直到框架停止。

## 框架捆绑包缓存

Apache Celix框架使用捆绑包缓存目录来存储已安装的捆绑包、它们的状态和持久化存储。
捆绑包缓存目录位置由框架属性`CELIX_FRAMEWORK_CACHE_DIR`（默认为".cache"）配置。
捆绑包缓存由状态属性文件、资源缓存、持久性存储缓存三部分组成。

资源缓存用于存储和访问捆绑包资源（例如捆绑包zip文件的内容），只读。
可以使用`celix_bundle_getEntry`或`celix::Bundle::getEntry`访问资源缓存。

持久性存储缓存可用于存储捆绑包的持久数据，可读写。
可以使用`celix_bundle_getDataFile`或`celix::Bundle::getDataFile`访问持久性存储缓存。

如果一个框架仅安装并启动了`Celix::shell`和`Celix::shell_tui`两个捆绑包，则会创建以下目录结构：

```bash
% find .cache
.cache/
.cache/bundle1
.cache/bundle1/bundle_state.properties
.cache/bundle1/storage
.cache/bundle1/resources
.cache/bundle1/resources/libshelld.so.2
.cache/bundle1/resources/libshell.so.2
.cache/bundle1/resources/META-INF
.cache/bundle1/resources/META-INF/MANIFEST.MF
.cache/bundle2
.cache/bundle2/bundle_state.properties
.cache/bundle2/storage
.cache/bundle2/resources
.cache/bundle2/resources/libshell_tuid.so.1
.cache/bundle2/resources/META-INF
.cache/bundle2/resources/META-INF/MANIFEST.MF
```

`.cache/bundle1/resources` 项是资源包缓存，而 `.cache/bundle1/storage` 项是 `Celix::shell` 包的持久存储包缓存。

## 框架配置选项

Apache Celix 框架可通过框架属性进行配置。

框架属性可通过以下方式提供：

- 使用带有 "config.properties" 文件的 Apache Celix 启动器。
- 使用框架工厂创建框架并提供一个`celix_properties_t*`。
- 设置具有 "CELIX_" 前缀的环境变量。

请注意，在创建框架时会一次性读取配置属性和环境变量。
因此，创建框架后更改环境变量将没有任何效果。

以下支持的框架属性：

| 框架属性                                                         | 默认值       | 说明                                                                                    |
|--------------------------------------------------------------|-----------|---------------------------------------------------------------------------------------|
| CELIX_FRAMEWORK_CACHE_DIR                                    | ".cache"  | Apache Celix框架捆绑包缓存目录。                                                                |
| CELIX_FRAMEWORK_CACHE_USE_TMP_DIR                            | "false"   | 如果为真，Apache Celix框架将使用系统临时目录作为缓存目录。                                                   |
| CELIX_FRAMEWORK_CLEAN_CACHE_DIR_ON_CREATE                    | "false"   | 如果为真，Apache Celix框架将在创建时清理缓存目录。                                                       |
| CELIX_FRAMEWORK_FRAMEWORK_UUID                               | ""        | Apache Celix框架的UUID。如果未设置，将生成一个随机的UUID。                                               |
| CELIX_BUNDLES_PATH                                           | "bundles" | Apache Celix框架搜索包的目录。可以用冒号分隔提供多个目录。                                                   |
| CELIX_LOAD_BUNDLES_WITH_NODELETE                             | "false"   | 如果为真，Apache Celix框架将使用RTLD_NODELETE标志加载包库。注意对于cmake构建类型Debug，默认值为"true"，否则默认值为"false" |
| CELIX_FRAMEWORK_STATIC_EVENT_QUEUE_SIZE                      | "100"     | 静态事件队列的大小。如果队列中需要超过100个事件，则将使用动态内存分配。                                                 |
| CELIX_FRAMEWORK_AUTO_START_0                                 | ""        | 在框架启动后要安装和启动的包。可以提供由空格分隔的多个包。                                                         |
| CELIX_FRAMEWORK_AUTO_START_1                                 | ""        | 在框架启动后要安装和启动的包。可以提供由空格分隔的多个包。                                                         |
| CELIX_FRAMEWORK_AUTO_START_2                                 | ""        | 在框架启动后要安装和启动的包。可以提供由空格分隔的多个包。                                                         |
| CELIX_FRAMEWORK_AUTO_START_3                                 | ""        | 在框架启动后要安装和启动的包。可以提供由空格分隔的多个包。                                                         |
| CELIX_FRAMEWORK_AUTO_START_4                                 | ""        | 在框架启动后要安装和启动的包。可以提供由空格分隔的多个包。                                                         |
| CELIX_FRAMEWORK_AUTO_START_5                                 | ""        | 在框架启动后要安装和启动的包。可以提供由空格分隔的多个包。                                                         |
| CELIX_AUTO_INSTALL                                           | ""        | 在框架启动后要安装的捆绑包。可以提供由空格分隔的多个捆绑包。                                                        |
| CELIX_LOGGING_DEFAULT_ACTIVE_LOG_LEVEL                       | "info"    | 为创建的日志服务设置的默认活动日志级别。可能的值有"trace"、"debug"、"info"、"warning"、"error"和"fatal"。            |
| CELIX_ALLOWED_PROCESSING_TIME_FOR_SCHEDULED_EVENT_IN_SECONDS | "2"       | 预定事件的允许处理时间（以秒为单位），如果处理时间更长，将记录一条警告消息。                                                |
