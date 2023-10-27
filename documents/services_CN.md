---
title: Apache Celix 服务
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

# Apache Celix 服务
Apache Celix 服务是一个关联了一组属性（元数据）并注册到 Celix 框架的指针。
可以动态地注册服务到 Apache Celix 框架中，并从其中查找已注册服务。

按照约定，Apache Celix 中的 C 服务是指向包含函数指针的结构体的指针，而 C++ 服务是指向一个实现（纯）抽象类的对象的指针
（可以提供为 `std::shared_ptr`）。

服务总是在一个服务名称下注册，该服务名称可用于查找服务。
对于 C 服务，服务名称必须由用户提供。
对于 C++ 服务，服务名称可以由用户提供，若没有提供，那么将会从服务模板参数中（使用 `celix::typeName<I>`) 自动推导。

注意，遵循 Java OSGi 规范，服务名称在服务属性中的属性名称是 `objectClass`，
还要注意，对于 Celix - 与 Java OSGi 不同 - 只能在单次服务注册中向 Celix 框架注册一个接口。
这个限制是因为 C 语言不能在单个对象/指针上同时支持多组接口（带有函数指针的结构体）。

## 一个C服务示例
如上所述，Apache Celix C 服务是一个注册过的结构体指针，该结构体包含函数指针。
理想情况下，该结构体应当包含一个句柄指针以及一组函数指针，并且具有高质量的文档以形成一个定义明确的服务契约。

一个Apache Celix C 服务的简单例子是 Shell 命令服务。
C Shell 命令服务头文件看起来就像这样:
```C
//celix_shell_command.h
...
#define CELIX_SHELL_COMMAND_NAME                "command.name"
#define CELIX_SHELL_COMMAND_USAGE               "command.usage"
#define CELIX_SHELL_COMMAND_DESCRIPTION         "command.description"

#define  CELIX_SHELL_COMMAND_SERVICE_NAME       "celix_shell_command"
#define  CELIX_SHELL_COMMAND_SERVICE_VERSION    "1.0.0"

typedef struct celix_shell_command celix_shell_command_t;

/**
 * 可以通过Shell命令注册额外的Shell命令。
 * 注册此服务时应提供以下属性：
 *  - command.name：必选，命令名，例如 'lb'
 *  - command.usage：可选，描述如何使用命令的字符串，例如 'lb [-l | -s | -u]'
 *  - command.description：可选，描述命令的字符串，例如 'list bundles.'
 */
struct celix_shell_command {
    void *handle;

    /**
    * 调用Shell命令.
    * @param handle        Shell命令的句柄.
    * @param commandLine   完整的命令行（例如，对于'stop'命令-> 'stop 42'）
    * @param outStream     输出流，用于打印正常的流信息。
    * @param errorStream   错误流，用于打印错误流信息。
    * @return              命令是否成功执行。
    */
    bool (*executeCommand)(void *handle, const char *commandLine, FILE *outStream, FILE *errorStream);
};
```

服务结构体包含一个句柄指针和一个`executeCommand`函数指针，它的文档解释了注册时需要提供哪些服务属性。

该`handle`字段和`handle`函数参数应该作为一个不透明的实例（`this` / `self`）句柄，通常对每个服务实例来说都是独一无二的。
调用服务函数时，服务用户应原封不动转发该句柄，例如：
```C
celix_shell_command_t* command = ...;
command->executeCommand(command->handle, "test 123", stdout, stderr);
```

## 一个C++服务示例
如前所述，Apache Celix C++ 服务是一个注册过的指向实现抽象类的对象的指针。
服务类在理想情况下应该具备高质量的文档以提供一个明确定义的服务契约。

