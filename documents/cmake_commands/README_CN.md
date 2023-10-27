---
title: CMake 命令
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

# Apache Celix - CMake 命令

为便于使用和部署 Apache Celix 捆绑包，我们提供了一些 cmake 命令。

# 捆绑包
捆绑包是一个可以动态加载的资源集合，包含共享库、配置文件、以及一个可以在 Celix 框架中动态安装和启动的可选激活器。

## add_celix_bundle
将一个 Celix 捆绑包添加到项目中。

```CMake
add_celix_bundle(<bundle_target_name>
        SOURCES source1 source2 ...
        [NAME bundle_name]
        [SYMBOLIC_NAME bundle_symbolic_name]
        [DESCRIPTION bundle_description]
        [GROUP bundle_group]
        [VERSION bundle_version]
        [FILENAME bundle_filename]
        [PRIVATE_LIBRARIES private_lib1 private_lib2 ...]
        [HEADERS "header1: header1_value" "header2: header2_value" ...]
        [DO_NOT_CONFIGURE_SYMBOL_VISIBILITY]
)
```

```CMake
add_celix_bundle(<bundle_target_name>
        ACTIVATOR <activator_lib>
        [NAME bundle_name]
        [SYMBOLIC_NAME bundle_symbolic_name]
        [DESCRIPTION bundle_description]
        [GROUP bundle_group]
        [VERSION bundle_version]
        [FILENAME bundle_filename]
        [PRIVATE_LIBRARIES private_lib1 private_lib2 ...]
        [HEADERS "header1: header1_value" "header2: header2_value" ...]
        [DO_NOT_CONFIGURE_SYMBOL_VISIBILITY]
)
```

```CMake
add_celix_bundle(<bundle_target_name>
        NO_ACTIVATOR
        [NAME bundle_name]
        [SYMBOLIC_NAME bundle_symbolic_name]
        [DESCRIPTION bundle_description]
        [GROUP bundle_group]
        [VERSION bundle_version]
        [FILENAME bundle_filename]
        [PRIVATE_LIBRARIES private_lib1 private_lib2 ...]
        [HEADERS "header1: header1_value" "header2: header2_value" ...]
        [DO_NOT_CONFIGURE_SYMBOL_VISIBILITY]
)
```

示例：
```CMake
add_celix_bundle(my_bundle SOURCES src/my_activator.c)
```

有以下三种变体:
- 使用SOURCES，会使用源文件列表作为激活器库的输入，创建使用该激活器的捆绑包。
- 使用ACTIVATOR, 会使用库目标或现有库的绝对路径作为激活器来创建捆绑包。
- 使用NO_ACTIVATOR，将会创建一个没有激活器的捆绑包（即，一个纯资源捆绑包）。

可选参数包括:
- NAME: 捆绑包的(人类可读的)名称。这将作为 Bundle-Name 清单条目使用。默认值是 `<bundle_target_name>`。
- SYMBOLIC_NAME: 捆绑包的符号名称。这将作为 Bundle-SymbolicName 清单条目使用。默认值是 `<bundle_target_name>`。
- DESCRIPTION: 捆绑包的描述。这将作为 Bundle-Description 清单条目使用。默认情况下是空的。
- GROUP: 捆绑包所属的组。这将作为 Bundle-Group 清单条目使用。默认情况下是空的(没有组)。
- VERSION: 捆绑包的版本。这将被用于 Bundle-Version 清单条目。 当和 SOURCES 一起使用时，版本也将被用于设定激活器库的 CMake 目标属性 VERSION 和 SOVERSION。
  对于 SOVERSION，只使用 major 部分。 预期的格式是 `"<major>.<minor>.<path>"`。 默认版本是 "0.0.0"。
- FILENAME: 捆绑包的文件名，不含扩展名。默认值是 `<bundle_target_name>`。与 BUILD_TYPE 一起使用时，这将产生类似 "bundle_target_name_Debug.zip" 的文件名。
- PRIVATE_LIBRARIES: 需要包含在捆绑包中的私有库。 被指定的库将添加到 "Private-Library" 清单声明中，并添加在捆绑包的根目录下。库可以是 CMake 库目标，或者现有库的绝对路径。
- HEADERS: 附加的清单属性，将被添加到捆绑包清单中。
- DO_NOT_CONFIGURE_SYMBOL_VISIBILITY: 默认情况下，捆绑包库将使用隐藏的符号可见性配置进行构建。 提供该选项可以禁用此功能。

