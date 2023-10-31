---
title: Apache Celix 编码规范
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

# Apache Celix 编码规范
遵守一致且有意义的代码规范对于维护代码的可读性和可维护性至关重要。
本文档概述了Apache Celix开发的推荐编码规范，包括命名规范，格式化，注释，控制结构，函数和错误处理。

请注意，并非所有已有的代码都符合这些规范。
新代码应遵循这些规范；只要可能，已有代码也应更新以符合这些规范。

## 命名约定

### C/C++ 变量

- 对于变量名称，使用 `camelCase`。
- 对于变量，使用描述性名字。
- 对于全局变量，使用 `celix_` 前缀或者 `celix::` (子)命名空间。
- 星号 `*` 和和号 `&` 应放在变量类型名上。

### C 结构体

- 对于结构体名称，使用 `snake_case`。
- 为结构体添加 typedef。
- 结构体的 typedef 使用 `_t` 后缀。
- 结构体的名称使用 `celix_` 前缀。
- 对于 C 对象，使用不透明结构体的 typedef。例如 `typedef struct celix_<obj> celix_<obj>_t;`
  - 这样的话，就可以将实现细节隐藏起来。

### C 函数

- 对函数使用描述性的名字。
- 使用 `celix_` 前缀。
- 对于对象/模块名称，使用形如`_<objName>_`的驼峰命名法中缀。
- 对于函数名称，使用形如`camelCase`的驼峰命名法后缀。
- 星号 `*` 应放在变量类型名上。
- 当函数有副作用时，使用动词作为函数名。
- 当函数没有副作用时，使用名词或 getter/setter 作为函数名。
- 对于获取/设置值的函数，使用 getters/setters 命名规则：
  - 对布尔值使用 `celix_<obj>_is<Value>` 和 `celix_<obj>_set<Value>`
  - 对其它值使用 `celix_<obj>_get<Value>` 和 `celix_<obj>_set<Value>`
- 对于 C 对象：
  - 将(不透明的)对象指针作为函数的第一个参数。
  - 确保对象可以通过 `celix_<obj>_create` 函数创建，通过 `celix_<obj>_destroy` 函数销毁。
  - `celix_<obj>_create` 函数应返回指向该对象的指针。
  - `celix_<obj>_destroy` 函数应返回 `void`，并且能够处理 NULL 指针。
    - 通过处理 NULL 指针，`celix_<obj>_destroy` 函数可以更易于在错误处理代码中使用。

示例：
- `long celix_bundleContext_installBundle(celix_bundle_context_t* ctx, const char* bundleUrl, bool autoStart)`
- `bool celix_utils_stringEquals(const char* a, const char* b)`
- `celix_status_t celix_utils_createDirectory(const char* path, bool failIfPresent, const char** errorOut)`

### C 常量

- 使用 `SNAKE_CASE` 为常量命名。
- 使用 `CELIX_` 前缀为常量命名。
- 使用 `#define` 定义常量。

### C 枚举

- 使用 `snake_case` 为枚举类型命名。
- 使用 `celix_` 前缀为枚举类型命名。
- 使用 `SNAKE_CASE` 为枚举值命名。
- 使用 `CELIX_` 前缀为枚举值命名。
- 为枚举添加 typedef -采用 `_e` 后缀。

示例：
```c
typedef enum celix_hash_map_key_type {
    CELIX_HASH_MAP_STRING_KEY,
    CELIX_HASH_MAP_LONG_KEY
} celix_hash_map_key_type_e;
```

### 宏

- 使用全大写的 `SNAKE_CASE` 为宏的命名。
- 使用 `CELIX_` 前缀为宏命名。

### C文件和目录

- 使用 `snake_case` 为文件命名。
- 用 `.h` 后缀命名头文件，用 `.c` 后缀命名源文件。
- 根据文件的用途在目录中组织文件。
  - 将公共头文件放在 `include`、`api` 或 `spi` 目录中。
  - 将私有头文件放在 `private` 和 `src` 目录中。
  - 将源文件放在 `src` 目录中。