一个Apache Celix C++服务的简单例子是C++ Shell命令。
C++ Shell命令服务头文件看起来就像这样:
```C++
//celix/IShellCommand.h
...
namespace celix {

    /**
    * 可以利用Shell命令接口来注册额外的Celix Shell命令。
    * 注册此服务时应提供以下属性：
    *  - name: 必选，命令名，例如 'celix::lb'
    *  - usage: 可选，描述如何使用该命令的字符串，例如 'celix::lb [-l | -s | -u]'
    *  - description: 可选，描述命令的字符串，例如 'list bundles.'
    */
    class IShellCommand {
    public:
        /**
        * Shell命令服务所需的名称（服务属性）
        */
        static constexpr const char * const COMMAND_NAME = "name";

        /**
        * Shell命令服务的可选使用方法描述文本（服务属性）
        */
        static constexpr const char * const COMMAND_USAGE = "usage";

        /**
        * Shell命令服务的可选描述文本（服务属性）
        */
        static constexpr const char * const COMMAND_DESCRIPTION = "description";

        virtual ~IShellCommand() = default;

        /**
        * 调用Shell命令。
        * @param commandLine   提供的完整命令行（例如，对于'stop'命令 -> 'stop 42'）。仅在调用期间有效。
        * @param commandArgs   命令的参数列表（例如，对于"stop 42 43"命令行 -> {"42", "43"}）。仅在调用期间有效。
        * @param outStream     用于打印正常流信息的C输出流。
        * @param errorStream   用于打印错误流信息的C错误流。
        * @return              命令是否已正确执行。
        */
        virtual void executeCommand(const std::string& commandLine, const std::vector<std::string>& commandArgs, FILE* outStream, FILE* errorStream) = 0;
    };
}
```

正如 C Ｓhell 命令结构体一样，C++ 服务类也需要文档来解释注册时需要提供哪些服务属性。
C++ 服务不需要 `handle` 参数，使用 C++ 服务接口就像调用任何 C++ 对象的成员函数一样。

## 动态服务的影响
Apache Celix中的服务是动态的，这意味着它们可以在任何时刻出现或消失。
这使得基于Celix服务的出现和消失来创建新功能成为可能。
如何应对这种动态行为对于创建稳定解决方案至关重要。

Java的垃圾收集器可以保证在服务提供方（捆绑包）被卸载的情况下服务对象依然存在。
即使如此，对于Java OSGi来说，正确编程来处理服务的动态行为也是一个挑战。
考虑到C和C++没有垃圾收集机制，正确处理动态行为就变得更为关键了：如果一个提供某项服务的捆绑包被卸载，则该服务的代码段/内存分配也将被删除/释放。

Apache Celix提供了几种方式来处理这种动态行为：

* 一种内置抽象，支持通过回调函数来使用服务，Celix框架确保服务在回调执行过程中不会被删除。
* 服务跟踪器，确保服务只能在处理完所有追踪器的服务删除回调后才能完成其注销操作。
* 支持服务依赖项声明的服务组件，因此服务组件生命周期与服务依赖项的可用性相绑定。有关服务组件的更多信息，请参阅服务组件文档部分。
* Celix框架将在 Celix 事件循环上处理所有服务注册/注销事件以及跟踪器的启停，以确保每次只能处理1个事件，且服务注册/注销和服务跟踪器的回调总是来自同一线程。
* 服务注册/注销和服务跟踪器启停可以异步完成。

## 服务注册和注销
在Celix中，服务的注册和注销可以同步或异步完成，尽管同步注册（注销）服务更符合OSGi规范，但Celix更倾向于使用异步注册（注销）服务。

在同步注册服务时，会处理服务注册事件和所有由服务注册引发的事件；
实际上，这意味着当同步服务注册返回时，所有的捆绑包都知晓了新服务，并相应地更新了他们各自的服务管理上下文（如果需要的话）。

同步服务的（注销）注册可能会导致问题，例如，如果在处理当前服务注册事件时触发了另一个服务注册事件。
在这种情况下，普通的互斥量通常是不够的，需要引用计数或递归互斥量。引用计数机制可能会比较复杂（特别是在C中），而递归互斥量可以说是一个坏主意。

有趣的是，对于Java，`synchronized`的使用是递归的，因此这似乎在Java中是一个较小的问题。

当异步注册服务时，服务属性（特别是`service.id`属性）在服务注册调用返回时就已经完全确定下来（因此不会再发生变化）。
实际的服务注册事件将由Celix事件线程异步完成，这可以在服务注册调用返回之前或之后完成。

要异步注册服务，可以使用以下的C函数/C++方法：
- `celix_bundleContext_registerServiceAsync`。
- `celix_bundleContext_registerServiceWithOptionsAsync`。
- `celix::BundleContext::registerService`。
- `celix::BundleContext::registerUnmanagedService`。

