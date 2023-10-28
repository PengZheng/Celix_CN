---
title: Apache Celix 服务组件
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

# Apache Celix 服务组件
在 Apache Celix 中，服务组件是由 Apache Celix 依赖管理器（DM）管理的普通的 C/C++ 对象（POCOs）。
服务组件可以提供服务并依赖服务。服务组件是通过依赖管理器API进行声明式配置的。

服务依赖性将影响服务组件的生命周期，因为只有在所有必需的依赖项都可用时，服务组件才会处于激活状态。
依赖管理器负责管理服务组件的服务依赖性、服务组件的生命周期以及何时注册/注销服务组件对外提供的服务。

请注意，Apache Celix 依赖管理器的灵感来自 [Apache Felix 依赖管理器](http://felix.apache.org/documentation/subprojects/apache-felix-dependency-manager.html)，
我们将其适配到了 Apache Celix 及 C/C++。

# 服务组件生命周期
每个服务组件都有自己的生命周期。
服务组件的生命周期状态模型在下面的状态图中描绘。

![服务组件生命周期](diagrams/component_lifecycle.png)

可以使用DM API来配置服务组件的生命周期回调，可配置以下回调：

- `init`
- `start`
- `stop`
- `deinit`

这些回调用于服务组件生命周期中间状态 `Initializing`, `Starting`, `Suspending`, `Resuming`, `Stopping` 和 `Deinitializing`，并且生命周期回调总是从Celix事件线程触发。

服务组件有如下生命周期状态：
- `Inactive`：服务组件处于非激活状态，依赖管理器尚未托管该服务组件。
- `Waiting For Required`：服务组件正在等待所需的服务依赖项。
- `Initializing`：服务组件发现其所需的依赖项，并通过调用 `init` 回调进行初始化。
- `Initialized And Waiting For Required`：服务组件已经初始化，但正在等待所需的依赖项。
  _注意_: 这可能意味着：
    - 在 `init` 回调过程中，新增了1个或多个当前不可用的必需服务依赖项。
    - 服务组件曾经被激活，但是1个或多个所需的服务依赖项已经被移除，因此组件不再是活动的。
- `Starting`：组件发现其所需的依赖项，并通过调用 `start` 回调进行启动并注册服务组件提供的服务。
- `Tracking Optional`：组件已找到其所需的依赖项并已启动。它仍在跟踪寻找额外的可选和必需服务。
- `Suspending`：组件找到了所需的依赖项，但正在通过注销组件提供的服务和调用 `stop` 回调的方式进行挂起，以准备处理依赖服务变更。
- `Suspended`：组件已找到其所需的依赖项，但已挂起，以便可以处理依赖服务变更。
- `Resuming`：组件已找到其所需的依赖项，已处理依赖服务变更，正在通过调用 `start` 回调并注册对外提供服务的方式进行恢复。
- `Stopping`：组件丢失了一个或多个所需的依赖项，正在通过注销对外提供服务和调用 `stop` 回调的方式进行停止。
- `Deinitializing`：组件正在被移除，并调用 `deinit` 回调进行去初始化。

## 服务组件应用编程接口

服务组件的C接口可以在`celix_dm_component.h`头文件中找到，而C++接口可以在`celix/dm/Component.h`头文件中找到。

## 例子：在C中创建和配置服务组件的生命周期回调
下面的例子展示了如何用C语言和依赖管理器创建并托管一个简单的服务组件。
因为服务组件的生命周期由依赖管理器托管，所以这也意味着如果配置正确，就不需要额外的代码来删除和销毁服务组件及其实现。

对C语言例子的注释：
1. 虽然这是一个简单的C服务组件，但其上的函数遵循组件封装方法，即使用组件指针作为第一个参数。
2. 服务组件的实现可以是任何普通的C/C++对象（POCO），只要它的生命周期回调和销毁函数签名都遵循组件方法：只有一个参数，其类型为组件实现指针，其生命周期回调返回 int，其销毁函数返回 void。
3. 创建服务组件，但注意到此时服务组件仍未被依赖管理器所知。这使得在它被依赖管理器托管之前，可以先使用服务组件API进行一定的配置。
4. 设置服务组件的实现上下文，使得组件实现上下文指针可以用于配置的服务组件回调。
5. 配置服务组件生命周期回调。这些回调应该接受组件实现上下文指针作为唯一的参数。`CELIX_DM_COMPONENT_SET_CALLBACKS` 宏替代了 `celix_dmComponent_setCallbacks`函数，使得在生命周期回调中可以直接使用组件实现上下文指针类型（而非`void*`）。
6. 配置组件实现上下文销毁回调。当服务组件从依赖管理器中移除并变为非活动状态时，调用此回调。回调将从Celix事件线程中被调用。配置这个回调的优点是由依赖管理器确定何时需要调用该回调，这为用户降低了实现复杂性。
   `CELIX_DM_COMPONENT_SET_IMPLEMENTATION_DESTROY_FUNCTION` 宏取代了 `celix_dmComponent_setImplementationDestroyFunction`函数，使得可以在回调中直接使用组件实现上下文指针类型（而非`void*`）。
7. 把服务组件加入依赖管理器，由依赖管理器托管服务组件的生命周期、服务依赖以及对外提供的服务。
8. 无需额外的代码来清理服务组件，因此无需配置激活器停止回调。生成的捆绑包激活器会确保当捆绑包被停止时所有的服务组件都从依赖管理器中移除，并且依赖管理器会确保服务组件被正确地停用并销毁。

```C
//src/simple_component_activator.c
#include <stdio.h>
#include <celix_bundle_activator.h>
#include <celix_dm_component.h>

//********************* 服务组件 *******************************/

typedef struct simple_component {
    int transitionCount; //无需加锁保护，只会在事件循环中读取和更新
} simple_component_t;

static simple_component_t* simpleComponent_create() {
    simple_component_t* cmp = calloc(1, sizeof(*cmp));
    cmp->transitionCount = 1;
    return cmp;
}

static void simpleComponent_destroy(simple_component_t* cmp) {
    free(cmp);
}

static int simpleComponent_init(simple_component_t* cmp) { // <------------------------------------------------------<1>
    printf("Initializing simple component. Transition nr %i\n", cmp->transitionCount++);
    return 0;
}

static int simpleComponent_start(simple_component_t* cmp) {
    printf("Starting simple component. Transition nr %i\n", cmp->transitionCount++);
    return 0;
}

static int simpleComponent_stop(simple_component_t* cmp) {
    printf("Stopping simple component. Transition nr %i\n", cmp->transitionCount++);
    return 0;
}

static int simpleComponent_deinit(simple_component_t* cmp) {
    printf("De-initializing simple component. Transition nr %i\n", cmp->transitionCount++);
    return 0;
}


//********************* 激活器 *******************************/

typedef struct simple_component_activator {
    //nop
} simple_component_activator_t;

static celix_status_t simpleComponentActivator_start(simple_component_activator_t *act, celix_bundle_context_t *ctx) {
    //创建服务组件
    simple_component_t* impl = simpleComponent_create(); // <--------------------------------------------------------<2>

    //使用 Apache Celix 依赖管理器创建并配置服务组件及其生命周期回调
    celix_dm_component_t* dmCmp = celix_dmComponent_create(ctx, "simple_component_1"); // <--------------------------<3>
    celix_dmComponent_setImplementation(dmCmp, impl); // <-----------------------------------------------------------<4>
    CELIX_DM_COMPONENT_SET_CALLBACKS(
            dmCmp,
            simple_component_t,
            simpleComponent_init,
            simpleComponent_start,
            simpleComponent_stop,
            simpleComponent_deinit); // <----------------------------------------------------------------------------<5>
    CELIX_DM_COMPONENT_SET_IMPLEMENTATION_DESTROY_FUNCTION(
            dmCmp,
            simple_component_t,
            simpleComponent_destroy); // <---------------------------------------------------------------------------<6>

    //将服务组件添加到依赖管理器
    celix_dependency_manager_t* dm = celix_bundleContext_getDependencyManager(ctx);
    celix_dependencyManager_add(dm, dmCmp); // <---------------------------------------------------------------------<7>
    return CELIX_SUCCESS;
}

CELIX_GEN_BUNDLE_ACTIVATOR(simple_component_activator_t, simpleComponentActivator_start, NULL) // <------------------<8>
```

## 示例：在 C++ 中创建和配置组件的生命周期回调
以下示例展示了如何使用 C++ 的依赖管理器来创建和管理一个简单的服务组件。
对于 C++，依赖管理器将托管服务组件，并确保在服务组件被托管期间，组件实现始终在作用范围内。

C++ 示例的注意事项：
1. 对于C++，依赖管理器可以直接在C++类上工作，因此生命周期回调可以是类方法。
2. 使用`unique_ptr`创建一个组件实现。
3. 创建一个C++服务组件并直接将其添加到依赖管理器中。
   对于C++服务组件，需要先进行“构建”，然后依赖管理器才会对其进行托管。
   C++服务组件可以通过流畅API进行构建，并通过调用`build()`方法来标记构建完成。
   组件管理器可接受一个`unique_ptr`/一个`shared_ptr`/一个值类型作为组件实现。
若没有提供组件实现，依赖管理器将使用模板参数和默认构造函数来创建一个组件实现（例如 `ctx->getDependencyManager()->createComponent<CmpWithDefaultCTOR>()`）。
4. 配置服务组件的生命周期回调为类方法。依赖管理器将使用组件实现的原始指针作为对象实例（`this`）来调用这些回调。
5. "构建"服务组件。只有在构建（调用`build()`方法）后，服务组件才会被依赖管理器托管。因此可以在标记服务组件构建完成之前通过多个方法调用来配置此服务组件。
   生成的C++捆绑包激活器也将使能在捆绑包激活期间创建的所有服务组件，这是为了确保构建行为与先前发布的依赖管理器实现向后兼容。
   我们建议用户在完成服务组件配置后，去显式构建它们（而不要依赖激活器）。

```C++
//src/SimpleComponentActivator.cc
#include <celix/BundleActivator.h>

class SimpleComponent {
public:
    void init() { // <-----------------------------------------------------------------------------------------------<1>
        std::cout << "Initializing simple component. Transition nr " << transitionCount++ << std::endl;
    }

    void start() {
        std::cout << "starting simple component. Transition nr " << transitionCount++ << std::endl;
    }

    void stop() {
        std::cout << "Stopping simple component. Transition nr " << transitionCount++ << std::endl;
    }

    void deinit() {
        std::cout << "De-initializing simple component. Transition nr " << transitionCount++ << std::endl;
    }
private:
    int transitionCount = 1; //无需加锁保护，只会在事件循环中读取和更新
};

class SimpleComponentActivator {
public:
    explicit SimpleComponentActivator(const std::shared_ptr<celix::BundleContext>& ctx) {
        auto cmp = std::make_unique<SimpleComponent>(); // <---------------------------------------------------------<2>
        ctx->getDependencyManager()->createComponent(std::move(cmp), "SimpleComponent1") // <------------------------<3>
                .setCallbacks(
                        &SimpleComponent::init,
                        &SimpleComponent::start,
                        &SimpleComponent::stop,
                        &SimpleComponent::deinit) // <---------------------------------------------------------------<4>
                .build(); // <---------------------------------------------------------------------------------------<5>
    }
};

CELIX_GEN_CXX_BUNDLE_ACTIVATOR(SimpleComponentActivator)
```

# 服务组件的提供的服务
可以配置服务组件对外提供服务。
这些服务将在服务组件处于`Staring`或`Resuming`状态（组件即将进入`Tracking Optional`状态）时进行注册。

如果一个服务组件对外提供服务，这些服务除了具有配置的服务属性外，还将具有一个额外自动添加的服务属性 - 名为 "component.uuid"。
"component.uuid"服务属性可以用来识别服务是否由服务组件提供以及由哪个服务组件提供。

## 示例：对外提供服务的 C 服务组件
下面的示例显示了服务组件如何对外提供`celix_shell_command`服务。

C示例的备注：
1. C服务不支持继承。所以即使一个C组件对外提供了某个服务，它也不是该服务的实例。
   这也意味着由服务组件提供的C服务结构体需要独立存储。在此示例中，捆绑激活器数据中保存了服务结构体。
   注意，捆绑激活器数据比服务组件存活时间更长，因为在捆绑包完全停止之前，所有的服务组件都会被移除并销毁。
2. 为服务组件配置对外提供的服务（接口）。服务并不会立即注册，而将在服务组件状态处于`Staring`和`Resuming`状态时再注册。

```C
 //src/component_with_provided_service_activator.c
#include <stdlib.h>
#include <celix_bundle_activator.h>
#include <celix_shell_command.h>

//********************* 服务组件 *******************************/

typedef struct component_with_provided_service {
    int callCount; //atomic
} component_with_provided_service_t;

static component_with_provided_service_t* componentWithProvidedService_create() {
    component_with_provided_service_t* cmp = calloc(1, sizeof(*cmp));
    return cmp;
}

static void componentWithProvidedService_destroy(component_with_provided_service_t* cmp) {
    free(cmp);
}

static bool componentWithProvidedService_executeCommand(
        component_with_provided_service_t *cmp,
        const char *commandLine,
        FILE *outStream,
        FILE *errorStream CELIX_UNUSED) {
    int count = __atomic_add_fetch(&cmp->callCount, 1, __ATOMIC_SEQ_CST);
    fprintf(outStream, "Hello from cmp. command called %i times. commandLine: %s\n", count, commandLine);
    return true;
}

//********************* 激活器 *******************************/

typedef struct component_with_provided_service_activator {
    celix_shell_command_t shellCmd; // <-----------------------------------------------------------------------------<1>
} component_with_provided_service_activator_t;

static celix_status_t componentWithProvidedServiceActivator_start(component_with_provided_service_activator_t *act, celix_bundle_context_t *ctx) {
    //创建服务组件
    component_with_provided_service_t* impl = componentWithProvidedService_create();

    //使用 Apache Celix 依赖管理器创建并配置服务组件及其生命周期回调
    celix_dm_component_t* dmCmp = celix_dmComponent_create(ctx, "component_with_provided_service_1");
    celix_dmComponent_setImplementation(dmCmp, impl);
    CELIX_DM_COMPONENT_SET_IMPLEMENTATION_DESTROY_FUNCTION(
            dmCmp,
            component_with_provided_service_t,
            componentWithProvidedService_destroy);

    //配置对外提供的服务
    act->shellCmd.handle = impl;
    act->shellCmd.executeCommand = (void*)componentWithProvidedService_executeCommand;
    celix_properties_t* props = celix_properties_create();
    celix_properties_set(props, CELIX_SHELL_COMMAND_NAME, "hello_component");
    celix_dmComponent_addInterface(
            dmCmp,
            CELIX_SHELL_COMMAND_SERVICE_NAME,
            CELIX_SHELL_COMMAND_SERVICE_VERSION,
            &act->shellCmd,
            props); // <---------------------------------------------------------------------------------------------<2>


    //将服务组件添加到依赖管理器
    celix_dependency_manager_t* dm = celix_bundleContext_getDependencyManager(ctx);
    celix_dependencyManager_add(dm, dmCmp);
    return CELIX_SUCCESS;
}

CELIX_GEN_BUNDLE_ACTIVATOR(
        component_with_provided_service_activator_t,
        componentWithProvidedServiceActivator_start,
        NULL)
```

## 示例：C++中的组件包含的提供的服务
以下示例展示了一个C++服务组件如何对外提供`celix::IShellCommand` C++服务以及`celix_shell_command` C服务。
C++服务组件既可以提供C服务，也可以提供C++服务。

C++示例的备注：
1. 若一个服务组件提供C++服务，则该组件实现应继承对应服务接口。
2. 被重写的 `celix::IShellCommand` 接口的 `executeCommand` 方法。
3. C语言服务接口中的方法可以被实现为类方法，但是激活器程序应确保底层的C语言服务接口结构体被赋予了兼容的C函数指针。
4. 使用模板参数创建一个服务组件。依赖管理器将使用默认构造器来构造一个组件实现实例。
5. 配置服务组件对外提供`celix::IShellCommand` C++服务。注意，因为组件实现是 `celix::IShellCommand` 的一个实例，因此不需要额外的存储空间。
   服务不会直接注册，而将在服务组件状态处于`Staring`和`Resuming`状态时注册。
6. 将 `celix_shell_command_t` C服务接口结构体的 `executeCommand` 函数指针设置为一个无捕获的lambda表达式。
   lambda表达式用来将调用转发到 `executeCCommand` 类方法。注意无捕获的lambda表达式可以退化为C风格的函数指针。
7. 将组件配置为对外提供 `celix_shell_command_t` C服务。注意对于C服务，必须使用 `createUnassociatedProvidedService` ，因为组件并没有继承 `celix_shell_command_t` 。
   服务不会立即注册，而将在服务组件状态处于`Staring`和`Resuming`状态时再注册。
8. “构建”服务组件，至此依赖管理器将托管该服务组件。

```C++
//src/ComponentWithProvidedServiceActivator.cc
#include <celix/BundleActivator.h>
#include <celix/IShellCommand.h>
#include <celix_shell_command.h>

class ComponentWithProvidedService : public celix::IShellCommand { // <----------------------------------------------<1>
public:
    ~ComponentWithProvidedService() noexcept override = default;

    void executeCommand(
            const std::string& commandLine,
            const std::vector<std::string>& /*commandArgs*/,
            FILE* outStream,
            FILE* /*errorStream*/) override {
        fprintf(outStream, "Hello from cmp. C++ command called %i times. commandLine is %s\n", 
                cxxCallCount++, commandLine.c_str());
    } // <-----------------------------------------------------------------------------------------------------------<2>

    void executeCCommand(const char* commandLine, FILE* outStream) {
        fprintf(outStream, "Hello from cmp. C command called %i times. commandLine is %s\n", cCallCount++, commandLine);
    } // <-----------------------------------------------------------------------------------------------------------<3>
private:
    std::atomic<int> cxxCallCount{1};
    std::atomic<int> cCallCount{1};
};

class ComponentWithProvidedServiceActivator {
public:
    explicit ComponentWithProvidedServiceActivator(const std::shared_ptr<celix::BundleContext>& ctx) {
        auto& cmp = ctx->getDependencyManager()->createComponent<ComponentWithProvidedService>(); // <---------------<4>

        cmp.createProvidedService<celix::IShellCommand>()
                .addProperty(celix::IShellCommand::COMMAND_NAME, "HelloComponent"); // <-----------------------------<5>

        auto shellCmd = std::make_shared<celix_shell_command_t>();
        shellCmd->handle = static_cast<void*>(&cmp.getInstance());
        shellCmd->executeCommand = [](void* handle, const char* commandLine, FILE* outStream, FILE*) -> bool {
            auto* impl = static_cast<ComponentWithProvidedService*>(handle);
            impl->executeCCommand(commandLine, outStream);
            return true;
        }; // <------------------------------------------------------------------------------------------------------<6>

        cmp.createUnassociatedProvidedService(std::move(shellCmd), CELIX_SHELL_COMMAND_SERVICE_NAME)
                .addProperty(CELIX_SHELL_COMMAND_NAME, "hello_component"); // < -------------------------------------<7>

        cmp.build(); // <--------------------------------------------------------------------------------------------<8>
    }
private:
};

CELIX_GEN_CXX_BUNDLE_ACTIVATOR(ComponentWithProvidedServiceActivator)
```

# 服务组件的服务依赖
可以配置服务组件的服务依赖项。这些服务依赖项将影响服务组件的生命周期。
服务组件有两类服务依赖项：可选依赖和必需依赖。服务组件只有在所有必需依赖项都可用时才能被激活；可用意味着找到至少1个匹配的服务依赖项。

在配置服务依赖项时，可以配置服务依赖回调来处理服务的添加、移除、最高优先级服务的变更。

服务依赖回调支持3种不同类型的参数签名：
- 单个参数用于服务指针（原始指针或shared_ptr）；
- 第一个参数是服务指针（原始指针或shared_ptr），第二个参数是服务属性。
- 第一个参数是服务指针（原始指针或shared_ptr），第二个参数是服务属性，第三个参数是提供服务的捆绑包。

服务依赖回调总是从Celix事件线程中触发。

服务组件可以选择使用锁定策略或者挂起策略来处理服务变更（注入/移除）。这种策略可以针对单个服务依赖项进行配置，并期望组件实现的行为如下：
- 锁定策略：组件实现必须确保存储的服务指针（如果适用，还包括服务属性及提供服务的捆绑包）使用锁定机制（例如，互斥）进行保护。
  应确保服务从服务组件中删除（或替换）后，不会再被用到，因此可以从内存中安全地删除。
- 挂起策略：依赖管理器将确保在调用服务依赖回调之前，所有提供的服务都（临时）取消注册，服务组件被挂起（使用服务组件的 `stop` 回调）。
  这意味着没有活跃的用户 - 通过提供的服务或活动线程 - 使用该服务依赖项，服务变更可以在不锁定的情况下安全地处理。
  组件实现必须确保在 `stop` 回调后，不存在任何使用该服务依赖项的线程、线程池、激活的定时器。

## 示例：具有服务依赖项的C服务组件
以下示例显示了一个C服务组件，它有两个对`celix_shell_command_t`的服务依赖项。

其中一个服务依赖项是必需依赖，具有挂起策略，并配置了`set`回调，确保注入的总是优先级最高的单个服务。
注意，如果没有匹配的服务，优先级最高的服务为 `NULL`。

另外一个服务依赖项是可选依赖，具有锁定策略，并配置了`addWithProps`和`removeWithProps`回调。
每一个被添加/移除的 `celix_shell_command_t`服务都会触发对应的回调。这些回调不仅提供服务指针，还提供关联的服务属性。

C示例的备注：
1. 创建一个互斥锁来保护`cmdShells` 字段，该字段将由具有锁定策略的服务依赖项上配置的服务依赖回调来维护。
2. 在不加锁的情况下更新 `highestRankingCmdShell` 字段。
   注意，由于对应服务依赖项上配置了挂起策略，所以 `componentWithServiceDependency_setHighestRankingShellCommand`函数将只在组件处于 `Suspended` 状态或者它不在 `Active`复合状态时被调用。
3. 锁定互斥锁并将新添加的服务添加到 `cmdShells` 列表。
   注意，由于对应服务依赖项配备了锁定策略，所以 `componentWithServiceDependency_addShellCommand` 和 `componentWithServiceDependency_removeShellCommand` 函数可以在任何服务组件生命周期状态被调用。
4. 创建一个新的服务依赖项。注意，服务组件尚不知道该服务依赖项。
5. 配置服务依赖项跟踪的目标服务名。还可以选择设置目标服务的版本范围，或设置其他服务过滤器。
6. 将服务依赖项的更新策略配置为挂起策略。
7. 设定服务依赖项为必需依赖。
8. 创建一个空的服务依赖回调选项结构体。 这个结构体可用来配置不同的服务依赖回调。
9. 将 `set` 服务依赖回调配置为 `componentWithServiceDependency_setHighestRankingShellCommand`
10. 配置服务依赖项以使用在上一步中配置的回调。
11. 将服务依赖项添加到服务组件对象。
12. 将服务依赖项的更新策略配置为锁定策略。
13. 设定服务依赖项为可选依赖。
14. 将 `addWithProps`服务依赖回调配置为 `componentWithServiceDependency_addShellCommand`。

```C
//src/component_with_service_dependency_activator.c
#include <stdlib.h>
#include <celix_bundle_activator.h>
#include <celix_shell_command.h>

//********************* 服务组件 *******************************/

typedef struct component_with_service_dependency {
    celix_shell_command_t* highestRankingCmdShell; //仅当服务组件出于未激活或者挂起时更新
    celix_thread_mutex_t mutex; //保护cmdShells字段
    celix_array_list_t* cmdShells;
} component_with_service_dependency_t;

static component_with_service_dependency_t* componentWithServiceDependency_create() {
    component_with_service_dependency_t* cmp = calloc(1, sizeof(*cmp));
    celixThreadMutex_create(&cmp->mutex, NULL); // <-----------------------------------------------------------------<1>
    cmp->cmdShells = celix_arrayList_create();
    return cmp;
}

static void componentWithServiceDependency_destroy(component_with_service_dependency_t* cmp) {
    celix_arrayList_destroy(cmp->cmdShells);
    celixThreadMutex_destroy(&cmp->mutex);
    free(cmp);
}

static void componentWithServiceDependency_setHighestRankingShellCommand(
        component_with_service_dependency_t* cmp,
        celix_shell_command_t* shellCmd) {
    printf("New highest ranking service (can be NULL): %p\n", shellCmd);
    cmp->highestRankingCmdShell = shellCmd; // <---------------------------------------------------------------------<2>
}

static void componentWithServiceDependency_addShellCommand(
        component_with_service_dependency_t* cmp,
        celix_shell_command_t* shellCmd,
        const celix_properties_t* props) {
    long id = celix_properties_getAsLong(props, CELIX_FRAMEWORK_SERVICE_ID, -1);
    printf("Adding shell command service with service.id %li\n", id);
    celixThreadMutex_lock(&cmp->mutex); // <-------------------------------------------------------------------------<3>
    celix_arrayList_add(cmp->cmdShells, shellCmd);
    celixThreadMutex_unlock(&cmp->mutex);
}

static void componentWithServiceDependency_removeShellCommand(
        component_with_service_dependency_t* cmp,
        celix_shell_command_t* shellCmd,
        const celix_properties_t* props) {
    long id = celix_properties_getAsLong(props, CELIX_FRAMEWORK_SERVICE_ID, -1);
    printf("Removing shell command service with service.id %li\n", id);
    celixThreadMutex_lock(&cmp->mutex);
    celix_arrayList_remove(cmp->cmdShells, shellCmd);
    celixThreadMutex_unlock(&cmp->mutex);
}

//********************* 激活器 *******************************/

typedef struct component_with_service_dependency_activator {
    //nop
} component_with_service_dependency_activator_t;

static celix_status_t componentWithServiceDependencyActivator_start(component_with_service_dependency_activator_t *act, celix_bundle_context_t *ctx) {
    //创建服务组件
    component_with_service_dependency_t* impl = componentWithServiceDependency_create();

    //使用 Apache Celix 依赖管理器创建并配置服务组件及其生命周期回调
    celix_dm_component_t* dmCmp = celix_dmComponent_create(ctx, "component_with_service_dependency_1");
    celix_dmComponent_setImplementation(dmCmp, impl);
    CELIX_DM_COMPONENT_SET_IMPLEMENTATION_DESTROY_FUNCTION(
            dmCmp,
            component_with_service_dependency_t,
            componentWithServiceDependency_destroy);

    //创建必需服务依赖项，跟踪具有最高优先级的服务，服务更新采用挂起策略
    celix_dm_service_dependency_t* dep1 = celix_dmServiceDependency_create(); // <-----------------------------------<4>
    celix_dmServiceDependency_setService(dep1, CELIX_SHELL_COMMAND_SERVICE_NAME, NULL, NULL); // <-------------------<5>
    celix_dmServiceDependency_setStrategy(dep1, DM_SERVICE_DEPENDENCY_STRATEGY_SUSPEND); // <------------------------<6>
    celix_dmServiceDependency_setRequired(dep1, true); // <----------------------------------------------------------<7>
    celix_dm_service_dependency_callback_options_t opts1 = CELIX_EMPTY_DM_SERVICE_DEPENDENCY_CALLBACK_OPTIONS; // <--<8>
    opts1.set = (void*)componentWithServiceDependency_setHighestRankingShellCommand; // <----------------------------<9>
    celix_dmServiceDependency_setCallbacksWithOptions(dep1, &opts1); // <-------------------------------------------<10>
    celix_dmComponent_addServiceDependency(dmCmp, dep1); // <-------------------------------------------------------<11>

    //创建可选服务依赖项，可跟踪多个服务，服务更新采用锁定策略
    celix_dm_service_dependency_t* dep2 = celix_dmServiceDependency_create();
    celix_dmServiceDependency_setService(dep2, CELIX_SHELL_COMMAND_SERVICE_NAME, NULL, NULL);
    celix_dmServiceDependency_setStrategy(dep2, DM_SERVICE_DEPENDENCY_STRATEGY_LOCKING);  // <----------------------<12>
    celix_dmServiceDependency_setRequired(dep2, false); // <--------------------------------------------------------<13>
    celix_dm_service_dependency_callback_options_t opts2 = CELIX_EMPTY_DM_SERVICE_DEPENDENCY_CALLBACK_OPTIONS;
    opts2.addWithProps = (void*)componentWithServiceDependency_addShellCommand;  // <-------------------------------<14>
    opts2.removeWithProps = (void*)componentWithServiceDependency_removeShellCommand;
    celix_dmServiceDependency_setCallbacksWithOptions(dep2, &opts2);
    celix_dmComponent_addServiceDependency(dmCmp, dep2);

    //将服务组件添加到依赖管理器
    celix_dependency_manager_t* dm = celix_bundleContext_getDependencyManager(ctx);
    celix_dependencyManager_add(dm, dmCmp);
    return CELIX_SUCCESS;
}

CELIX_GEN_BUNDLE_ACTIVATOR(
        component_with_service_dependency_activator_t,
        componentWithServiceDependencyActivator_start,
        NULL)
```

## 示例：具有服务依赖项的C++服务组件
以下示例展示了一个C++服务组件，该组件具有两个服务依赖项。
其中一个依赖于`celix::IShellCommand` C++服务，另一个依赖于`celix_shell_command_t` C服务。

`celix::IShellCommand` 服务依赖项是一个配置了挂起更新策略的必需依赖，使用`set`回调以确保注入的始终是优先级最高的单一服务。
注意，如果没有匹配的服务，优先级最高的服务为一个空的`shared_ptr`。

`celix_shell_command_t` 服务依赖项是一个配置了锁定更新策略的可选依赖，使用 `addWithProperties` 和 `removeWithProperties` 回调。
每一个被添加/移除的 `celix_shell_command_t`服务都会触发对应的回调。这些回调不仅提供服务的`shared_ptr`，还提供关联的服务属性。

注意，对于C++服务组件来说，C++和C服务依赖项之间没有本质区别；
在两种情况下，服务的指针都以`shared_ptr`形式注入，如果适用，服务属性和捆绑包参数也会以指向`celix::Properties`和`celix::Bundle`的`shared_ptr`形式提供。

C++示例的备注：
1. 创建一个互斥锁来保护`shellCommands` 字段，该字段将由配置了锁定更新策略的服务依赖项的服务依赖回调来维护。
2. 更新 `highestRankingShellCmd` 字段而不加锁。
   注意，由于对应服务依赖项配置了挂起更新策略，`ComponentWithServiceDependency::setHighestRankingShellCommand` 方法仅在服务组件处于 `Suspended` 状态，或者不在 `Active` 复合状态时被调用。
3. 锁定互斥锁并将新添加的服务加入 `shellCommands` 列表。
   注意，由于服务依赖项配置为锁定策略，所以 `ComponentWithServiceDependency::addCShellCmd` 和 `ComponentWithServiceDependency::removeCShellCmd` 方法可以在任何服务组件生命周期状态调用。
4. 创建一个新的服务依赖项。除非经过构建（在服务依赖项、服务组件或依赖管理器上调用`build()`方法），否则服务依赖项处于未完成状态。
   注意，`celix::dm::Component::createServiceDependency`方法在没有提供服务名称的情况下被调用，服务名称将从 `celix::typeName` 推断。
5. 配置服务依赖项`set`回调。
6. 将服务依赖项配置为必需服务依赖项。
7. 将服务依赖项的更新策略配置为挂起。
8. 创建另一个新的服务依赖项，这一次明确提供了要使用的服务名称（`CELIX_SHELL_COMMAND_SERVICE_NAME`）。
9. 构建服务组件并构建服务组件中各服务依赖项（即，将它们标记为完成）。

```C++
//src/ComponentWithServiceDependencyActivator.cc
#include <celix/BundleActivator.h>
#include <celix/IShellCommand.h>
#include <celix_shell_command.h>

class ComponentWithServiceDependency {
public:
    void setHighestRankingShellCommand(const std::shared_ptr<celix::IShellCommand>& cmdSvc) {
        std::cout << "New highest ranking service (can be NULL): " << (intptr_t)cmdSvc.get() << std::endl;
        highestRankingShellCmd = cmdSvc; // <------------------------------------------------------------------------<2>
    }

    void addCShellCmd(
            const std::shared_ptr<celix_shell_command_t>& cmdSvc,
            const std::shared_ptr<const celix::Properties>& props) {
        auto id = props->getAsLong(celix::SERVICE_ID, -1);
        std::cout << "Adding shell command service with service.id: " << id << std::endl;
        std::lock_guard lck{mutex}; // <-----------------------------------------------------------------------------<3>
        shellCommands.emplace(id, cmdSvc);
    }

    void removeCShellCmd(
            const std::shared_ptr<celix_shell_command_t>& /*cmdSvc*/,
            const std::shared_ptr<const celix::Properties>& props) {
        auto id = props->getAsLong(celix::SERVICE_ID, -1);
        std::cout << "Removing shell command service with service.id: " << id << std::endl;
        std::lock_guard lck{mutex};
        shellCommands.erase(id);
    }
private:
    std::shared_ptr<celix::IShellCommand> highestRankingShellCmd{};
    std::mutex mutex{}; //保护 shellCommands // <---------------------------------------------------------------------<1>
    std::unordered_map<long, std::shared_ptr<celix_shell_command_t>> shellCommands{};
};

class ComponentWithServiceDependencyActivator {
public:
    explicit ComponentWithServiceDependencyActivator(const std::shared_ptr<celix::BundleContext>& ctx) {
        using Cmp = ComponentWithServiceDependency;
        auto& cmp = ctx->getDependencyManager()->createComponent<Cmp>(); 

        cmp.createServiceDependency<celix::IShellCommand>() // <-----------------------------------------------------<4>
                .setCallbacks(&Cmp::setHighestRankingShellCommand) // <----------------------------------------------<5>
                .setRequired(true) // <------------------------------------------------------------------------------<6>
                .setStrategy(DependencyUpdateStrategy::suspend); // <------------------------------------------------<7>

        cmp.createServiceDependency<celix_shell_command_t>(CELIX_SHELL_COMMAND_SERVICE_NAME) // <--------------------<8>
                .setCallbacks(&Cmp::addCShellCmd, &Cmp::removeCShellCmd) 
                .setRequired(false)
                .setStrategy(DependencyUpdateStrategy::locking);

        cmp.build(); // <--------------------------------------------------------------------------------------------<9>
    }
};

CELIX_GEN_CXX_BUNDLE_ACTIVATOR(ComponentWithServiceDependencyActivator)
```
# 服务组件何时会被挂起
只有在以下情况下，服务组件才会被挂起：
- 服务组件处于 `Tracking Optional` 状态；
- 一个服务更新事件正在进行；
- 服务组件存在一个与该服务更新事件匹配的配置了挂起策略且配置了服务注入/移除回调的服务依赖项。

# `celix::dm` Shell命令
为了交互式地查看可用的服务组件、它们当前的生命周期状态、对外提供的服务和服务依赖项，可以使用`celix::dm` Shell命令。

支持的`dm`命令行用法示例：
- `celix::dm` - 显示Celix框架中所有服务组件的概览。只显示服务组件的生命周期状态。
- `dm` - 与`celix::dm`相同（只要没有其他冲突的`dm`命令）。
- `dm full` - 显示Celix框架中所有服务组件的详细概览。这也会显示每个服务组件对外提供的服务和服务依赖项。