- Google test 文件应该放在带有自己的 `CMakeLists.txt` 文件和 `src` 目录的 `gtest` 目录中。
- 使用 `celix_` 前缀为头文件命名。
- 使用头文件保护宏。
- 在头文件中使用C++ "extern C" 块，以确保 C 头文件可以在 C++ 中使用。

### C库

- CMake目标名称应使用 `snake_case`。
- 库应有 `celix::` 前缀的CMake别名。
- C 共享库应配置一个带有 `celix_` 前缀的输出名称。

### C服务

- 获取服务头文件应该通过 CMake INTERFACE 库目标以仅包含头文件的API/SPI库的形式对外暴露（如 `celix::shell_api`）。
- C 服务应为 C 结构体，其中第一个成员为服务句柄（`void* handle;`），其余成员为函数指针。
- 服务函数的第一个参数应为服务句柄。
- 如果服务函数中需要内存分配或可能发生其他错误，要确保可以使用返回值来检查错误。这可以通过以下方式实现：
  - 返回 `celix_status_t`，如果需要，也可以使用出参。
  - 如果函数返回指针类型，则返回`NULL`指针。
  - 返回布尔值，其中 `true` 表示成功，`false` 表示失败。
- 在C服务结构体所在的头文件中，还应提供服务名称和版本的定义。
- 服务名称宏应为全大写的 `SNAKE_CASE`，前缀为 `CELIX_`，后缀为 `_NAME`。
- 服务版本宏应为全大写的 `SNAKE_CASE`，前缀为 `CELIX_`，后缀为 `_VERSION`。
- 服务名称宏的值应为服务结构体名（因此没有 `_t` 后缀)
- 服务版本宏的值应为服务的版本。

示例：
```c
//celix_foo.h
#include "celix_errno.h"

#define CELIX_FOO_NAME "celix_foo"
#define CELIX_FOO_VERSION 1.0.0

typedef struct celix_foo {
    void* handle;
    celix_status_t (*doFoo)(void* handle, char** outMsg);
} celix_foo_t;
```

### C捆绑包

- 使用 `snake_case` 为C捆绑包目标名称命名。
- 不使用 `celix_` 前缀为C捆绑目标名称命名。
- 为C捆绑包目标使用 `celix::` 前缀的别名。
- 使用 `snake_case` 为C捆绑包的符号名称命名。
- 为C捆绑包目标至少配置 SYMBOLIC_NAME、NAME、FILENAME、VERSION 和 GROUP。
- 使用 `apache_celix_` 前缀为C捆绑包的符号名称命名。
- 使用 `Apache Celix ` 前缀为C捆绑包命名。
- 使用 `celix_` 前缀为C捆绑包文件名命名。
- 使用以 `celix/` 开头的组名为C捆绑包组命名。

示例：
```cmake
add_celix_bundle(my_bundle
    SOURCES src/my_bundle.c
    SYMBOLIC_NAME "apache_celix_my_bundle"
    NAME "Apache Celix My Bundle"
    FILENAME "celix_my_bundle"
    VERSION "1.0.0"
    GROUP "celix/my_bundle_group"
)
add_library(celix::my_bundle ALIAS my_bundle)
```

### C++命名空间

- 使用 `snake_case` 为命名空间命名。
- 所有的命名空间都应该是 `celix` 命名空间的一部分。
- 尽量将命名空间的级别限制在3级以内。
- 对于实现细节，使用以 `detail` 结尾的命名空间。

### C++类

- 使用 `CamelCase`模式（以大写字母开头）命名类。
- 对类使用描述性的名字。
- 类应该是 `celix::` 命名空间或子 `celix::` 命名空间的一部分。

### C++函数