要同步注册服务，可以使用以下的C函数/C++方法：
- `celix_bundleContext_registerService`。
- `celix_bundleContext_registerServiceWithOptions`。
- `celix::BundleContext::registerService`，使用`celix::ServiceRegistrationBuilder::setRegisterAsync`来配置同步注册，因为默认是异步的。
- `celix::BundleContext::registerUnmanagedService`，使用`celix::ServiceRegistrationBuilder::setRegisterAsync`来配置同步注册，因为默认是异步的。

要异步注销服务，可以使用以下的C函数：
- `celix_bundleContext_unregisterServiceAsync`。

要同步注销服务，可以使用以下的C函数：
- `celix_bundleContext_unregisterService`。

对于C++，服务注销发生在相应的`celix::ServiceRegistration`对象超出范围时。C++服务可以使用ServiceRegistrationBuilder进行同步注销的配置，特别是：
- `celix::ServiceRegistrationBuilder::setUnregisterAsync`。默认为异步。

### 示例：使用C注册服务
```C
//src/my_shell_command_provider_bundle_activator.c
#include <celix_bundle_activator.h>
#include <celix_shell_command.h>

typedef struct my_shell_command_provider_activator_data {
    celix_bundle_context_t* ctx;
    celix_shell_command_t shellCmdSvc;
    long shellCmdSvcId;
} my_shell_command_provider_activator_data_t;

static bool my_shell_command_executeCommand(void *handle, const char *commandLine, FILE *outStream, FILE *errorStream CELIX_UNUSED) {
    my_shell_command_provider_activator_data_t* data = handle;
    celix_bundle_t* bnd = celix_bundleContext_getBundle(data->ctx);
    fprintf(outStream, "Hello from bundle %s with command line '%s'\n", celix_bundle_getName(bnd), commandLine);
    return true;
}

static celix_status_t my_shell_command_provider_bundle_start(my_shell_command_provider_activator_data_t *data, celix_bundle_context_t *ctx) {
    data->ctx = ctx;
    data->shellCmdSvc.handle = data;
    data->shellCmdSvc.executeCommand = my_shell_command_executeCommand;
    
    celix_properties_t* props = celix_properties_create();
    celix_properties_set(props, CELIX_SHELL_COMMAND_NAME, "my_command");
    
    data->shellCmdSvcId = celix_bundleContext_registerServiceAsync(ctx, &data->shellCmdSvc, CELIX_SHELL_COMMAND_SERVICE_NAME, props);
    return CELIX_SUCCESS;
}

static celix_status_t my_shell_command_provider_bundle_stop(my_shell_command_provider_activator_data_t *data, celix_bundle_context_t *ctx) {
    celix_bundleContext_unregisterServiceAsync(ctx, data->shellCmdSvcId, NULL, NULL);
    return CELIX_SUCCESS;
}

CELIX_GEN_BUNDLE_ACTIVATOR(my_shell_command_provider_activator_data_t, my_shell_command_provider_bundle_start, my_shell_command_provider_bundle_stop)
```

### 示例：使用C++注册服务
```C++
//src/MyShellCommandBundleActivator.cc
#include <celix/BundleActivator.h>
#include <celix/IShellCommand.h>

class MyCommand : public celix::IShellCommand {
public:
    explicit MyCommand(std::string_view _name) : name{_name} {}

    ~MyCommand() noexcept override = default;

    void executeCommand(
            const std::string& commandLine,
            const std::vector<std::string>& /*commandArgs*/,
            FILE* outStream,
            FILE* /*errorStream*/) override {
        fprintf(outStream, "Hello from bundle %s with command line '%s'\n", name.c_str(), commandLine.c_str());
    }
private:
    const std::string name;
};

class MyShellCommandProviderBundleActivator {
public:
    explicit MyShellCommandProviderBundleActivator(const std::shared_ptr<celix::BundleContext>& ctx) {
        auto svcObject = std::make_shared<MyCommand>(ctx->getBundle().getName());
        cmdShellRegistration = ctx->registerService<celix::IShellCommand>(std::move(svcObject))
                .addProperty(celix::IShellCommand::COMMAND_NAME, "MyCommand")
                .build();
    }

    ~MyShellCommandProvider() noexcept = default;
private:
    //注意，当 celix::ServiceRegistration 超出范围时，对应的服务将被注销
    std::shared_ptr<celix::ServiceRegistration> cmdShellRegistration{};
};

CELIX_GEN_CXX_BUNDLE_ACTIVATOR(MyShellCommandProviderBundleActivator)
```