## celix_bundle_private_libs
将库添加到捆绑包中。

```CMake
celix_bundle_private_libs(<bundle_target>
    lib1 lib2 ...
)
```

示例：
```
celix_bundle_private_libs(my_bundle my_lib1 my_lib2)
```

一个库应当是一个CMake库目标或一个指向现有库的绝对路径。
这些库将会被打包到捆绑包的zip文件中，并且激活器库将会以私有的方式链接到它们。

Apache Celix 使用设置了RTLD_LOCAL的 dlopen 来载入捆绑包里的激活器库。
要注意，dlopen 总会载入激活器库，但并不总是载入捆绑包激活器库所链接的库。
如果激活器库链接到一个已经被载入的库，那么就会使用已经载入的库。
更具体地说，dlopen 将会基于激活器库中的NEEDED标头和已经载入库的SO_NAME标头来决定这一点。

例如按顺序安装：
- 包含私有库 libfoo（SONAME=libfoo.so）的捆绑包 A 以及
- 包含私有库 libfoo（SONAME=libfoo.so）的捆绑包 B。
  这将导致 B 也使用从 A 的缓存目录中载入的 libfoo。

如果在同一个进程中创建了多个 Celix 框架实例，也适用此规则。例如按顺序安装：
- 在 Celix 框架 A 中包含私有库 libfoo（SONAME=libfoo.so）的捆绑包 A 以及
- 在 Celix 框架 B 中相同的捆绑包 A。
  这将导致框架 B 中的捆绑包 A 使用从框架 A 中捆绑包 A 的缓存目录中载入的 libfoo。

## celix_bundle_files
将文件添加到目标捆绑包。

```CMake
celix_bundle_files(<bundle_target>
    files... DESTINATION <dir>
    [FILE_PERMISSIONS permissions...]
    [DIRECTORY_PERMISSIONS permissions...]
    [NO_SOURCE_PERMISSIONS] [USE_SOURCE_PERMISSIONS]
    [FILES_MATCHING]
    [PATTERN <pattern> | REGEX <regex>]
    [EXCLUDE] [PERMISSIONS permissions...] [...]
)
```

示例：
```CMake
celix_bundle_files(my_bundle ${CMAKE_CURRENT_LIST_DIR}/resources/my_file.txt DESTINATION META-INF/subdir)
```

DESTINATION 是相对于捆绑包存档根目录的。
其余的命令基于 file(COPY ...) CMake 命令。
有关更多信息，请参见 CMake file(COPY ...) 命令。

注意，使用 celix_bundle_files 时，文件将在 CMake 生成阶段被复制。
若后续文件内容有更新，更新的内容将不会打包到捆绑包中！！

## celix_bundle_add_dir
将目录的内容复制到一个捆绑包中。

```CMake
celix_bundle_add_dir(<bundle_target> <input_dir>
    [DESTINATION <relative_path_in_bundle>]
)
```

示例：
```CMake
celix_bundle_add_dir(my_bundle bundle_resources/ DESTINATION "resources")
```

可选参数：
- DESTINATION：文件的目标位置，相对于bundle存档根目录。默认为"."。

请注意，celix_bundle_add_dir会复制目录并可以追踪变化。

## celix_bundle_add_files
将指定的文件复制到捆绑包中。

```CMake
celix_bundle_add_files(<bundle_target>
    [FILES <file1> <file2> ...]
    [DESTINATION <relative_path_in_bundle>]
)
```

示例：
```CMake
celix_bundle_add_files(my_bundle FILES my_file1.txt my_file2.txt DESTINATION "resources")
```

可选参数:
- FILES: 需要复制到捆绑包的文件。
- DESTINATION: 相对于捆绑包存档根目录的文件目的地。默认值是“.”。

注意，celix_bundle_add_files会复制文件并可以跟踪更改。

