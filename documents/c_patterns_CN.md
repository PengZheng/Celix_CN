---
title: Apache Celix C语言模式
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

# Apache Celix C语言模式

Apache Celix的核心是用C语言编写的，因为C语言便于其他语言进行跨语言调用。
但C语言缺少类和对象的概念，基于作用域的资源管理 - 类似RAII - 以及其他现代C++特性。为了在某种程度上克服这一点，Apache Celix采用了若干设计模式。

请注意，在理想情况下，所有的Apache Celix C代码都遵循本节所描述的模式，但也有例外（特别是部分老代码）。

## Apache Celix C 对象

第一种模式是 Apache Celix C 对象模式。这种模式用于以 C 语言方式创建、销毁和管理对象。
C 对象使用指向结构体的不透明指针实现，该结构体包含对用户不可见的对象详细信息。C 对象应该提供 C 函数来创建、销毁和操作对象。

用于对象结构体的命名方案是 `<celix_object_name>`，通常 typedef 到 `<celix_object_name>_t`。
对于对象函数，使用以下命名方案：`<celix_objectName>_<functionName>`。注意对象名称和函数名称为驼峰命名。

Apache Celix C 对象应该总是有一个构造函数和一个析构函数。
如果涉及到内存分配，`celix_<objectName>_create` 函数用于创建并返回一个新对象，并且 `celix_<objectName>_destroy` 函数用于销毁对象和释放对象的内存。
否则，使用一个具有 `celix_status_t` 返回值的 `celix_<objectName>_init` 函数来初始化对象的已提供内存，并使用一个 `celix_<objectName>_deinit` 函数来取消初始化对象。
`celix_<objectName>_deinit` 函数不应释放对象的内存。

Apache Celix C 对象也可以有额外的函数来访问对象信息或操作对象。如果一个对象包含属性，它应该为每个属性提供一个 getter 和 setter 函数。

## Apache Celix C 容器类型

Apache Celix 提供了几种容器类型：`celix_array_list`，`celix_properties`，`celix_string_hash_map`，
和 `celix_long_hash_map`。尽管这些容器不具备类型安全性，但它们提供了额外的函数来处理
不同的元素类型。请参考头文件以获取更多信息。

## Apache Celix 的C语言基于作用域的资源管理

Apache Celix 提供了一些宏来辅助现有类型实现基于作用域的资源管理(SBRM)。
这些宏受到了[内核的基于作用域的资源管理](https://lwn.net/Articles/934838/)的启发。

主要用于 SBRM 的宏有：
- `celix_autofree`：在变量超出作用域时，自动使用 `free` 释放内存。
- `celix_auto`：在变量超出作用域时，自动调用基于值的清理函数。
- `celix_autoptr`：在变量超出作用域时，自动调用基于指针的清理函数。
- `celix_steal_ptr`：被用来从变量中"偷取"出被其托管的指针，以防止当变量超出作用域时，自动进行清理。

这些宏可以在 Apache Celix 的utils头文件 `celix_cleanup.h` 和 `celix_stdlib_cleanup.h` 中找到。

在 Apache Celix 中，C 语言对象必须选择加入 SBRM。这是通过使用 `CELIX_DEFINE_AUTO` 宏来完成的，它确定了清理对象所期望的 C 函数。

## 对资源分配即初始化（RAII）类似结构的支持

基于先前提到的 SBRM，Apache Celix 也为类似 RAII 的结构提供了支持。
它们可以用来保护锁，管理服务注册等。这些 guard 应遵循命名约定
`celix_<obj_to_guard>_guard_t`。通过提供额外的 cleanup 函数，能够支持类似 RAII 的结构，
这些函数可以与 `celix_auto` 或 `celix_autoptr` 宏一起工作。

示例包括：
- `celix_mutex_lock_guard_t`
- `celix_service_registration_guard_t`

我们确保这些结构不需要额外的内存分配，仅产生最小开销（甚至没有开销）。

## Apache Celix中的多态性

值得一提的是，上述的模式和附加功能并未支持多态。
目前Apache Celix主要使用服务来处理多态，即为同一服务的多个实现注册多个服务实例。
欲了解更多信息，请参阅"Apache Celix服务"部分。
