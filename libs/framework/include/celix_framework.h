/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef CELIX_FRAMEWORK_H_
#define CELIX_FRAMEWORK_H_

#include <stdarg.h>

#include "celix_types.h"
#include "celix_properties.h"
#include "celix_log_level.h"
#include "celix_array_list.h"
#include "celix_framework_export.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief The Celix Framework API.
 *
 * The Celix Framework API provides the functionality to create and destroy a Celix framework instance.
 *
 * The Celix framework instance provides the following functionality:
 *  - Bundle management
 *  - Service management
 *  - Event management
 *  - Logging
 *  - Framework config properties
 *  - Framework events
 *  - Framework shutdown
 *
 * @note The Celix framework instance is thread safe.
 */

/**
 * @brief Returns the framework UUID. This is unique for every created framework and will not be the same if the process is
 * restarted.
 */
CELIX_FRAMEWORK_EXPORT const char* celix_framework_getUUID(const celix_framework_t *fw);

/**
 * @brief Returns the framework bundle context. This is the same as a 'normal' bundle context and can be used to register, use
 * and track services. The only difference is that the framework is the bundle.
 * @param fw The framework
 * @return A pointer to the bundle context of the framework or NULL if something went wrong.
 */
CELIX_FRAMEWORK_EXPORT celix_bundle_context_t* celix_framework_getFrameworkContext(const celix_framework_t *fw);

/**
 * @brief Returns the framework bundle. This is the same as a 'normal' bundle, expect that this bundle cannot be uninstalled
 * and the `celix_bundle_getEntry` return a entries relative from the working directory.
  * @param fw The framework
 * @return A pointer to the bundle of the framework or NULL if something went wrong.
 */
CELIX_FRAMEWORK_EXPORT celix_bundle_t* celix_framework_getFrameworkBundle(const celix_framework_t *fw);

/**
 * @brief Use the currently installed bundles.
 * The provided callback will be called for all the currently installed bundles.
 *
 * @warning It is dangerous to use the provided bundle's context from the callback, since it may be invalid for an inactive bundle.
 *
 * @param fw                        The framework.
 * @param includeFrameworkBundle    If true the callback will also be triggered for the framework bundle.
 * @param callbackHandle            The data pointer, which will be used in the callbacks
 * @param use                       The callback which will be called for the currently installed bundles.
 *                                  The bundle pointers are only guaranteed to be valid during the callback.
 * @return                          The number of times the use callback is called.
 */
CELIX_FRAMEWORK_EXPORT size_t celix_framework_useBundles(celix_framework_t* fw,
                                                         bool includeFrameworkBundle,
                                                         void* callbackHandle,
                                                         void (*use)(void* handle, const celix_bundle_t* bnd));

/**
 * @brief Use the currently active bundles.
 * The provided callback will be called for all the currently active bundles.
 * The bundle state is guaranteed to be active during the callback.
 *
 * @warning Calling synchronous bundle-state changing functions (e.g. celix_bundleContext_stopBundle) from the callback
 * will lead to deadlocks.
 *
 * @param fw                        The framework.
 * @param includeFrameworkBundle    If true the callback will also be triggered for the framework bundle.
 * @param callbackHandle            The data pointer, which will be used in the callbacks
 * @param use                       The callback which will be called for the currently active bundles.
 * @return                          The number of times the use callback is called.
 */
CELIX_FRAMEWORK_EXPORT size_t celix_framework_useActiveBundles(celix_framework_t* fw,
                                                               bool includeFrameworkBundle,
                                                               void* callbackHandle,
                                                               void (*use)(void* handle, const celix_bundle_t* bnd));

/**
 * @brief Use the bundle with the provided bundle id
 * The provided callback will be called if the bundle is found.
 *
 * @warning Calling synchronous bundle-state changing functions (e.g. celix_bundleContext_stopBundle) with onlyActive=true
 * from the callback will lead to deadlocks. Using a bundle's context, e.g. calling celix_bundle_listServiceTrackers,
 * with onlyActive=false from the callback is generally dangerous. However, in some cases, the target bundle's context is guaranteed to be valid,
 * e.g. the bundle is providing a service protected by a service tracker.
 *
 * @param fw                The framework.
 * @param onlyActive        If true only starting and active bundles will trigger the callback.
 * @param bundleId          The bundle id.
 * @param callbackHandle    The data pointer, which will be used in the callbacks
 * @param use               The callback which will be called for the currently started bundles.
 *                          The bundle pointers are only guaranteed to be valid during the callback.
 * @return                  Returns true if the bundle is found and the callback is called.
 */