## celix_bundle_headers
将提供的清单属性追加到目标包清单中。

```CMake
celix_bundle_headers(<bundle_target>
    "header1: header1_value"
    "header2: header2_value"
    ...
)
```

## celix_bundle_symbolic_name
设置捆绑包符号名

```CMake
celix_bundle_symbolic_name(<bundle_target> symbolic_name)
```

## celix_get_bundle_symbolic_name
从（导入的）捆绑包目标获取捆绑包符号名。

```CMake
celix_get_bundle_symbolic_name(<bundle_target> VARIABLE_NAME)
```

示例： `celix_get_bundle_symbolic_name(Celix::shell SHELL_BUNDLE_SYMBOLIC_NAME)`

## celix_bundle_name
设置捆绑包名称。

```CMake
celix_bundle_name(<bundle_target> name)
```

## celix_bundle_version
设置捆绑包版本。

```CMake
celix_bundle_version(<bundle_target> version)
```

## celix_bundle_description
设置捆绑包描述。

```CMake
celix_bundle_description(<bundle_target> description)
```

## celix_bundle_group
设置捆绑包所属的组。

```CMake  
celix_bundle_group(<bundle_target> bundle group)
```

## celix_get_bundle_filename
根据使用的CMAKE_BUILD_TYPE和可用的bundle配置，从（导入的）捆绑包目标获取捆绑包文件名。

```CMake
celix_get_bundle_filename(<bundle_target> VARIABLE_NAME)
```

示例：
```CMake
celix_get_bundle_filename(Celix::shell SHELL_BUNDLE_FILENAME)
``` 

## celix_get_bundle_file
根据使用的CMAKE_BUILD_TYPE和可用的bundle配置，从（导入的）捆绑包目标获取捆绑包文件（捆绑包的绝对路径）。

```CMake
celix_get_bundle_file(<bundle_target> VARIABLE_NAME)
```

示例：
```CMake
celix_get_bundle_file(Celix::shell SHELL_BUNDLE_FILE)
```

## install_celix_bundle
在执行`make install`时安装捆绑包。

```CMake
install_celix_bundle(<bundle_target>
    [EXPORT] export_name
    [PROJECT_NAME] project_name
    [BUNDLE_NAME] bundle_name
    [HEADERS header_file1 header_file2 ...]
    [RESOURCES resource1 resource2 ...]
)
```

捆绑包安装在 `<install-prefix>/share/<project_name>/bundles`.
头文件安装在 `<install-prefix>/include/<project_name>/<bundle_name>`
资源安装在 `<install-prefix>/shared/<project_name>/<bundle_name>`

可选的参数:
- EXPORT: 将安装的捆绑包与一个导出名称相关联。
  该导出名称可被用来生成一个 CelixTargets.cmake 文件 (参见 install_celix_bundle_targets)
- PROJECT_NAME: 安装时的项目名。默认值是 CMake 项目名。
- BUNDLE_NAME: 安装头文件/资源时使用的捆绑包名称。默认值是捆绑包目标名。
- HEADERS: 要为捆绑包安装的头文件列表。
- RESOURCES: 要为捆绑包安装的资源文件列表。

## install_celix_targets
生成并安装一个包含CMake指令的Celix目标 CMake文件。
这些指令将为与给定导出名称相关联的捆绑包创建CMake导入目标。
可以通过这些CMake导入目标在CMake项目中使用这些已安装的捆绑包。

```CMake
install_celix_targets(<export_name>
    NAMESPACE <namespace>
    [FILE <celix_target_filename>]
    [PROJECT_NAME <project_name>]
    [DESTINATION <celix_targets_destination>]
)
```

示例：
```CMake
install_celix_targets(celix NAMESPACE Celix:: DESTINATION share/celix/cmake FILE CelixTargets)
```

可选参数：
- FILE：待生成的Celix目标 CMake文件名，不包括cmake扩展名。默认是 <export_name>BundleTargets
- PROJECT_NAME：用于设置share子目录的项目名称。默认是CMake项目名称。
- DESTINATION：安装Celix目标 CMake文件的（相对）位置。默认是 share/<PROJECT_NAME>/cmake。