- 使用 `camelCase`模式命名函数。
- 如果函数不是类/结构的一部分，那么它应该是 `celix::` 命名空间或子 `celix::` 命名空间的一部分。
- 星号 `*` 和和号 `&` 应放在变量类型名上。
- 当函数有副作用时，使用动词作为函数名。
- 当函数没有副作用时，使用名词或 getter/setter 作为函数名。
- 对于获取/设置值的函数，使用 getters/setters 命名规则。

### C++常量

- 使用 `SNAKE_CASE` 为常量命名。
- 使用 constexpr 定义常量。
- 将常量放在 `celix::` 命名空间或子 `celix::` 命名空间中。

示例：
```c++
namespace celix {
    constexpr long FRAMEWORK_BUNDLE_ID = 0;
    constexpr const char* const SERVICE_ID = "service.id";
}
```

### C++枚举

- 使用 `CamelCase`（首字母大写）为枚举类型命名。
- 使用 `enum class` 而非 `enum`，并在可能的情况下使用 `std::int8_t` 作为基类型。
- 枚举值使用 `SNAKE_CASE`，且无需 celix/class 前缀。注意，对于枚举值，由于 enum class 值是有作用域的，因此不需要前缀。

示例：
```c++
namespace celix {
    enum class ServiceRegistrationState {
        REGISTERING,
        REGISTERED,
        UNREGISTERING,
        UNREGISTERED
    };
}
```

### C++文件和目录

- 使用 `CamelCase`（首字母大写）为文件命名。
- 用 `.h` 后缀命名头文件，用 `.cc` 后缀命名源文件。
- 根据命名空间的不同，将头文件放在不同的目录中（例如 `celix/Bundle.h`, `celix/dm/Component.h`）。
- 根据文件的用途在目录中组织文件。
  - 将公共头文件放在 `include`、`api` 或 `spi` 目录中。
  - 将私有头文件放在 `private` 和 `src` 目录中。
  - 将源文件放在 `src` 目录中。
- 使用 `#pragma once` 做头文件保护。

### C++库

- 目标名称应为 `CamelCase`（首字母大写）。
- 库应有 `celix::` 前缀的CMake目标别名。
- C++ 库应支持 C++14。
  - 例外是 `celix::Promises` 和 `celix::PushStreams`，它们需要 C++17。
- Apache Celix 框架库（`Celix::framework`）和 Apache Celix 工具库（`Celix::utils`）只能以C++头文件库的形式提供。这确保了框架和工具库可以在纯C项目中使用，且不引入 C++ ABI。
- 对于其他库，优先选择以C++头文件库的形式提供，但这不是必需的。
- C++头文件库不需要生成导出头文件，也不需要配置符号可见性。
- C++ 共享库（带 C++ 源文件的库）应配置一个带有 `celix_` 前缀的输出名称。
- C++ 共享库（带 C++ 源文件的库）应使用导出头文件并配置符号可见性。
  - 有关更多信息，请参见 C 库部分。

### C++服务

- 使用 `CamelCase`（首字母大写）为服务命名。
- 在服务接口名称前添加 'I' 前缀。
- 将服务类放在 `celix::` 命名空间或子 `celix::` 命名空间中。
- 在服务类中添加 `static constexpr const char* const NAME` ，用于服务名称。
- 在服务类中添加 `static constexpr const char* const VERSION`，用于服务版本。

### C++捆绑包

- 使用 `CamelCase` 为C++捆绑包目标名称命名。
- C++捆绑包目标名称不使用 `Celix` 前缀。
- C++捆绑包目标使用 `celix::` 前缀的别名。
- 使用 `CamelCase` 为C++捆绑包的符号名称命名。
- 为C++捆绑包目标至少配置 SYMBOLIC_NAME、NAME、FILENAME、VERSION 和 GROUP。
- 使用 `Apache_Celix_` 前缀为C++捆绑包的符号名称命名。
- 使用 `Apache Celix ` 前缀为C++捆绑包命名。
- 使用 `celix_` 前缀为C++捆绑包文件名命名。
- 使用以 `celix/` 开头的组名为C++捆绑包组命名。