### 示例：在C++中注册C服务
```C++
//src/MyCShellCommandProviderBundleActivator.cc
#include <celix/BundleActivator.h>
#include <celix_shell_command.h>

struct MyCShellCommand : public celix_shell_command {
    explicit MyCShellCommand(std::shared_ptr<celix::BundleContext> _ctx) : celix_shell_command(), ctx{std::move(_ctx)} {
        handle = this;
        executeCommand = [] (void *handle, const char* commandLine, FILE* outStream, FILE* /*errorStream*/) -> bool {
            auto* cmdProvider = static_cast<MyCShellCommand*>(handle);
            fprintf(outStream, "Hello from bundle %s with command line '%s'\n", cmdProvider->ctx->getBundle().getName().c_str(), commandLine);
            return true;
        };
    }

    const std::shared_ptr<celix::BundleContext> ctx;
};

class MyCShellCommandProviderBundleActivator {
public:
    explicit MyCShellCommandProviderBundleActivator(const std::shared_ptr<celix::BundleContext>&  ctx) {
        auto shellCmd = std::make_shared<MyCShellCommand>(ctx);
        cmdShellRegistration = ctx->registerService<celix_shell_command>(std::move(shellCmd), CELIX_SHELL_COMMAND_SERVICE_NAME)
                .addProperty(CELIX_SHELL_COMMAND_NAME, "MyCCommand")
                .setUnregisterAsync(false)
                .build();
    }
private:
    //注意，当 celix::ServiceRegistration 超出范围时，对应的服务将被注销
    std::shared_ptr<celix::ServiceRegistration> cmdShellRegistration{};
};

CELIX_GEN_CXX_BUNDLE_ACTIVATOR(MyCShellCommandProviderBundleActivator)
```

### 服务注册的序列图

![Register Service Async](diagrams/services_register_service_async_seq.png)
*An asynchronized service registration*

---

![Register Service Async](diagrams/services_register_service_seq.png)
*A synchronized service registration*

---

![Unregister Service Async](diagrams/services_unregister_service_async_seq.png)
*An asynchronized service un-registration*

---

![Unregister Service Async](diagrams/services_unregister_service_seq.png)
*A synchronized service un-registration*

## 服务的使用
可以直接使用捆绑包上下文的 C 函数或 C++ 方法来使用服务：
- `celix_bundleContext_useServiceWithId`
- `celix_bundleContext_useService`
- `celix_bundleContext_useServices`
- `celix_bundleContext_useServiceWithOptions`
- `celix_bundleContext_useServicesWithOptions`
- `celix::BundleContext::useService`
- `celix::BundleContext::useServices`

这些函数和方法通过提供一个回调函数来工作，该回调函数将由 Celix 框架调用并匹配相应的单个服务或若干服务。
当一个"使用服务"的函数/方法返回时，回调函数可以被安全地释放。
"使用服务"的函数/方法的返回值将表明找到了匹配的服务，或者找到了多少匹配的服务。

Celix框架通过回调提供服务——而不是直接返回一个服务指针——以确保服务在使用时不会被移除。
这样，用户就无需额外调用API来"锁定"和"解锁"对服务的使用。