CELIX_FRAMEWORK_EXPORT bool celix_framework_useBundle(celix_framework_t* fw,
                                                      bool onlyActive,
                                                      long bndId,
                                                      void* callbackHandle,
                                                      void (*use)(void* handle, const celix_bundle_t* bnd));

/**
 * @brief Check whether a bundle is installed.
 * @param fw        The Celix framework
 * @param bndId     The bundle id to check
 * @return          true if the bundle is installed.
 */
CELIX_FRAMEWORK_EXPORT bool celix_framework_isBundleInstalled(celix_framework_t *fw, long bndId);

/**
 * @brief Check whether the bundle is active.
 * @param fw        The Celix framework
 * @param bndId     The bundle id to check
 * @return          true if the bundle is installed and active.
 */
CELIX_FRAMEWORK_EXPORT bool celix_framework_isBundleActive(celix_framework_t *fw, long bndId);


/**
 * @brief Install and optional start a bundle.
 * Will silently ignore bundle ids < 0.
 *
 * @param fw The Celix framework
 * @param bundleLoc The bundle location to the bundle zip file.
 * @param autoStart If the bundle should also be started.
 * @return the bundleId (>= 0) or < 0 if the bundle could not be installed and possibly started.
 */
CELIX_FRAMEWORK_EXPORT long celix_framework_installBundle(celix_framework_t *fw, const char *bundleLoc, bool autoStart);

/**
 * @brief Uninstall the bundle with the provided bundle id. If needed the bundle will be stopped first.
 * Will silently ignore bundle ids < 0.
 *
 * @param fw The Celix framework
 * @param bndId The bundle id to uninstall.
 * @return true if the bundle is correctly uninstalled. False if not.
 */
CELIX_FRAMEWORK_EXPORT bool celix_framework_uninstallBundle(celix_framework_t *fw, long bndId);

/**
 * @brief Unload the bundle with the provided bundle id. If needed the bundle will be stopped first.
 * Will silently ignore bundle ids < 0.
 * Note that unloaded bundle is kept in bundle cache and can be reloaded with the celix_framework_installBundle function.
 *
 * @param fw The Celix framework
 * @param bndId The bundle id to unload.
 * @return true if the bundle is correctly unloaded. False if not.
 */
CELIX_FRAMEWORK_EXPORT bool celix_framework_unloadBundle(celix_framework_t *fw, long bndId);

/**
 * @brief Update the bundle with the provided bundle id.
 *
 * This will do the following:
 *  - unload the bundle with the specified bundle id;
 *  - reload the bundle from the specified location with the specified bundle id;
 *  - start the bundle, if it was previously active.
 *
 *  Will silently ignore bundle ids < 0.
 *
 *  Note if specified bundle location already exists in the bundle cache but with a different bundle id, the bundle
 *  will NOT be reloaded, and the update is cancelled.
 *
 * @param [in] fw The Celix framework
 * @param [in] bndId the bundle id to update.
 * @param [in] updatedBundleUrl The optional updated bundle url to the bundle zip file.
 * If NULL, the existing bundle url from the bundle cache will be used, and the cache will only be updated if the zip file is newer.
 * @return true if the bundle is correctly updated. False if not.
 */
CELIX_FRAMEWORK_EXPORT bool celix_framework_updateBundle(celix_framework_t *fw, long bndId, const char* updatedBundleUrl);

/**
 * @brief Stop the bundle with the provided bundle id.
 * Will silently ignore bundle ids < 0.
 *
 * @param fw The Celix framework
 * @param bndId The bundle id to stop.
 * @return true if the bundle is found & correctly stop. False if not.
 */
CELIX_FRAMEWORK_EXPORT bool celix_framework_stopBundle(celix_framework_t *fw, long bndId);

/**
 * @brief Start the bundle with the provided bundle id.
 * Will silently ignore bundle ids < 0.
 *
 * @param fw The Celix framework
 * @param bndId The bundle id to start.
 * @return true if the bundle is found & correctly started. False if not.
 */