# Celix 容器
Celix 容器是用来按照预先配置的属性拉起 Celix 框架实例的可执行文件，它会安装或安装并启动一组预先配置的捆绑包。

## add_celix_container
添加一个Celix容器，它由一组选定的捆绑包和一个Celix启动器组成。
Celix容器可用于启动一个Celix框架以及一组捆绑包。

Celix容器会在 `<cmake_build_dir>/deploy[/<group_name>]/<celix_container_name>` 中构建。
可使用 `<celix_container_name>` 可执行文件来运行容器。

'add_celix_container'有三种变体：
- 如果未指定启动器，则会生成一个自定义的Celix启动器。这个启动器还包含了配置的属性。
- 如果提供了LAUNCHER_SRC，则将使用提供的源代码构建一个Celix启动器。可以使用CMake的'target_sources'命令添加额外的源代码。
- 如果提供了一个LAUNCHER（CMake `add_executable` 目标的绝对路径），那么它将作为Celix启动器使用。

使用'add_celix_container'创建一个Celix容器会生成一个CMake可执行目标（除非设置了LAUNCHER）。
这些目标可用于从IDE（如果IDE支持CMake）运行/调试Celix容器。

可选参数：
- COPY：使用此选项，容器中的捆绑包将被复制到容器可执行文件旁的一个目录，且容器将以文件名从此目录加载捆绑包。 不能同时提供 COPY 或 NO_COPY 选项。默认为 COPY。
- NO_COPY：使用此选项，容器将使用捆绑包 zip 文件的绝对路径从捆绑包的安装位置加载捆绑包。不能同时提供 COPY 或 NO_COPY 选项。默认为 COPY。
- CXX：使用此选项，Celix 启动器将使用 C++ 生成（如果需要生成）。这确保 Celix 启动器链接到 stdlibc++。只能提供 C 或 CXX 中的一个选项。默认为 CXX。
- C：使用此选项，Celix 启动器将使用 C 生成（如果需要生成）。只能提供 C 或 CXX 中的一个选项。默认为 CXX。
- FAT：使用此选项，将只允许向容器添加有内嵌捆绑包。确保容器可执行文件不依赖于外部的捆绑包 zip 文件。请注意，此选项不会改变容器的任何内容。
- USE_CONFIG：使用此选项，配置属性将在 'config.properties' 中生成，而不是嵌入到 Celix 启动器中。
- GROUP：若配置，构建位置将以 GROUP 为前缀。默认为空。
- NAME：可执行文件的名称。默认为 <celix_container_name>。只对自动生成或从 LAUNCHER_SRC 生成的Celix 启动器有用。
- DIR：Celix 容器的基本构建目录。默认为 `<cmake_build_dir>/deploy`。
- BUNDLES：Celix 容器中要安装并启动的捆绑包列表。这些捆绑包将被配置为运行级别3。有关更多信息，请参阅 'celix_container_bundles'。
- INSTALL_BUNDLES：Celix 容器中要安装（但不启动）的捆绑包列表。
- EMBEDDED_BUNDLES：要嵌入到 Celix 容器可执行文件中，并被 Celix 容器安装和启动的捆绑包列表。关于内嵌捆绑包的更多信息，请参阅 `celix_target_embedded_bundle`。
- INSTALL_EMBEDDED_BUNDLES：要嵌入到 Celix 容器可执行文件中，并被 Celix 容器安装（但不启动）的捆绑包列表。关于内嵌捆绑包的更多信息，请参阅 `celix_target_embedded_bundle`。
- PROPERTIES：配置属性列表，可以用于配置 Celix 框架和/或捆绑包。一般作为 EMBEDED_PROPERTIES使用，但若设置了 USE_CONFIG 选项，这将作为 RUNTIME_PROPERTIES使用。请查看框架库或捆绑包文档获取可用的配置选项。
- EMBEDDED_PROPERTIES：嵌入到自动生成的 Celix 启动器（如果有的话）中的配置属性列表。
- RUNTIME_PROPERTIES：添加到生成的config.properties文件中的配置属性列表。

