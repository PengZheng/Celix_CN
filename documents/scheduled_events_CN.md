---
title: Apache Celix 计划任务事件
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

# Apache Celix 计划任务事件

Apache Celix 支持在 Apache Celix 事件线程上调度事件。
这使用户能够复用现有的 Apache Celix 事件线程来执行任务。

计划任务事件将使用提供的间隔重复触发，若只提供了初始延迟，则只会触发一次。我们使用单调时钟计算间隔时间。

事件回调的处理应相对较快，计划任务事件的间隔应相对较长；否则，框架事件队列中将堆积大量待处理事件，Celix框架将无法正常工作。
不要在事件处理回调中进行网络IO或者其他可能长时间阻塞的工作，而应该使用单独的线程来处理这类任务。

## 安排一个计划任务事件

可以使用 `celix_bundleContext_scheduleEvent` C函数或`celix::BundleContext::scheduleEvent` C++方法在Apache Celix 框架中安排一个事件。
在C中使用一个选项结构体来配置计划任务事件，而在C++中使用构建器模式来配置计划任务事件。

### C 示例

```c
#include <stdio.h>
#include <celix_bundle_activator.h>

typedef struct schedule_events_bundle_activator_data {
    celix_bundle_context_t* ctx;
    long scheduledEventId;
} schedule_events_bundle_activator_data_t;

void scheduleEventsBundle_oneShot(void* data) {
    schedule_events_bundle_activator_data_t* act = data;
    celix_bundleContext_log(act->ctx, CELIX_LOG_LEVEL_INFO, "One shot scheduled event fired");
}

void scheduleEventsBundle_process(void* data) {
    schedule_events_bundle_activator_data_t* act = data;
    celix_bundleContext_log(act->ctx, CELIX_LOG_LEVEL_INFO, "Recurring scheduled event fired");
}

static celix_status_t scheduleEventsBundle_start(schedule_events_bundle_activator_data_t *data, celix_bundle_context_t *ctx) {
    data->ctx = ctx;

    //安排重复事件
    {
        celix_scheduled_event_options_t opts = CELIX_EMPTY_SCHEDULED_EVENT_OPTIONS;
        opts.name = "recurring scheduled event example";
        opts.initialDelayInSeconds = 0.1;
        opts.intervalInSeconds = 1.0;
        opts.callbackData = data;
        opts.callback = scheduleEventsBundle_process;
        data->scheduledEventId = celix_bundleContext_scheduleEvent(ctx, &opts);
    }

    //安排一次性事件
    {
        celix_scheduled_event_options_t opts = CELIX_EMPTY_SCHEDULED_EVENT_OPTIONS;
        opts.name = "one shot scheduled event example";
        opts.initialDelayInSeconds = 0.1;
        opts.callbackData = data;
        opts.callback = scheduleEventsBundle_oneShot;
        celix_bundleContext_scheduleEvent(ctx, &opts);
    }

    return CELIX_SUCCESS;
}

static celix_status_t scheduleEventsBundle_stop(schedule_events_bundle_activator_data_t *data, celix_bundle_context_t *ctx) {
    celix_bundleContext_removeScheduledEvent(ctx, data->scheduledEventId);
    return CELIX_SUCCESS;
}

CELIX_GEN_BUNDLE_ACTIVATOR(schedule_events_bundle_activator_data_t, scheduleEventsBundle_start, scheduleEventsBundle_stop)
```

### C++ 示例

```cpp
#include <iostream>
#include "celix/BundleActivator.h"

class ScheduleEventsBundleActivator {
public:
    explicit ScheduleEventsBundleActivator(const std::shared_ptr<celix::BundleContext>& ctx) {
        //安排重复事件
        event = ctx->scheduledEvent()
                .withInitialDelay(std::chrono::milliseconds{10})
                .withInterval(std::chrono::seconds{1})
                .withCallback([ctx] {
                    ctx->logInfo("Recurring scheduled event fired");
                })
                .build();

        //安排一次性事件
        ctx->scheduledEvent()
                .withInitialDelay(std::chrono::milliseconds{10})
                .withCallback([ctx] {
                    ctx->logInfo("One shot scheduled event fired");
                })
                .build();
    }

    ~ScheduleEventsBundleActivator() noexcept {
        std::cout << "Goodbye world" << std::endl;
    }
private:
    celix::ScheduledEvent event{};
};

CELIX_GEN_CXX_BUNDLE_ACTIVATOR(ScheduleEventsBundleActivator)
```

## Waking up a Scheduled Event

## 唤醒一个计划任务事件

要直接处理一个计划任务事件，您可以使用 `celix_bundleContext_wakeupScheduledEvent` C函数或`celix::ScheduledEvent::wakup` C++方法。
这将唤醒计划任务事件并调用其回调函数。