### 示例：在C中使用服务
```C
#include <stdio.h>
#include <celix_bundle_activator.h>
#include <celix_shell_command.h>

typedef struct use_command_service_example_data {
    //nop
} use_command_service_example_data_t;

static void useShellCommandCallback(void *handle CELIX_UNUSED, void *svc) {
    celix_shell_command_t* cmdSvc = (celix_shell_command_t*)svc;
    cmdSvc->executeCommand(cmdSvc->handle, "my_command test call from C", stdout, stderr);
}

static celix_status_t use_command_service_example_start(use_command_service_example_data_t *data CELIX_UNUSED, celix_bundle_context_t *ctx) {
    celix_service_use_options_t opts = CELIX_EMPTY_SERVICE_USE_OPTIONS;
    opts.callbackHandle = NULL;
    opts.use = useShellCommandCallback;
    opts.filter.serviceName = CELIX_SHELL_COMMAND_SERVICE_NAME;
    opts.filter.filter = "(command.name=my_command)";
    bool called = celix_bundleContext_useServicesWithOptions(ctx, &opts);
    if (!called) {
        fprintf(stderr, "%s: Command service not called!\n", __PRETTY_FUNCTION__);
    }
    return CELIX_SUCCESS;
}

static celix_status_t use_command_service_example_stop(use_command_service_example_data_t *data CELIX_UNUSED, celix_bundle_context_t *ctx CELIX_UNUSED) {
    return CELIX_SUCCESS;
}

CELIX_GEN_BUNDLE_ACTIVATOR(use_command_service_example_data_t, use_command_service_example_start, use_command_service_example_stop)
```

### 示例：在C++中使用服务
```C++
//src/UsingCommandServicesExample.cc
#include <celix/IShellCommand.h>
#include <celix/BundleActivator.h>
#include <celix_shell_command.h>

static void useCxxShellCommand(const std::shared_ptr<celix::BundleContext>& ctx) {
    auto called = ctx->useService<celix::IShellCommand>()
            .setFilter("(name=MyCommand)")
            .addUseCallback([](celix::IShellCommand& cmdSvc) {
                cmdSvc.executeCommand("MyCommand test call from C++", {}, stdout, stderr);
            })
            .build();
    if (!called) {
        std::cerr << __PRETTY_FUNCTION__  << ": Command service not called!" << std::endl;
    }
}

static void useCShellCommand(const std::shared_ptr<celix::BundleContext>& ctx) {
    auto calledCount = ctx->useServices<celix_shell_command>(CELIX_SHELL_COMMAND_SERVICE_NAME)
            //Note the filter should match 2 shell commands
            .setFilter("(|(command.name=MyCCommand)(command.name=my_command))") 
            .addUseCallback([](celix_shell_command& cmdSvc) {
                cmdSvc.executeCommand(cmdSvc.handle, "MyCCommand test call from C++", stdout, stderr);
            })
            .build();
    if (calledCount == 0) {
        std::cerr << __PRETTY_FUNCTION__  << ": Command service not called!" << std::endl;
    }
}

class UsingCommandServicesExample {
public:
    explicit UsingCommandServicesExample(const std::shared_ptr<celix::BundleContext>& ctx) {
        useCxxShellCommand(ctx);
        useCShellCommand(ctx);
    }

    ~UsingCommandServicesExample() noexcept = default;
};

CELIX_GEN_CXX_BUNDLE_ACTIVATOR(UsingCommandServicesExample)
```

## 跟踪服务
为了监控服务的来去，可以使用服务跟踪器。服务跟踪器使用用户提供的回调来处理匹配服务的添加/删除。使用服务名称和可选的LDAP过滤器来选择需要监控的服务。
服务名称 `*` 可以用来匹配任意服务。当一个服务注销时，实际注销操作只能在所有匹配的服务跟踪器删除回调被处理之后进行。

对于C语言，可以使用以下捆绑包上下文函数来创建服务跟踪器：
- `celix_bundleContext_trackServicesAsync`
- `celix_bundleContext_trackServices`
- `celix_bundleContext_trackServicesWithOptionsAsync`
- `celix_bundleContext_trackServicesWithOptions`

"跟踪服务"的C函数总是返回一个跟踪器Id（long），可以利用它来关闭和销毁服务跟踪器：
- `celix_bundleContext_stopTrackerAsync`
- `celix_bundleContext_stopTracker`

对于C++，可以使用以下捆绑包上下文方法来创建服务跟踪器：
- `celix::BundleContext::trackServices`
- `celix::BundleContext::trackAnyServices`

该C++方法使用构建器API，最后会返回一个`std::shared_ptr<celix::ServiceTracker<I>>`对象。
如果关联的ServiceTracker对象超出范围，服务跟踪器将会被关闭和销毁。