示例：
```cmake
add_celix_bundle(MyBundle
    SOURCES src/MyBundle.cc
    SYMBOLIC_NAME "Apache_Celix_MyBundle"
    NAME "Apache Celix My Bundle"
    FILENAME "celix_MyBundle"
    VERSION "1.0.0"
    GROUP "celix/MyBundleGroup"
)
add_library(celix::MyBundle ALIAS MyBundle)
```

### 单元测试命名

- 测试夹具（fixture）应有 `TestSuite` 后缀。
- 源文件应根据测试夹具名称命名，并使用 `.cc` 扩展名。
- 测试用例名称应使用 `CamelCase`（起始字母大写）并在后面加上 `Test` 后缀。
- 使用错误注入（`error_injector`）时，应使用单独的测试套件。
  - 测试夹具应使用 `ErrorInjectionTestSuite` 后缀。
  - 错误注入设置应在测试夹具的 `TearDown` 函数或析构函数中重置。

## 注释和文档

- 使用 Doxygen 文档，除了行内注释。
- 编写注释解释代码的目的，着重说明 "为什么" 而非 "怎么做"。
- 将 Doxygen 文档应用于所有公共API。
- 使用 javadoc 风格编写 Doxygen 文档。
- 使用 `@`，而非 `\` 标记 Doxygen 命令。
- 以 `@brief` 命令和简短描述开始 Doxygen 文档。
- 对于 `@param` 命令，还应提供输入、输出或输入/输出信息。
- 对于 `@return` 命令，还应提供返回值的描述。
- 如果函数可以返回多个错误码，可使用专门的段落来记录可能的错误。可以拿 `man 2 write` 作为良好错误码说明的范例。

## 格式和缩进

- 使用空格进行缩进，每个缩进级别使用4个空格。
- 如果可能，保持行长度在120个字符以下，以增强可读性。
- 将开始的大括号放在控制语句或函数定义的同一行，并将关闭的大括号放在与控制语句或函数定义对齐的新行上。
- 在运算符之前和之后以及赋值语句周围使用一个空格。
- 在跟随括号的控制关键字（`if`、`for`、`while` 等）之后添加一个空格。
- 控制结构中始终使用大括号 ({ })，即使对于只包含一个语句的块，也可以防止错误。
- 在跟随大括号的控制关键字（`else`、`do` 等）之后添加一个空格。
- 函数名和开括号之后不要添加空格。
- 对于新文件，使用项目配置的clang-format（.clang-format）格式化代码。
  - 注意，通常可以由IDE插件或运行 `clang-format -i <file>` 来实现。

## 控制结构

- 使用 `if`、`else if` 和 `else` 语句处理多个条件。
- 对于具有默认情况的多个条件，使用 `switch` 语句。
- 对于可能不执行的循环，使用 `while` 语句。
- 对于至少执行一次的循环，使用 `do`/`while` 语句。
- 对于具有已知迭代次数的循环，使用 `for` 语句。
- 除非用于C语言错误处理（对于 C++ 使用 RAII），否则不允许使用 `goto`。
- 对于 C，尝试防止深度嵌套的控制结构，优选提早返回/错误处理`goto`。
  - 为了防止深度嵌套控制结构，也可以使用 `CELIX_DO_IF`、`CELIX_GOTO_IF_NULL` 和 `CELIX_GOTO_IF_ERR` 宏。

## 函数和方法

- 让函数负责单一的责任或目的，遵循单一责任原则（SRP）。
- 保持函数短小并专注，目标是少于50行代码。
- 保证 const 正确性。
- 对于具有很多不同参数的C函数，考虑使用选项结构体。
  - 通过EMPTY_OPTIONS宏来为选项结构体提供默认值，选项结构体可以向后兼容地进行更新。
  - 选项结构体确保可以配置大量参数，并且可以在创建时直接进行设置。
- 对于具有很多不同参数的C++函数，考虑使用建造者模式。
  - 建造者模式支持向后兼容地进行更新。
  - 建造者模式确保可以配置大量参数，并且可以在构造时直接进行设置。

## 错误处理和日志记录

- 在C++中，出现错误时抛出异常，并使用资源分配即初始化（RAII）确保资源被释放。
- 对于C，如果需要内存分配或可能发生其他错误，请确保函数返回一个可用于检查错误的值。可以通过以下方式实现：
  - 返回 `celix_status_t`，如果需要，使用输出参数。
  - 如果函数返回一个指针，则返回 NULL 指针。
  - 返回布尔值，其中 `true` 表示成功，`false` 表示失败。
- 使用一致的错误处理技术，例如返回错误码或使用指定的错误处理函数。
- 记录错误、警告和其他重要事件：在可以访问捆绑包上下文的环境中使用 Apache Celix 日志助手函数进行记录；在一般库（没有捆绑包上下文）中使用 `celix_err` 进行记录。
- 始终检查错误并记录它们。
- 错误处理应以资源分配/创建的相反顺序释放资源（逆序释放原则）。
- 使用带错误注入的测试套件确保错误处理正确无误。

对于日志级别，遵循以下指南：
- trace：使用此级别进行非常详细的日志记录，只在诊断问题时需要。
- debug：此级别应用于可能有助于诊断问题或理解问题的信息，但默认情况下过于冗长。
- info：此级别对应于与任何特定问题或错误条件无关的一般操作消息。它们提供了对系统正常行为的洞察。例子包括启动/关闭消息、配置下发等。
- warning：使用此级别报告系统可以恢复的问题，但这表明系统可能存在潜在的问题。
- error：应使用此级别报告需要立即关注的问题，这些问题可能会阻止系统正常运行。这些问题是意料之外的，并影响功能性，但尚未不严重到需要停止进程。示例包括运行时错误、无法连接到服务等。
- fatal：使用此级别报告导致程序无法继续运行的严重错误。记录致命错误后，程序通常会终止。

下面是错误处理和日志记录的示例：
```c
celix_foo_t* celix_foo_create(celix_log_helper_t* logHelper) {
    celix_foo_t* foo = calloc(1, sizeof(*foo));
    if (!foo) {
        goto create_enomem_err;
    }
    
    CELIX_GOTO_IF_ERR(create_mutex_err, celixThreadMutex_create(&foo->mutex, NULL));
    
    foo->list = celix_arrayList_create();
    foo->map = celix_longHashMap_create();
    if (!foo->list ||  !foo->map) {
        goto create_enomem_err;
    }
    
  return foo;
create_mutex_err:
  celix_logHelper_log(logHelper, CELIX_LOG_LEVEL_ERROR, "Error creating mutex");
  free(foo); //mutex not created, do not use celix_foo_destroy to prevent mutex destroy
  return NULL;
create_enomem_err:
  celix_logHelper_log(logHelper, CELIX_LOG_LEVEL_ERROR, "Error creating foo, out of memory");
  celix_foo_destroy(foo); //note celix_foo_destroy can handle NULL
  return NULL;
}