```CMake
add_celix_container(<celix_container_name>
    [COPY]
    [NO_COPY]
    [CXX]
    [C]
    [FAT]
    [USE_CONFIG]
    [GROUP group_name]
    [NAME celix_container_name]
    [DIR dir]
    [BUNDLES <bundle1> <bundle2> ...]
    [INSTALL_BUNDLES <bundle1> <bundle2> ...]
    [EMBEDDED_BUNDLES <bundle1> <bundle2> ...]
    [INSTALL_EMBEDDED_BUNDLES <bundle1> <bundle2> ...]
    [PROPERTIES "prop1=val1" "prop2=val2" ...]
    [EMBEDDED_PROPERTIES "prop1=val1" "prop2=val2" ...]
    [RUNTIME_PROPERTIES "prop1=val1" "prop2=val2" ...]
)
```

```CMake
add_celix_container(<celix_container_name>
    LAUNCHER launcher
    [COPY]
    [NO_COPY]
    [CXX]
    [C]
    [FAT]
    [USE_CONFIG]
    [GROUP group_name]
    [NAME celix_container_name]
    [DIR dir]
    [BUNDLES <bundle1> <bundle2> ...]
    [INSTALL_BUNDLES <bundle1> <bundle2> ...]
    [EMBEDDED_BUNDLES <bundle1> <bundle2> ...]
    [INSTALL_EMBEDDED_BUNDLES <bundle1> <bundle2> ...]
    [PROPERTIES "prop1=val1" "prop2=val2" ...]
    [EMBEDDED_PROPERTIES "prop1=val1" "prop2=val2" ...]
    [RUNTIME_PROPERTIES "prop1=val1" "prop2=val2" ...]
)
```

```CMake
add_celix_container(<celix_container_name>
    LAUNCHER_SRC launcher_src
    [COPY]
    [NO_COPY]
    [CXX]
    [C]
    [FAT]
    [USE_CONFIG]
    [GROUP group_name]
    [NAME celix_container_name]
    [DIR dir]
    [BUNDLES <bundle1> <bundle2> ...]
    [INSTALL_BUNDLES <bundle1> <bundle2> ...]
    [EMBEDDED_BUNDLES <bundle1> <bundle2> ...]
    [INSTALL_EMBEDDED_BUNDLES <bundle1> <bundle2> ...]
    [PROPERTIES "prop1=val1" "prop2=val2" ...]
    [EMBEDDED_PROPERTIES "prop1=val1" "prop2=val2" ...]
    [RUNTIME_PROPERTIES "prop1=val1" "prop2=val2" ...]
)
```

示例：
```CMake
#Creates a Celix container in ${CMAKE_BINARY_DIR}/deploy/simple_container which starts 3 bundles located at
#${CMAKE_BINARY_DIR}/deploy/simple_container/bundles.
add_celix_container(simple_container
    BUNDLES
        Celix::shell
        Celix::shell_tui
        Celix::log_admin
    PROPERTIES
        CELIX_LOGGING_DEFAULT_ACTIVE_LOG_LEVEL=debug
)
```

```CMake
#Creates a "fat" Celix container in ${CMAKE_BINARY_DIR}/deploy/simple_fat_container which starts 3 bundles embedded
#in the container executable.
add_celix_container(simple_fat_container
        FAT
        EMBEDDED_BUNDLES
        Celix::shell
        Celix::shell_tui
        Celix::log_admin
        PROPERTIES
        CELIX_LOGGING_DEFAULT_ACTIVE_LOG_LEVEL=debug
        )
```

## celix_container_bundles
将一些捆绑包添加到给定 Celix 容器中。

```CMake
celix_container_bundles(<celix_container_target_name>
      [COPY]
      [NO_COPY]
      [LEVEL (0..6)]
      [INSTALL]
      bundle1
      bundle2
      ...
)
```

示例：
```CMake
celix_container_bundles(my_container Celix::shell Celix::shell_tui)
```

所选捆绑包（如果已配置）会被复制到容器构建目录，并添加到配置属性中，以便在执行 Celix 容器时进行安装和启动。