C++服务跟踪器的创建和打开是异步的，但关闭是同步的。这样做是为了保证在调用了`celix::ServiceTracker::close()`之后，已添加的回调将不会再触发。

### 示例：在C中跟踪服务
```C
//src/track_command_services_example.c
#include <stdio.h>
#include <celix_bundle_activator.h>
#include <celix_threads.h>
#include <celix_constants.h>
#include <celix_shell_command.h>

typedef struct track_command_services_example_data {
    long trackerId;
    celix_thread_mutex_t mutex; //protects below
    celix_array_list_t* commandServices;
} track_command_services_example_data_t;


static void addShellCommandService(void* data,void* svc, const celix_properties_t * properties) {
    track_command_services_example_data_t* activatorData = data;
    celix_shell_command_t* cmdSvc = svc;

    printf("Adding command service with svc id %li\n", celix_properties_getAsLong(properties, CELIX_FRAMEWORK_SERVICE_ID, -1));
    celixThreadMutex_lock(&activatorData->mutex);
    celix_arrayList_add(activatorData->commandServices, cmdSvc);
    printf("Nr of command service found: %i\n", celix_arrayList_size(activatorData->commandServices));
    celixThreadMutex_unlock(&activatorData->mutex);
}

static void removeShellCommandService(void* data,void* svc, const celix_properties_t * properties) {
    track_command_services_example_data_t* activatorData = data;
    celix_shell_command_t* cmdSvc = svc;

    printf("Removing command service with svc id %li\n", celix_properties_getAsLong(properties, CELIX_FRAMEWORK_SERVICE_ID, -1));
    celixThreadMutex_lock(&activatorData->mutex);
    celix_arrayList_remove(activatorData->commandServices, cmdSvc);
    printf("Nr of command service found: %i\n", celix_arrayList_size(activatorData->commandServices));
    celixThreadMutex_unlock(&activatorData->mutex);
}

static celix_status_t track_command_services_example_start(track_command_services_example_data_t *data, celix_bundle_context_t *ctx) {
    celixThreadMutex_create(&data->mutex, NULL);
    data->commandServices = celix_arrayList_create();

    celix_service_tracking_options_t opts = CELIX_EMPTY_SERVICE_TRACKING_OPTIONS;
    opts.filter.serviceName = CELIX_SHELL_COMMAND_SERVICE_NAME;
    opts.filter.filter = "(command.name=my_command)";
    opts.callbackHandle = data;
    opts.addWithProperties = addShellCommandService;
    opts.removeWithProperties = removeShellCommandService;
    data->trackerId = celix_bundleContext_trackServicesWithOptionsAsync(ctx, &opts);
    return CELIX_SUCCESS;
}

static celix_status_t track_command_services_example_stop(track_command_services_example_data_t *data, celix_bundle_context_t *ctx) {
    celix_bundleContext_stopTracker(ctx, data->trackerId);
    celixThreadMutex_lock(&data->mutex);
    celix_arrayList_destroy(data->commandServices);
    celixThreadMutex_unlock(&data->mutex);
    return CELIX_SUCCESS;
}

CELIX_GEN_BUNDLE_ACTIVATOR(track_command_services_example_data_t, track_command_services_example_start, track_command_services_example_stop)
```