void celix_foo_destroy(celix_foo_t* foo) {
    if (foo != NULL) {
        //note reverse order of creation
        celixThreadMutex_destroy(&foo->mutex);
        celix_arrayList_destroy(foo->list);
        celix_longHashMap_destroy(foo->map);
        free(foo);
    }
}
```

## 错误注入

- 使用Apache Celix的error_injector库以受控方式在单元测试中注入错误。
- 为错误注入测试创建一个独立的测试套件，并将它们放在 `EI_TESTS` 的CMake条件下。
- 在测试夹具的 `TearDown` 函数或析构函数上重置错误注入设置。
- 如果某个-内部或外部-函数缺少错误注入支持，请将其添加到error_injector库中。
  - 尝试为特定功能创建小型的错误注入器库。

## 单元测试方法

- 使用Google Test框架进行单元测试。
- 使用Google Mock框架进行模拟。
- 使用Apache Celix的error_injector库以受控方式在单元测试中注入错误。
- 通过在程序创建的框架中安装捆绑包来测试它们。
- 通过使用其提供的服务及提供其使用的服务来测试捆绑包。
- 在大多数情况下，可以使用白盒方法测试库，并使用黑盒方法测试捆绑包。
- 对于用Apache Celix错误注入器库测试或需要访问私有/隐藏函数（白盒测试）的库，应创建一个单独的"被测试代码"（code under test）静态库。
  该库不应隐藏其符号，并应附加 `_cut` 后缀。

```cmake
set(MY_LIB_SOURCES ...)
set(MY_LIB_PUBLIC_LIBS ...)
set(MY_LIB_PRIVATE_LIBS ...)
add_library(my_lib SHARED ${MY_LIB_SOURCES})
target_link_libraries(my_lib PUBLIC ${MY_LIB_PUBLIC_LIBS} PRIVATE ${MY_LIB_PRIVATE_LIBS})
celix_target_hide_symbols(my_lib)

