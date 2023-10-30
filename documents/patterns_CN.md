---
title: Apache Celix 设计模式
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

# Apache Celix / OSGi 设计模式
Apache Celix 和 OSGi 中使用了几种设计模式。

## 白板模式
在白板模式中，只要注册服务就足以参与或扩展现有功能，而无需了解服务的使用者。
其典型特征是，无论使用白板服务的捆绑包是否存在，应用程序都应该解析并启动。

![Whiteboard Pattern](diagrams/whiteboard_pattern.png)

许多Apache Celix服务是白板服务。例如：
- `celix_shell_command_t` 和 `celix::IShellCommand` 服务。这些服务可以被`Celix::shell`捆绑包获取，但是如果没有安装并启动 `Celix::shell`，应用程序仍应正常工作。<br/>
  ![Celix Shell](diagrams/whiteboard_pattern_shell.png)
- `celix_http_service_t` 和 `celix_websocket_service_t` 服务。这些服务可以被`Celix::http_admin`捆绑包获取以提供HTTP URL端点或WebSocket URL端点。<br/>
  ![Celix HTTP 管理](diagrams/whiteboard_pattern_http_admin.png)
- `celix_log_sink_t` 服务。如果没有安装并启动`Celix::log_admin`捆绑包，日志接收服务将永远不会被调用，但应用程序应仍然正常工作。请注意，`Celix::log_admin`捆绑包还使用了按需服务模式（见下文）。<br/>
  ![Celix 日志管理](diagrams/whiteboard_pattern_log_sink.png)
- 标记为远程服务(`service.exported.interface=*`)的服务。如果没有安装并启动任何远程服务管理捆绑包，这些服务将正常工作，但只作为本地服务。

白板模式非常适合用来做模块化，因为白板服务管理无需知道白板服务的具体数量和实现细节（只要实现遵循服务合同即可）。

