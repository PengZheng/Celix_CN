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

#ifndef __DYN_FUNCTION_H_
#define __DYN_FUNCTION_H_

#include "dyn_type.h"
#include "dfi_log_util.h"
#include "celix_dfi_export.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Uses the following schema
 * (Name)([Type]*)Type
 *
 * Dyn function argument meta (am) as meta info, with the following possible values
 * am=handle #void pointer for the handle
 * am=pre #output pointer with memory pre-allocated
 * am=out #output pointer
 *
 * text argument (t) can also be annotated to be considered const string.
 * Normally a text argument will be handled as char*, meaning that the callee is expected to take of ownership.
 * If a const=true annotation is used the text argument will be handled as a const char*, meaning that the caller
 * keeps ownership of the string.
 */

typedef struct _dyn_function_type dyn_function_type;

DFI_SETUP_LOG_HEADER(dynFunction);
DFI_SETUP_LOG_HEADER(dynAvprFunction);

enum dyn_function_argument_meta {
    DYN_FUNCTION_ARGUMENT_META__STD = 0,
    DYN_FUNCTION_ARGUMENT_META__HANDLE = 1,
    DYN_FUNCTION_ARGUMENT_META__PRE_ALLOCATED_OUTPUT = 2,
    DYN_FUNCTION_ARGUMENT_META__OUTPUT = 3
};

/**
 * @brief Creates a dyn_function_type according to the given function descriptor stream.
 *
 * The caller is the owner of the dynFunc and the dynFunc should be freed using dynFunction_destroy.
 *
 * In case of an error, an error message is added to celix_err.
 *
 * @param[in] descriptorStream The stream containing the function descriptor.
 * @param[in] refTypes A list if reference-able dyn types.
 * @param[out] dynFunc The created dynamic type instance for function.
 * @return 0 If successful
 * @retval 1 If there is not enough memory to create dyn_function_type.
 * @retval 2 Errors other than out-of-memory.
 */
CELIX_DFI_EXPORT int dynFunction_parse(FILE *descriptorStream, struct types_head *refTypes, dyn_function_type **dynFunc);

/**
 * @brief Creates a dyn_function_type according to the given function descriptor string.
 *
 * The caller is the owner of the dynFunc and the dynFunc should be freed using dynFunction_destroy.
 *
 * In case of an error, an error message is added to celix_err.
 *
 * @param[in] descriptor The string containing the function descriptor.
 * @param[in] refTypes A list if reference-able dyn types.
 * @param[out] dynFunc The created dynamic type instance for function.
 * @return 0 If successful
 * @retval 1 If there is not enough memory to create dyn_function_type.
 * @retval 2 Errors other than out-of-memory.
 */
CELIX_DFI_EXPORT int dynFunction_parseWithStr(const char *descriptor, struct types_head *refTypes, dyn_function_type **dynFunc);

/**
 * @brief Returns the number of arguments of the given dynamic function type instance.
 * @param[in] dynFunc The dynamic type instance for function.
 * @return The number of arguments.
 */
CELIX_DFI_EXPORT int dynFunction_nrOfArguments(dyn_function_type *dynFunc);

/**
 * @brief Returns the argument type for the given argument index.
 * @param[in] dynFunc The dynamic type instance for function.
 * @param[in] argumentNr The argument index.
 * @return The argument type.
 */
CELIX_DFI_EXPORT dyn_type *dynFunction_argumentTypeForIndex(dyn_function_type *dynFunc, int argumentNr);

/**
 * @brief Returns the argument meta for the given argument index.
 * @param[in] dynFunc The dynamic type instance for function.
 * @param[in] argumentNr The argument index.
 * @return The argument meta.
 */
CELIX_DFI_EXPORT enum dyn_function_argument_meta dynFunction_argumentMetaForIndex(dyn_function_type *dynFunc, int argumentNr);

/**
 * @brief Returns the return value type for the given dynamic function type instance.
 * @param[in] dynFunc The dynamic type instance for function.
 * @return The return value type.
 */
CELIX_DFI_EXPORT dyn_type * dynFunction_returnType(dyn_function_type *dynFunction);

/**
 * @brief Destroys the given dynamic function type instance.
 * @param[in] dynFunc The dynamic type instance for function.
 */
CELIX_DFI_EXPORT void dynFunction_destroy(dyn_function_type *dynFunc);

/**
 * @brief Calls the given dynamic type function.
 * @param[in] dynFunc The dynamic type instance for function.
 * @param[in] fn The function pointer to call.
 * @param[in] returnValue The return value pointer.
 * @param[in] argValues The argument values.
 * @return 0
 */
CELIX_DFI_EXPORT int dynFunction_call(dyn_function_type *dynFunc, void(*fn)(void), void *returnValue, void **argValues);

/**
 * @brief Creates a closure for the given dynamic function type instance.
 * @param[in] func The dynamic type instance for function.
 * @param[in] bind The bind function to use for the closure.
 * @param[in] userData The user data to use for the closure.
 * @param[out] fn The function pointer to call.
 * @return 0 If successful
 * @retval 1 If there is not enough memory to create the closure.
 * @retval 2 Errors other than out-of-memory.
 */
CELIX_DFI_EXPORT int dynFunction_createClosure(dyn_function_type *func, void (*bind)(void *, void **, void*), void *userData, void(**fn)(void));

/**
 * @brief Returns the function pointer for the given dynamic function type instance.
 * @param[in] func The dynamic type instance for function.
 * @param[out] fn The function pointer.
 * @return 0 If successful, 1 if the dynamic function type instance has no function pointer.
 */
CELIX_DFI_EXPORT int dynFunction_getFnPointer(dyn_function_type *func, void (**fn)(void));

/**
 * Returns whether the function has a return type.
 * Will return false if return is void.
 */
CELIX_DFI_EXPORT bool dynFunction_hasReturn(dyn_function_type *dynFunction);

/**
 * @brief Creates a dyn_function_type according to the given avpr descriptor string.
 *
 * The caller is the owner of the dynFunc and the dynFunc should be freed using dynFunction_destroy.
 *
 * In case of an error, an error message is added to celix_err.
 *
 * @param[in] avpr The string containing the avpr descriptor.
 * @param[in] fqn The fully qualified name of the function.
 * @return The dynamic type instance for function or NULL if the avpr could not be parsed.
 * @deprecated AVRO is deprecated and will be removed in the future.
 */
CELIX_DFI_DEPRECATED_EXPORT dyn_function_type * dynFunction_parseAvprWithStr(const char * avpr, const char * fqn);

/**
 * @brief Creates a dyn_function_type according to the given avpr descriptor stream.
 *
 * The caller is the owner of the dynFunc and the dynFunc should be freed using dynFunction_destroy.
 *
 * In case of an error, an error message is added to celix_err.
 *
 * @param[in] avprStream The stream containing the avpr descriptor.
 * @param[in] fqn The fully qualified name of the function.
 * @return The dynamic type instance for function or NULL if the avpr could not be parsed.
 * @deprecated AVRO is deprecated and will be removed in the future.
 */
CELIX_DFI_DEPRECATED_EXPORT dyn_function_type * dynFunction_parseAvpr(FILE * avprStream, const char * fqn);

#ifdef __cplusplus
}
#endif

#endif
