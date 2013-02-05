/**
 *Licensed to the Apache Software Foundation (ASF) under one
 *or more contributor license agreements.  See the NOTICE file
 *distributed with this work for additional information
 *regarding copyright ownership.  The ASF licenses this file
 *to you under the Apache License, Version 2.0 (the
 *"License"); you may not use this file except in compliance
 *with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *Unless required by applicable law or agreed to in writing,
 *software distributed under the License is distributed on an
 *"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 *specific language governing permissions and limitations
 *under the License.
 */
/*
 * service_tracker_customizer.c
 *
 *  \date       Nov 15, 2012
 *  \author     <a href="mailto:celix-dev@incubator.apache.org">Apache Celix Project Team</a>
 *  \copyright  Apache License, Version 2.0
 */

#include <stdlib.h>

#include "service_reference.h"

#include "service_tracker_customizer.h"

static apr_status_t serviceTrackerCustomizer_destroy(void *customizerPointer);

struct serviceTrackerCustomizer {
	void * handle;
	celix_status_t (*addingService)(void * handle, service_reference_t reference, void **service);
	celix_status_t (*addedService)(void * handle, service_reference_t reference, void * service);
	celix_status_t (*modifiedService)(void * handle, service_reference_t reference, void * service);
	celix_status_t (*removedService)(void * handle, service_reference_t reference, void * service);
};

celix_status_t serviceTrackerCustomizer_create(apr_pool_t *pool, void *handle,
		addingCallback addingFunction, addedCallback addedFunction,
		modifiedCallback modifiedFunction, removedCallback removedFunction, service_tracker_customizer_t *customizer) {
	celix_status_t status = CELIX_SUCCESS;

	if (pool == NULL || handle == NULL || *customizer != NULL) {
		status = CELIX_ILLEGAL_ARGUMENT;
	} else {
		*customizer = apr_palloc(pool, sizeof(**customizer));
		if (!*customizer) {
			status = CELIX_ENOMEM;
		} else {
			apr_pool_pre_cleanup_register(pool, *customizer, serviceTrackerCustomizer_destroy);

			(*customizer)->handle = handle;
			(*customizer)->addingService = addingFunction;
			(*customizer)->addedService = addedFunction;
			(*customizer)->modifiedService = modifiedFunction;
			(*customizer)->removedService = removedFunction;
		}
	}

	return status;
}

static apr_status_t serviceTrackerCustomizer_destroy(void *customizerPointer) {
	service_tracker_customizer_t customizer = (service_tracker_customizer_t) customizerPointer;

	customizer->handle = NULL;
	customizer->addingService = NULL;
	customizer->addedService = NULL;
	customizer->modifiedService = NULL;
	customizer->removedService = NULL;

	return APR_SUCCESS;
}

celix_status_t serviceTrackerCustomizer_getHandle(service_tracker_customizer_t customizer, void **handle) {
	celix_status_t status = CELIX_SUCCESS;

	*handle = customizer->handle;

	return status;
}

celix_status_t serviceTrackerCustomizer_getAddingFunction(service_tracker_customizer_t customizer, addingCallback *function) {
	celix_status_t status = CELIX_SUCCESS;

	*function = customizer->addingService;

	return status;
}

celix_status_t serviceTrackerCustomizer_getAddedFunction(service_tracker_customizer_t customizer, addedCallback *function) {
	celix_status_t status = CELIX_SUCCESS;

	*function = customizer->addedService;

	return status;
}

celix_status_t serviceTrackerCustomizer_getModifiedFunction(service_tracker_customizer_t customizer, modifiedCallback *function) {
	celix_status_t status = CELIX_SUCCESS;

	*function = customizer->modifiedService;

	return status;
}

celix_status_t serviceTrackerCustomizer_getRemovedFunction(service_tracker_customizer_t customizer, removedCallback *function) {
	celix_status_t status = CELIX_SUCCESS;

	*function = customizer->removedService;

	return status;
}