### 示例：在C++中跟踪服务
```C++
//src/TrackingCommandServicesExample.cc
#include <unordered_map>
#include <celix/IShellCommand.h>
#include <celix/BundleActivator.h>
#include <celix_shell_command.h>

class TrackingCommandServicesExample {
public:
    explicit TrackingCommandServicesExample(const std::shared_ptr<celix::BundleContext>& ctx) {
        //Tracking C++ IShellCommand services and filtering for services that have a "name=MyCommand" property.
        cxxCommandServiceTracker = ctx->trackServices<celix::IShellCommand>()
                .setFilter("(name=MyCommand)")
                .addAddWithPropertiesCallback([this](const auto& svc, const auto& properties) {
                    long svcId = properties->getAsLong(celix::SERVICE_ID, -1);
                    std::cout << "Adding C++ command services with svc id" << svcId << std::endl;
                    std::lock_guard lock{mutex};
                    cxxCommandServices[svcId] = svc;
                    std::cout << "Nr of C++ command services found: " << cxxCommandServices.size() << std::endl;
                })
                .addRemWithPropertiesCallback([this](const auto& /*svc*/, const auto& properties) {
                    long svcId = properties->getAsLong(celix::SERVICE_ID, -1);
                    std::cout << "Removing C++ command services with svc id " << svcId << std::endl;
                    std::lock_guard lock{mutex};
                    auto it = cxxCommandServices.find(svcId);
                    if (it != cxxCommandServices.end()) {
                        cxxCommandServices.erase(it);
                    }
                    std::cout << "Nr of C++ command services found: " << cxxCommandServices.size() << std::endl;
                })
                .build();

        //Tracking C celix_shell_command services and filtering for services that have a "command.name=MyCCommand" or
        // "command.name=my_command" property.
        cCommandServiceTracker = ctx->trackServices<celix_shell_command>()
                .setFilter("(|(command.name=MyCCommand)(command.name=my_command))")
                .addAddWithPropertiesCallback([this](const auto& svc, const auto& properties) {
                    long svcId = properties->getAsLong(celix::SERVICE_ID, -1);
                    std::cout << "Adding C command services with svc id " << svcId << std::endl;
                    std::lock_guard lock{mutex};
                    cCommandServices[svcId] = svc;
                    std::cout << "Nr of C command services found: " << cxxCommandServices.size() << std::endl;
                })
                .addRemWithPropertiesCallback([this](const auto& /*svc*/, const auto& properties) {
                    long svcId = properties->getAsLong(celix::SERVICE_ID, -1);
                    std::cout << "Removing C command services with svc id " << svcId << std::endl;
                    std::lock_guard lock{mutex};
                    auto it = cCommandServices.find(svcId);
                    if (it != cCommandServices.end()) {
                        cCommandServices.erase(it);
                    }
                    std::cout << "Nr of C command services found: " << cxxCommandServices.size() << std::endl;
                })
                .build();
    }

    ~TrackingCommandServicesExample() noexcept {
        cxxCommandServiceTracker->close();
        cCommandServiceTracker->close();
    };
private:
    std::mutex mutex; //protects cxxCommandServices and cCommandServices
    std::unordered_map<long, std::shared_ptr<celix::IShellCommand>> cxxCommandServices{};
    std::unordered_map<long, std::shared_ptr<celix_shell_command>> cCommandServices{};

    std::shared_ptr<celix::ServiceTracker<celix::IShellCommand>> cxxCommandServiceTracker{};
    std::shared_ptr<celix::ServiceTracker<celix_shell_command>> cCommandServiceTracker{};
};

CELIX_GEN_CXX_BUNDLE_ACTIVATOR(TrackingCommandServicesExample)
```

### 服务跟踪器和服务注册的序列图

![异步注册服务](diagrams/services_tracker_services_add_async_seq.png)
*带有异步服务注册的服务跟踪器回调*

---

![异步注销服务](diagrams/services_tracker_services_rem_async_seq.png)
*带有异步服务注销的服务跟踪器回调*

---

![同步注册服务](diagrams/services_tracker_services_add_seq.png)
*带有同步服务注册的服务跟踪器回调*

---

![同步注销服务](diagrams/services_tracker_services_rem_seq.png)
*带有同步服务注销的服务跟踪器回调*

# `celix::query` Shell命令
要交互式地查看哪些服务和服务跟踪器可用，可以使用`celix::query` Shell命令。

支持的`query`命令行用法示例：
- `celix::query` - 对每个捆绑包显示已注册服务及激活的服务跟踪器的概述。
- `query` - 与`celix::query`相同（只要没有其他冲突的`query`命令）。
- `query -v` - 对每个捆绑包显示已注册服务和激活的服务跟踪器的详细概述。
  对于已注册的服务，打印服务属性；对于激活的服务跟踪器，打印跟踪到的服务数量。
- `query foo` - 显示满足条件的已注册服务和激活的服务跟踪器的概述，其中"foo"是已注册/被追踪服务名的一部分（不区分大小写）。
- `query (service.id>=10)` - 显示符合提供的LDAP过滤器的已注册服务的概述。
