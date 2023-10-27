---
title: 介绍
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

# Apache Celix 介绍

## 什么是 Apache Celix
Apache Celix 是一个 C/C++(C++17) 实现的动态服务框架，其灵感来源于 [OSGi 规范](https://www.osgi.org/developer/specifications)。
它是一个动态模块化软件应用开发框架，支持面向构件和面向服务编程。

Apache Celix 的核心使用 C 语言编写，并在 C 应用编程接口之上封装了仅含头文件的 C++17 应用编程接口。

在 Apache Celix 框架下，可以通过创建提供和使用`服务`的`捆绑包`来构建应用；
这些`服务`既可以直接使用，也可以通过声明了服务供求关系的`服务组件`来使用。
要运行一组捆绑包，可以创建一个名为 Apache Celix `容器`的可执行文件。
Apache Celix `容器`将拉起一个 Apache Celix 框架实例，安装并启动预配置的捆绑包。

## C语言模式
Apache Celix 是用 C 语言编写的，使用一些C语言设计模式来提升开发体验。

## 捆绑包
Apache Celix 捆绑包是一个 zip 文件，其内包含一系列的共享库，配置文件以及可选的激活器条目。
捆绑包可以在 Apache Celix框架中动态安装、卸载、启停。

## 服务
Apache Celix 服务是注册给 Apache Celix 框架的指针，关联了一组属性（元数据）。
服务可以动态注册，并可以通过框架进行查找。

按照惯例，Apache Celix 的 C 服务是一个指向包含函数指针的结构体的指针，而 C++ 服务是一个
指向实现（纯）抽象类的对象的指针（可以作为`std::shared_ptr`提供）。

## 服务组件
Apache Celix 还提供了创建服务组件的方法，服务组件能够以声明式的方式与动态服务进行交互。

通过向框架声明服务组件的服务依赖和配置依赖项（如必需或可选），可以消除部分动态服务的复杂性。

Apache Celix 服务组件可以使用内置的 C/C++ 依赖管理器创建。

请注意，依赖管理器不是 OSGi 标准的一部分，它受到了 Apache Felix 依赖管理器的启发。

## 容器
Apache Celix 容器是可执行文件，它负责启动一个具有一组预配置属性和一组预配置捆绑包的 Apache Celix 框架实例。
尽管也可以在手写代码中创建和启动 Celix 框架，但使用 Celix 容器的好处在于，只要通过单个`add_celix_container` CMake 命令就能一键完成。

## C++支持

选用C作为实现语言的原因之一是，C便于实现各种语言之间（面向服务的）互操作性。

对 C++ 的支持建立在C应用编程接口之上，并实现为仅含头文件的 C++ 库。这意味着 Apache Celix 框架和 utils 库的所有二进制文件都是纯 C，并且不依赖于 C++ 标准库。

Apache Celix 还提供了一些纯 C++ 的库和捆绑包。虽然这些 C++ 库同样仅含头文件，但 C++ 捆绑包含有依赖于 C++ 标准库的二进制文件。

## 更多信息

* 构建
  * [构建和安装 Apache Celix](building/README_CN.md)
  * [使用 CLion 构建和开发 Apache Celix](building/dev_celix_with_clion_CN.md)
* C模式
  * [Apache Celix C 语言模式](c_patterns_CN.md)
* 框架
  * [Apache Celix 捆绑包](bundles_CN.md)
  * [Apache Celix 服务](services_CN.md)
  * [Apache Celix 服务组件](components_CN.md)
  * [Apache Celix 框架](framework.md)
  * [Apache Celix 容器](containers.md)
  * [Apache Celix 设计模式](patterns.md)
  * [Apache Celix 预定事件](scheduled_events.md)
* [Apache Celix CMake 命令](cmake_commands/README_CN.md)
* [Apache Celix 子项目](subprojects.md)
* [Apache Celix 编码规范指南](development/README.md)
