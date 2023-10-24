---
title: 使用 CLion 构建和开发 Apache Celix
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

# 使用 CLion 构建和开发 Apache Celix
Apache Celix 可以使用 Conan 包管理器在 CLion 中进行构建和开发。
Conan 将管理 Apache Celix 的依赖项的构建并为这些依赖项生成 Find<package> 文件。

Conan 还会生成 `conanrun.sh` 和 `deactivate_conanrun.sh` 脚本，这些脚本会设置、反设置可执行程序的运行时环境（即配置`PATH` 和 `LD_LIBRARY_PATH`/`DYLD_LIBRARY_PATH`以找到 Conan 缓存中的二进制依赖）。

## 配置 CLion 支持 C++17
Celix中的C++代码默认配置为C++14。 对于部分 C++17 模块和测试代码，CMAKE_CXX_STANDARD 变量在各自的 CMakelists.txt 文件中被手动更改为 C++17 （`set(CMAKE_CXX_STANDARD 17`）。

这样做的缺点是，CLion 似乎只考虑到了顶级的 CMAKE_CXX_STANDARD 值。
为确保 CLion 为 C++17 提供正确的语法支持，请在 `文件` -> `设置` -> `构建，执行，部署` -> `CMake` 的 `CMake 选项`中添加 `-DCMAKE_CXX_STANDARD=17`。

## 设置构建目录
```shell
# 克隆 git 仓库
git clone https://github.com/apache/celix.git
cd celix

#如果需要，设置 conan 的 default 和 debug 配置文件
conan profile new default --detect
conan profile new debug --detect
conan profile update settings.build_type=Debug debug

# 生成并配置 cmake-build-debug 目录
# 如果已启用 CLion 的 CMake Preset 集成，则 CLion 将从项目根目录的 CMakeUserPresets.json 文件中加载可用的 CMake Profiles
# 该文件由 `conan install` 生成。
conan install . celix/2.3.0 -pr:b default -pr:h debug -if cmake-build-debug/ -o celix:enable_testing=True -o celix:enable_address_sanitizer=True -o celix:build_all=True -b missing

# 构建（可选）
conan build . -bf cmake-build-debug/

# 设置运行时环境并运行测试（可选）
cd cmake-build-debug
source conanrun.sh 
ctest --verbose
# 反设置运行时环境
source deactivate_conanrun.sh 
```

### 使用 Conan 2

上述内容适用于Conan 1.x。
Conan 2极大地简化了其与CLion的集成。
执行以下命令将在项目根目录生成一个CMakeUserPresets.json，CLion将自动加载以设置CMake Profiles。
接下来就可以在IDE中构建 Celix 了。

```shell
conan install . -pr:b default -pr:h default -s:h build_type=Debug -o celix/*:build_all=True -o celix/*:celix_cxx17=True -o celix/*:enable_testing=True -b missing  -o celix/*:enable_address_sanitizer=True -of cmake-build-debug
```

## 配置 CLion
为确保在运行测试/其他二进制程序时能找到所有 Conan 构建依赖，需要更改 CLion 的运行/调试配置。

可以在菜单 "运行->编辑配置..."中选择 "编辑配置模板..." 然后更新 "Google 测试" 模板，以便在
"环境变量" 项中引用 Conan 生成的脚本`conanrun.sh`。

假设 Apache Celix CMake 构建目录是 `home/joe/workspace/celix/cmake-build-debug`，那么 "环境变量" 的值应为: `source /home/joe/workspace/celix/cmake-build-debug/conanrun.sh`

![Configure CLion](media/clion_run_configuration_template.png)