Celix 框架支持7个（从0到6）运行级别。运行级别可以用来控制捆绑包的启停顺序。
运行级别0的捆绑包最先启动，运行级别6捆绑包最后启动。
当框架停止时，运行级别6的捆绑包最先停止，运行级别0的捆绑包最后停止。
在一个运行级别内，配置的顺序决定了启动顺序；先添加的捆绑包先启动。

可选参数：
- LEVEL：添加捆绑包的运行级别。默认为3级。
- INSTALL：若存在此选项，则捆绑包只会被安装，而非默认的安装并启动。
  它们将在所有运行级别为0到6的捆绑包安装并启动后安装。
- COPY：若设置了此选项，捆绑包将被复制到容器构建目录。此选项覆盖了在 add_celix_container 调用中使用的 NO_COPY 选项。
- NO_COPY：若设置了此选项，容器将使用绝对路径安装/启动捆绑包。此选项覆盖了在 add_celix_container 调用中可选使用的 COPY 选项。

## celix_container_embedded_bundles
将一些捆绑包嵌入到给定的 Celix 容器中。

```CMake
celix_container_embedded_bundles(<celix_container_target_name>
    [LEVEL (0..6)]
    [INSTALL]
    bundle1
    bundle2
    ...
)
```

示例：
```CMake
celix_container_embedded_bundles(my_container Celix::shell Celix::shell_tui)
```

使用 `celix_target_embedded_bundle` CMake命令将一组捆绑包嵌入到容器可执行文件中，并将其添加到配置属性中，以便在执行Celix容器时进行安装和启动。

请参阅 `celix_target_embedded_bundle` 以了解如何在可执行文件中嵌入捆绑包。

Celix 框架支持7个（从0到6）运行级别。运行级别可以用来控制捆绑包的启停顺序。
运行级别0的捆绑包最先启动，运行级别6捆绑包最后启动。
当框架停止时，运行级别6的捆绑包最先停止，运行级别0的捆绑包最后停止。
在一个运行级别内，配置的顺序决定了启动顺序；先添加的捆绑包先启动。

可选参数:
- LEVEL: 添加捆绑包的运行级别。默认为3级。
- INSTALL：若存在此选项，则捆绑包只会被安装，而非默认的安装并启动。
  它们将在所有运行级别为0到6的捆绑包安装并启动后安装。

## celix_container_properties
将提供的属性添加到目标Celix容器配置属性中。
如果设置了USE_CONFIG选项，这些配置属性将被添加到'config.properties'文件中，否则它们将被添加到生成的Celix启动器中。

```CMake
celix_container_properties(<celix_container_target_name>
    "prop1=val1"
    "prop2=val2"
    ...
)
```

## celix_container_embedded_properties
将所提供的属性添加到目标Celix容器配置属性中。
这些属性将被嵌入到生成的Celix启动器中。

```CMake
celix_container_embedded_properties(<celix_container_target_name>
    "prop1=val1" 
    "prop2=val2" 
    ...
)
```

## celix_container_runtime_properties
将提供的属性添加到目标Celix容器配置属性中。
这些属性将被添加到容器构建目录下的config.properties文件中。

```CMake
celix_container_runtime_properties(<celix_container_target_name>
    "prop1=val1" 
    "prop2=val2" 
    ...
)
```

# 用于通用CMake目标的Celix CMake命令
Celix提供了几个CMake命令，可在通用CMake目标（如可执行文件，共享库等）上进行操作。

Celix的CMake命令将始终使用带关键字签名（`PRIVATE`，`PUBLIC`，`INTERFACE`）版本的CMake命令进行链接、添加源文件等操作。
这意味着这些命令无法被用在由"all-plain" CMake 命令所创建的目标上。

## add_celix_bundle_dependencies
将指定捆绑包添加为 CMake 目标的依赖项，这样在该目标之前将创建这些捆绑包的 zip 文件。

```CMake
add_celix_bundle_dependencies(<cmake_target>
    bundles...
)
```

```CMake
add_celix_bundle_dependencies(my_exec my_bundle1 my_bundle2)
```

## celix_target_embedded_bundle
将 Celix 捆绑包嵌入到 CMake 目标对应的二进制文件中。

```CMake
celix_target_embedded_bundle(<cmake_target>
        BUNDLE <bundle>
        [NAME <name>]
        )
```