if (ENABLE_TESTING)
    add_library(my_lib_cut STATIC ${MY_LIB_SOURCES})
    target_link_libraries(my_lib_cut PUBLIC ${MY_LIB_PUBLIC_LIBS} ${MY_LIB_PRIVATE_LIBS})
    target_include_directories(my_lib_cut PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_BINARY_DIR}/celix/gen/includes/my_lib
    )
endif ()
```

## 支持的C和C++标准

- C库应支持C99。
- C++库应支持C++14。
  - `celix::Promises` 和 `celix::PushStreams`是例外，它们需要C++17。
- `celix::framework` 和 `celix::utils` 必须以C++头文件库的形式提供。
- 单元测试代码可以用C++17编写。

## 库目标属性

对于C和C++共享库，应设置以下目标属性：
- `VERSION` 应设置为库版本。
- `SOVERSION` 应设置为库主版本。
- `OUTPUT_NAME` 应设置为库名称，并应包含一个 `celix_` 前缀。

```cmake
add_library(my_lib SHARED
    src/my_lib.c)
set_target_properties(my_lib 
    PROPERTIES
        VERSION 1.0.0
        SOVERSION 1
        OUTPUT_NAME celix_my_lib)
```

对于C和C++的静态库，应设置以下目标属性：
- 对于静态库， `POSITION_INDEPENDENT_CODE` 应设置为 `ON` 。
- `OUTPUT_NAME` 应设置为库名称，并应包含一个 `celix_` 前缀。

```cmake
add_library(my_lib STATIC
    src/my_lib.c)
set_target_properties(my_lib
    PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        OUTPUT_NAME celix_my_lib)
```

## 符号可见性

- 仅头文件（INTERFACE）库不应配置符号可见性。
- 共享库和静态库应配置符号可见性。
  - 预计作为私有链接的静态库应隐藏符号。
- 应配置包的符号可见性（这是默认操作）。

### 为C/C++库配置符号可见性

对于Apache Celix共享库，应使用CMake目标属性 `C_VISIBILITY_PRESET`、`CXX_VISIBILITY_PRESET` 和 `VISIBILITY_INLINES_HIDDEN` 以及生成的导出头文件来配置符号可见性。

`C_VISIBILITY_PRESET` 和 `CXX_VISIBILITY_PRESET` 目标属性可以用于配置C和C++代码中符号的默认可见性。 `VISIBILITY_INLINES_HIDDEN` 属性可以用于配置内联函数的可见性。 `VISIBILITY_INLINES_HIDDEN` 属性仅支持C++代码。

默认的可见性应配置为隐藏，并且应明确地使用生成的导出头文件中的导出宏来导出符号。可以使用CMake函数 `generate_export_header` 来生成导出头文件。每个库都应该有自己的导出头文件。

对于共享库，可以使用以下CMake代码来完成：

```cmake
add_library(my_lib SHARED
        src/my_lib.c)
