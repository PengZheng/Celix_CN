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

# Celix Containers
Celix containers are executables preconfigured to start a Celix framework with a set of configuration properties 
and a set of bundles to install or install and start. 

## add_celix_container
Add a Celix container, consisting out of a selection of bundles and a Celix launcher.
Celix containers can be used to start a Celix framework together with a selection of bundles.

A Celix container will be build in `<cmake_build_dir>/deploy[/<group_name>]/<celix_container_name>`.
Use the `<celix_container_name>` executable to run the containers.

There are three variants of 'add_celix_container':
- If no launcher is specified a custom Celix launcher will be generated. This launcher also contains the configured properties.
- If a LAUNCHER_SRC is provided a Celix launcher will be build using the provided sources. Additional sources can be added with the
  CMake 'target_sources' command.
- If a LAUNCHER (absolute path to a executable of CMake `add_executable` target) is provided that will be used as Celix launcher.

Creating a Celix containers using 'add_celix_container' will lead to a CMake executable target (expect if a LAUNCHER is used).
These targets can be used to run/debug Celix containers from a IDE (if the IDE supports CMake).

Optional Arguments:
- COPY: With this option the bundles used in the container will be copied in and configured for a bundles directory
  next to the container executable. Only one of the COPY or NO_COPY options can be provided.
  Default is COPY.
- NO_COPY: With this option the bundles used in the container will be configured using absolute paths to the bundles
  zip files. Only one of the COPY or NO_COPY options can be provided.
  Default is COPY.
- CXX: With this option the generated Celix launcher (if used) will be a C++ source.
  This ensures that the Celix launcher is linked against stdlibc++. Only one of the C or CXX options can be provided.
  Default is CXX
- C: With this option the generated Celix launcher (if used) will be a C source. Only one of the C or CXX options can
  be provided.
  Default is CXX
- FAT: With this option only embedded bundles are allowed to be added to the container. Ensuring a container executable
  this is not dependent on external bundle zip files.
  Note that this option does not change anything to the container, it just ensure that all added bundles are embedded
  bundles.
- USE_CONFIG: With this option the config properties are generated in a 'config.properties' instead of embedded in
  the Celix launcher.
- GROUP: If configured the build location will be prefixed the GROUP. Default is empty.
- NAME: The name of the executable. Default is <celix_container_name>. Only useful for generated/LAUNCHER_SRC
  Celix launchers.
- DIR: The base build directory of the Celix container. Default is `<cmake_build_dir>/deploy`.
- BUNDLES: A list of bundles for the Celix container to install and start.
  These bundle will be configured for run level 3. See 'celix_container_bundles' for more info.
- INSTALL_BUNDLES: A list of bundles for the Celix container to install (but not start).
- EMBEDDED_BUNDLES: A list of bundles to embed in the Celix container (inject as binary in the executable) and
  to install and start for the Celix container.
  See `celix_target_embedded_bundle` for more info about embedded bundles.
- INSTALL_EMBEDDED_BUNDLES: A list of bundles to embed in the Celix container (inject as binary in the executable) and
  to install (but not start) for the Celix container.
  See `celix_target_embedded_bundle` for more info about embedded bundles.
- PROPERTIES: A list of configuration properties, these can be used to configure the Celix framework and/or bundles.
  Normally this will be EMBEDED_PROPERTIES, but if the USE_CONFIG option is used this will be RUNTIME_PROPERTIES.
  See the framework library or bundles documentation about the available configuration options.
- EMBEDDED_PROPERTIES: A list of configuration properties which will be used in the generated Celix launcher.
- RUNTIME_PROPERTIES: A list of configuration properties which will be used in the generated config.properties file.

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

Examples:
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
Add a selection of bundles to the Celix container.

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

Example:
```CMake
celix_container_bundles(my_container Celix::shell Celix::shell_tui)
```

The selection of  bundles are (if configured) copied to the container build dir and
are added to the configuration properties so that they are installed and started when the Celix container is executed.

The Celix framework supports 7 (0 - 6) run levels. Run levels can be used to control the start and stop order of bundles.
Bundles in run level 0 are started first and bundles in run level 6 are started last.
When stopping bundles in run level 6 are stopped first and bundles in run level 0 are stopped last.
Within a run level the order of configured decides the start order; bundles added earlier are started first.

Optional Arguments:
- LEVEL: The run level for the added bundles. Default is 3.
- INSTALL: If this option is present, the bundles will only be installed instead of the default install and start.
  The bundles will be installed after all bundle in LEVEL 0..6 are installed and started.
- COPY: If this option is present, the bundles will be copied to the container build dir. This option overrides the
  NO_COPY option used in the add_celix_container call.
- NO_COPY: If this option is present, the install/start bundles will be configured using a absolute path to the
  bundle. This option overrides optional COPY option used in the add_celix_container call.

## celix_container_embedded_bundles
Embed a selection of bundles to the Celix container.

```CMake
celix_container_embedded_bundles(<celix_container_target_name>
    [LEVEL (0..6)]
    [INSTALL]
    bundle1
    bundle2
    ...
)
```

Example:
```CMake
celix_container_embedded_bundles(my_container Celix::shell Celix::shell_tui)
```

The selection of bundles are embedded in the container executable using the
`celix_target_embedded_bundle` Celix CMake command and are added to the configuration properties so that they are
installed and started when the Celix container is executed.

See `celix_target_embedded_bundle` for how bundle is embedded in a executable.

The Celix framework supports 7 (0 - 6) run levels. Run levels can be used to control the start and stop order of bundles.
Bundles in run level 0 are started first and bundles in run level 6 are started last.
When stopping bundles in run level 6 are stopped first and bundles in run level 0 are stopped last.
Within a run level the order of configured decides the start order; bundles added earlier are started first.