白板模式服务始终是 [消费者类型](https://docs.osgi.org/javadoc/osgi.annotation/8.0.0/org/osgi/annotation/versioning/ConsumerType.html)，尽管对于Apache Celix，接口不能注解为消费者或提供者类型。

白板模式有一个缺点，一旦应用程序没有按预期工作，问题的原因不总是一目了然。
这是因为在白板模式下未使用的白板服务不是错误，因此没有错误来帮助用户识别程序缺失的环节。

例如：一个提供`celix_log_sink_t`服务的`log_collector`捆绑包被安装并启动，以便可以在中央日志数据库中收集日志。
但是，中央日志数据库中未添加任何日志。在应用没有打印任何告警或错误的情况下，最初很可能会怀疑`log_collector`捆绑包没有工作。
但若`Celix::log_admin`包没有安装并启动，`log_collector`捆绑包提供的`celix_log_sink_t`服务将永远不会被调用。
因此，未安装并启动`Celix::log_admin`同样会造成数据库中没有日志。

## 扩展器模式
扩展器模式是一种利用资源捆绑包概念的设计模式。
通过扩展器模式，可以通过安装所谓的被扩展捆绑包来扩展扩展器捆绑包的功能。
被扩展捆绑包包含某些资源文件和/或被扩展器使用的捆绑包清单条目。

![扩展器模式](diagrams/extender_pattern.png)

扩展器模式的一个示例是 `Celix::http_admin` 捆绑包。
扩展器捆绑包 `Celix::http_admin`监控已装捆绑包并读取其`MANIFEST.MF`以获取`X-Web-Resource`条目。
若找到 `X-Web-Resource` 条目，将利用其值和被扩展捆绑包中的静态网页资源在`Celix::http_admin`捆绑包的HTTP服务器中设置新的HTTP端点。

### `Celix::http_admin`被扩展捆绑包示例
以下示例展示了一个非常简单的`Celix::http_admin`被扩展捆绑包，它为`Celix::http_admin`提供了一个极简风格的"hello world"主页`index.html`。

`Celix::http_admin`被扩展捆绑包示例的备注：
1. 创建一个`Celix::http_admin`的被扩展捆绑包。
2. 将该捆绑包标记为仅包含资源，即没有C/C++激活器的捆绑包。
3. 在CMake中创建一个非常简单的`index.html`文件。
4. 将`index.html`文件添加到`http_admin_extendee_bundle`捆绑包的资源目录中。
5. 添加一个`X-Web-Resource`捆绑包清单条目，该条目将该捆绑包标记为`Celix::http_admin`的被扩展捆绑包。
   详见`Celix::http_admin`以获取更多信息。注意使用了`$<SEMICOLON>`，因为在CMake中，字面量`；`有特殊含义。
6. 创建一个容器，该容器安装并启动`Celix::http_admin`（扩展者）捆绑包和`http_admin_extendee_bundle`（被扩展）捆绑包。
 
```CMake
#CMakeLists.txt
add_celix_bundle(http_admin_extendee_bundle # <----------------------------------------------------------------------<1>
    VERSION 1.0.0
    NO_ACTIVATOR # <-------------------------------------------------------------------------------------------------<2>
)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/index.html" "<html><body>Hello World</body></html>") # <---------------------<3>
celix_bundle_files(http_admin_extendee_bundle "${CMAKE_CURRENT_BINARY_DIR}/index.html" DESTINATION resources) # <----<4>
celix_bundle_headers(http_admin_extendee_bundle "X-Web-Resource: /hello$<SEMICOLON>/resources") # <------------------<5>

add_celix_container(extender_pattern_example_container # <-----------------------------------------------------------<6>
    BUNDLES
        Celix::http_admin
        http_admin_extendee_bundle
)
```

当 `extender_pattern_example_container` 可执行文件正在运行时，网址 `http://localhost:8080/hello` 将显示 `index.html` 的内容。

## 按需服务（Service on Demand，简称 SOD）模式

Apache Celix / OSGi 中一个较少为人所知的模式是按需服务（Service on Demand，简称SOD）模式。在此模式下，服务只有在被请求时才进行临时注册。

白板模式支持以模块化和面向服务的方式扩展功能，而按需服务模式支持以面向服务的方式为用户提供更具功能内聚性的服务。

对于按需服务模式，可以从服务请求者设置的服务过滤器中提取是否需要以及如何创建按需服务的信息。

![按需服务模式](diagrams/sod_pattern.png)

一些 Apache Celix 捆绑包使用按需服务模式。例如：
- `Celix::log_admin` 捆绑包会使用所请求的日志记录器（logger）名称创建并注册`celix_log_service_t` 服务。<br/>
  ![Celix 日志管理](diagrams/sod_pattern_log_service.png)
- Celix 订阅发布捆绑包只有收到带有效"topic.name"和"topic.scope"过滤器属性的服务请求时才会创建并注册 `pubsub_publisher_t` 服务。
  Celix 订阅发布拓扑管理器捆绑包会监控对`pubsub_publisher_t`服务的请求，并指示可用的 Celix 订阅发布管理捆绑包创建`pubsub_publisher_t`。 <br/>
  ![Celix 订阅发布](diagrams/sod_pattern_publisher_service.png)
- Apache Celix/OSGi 远程服务是按需服务，仅在它们被发现并被请求时才会实际导入。

按需服务总是[提供者类型](https://docs.osgi.org/javadoc/osgi.annotation/8.0.0/org/osgi/annotation/versioning/ProviderType.html)，尽管对于 Apache Celix， 接口不能标注为
消费者或提供者类型。

对于 OSGi，可以使用 [FindHook](https://docs.osgi.org/javadoc/osgi.core/8.0.0/org/osgi/framework/hooks/service/FindHook.html)
服务进一步微调哪些服务对请求按需服务的捆绑包是可见的。
Apache Celix 还不支持 FindHook 服务。