示例：
```CMake
celix_target_embedded_bundle(my_executable
        BUNDLE Celix::shell
        NAME celix_shell
        )
# 以下符号：
# - celix_embedded_bundle_celix_shell_start
# - celix_embedded_bundle_celix_shell_end
# - celix_embedded_bundles = "embedded://celix_shell"
# 将被添加到 `my_executable`
```

Celix 捆绑包将会被嵌入到 CMake 目标对应的可执行文件中，且位于符号`celix_embedded_bundle_${NAME}_start`和`celix_embedded_bundle_${NAME}_end`之间。

另外还会添加或更新一个名为`celix_embedded_bundles`的`const char * const`符号，其中包含一个由`,`分隔的内嵌 Celix 捆绑包 URL 列表。
URL格式为：`embedded://${NAME}`。

在Linux上，使用链接标志 `--export-dynamic` 以确保上述符号可以通过 `dlsym` 访问。

必要的参数:
- BUNDLE: 要嵌入到指定可执行文件的捆绑包目标或捆绑包文件绝对路径。

可选的参数：
- NAME: 内嵌捆绑包名称。这个名称用于生成 _start 和 _end 符号以及内嵌捆绑包的 URL。
  对于捆绑包的 CMake 目标，默认为捆绑包符号名，对于捆绑包文件，默认为不含扩展名的捆绑包文件名。NAME 必须是一个有效的 C 语言标识符。

可以使用`celix_bundleContext_installBundle` (C) 或 `celix::BundleContext::installBundle` (C++) 来安装/启动由内嵌捆绑包URL（格式为"embedded://${NAME}"）指定的捆绑包。
也可以使用框架工具函数 `celix_framework_utils_installEmbeddedBundles` (C) 或 `celix::installEmbeddedBundles` (C++) 来安装可执行文件中的全部内嵌捆绑包。

## celix_target_embedded_bundles
将多个 Celix 捆绑包嵌入到 CMake 目标对应的二进制文件中。

```CMake
celix_target_embedded_bundles(<cmake_target> [<bundle1> <bundle2> ...])
```

示例：
```CMake
celix_target_embedded_bundles(my_executable Celix::shell Celix::shell_tui)
```

若捆绑包为 CMake 目标，则使用捆绑包符号名作为内嵌捆绑包名称；若捆绑包为文件，则使用不含扩展名的捆绑包文件名作为内嵌捆绑包名称。

## celix_target_bundle_set_definition
为给定目标定义一个捆绑包集合，并为该集合新增一个编译宏定义（内容为逗号分割的捆绑包路径列表），并将该集合中的捆绑包全部添加为给定目标的依赖项。

```CMake
celix_target_bundle_set_definition(<cmake_target>
        NAME <set_name>
        [<bundle1> <bundle2>..]
        )
```

示例：
```CMake
celix_target_bundle_set_definition(test_example NAME TEST_BUNDLES Celix::shell Celix::shell_tui)
```

新增编译宏名称为`${NAME}`，其内容为逗号分隔的捆绑包路径列表。
可使用Celix框架工具函数 `celix_framework_utils_installBundleSet`（C） 或 `celix::installBundleSet`（C++）来安装该捆绑包集合。

捆绑包集合编译宏对于测试来说很有用。

## celix_target_hide_symbols
将给定目标的符号可见性预设为隐藏。

这是通过将目标属性 C_VISIBILITY_PRESET 设为隐藏，CXX_VISIBILITY_PRESET 设为隐藏以及 VISIBILITY_INLINES_HIDDEN 设为 ON 来完成的。

```CMake
celix_target_hide_symbols(<cmake_target> [RELEASE] [DEBUG] [RELWITHDEBINFO] [MINSIZEREL])
```

可选参数包括：
- RELEASE：为发布构建隐藏符号
- DEBUG：为调试构建隐藏符号
- RELWITHDEBINFO：为带调试信息的发布构建隐藏符号
- MINSIZEREL：为最小尺寸发布构建隐藏符号

如果未提供可选参数，则为所有类型构建隐藏符号。

示例：
```CMake
celix_target_hide_symbols(my_bundle RELEASE MINSIZEREL)
```