set_target_properties(my_lib PROPERTIES
        C_VISIBILITY_PRESET hidden
        #对于C++共享库，还应配置CXX_VISIBILITY_PRESET
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        OUTPUT_NAME celix_my_lib)
target_include_directories(my_lib
      PUBLIC
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/celix/gen/includes/my_lib>
      PRIVATE
        src)

#生成导出头文件
generate_export_header(my_lib
        BASE_NAME "CELIX_MY_LIB"
        EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/celix/gen/includes/my_lib/celix_my_lib_export.h")

#安装
install(TARGETS my_lib EXPORT celix LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/celix_my_lib)
install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/celix_my_lib)
install(DIRECTORY ${CMAKE_BINARY_DIR}/celix/gen/includes/my_lib/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/celix_my_lib)
```

### 为C/C++捆绑包配置符号可见性

对于捆绑包，符号可见性将默认配置为隐藏。
只需要在CMake的 `add_celix_bundle` 函数中提供 `DO_NOT_CONFIGURE_SYMBOL_VISIBILITY` 选项，即可修改此默认配置。

如果在 `add_celix_bundle` 中没有配置符号可见性，那么符号可见性的配置方式与共享库的配置方式相同。

```cmake
add_celix_bundle(my_bundle
    SOURCES src/my_bundle.c
    SYMBOLIC_NAME "apache_celix_my_bundle"
    NAME "Apache Celix My Bundle"
    FILENAME "celix_my_bundle"
    VERSION "1.0.0"
    GROUP "celix/my_bundle_group"
)
add_library(celix::my_bundle ALIAS my_bundle)
```

## 分支命名规则

- 用 `feature/` 前缀表示功能分支，用 `hotfix/` 前缀表示热修复分支，用 `bugfix/` 前缀表示修复缺陷的分支，`release/` 前缀表示发布分支。
- 如果您正在处理一个问题，请使用问题编号作为分支名的前缀。例如，`feature/1234-add-feature`。
- 热修复分支主要用于需要尽快应用的紧急修复。
- 使用简短且描述性强的分支名。

## 提交信息

- 在编写提交消息时，请使用祈使语气 (例如 "Add feature" 而不是 "Adds feature" 或 "Added feature")。这种风格与 git 自动生成的合并提交或还原操作的消息相符。
- 确保提交信息具有描述性并提供有意义的上下文。
- 保持提交信息的第一行简洁，理想情况下在 50 个字符以内。
  这一概要行作为对更改的快速概述，应保持在git日志条目中易于阅读。
- 若需提供更多的上下文，用空行将概要行与主体部分分隔开。
  主体部分可以提供有关更改背后的详细信息和解释。
  为了达到最佳的可读性，尽量确保提交消息主体的每行都尽可能在 72 个字符处换行。
- 当在提交消息主体中列出多个更改或要点时，使用要点列表，编号列表或其他格式约定以提高可读性。
- 如果适用，可以在提交消息主体中引用相关的问题、错误报告或拉取请求，以提供额外的上下文并将本次提交与更大的项目关联起来。
  - 如果本次提交修复了、关闭了或解决了一个问题，使用这些关键词之一，再后跟问题编号(例如 "fixes #42"，"closes #42" 或 "resolves #42")。
  - 如果你想引用一个问题，但不关闭它，就只需要提及问题编号(例如 "related to #42" 或 "#42")。

## 基准测试

- 需要时，使用基准测试来测量性能。
- 使用 Google Benchmark 框架进行基准测试。

## 代码质量

- 新代码应通过拉取请求进行审查，不允许在 master 分支上直接提交。
  - 至少应有1位审查者审查代码。
- 热修复的拉取请求可以先合并后审查，其余的需先审查后合并。
- 所有新代码都应编写单元测试。
- 应测量代码覆盖率，并力争达到至少 95% 的代码覆盖率。
- 对于现有代码，应维持或提高代码覆盖率。
- 代码应使用 AddressSanitizer 检查内存泄漏。
- 应定期在 master 分支上执行 Coverity 扫描。理想情况下，新的 Coverity 问题应尽快修复。