CELIX_FRAMEWORK_EXPORT bool celix_framework_startBundle(celix_framework_t *fw, long bndId);

/**
 * @brief Install and optional start a bundle async.
 * Will silently ignore bundle ids < 0.
 *
 * If the bundle needs to be started this will be done a separate spawned thread.
 *
 * @param fw The Celix framework
 * @param bundleLoc The bundle location to the bundle zip file.
 * @param autoStart If the bundle should also be started.
 * @return The bundle id of the installed bundle or -1 if the bundle could not be installed
 */
CELIX_FRAMEWORK_EXPORT long celix_framework_installBundleAsync(celix_framework_t *fw, const char *bundleLoc, bool autoStart);

/**
 * @brief Update the bundle with the provided bundle id async.
 *
 * This will do the following:
 *  - unload the bundle with the specified bundle id;
 *  - reload the bundle from the specified location with the specified bundle id;
 *  - start the bundle, if it was previously active.
 *
 *  Will silently ignore bundle ids < 0.
 *
 *  Note if specified bundle location already exists in the bundle cache but with a different bundle id, the bundle
 *  will NOT be reinstalled, and the update is cancelled.
 *
 * @param [in] fw The Celix framework
 * @param [in] bndId the bundle id to update.
 * @param [in] updatedBundleUrl The optional updated bundle url to the bundle zip file.
 * If NULL, the existing bundle url from the bundle cache will be used, and the cache will only be updated if the zip file is newer.
 */
CELIX_FRAMEWORK_EXPORT void celix_framework_updateBundleAsync(celix_framework_t *fw, long bndId, const char* updatedBundleUrl);

/**
 * @brief Uninstall the bundle with the provided bundle id async. If needed the bundle will be stopped first.
 * Will silently ignore bundle ids < 0.
 *
 * The bundle will be uninstalled on a separate spawned thread.
 *
 * @param fw The Celix framework
 * @param bndId The bundle id to uninstall.
 */
CELIX_FRAMEWORK_EXPORT void celix_framework_uninstallBundleAsync(celix_framework_t *fw, long bndId);

/**
 * @brief Unload the bundle with the provided bundle id async. If needed the bundle will be stopped first.
 * Will silently ignore bundle ids < 0.
 * Note that unloaded bundle is kept in bundle cache and can be reloaded with the celix_framework_installBundle function.
 * The bundle will be unloaded on a separate spawned thread.
 *
 * @param fw The Celix framework
 * @param bndId The bundle id to unload.
 */
CELIX_FRAMEWORK_EXPORT void celix_framework_unloadBundleAsync(celix_framework_t *fw, long bndId);

/**
 * @brief Stop the bundle with the provided bundle id async.
 * Will silently ignore bundle ids < 0.
 *
 * The bundle will be stopped on a separate spawned thread.
 *
 * @param fw The Celix framework
 * @param bndId The bundle id to stop.
 */
CELIX_FRAMEWORK_EXPORT void celix_framework_stopBundleAsync(celix_framework_t *fw, long bndId);

/**
 * @brief Start the bundle with the provided bundle id async.
 * Will silently ignore bundle ids < 0.
 *
 * The bundle will be started on a separate spawned thread.
 *
 * @param fw The Celix framework
 * @param bndId The bundle id to start.
 */
CELIX_FRAMEWORK_EXPORT void celix_framework_startBundleAsync(celix_framework_t *fw, long bndId);

/**
 * @brief List the installed and started bundle ids.
 * The bundle ids does not include the framework bundle (bundle id CELIX_FRAMEWORK_BUNDLE_ID).
 *
 * @param framework The Celix framework.
 * @return A array with bundle ids (long). The caller is responsible for destroying the array.
 */
CELIX_FRAMEWORK_EXPORT celix_array_list_t* celix_framework_listBundles(celix_framework_t* framework);

/**
 * @brief List the installed bundle ids.
 * The bundle ids does not include the framework bundle (bundle id CELIX_FRAMEWORK_BUNDLE_ID).
 *
 * @param framework The Celix framework.
 * @return A array with bundle ids (long). The caller is responsible for destroying the array.
 */
CELIX_FRAMEWORK_EXPORT celix_array_list_t* celix_framework_listInstalledBundles(celix_framework_t* framework);

