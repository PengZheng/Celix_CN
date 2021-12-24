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

#pragma once

#include <memory>

#include "celix_filter.h"
#include "celix/Properties.h"

namespace celix {

    /**
     * @brief FilterException
     */
    class FilterException : public std::exception {
    public:
        explicit FilterException(std::string msg) : w{std::move(msg)} {}

        FilterException(const FilterException&) = default;
        FilterException(FilterException&&) = default;
        FilterException& operator=(const FilterException&) = default;
        FilterException& operator=(FilterException&&) = default;

        [[nodiscard]] const char* what() const noexcept override {
            return w.c_str();
        }
    private:
        std::string w;
    };

    /**
     * @brief An RFC 1960-based (LDAP) Filter.
     *
     * Some examples:
     *        "(cn=Babs Jensen)"
     *        "(!(cn=Tim Howes))"
     *        "(&(" + celix::Constants::SERVICE_NAME + "=Person)(|(sn=Jensen)(cn=Babs J*)))"
     *
     * @note Not thread safe.
     */
    class Filter {
    public:
        Filter() : cFilter{createFilter("")} {}
        explicit Filter(const std::string& filterStr) : cFilter{createFilter(filterStr)} {}

        Filter(Filter&&) = default;
        Filter& operator=(Filter&&) = default;

        Filter(const Filter& rhs) : cFilter{createFilter(rhs.getFilterString())} {}

        Filter& operator=(const Filter& rhs) {
            if (this != &rhs) {
                cFilter = createFilter(rhs.getFilterString());
            }
            return *this;
        }

        /**
         * @brief Warps a C filter to a C++ filter
         * @warning Takes no ownership. Dealloction still has to be done the "C way".
         */
        static Filter wrap(celix_filter_t* f) {
            return Filter{f};
        }

        /**
         * @brief Gets the filter string
         */
        [[nodiscard]] std::string getFilterString() const {
            auto cStr = getFilterCString();
            return cStr == nullptr ? std::string{} : std::string{cStr};
        }

        /**
         * @brief Get the C string. valid as long as the filter object is valid.
         */
        [[nodiscard]] const char* getFilterCString() const {
            return celix_filter_getFilterString(cFilter.get());
        }

        /**
         * @brief match the filter against the provided properties
         */
        [[nodiscard]] bool match(const celix::Properties& properties)  const {
            return celix_filter_match(cFilter.get(), properties.getCProperties());
        }

        /**
         * @brief Find the attribute based on the provided key.
         * @return The found attribute value or an empty string if the attribute was not found.
         */
        [[nodiscard]] std::string findAttribute(std::string_view attributeKey) const {
            auto* cValue = celix_filter_findAttribute(cFilter.get(), attributeKey.data());
            return cValue == nullptr ? std::string{} : std::string{cValue};
        }

        /**
         * @brief Check whether the filter has a attribute with the provided attribute key.
         */
        [[nodiscard]] bool hasAttribute(std::string_view attributeKey) const {
            return celix_filter_findAttribute(cFilter.get(), attributeKey.data()) != nullptr;
        }

        /**
         * @brief Check whether the filter indicates the mandatory presence of an attribute with a specific value for the provided attribute key.
         *
         * Example:
         *   using this method for attribute key "key1" on filter "(key1=value1)" yields true.
         *   using this method for attribute key "key1" on filter "(!(key1=value1))" yields false.
         *   using this method for attribute key "key1" on filter "(key1>=value1)" yields false.
         *   using this method for attribute key "key1" on filter "(|(key1=value1)(key2=value2))" yields false.
         */
        [[nodiscard]] bool hasMandatoryEqualsValueAttribute(std::string_view attributeKey) const {
             return celix_filter_hasMandatoryEqualsValueAttribute(cFilter.get(), attributeKey.data());
        }

        /**
         * @brief Chek whether the filter indicates the mandatory absence of an attribute, regardless of its value, for the provided attribute key.
         *
         * example:
         *   using this function for attribute key "key1" on the filter "(!(key1=*))" yields true.
         *   using this function for attribute key "key1" on the filter "(key1=*) yields false.
         *   using this function for attribute key "key1" on the filter "(key1=value)" yields false.
         *   using this function for attribute key "key1" on the filter "(|(!(key1=*))(key2=value2))" yields false.
         */
        [[nodiscard]] bool hasMandatoryNegatedPresenceAttribute(std::string_view attributeKey) const {
            return celix_filter_hasMandatoryNegatedPresenceAttribute(cFilter.get(), attributeKey.data());
        }

        /**
         * @brief Get the underlining C filter object.
         *
         * @warning Try not the depend on the C API from a C++ bundle. If features are missing these should be added to
         * the C++ API.
         */
        [[nodiscard]] celix_filter_t* getCFilter() const {
            return cFilter.get();
        }

        /**
         * @brief Return whether the filter is empty.
         */
        [[nodiscard]] bool empty() const {
            return cFilter == nullptr;
        }

    private:
        static std::shared_ptr<celix_filter_t> createFilter(const std::string& filterStr) {
            if (filterStr.empty()) {
                return nullptr;
            }
            auto* cf = celix_filter_create(filterStr.c_str());
            if (cf == nullptr) {
                throw celix::FilterException{"Invalid LDAP filter '" + filterStr + "'"};
            }
            return std::shared_ptr<celix_filter_t>{cf, [](celix_filter_t *f) {
                celix_filter_destroy(f);
            }};
        }

        explicit Filter(celix_filter_t* f) : cFilter{f, [](celix_filter_t*){/*nop*/}} {}

        std::shared_ptr<celix_filter_t> cFilter;
    };
}