---
title: 子项目
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

# Apache Celix - 包含库和捆绑

Apache Celix 项目被组织成若干库、捆绑包、捆绑包组和示例。

## 核心库
Apache Celix 的核心在以下库中实现：

* [框架](../libs/framework) - Apache Celix 框架，一个适配 C/C++14 的OSGi实现。
* [工具](../libs/utils/README.md) - Celix 的工具库，包含大量的通用工具和支持 OSGi 的类型（属性，版本，过滤器，字符串工具，文件工具等）。

## 独立库
Apache Celix还提供了几个独立的库，这些库可以在没有框架的情况下使用：

* [ETCD库](../libs/etcdlib/README.md) - 一个与ETCD交互的C语言库。
* [Promises库](../libs/promises/README.md) - 一个仅包含头文件的OSGi Promises规范的C++17实现。
* [Push Streams库](../libs/pushstreams/README.md) - 一个仅包含头文件的OSGi Push Stream规范的C++17实现。
* [动态函数接口库](../libs/dfi/README.md) - 在libffi之上构建的C语言库，能够基于描述符文件动态创建C函数代理。

当使用Conan作为构建系统时，可以使用以下命令构建这些独立库：

* ETCD库： `conan create . -b missing -o build_celix_etcdlib=True`
* Promises库： `conan create . -b missing -o build_promises=True`
* Push Streams库： `conan create . -b missing -o build_pushstreams=True`
* 动态函数接口库： `conan create . -b missing -o build_celix_dfi=True`

## 捆绑包/捆绑包组
模块化是 OSGi 规范的重要方面。Apache Celix 提供了多个捆绑包或捆绑包组，这些捆绑包扩展了 Apache Celix 的功能。
这些捆绑包大多基于 OSGi 规范，并适配了到C/C++。

* [HTTP 管理](../bundles/http_admin/README.md) - 一种基于 civetweb 的OSGi HTTP白板的C实现。
* [日志服务](../bundles/logging/README.md) - Apache Celix 的日志服务抽象。
    * [syslog写入器](../bundles/logging/log_writers/README.md) - 与日志服务配合使用的syslog写入器。
* [Shell](../bundles/shell/README.md) - OSGi Shell的C/C++实现，可通过 Shell 命令服务扩展功能。
* [订阅发布](../bundles/pubsub/README.md) - 一种发布-订阅远程消息通信系统的实现。
* [远程服务](../bundles/remote_services/README.md) - OSGi 远程服务管理规范的C语言实现。
* [C++远程服务](../bundles/cxx_remote_services/README.md) - OSGi 远程服务管理规范的C++17实现。需要导入/导出工厂（手写/自动生成）才能工作。
* [组件就绪检查](../bundles/components_ready_check/README.md) - 一个检查所有服务组件是否就绪的捆绑包。

## 示例
Apache Celix提供了若干[示例](../examples/celix-examples)，以展示如何使用框架和捆绑包。
这些示例也可以与已安装的Apache Celix一起使用。