/**
 * @brief Sets the log function for this framework.
 * Default the celix framework will log to stdout/stderr.
 *
 * A log function can be injected to change how the Celix framework logs.
 * Can be reset by setting the log function to NULL.
 */
CELIX_FRAMEWORK_EXPORT void celix_framework_setLogCallback(celix_framework_t* fw, void* logHandle, void (*logFunction)(void* handle, celix_log_level_e level, const char* file, const char *function, int line, const char *format, va_list formatArgs));

/**
 * @brief Wait until the framework event queue is empty.
 *
 * The Celix framework has an event queue which (among others) handles various events.
 * This function can be used to ensure that all queue events are handled.
 * 
 * Note scheduled events are not part of the event queue.
 *
 * @param fw The Celix Framework
 */
CELIX_FRAMEWORK_EXPORT void celix_framework_waitForEmptyEventQueue(celix_framework_t *fw);

/**
 * @brief Wait until the framework event queue is empty or the provided period is reached.
 *
 * The Celix framework has an event queue which (among others) handles various events.
 * This function can be used to ensure that all queue events are handled.
 * 
 * Note scheduled events are not part of the event queue.
 *
 * @param[in] fw The Celix Framework.
 * @param[in] timeoutInSeconds The period in seconds to wait for the event queue to be empty. 0 means wait forever.
 * @return CELIX_SUCCESS if the event queue is empty or ETIMEDOUT if the timeoutInSeconds is reached.
 */
CELIX_FRAMEWORK_EXPORT celix_status_t celix_framework_waitForEmptyEventQueueFor(celix_framework_t *fw, double timeoutInSeconds);

/**
 * @brief wait until all events from the event queue for the bundle identified by the bndId are processed.
 *
 * If bndId < 0, wait until all bundle events (events associated with a bundle) from the event queue are processed.
 * Note scheduled events are not part of the event queue.
 * 
 */
CELIX_FRAMEWORK_EXPORT void celix_framework_waitUntilNoEventsForBnd(celix_framework_t* fw, long bndId);

/**
 * @brief wait until all pending service registration are processed.
 */
CELIX_FRAMEWORK_EXPORT void celix_framework_waitUntilNoPendingRegistration(celix_framework_t* fw);

/**
 * @brief Returns whether the current thread is the Celix framework event loop thread.
 *
 */
CELIX_FRAMEWORK_EXPORT bool celix_framework_isCurrentThreadTheEventLoop(celix_framework_t* fw);

/**
 * @brief Fire a generic event. The event will be added to the event loop and handled on the event loop thread.
 *
 * The process callback should be fast and non-blocking, otherwise
 * the framework event queue will be blocked and framework will not function properly.
 *
 * if bndId >=0 the bundle usage count will be increased while the event is not yet processed or finished processing.
 * The name is expected to be const char* valid during til the event is finished processing.
 *
 * if eventId >=0 this will be used, otherwise a new event id will be generated
 * return eventId
 */
CELIX_FRAMEWORK_EXPORT long celix_framework_fireGenericEvent(celix_framework_t* fw,
                                                             long eventId,
                                                             long bndId,
                                                             const char* eventName,
                                                             void* processData,
                                                             void (*processCallback)(void* data),
                                                             void* doneData,
                                                             void (*doneCallback)(void* doneData));

/**
 * @brief Get the next event id.
 *
 * This can be used to ensure celix_framework_waitForGenericEvent can be used to wait for an event.
 * The returned event id will not be used by the framework itself unless followed up with a
 * celix_framework_fireGenericEvent call using the returned event id.
 */
CELIX_FRAMEWORK_EXPORT long celix_framework_nextEventId(celix_framework_t *fw);

/**
 * @brief Wait until a event from the event queue with the provided event id is completely handled.
 * This function will directly return if the provided event id is not in the event loop (already done or never issued).
 * 
 * Note scheduled events are not part of the event queue.
 */
CELIX_FRAMEWORK_EXPORT void celix_framework_waitForGenericEvent(celix_framework_t *fw, long eventId);

/**
 * @brief Wait until the framework is stopped.
 */
CELIX_FRAMEWORK_EXPORT void celix_framework_waitForStop(celix_framework_t *framework);

#ifdef __cplusplus
}
#endif

#endif /* CELIX_FRAMEWORK_H_ */