Optional Arguments:
- LEVEL: The run level for the added bundles. Default is 3.
- INSTALL: If this option is present, the bundles will only be installed instead of the default install and start.
  The bundles will be installed after all bundle in LEVEL 0..6 are installed and started.

## celix_container_properties
Add the provided properties to the target Celix container config properties.
If the USE_CONFIG option is used these configuration properties will be added to the 'config.properties' file else they
will be added to the generated Celix launcher.

```CMake
celix_container_properties(<celix_container_target_name>
    "prop1=val1"
    "prop2=val2"
    ...
)
```

## celix_container_embedded_properties
Add the provided properties to the target Celix container config properties.
These properties will be embedded into the generated Celix launcher.

```CMake
celix_container_embedded_properties(<celix_container_target_name>
    "prop1=val1" 
    "prop2=val2" 
    ...
)
```

## celix_container_runtime_properties
Add the provided properties to the target Celix container config properties.
These properties will be added to the config.properties in the container build dir. 

```CMake
celix_container_runtime_properties(<celix_container_target_name>
    "prop1=val1" 
    "prop2=val2" 
    ...
)
```

# Celix CMake commands for generic CMake targets
Celix provides several CMake commands that operate on the generic CMake targets (executable, shared library, etc). 

Celix CMake commands for generic CMake target will always use the keyword signature (`PRIVATE`, `PUBLIC`, `INTERFACE`) 
version for linking, adding sources, etc. This means that these command will not work on targets created with
an "all-plain" CMake version command.

## add_celix_bundle_dependencies
Add bundles as dependencies to a cmake target, so that the bundle zip files will be created before the cmake target.

```CMake
add_celix_bundle_dependencies(<cmake_target>
    bundles...
)
```

```CMake
add_celix_bundle_dependencies(my_exec my_bundle1 my_bundle2)
```

## celix_target_embedded_bundle
Embeds a Celix bundle into a CMake target.

```CMake
celix_target_embedded_bundle(<cmake_target>
        BUNDLE <bundle>
        [NAME <name>]
        )
```

Example:
```CMake
celix_target_embedded_bundle(my_executable
        BUNDLE Celix::shell
        NAME celix_shell
        )
# result in the symbols:
# - celix_embedded_bundle_celix_shell_start
# - celix_embedded_bundle_celix_shell_end
# - celix_embedded_bundles = "embedded://celix_shell"
# to be added to `my_executable`
```

The Celix bundle will be embedded into the CMake target between the symbols: `celix_embedded_bundle_${NAME}_start` and
`celix_embedded_bundle_${NAME}_end`.

Also a `const char * const` symbol with the name `celix_embedded_bundles` will be added or updated containing a `,`
seperated list of embedded Celix bundle urls. The url will be: `embedded://${NAME}`.

For Linux the linking flag `--export-dynamic` is added to ensure that the previous mentioned symbols can be retrieved
using `dlsym`.

Mandatory Arguments:
- BUNDLE: The bundle target or bundle file (absolute path) to embed in the CMake target.

Optional Arguments:
- NAME: The name to use when embedding the Celix bundle. This name is used in the _start and _end symbol, but also
  for the embedded bundle url.
  For a bundle CMake target the default is the bundle symbolic name and for a bundle file the default is the
  bundle filename without extension. The NAME must be a valid C identifier.

Bundles embedded in an executable can be installed/started using the bundle url: "embedded://${NAME}" in
combination with `celix_bundleContext_installBundle` (C) or `celix::BundleContext::installBundle` (C++).
All embedded bundle can be installed using the framework utils function
`celix_framework_utils_installEmbeddedBundles` (C) or `celix::installEmbeddedBundles` (C++).

## celix_target_embedded_bundles
Embed multiple Celix bundles into a CMake target.

```CMake
celix_target_embedded_bundles(<cmake_target> [<bundle1> <bundle2> ...])
```

Example:
```CMake
celix_target_embedded_bundles(my_executable Celix::shell Celix::shell_tui)
```

The bundles will be embedded using their symbolic name if the bundle is a CMake target or their filename (without
extension) if the bundle is a file (absolute path).

## celix_target_bundle_set_definition
Add a compile-definition with a set of comma seperated bundles paths to a target and also adds the bundles as 
dependency to the target.

```CMake
celix_target_bundle_set_definition(<cmake_target>
        NAME <set_name>
        [<bundle1> <bundle2>..]
        )
```

Example:
```CMake
celix_target_bundle_set_definition(test_example NAME TEST_BUNDLES Celix::shell Celix::shell_tui)
```

The compile-definition will have the name `${NAME}` and will contain a `,` separated list of bundle paths.
The bundle set can be installed using the Celix framework util function `celix_framework_utils_installBundleSet` (C)
or `celix::installBundleSet` (C++).

Adding a compile-definition with a set of bundles can be useful for testing purpose.

## celix_target_hide_symbols
Configure the symbol visibility preset of the provided target to hidden.

This is done by setting the target properties C_VISIBILITY_PRESET to hidden, the CXX_VISIBILITY_PRESET to hidden and
VISIBILITY_INLINES_HIDDEN to ON.

```CMake
celix_target_hide_symbols(<cmake_target> [RELEASE] [DEBUG] [RELWITHDEBINFO] [MINSIZEREL])
```

Optional arguments are:
- RELEASE: hide symbols for the release build type
- DEBUG: hide symbols for the debug build type
- RELWITHDEBINFO: hide symbols for the relwithdebinfo build type
- MINSIZEREL: hide symbols for the minsizerel build type

If no optional arguments are provided, the symbols are hidden for all build types.

Example:
```CMake
celix_target_hide_symbols(my_bundle RELEASE MINSIZEREL)
```