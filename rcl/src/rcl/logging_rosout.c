// Copyright 2017 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RCL__LOGGING_H_
#define RCL__LOGGING_H_


#include "rcl/allocator.h"
#include "rcl/error_handling.h"
#include "rcl/node.h"
#include "rcl/publisher.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"
#include "rcl_interfaces/msg/log.h"
#include "rcutils/allocator.h"
#include "rcutils/macros.h"
#include "rcutils/types/hash_map.h"
#include "rosidl_generator_c/string_functions.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ROSOUT_TOPIC_NAME "rosout"

#define RCL_LOGGING_ROSOUT_VERIFY_INITIALIZED \
  if (!__is_initialized) { \
    /* Return RCL_RET_OK because we won't check throughout rcl if rosout is initialized or not and
     * in the case it's not we want things to continue working.
     */ \
    return RCL_RET_OK; \
  }

typedef struct rosout_map_entry_t
{
  rcl_node_t * node;
  rcl_publisher_t publisher;
} rosout_map_entry_t;

static rcutils_hash_map_t __logger_map;
static bool __is_initialized = false;
static rcl_allocator_t __rosout_allocator;

rcl_ret_t rcl_logging_rosout_init(
  const rcl_allocator_t * allocator)
{
  RCL_CHECK_ARGUMENT_FOR_NULL(allocator, RCL_RET_INVALID_ARGUMENT);
  rcl_ret_t status = RCL_RET_OK;
  if (__is_initialized) {
    return RCL_RET_OK;
  }
  __logger_map = rcutils_get_zero_initialized_hash_map();
  status = rcutils_hash_map_init(&__logger_map, 2, sizeof(const char *), sizeof(rosout_map_entry_t),
      rcutils_hash_map_string_hash_func, rcutils_hash_map_string_cmp_func, allocator);
  if (RCL_RET_OK == status) {
    __rosout_allocator = *allocator;
    __is_initialized = true;
  }
  return status;
}

rcl_ret_t rcl_logging_rosout_fini()
{
  RCL_LOGGING_ROSOUT_VERIFY_INITIALIZED
  rcl_ret_t status = RCL_RET_OK;
  const char * key = NULL;
  rosout_map_entry_t entry;

  // fini all the outstanding publishers
  status = rcutils_hash_map_get_next_key_and_data(&__logger_map, NULL, &key, &entry);
  while (RCUTILS_RET_OK == status) {
    // Teardown publisher
    status = rcl_publisher_fini(&entry.publisher, entry.node);

    if (RCL_RET_OK == status) {
      status = rcutils_hash_map_get_next_key_and_data(&__logger_map, key, &key, &entry);
    }
  }
  if (RCUTILS_RET_HASH_MAP_NO_MORE_ENTRIES == status) {
    status = RCL_RET_OK;
  }

  if (RCL_RET_OK == status) {
    status = rcutils_hash_map_fini(&__logger_map);
  }

  if (RCL_RET_OK == status) {
    __is_initialized = false;
  }
  return status;
}

rcl_ret_t rcl_logging_rosout_init_publisher_for_node(
  rcl_node_t * node)
{
  RCL_LOGGING_ROSOUT_VERIFY_INITIALIZED
  const char * logger_name = NULL;
  rosout_map_entry_t new_entry;
  rcl_ret_t status = RCL_RET_OK;

  // Verify input and make sure it's not already initialized
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  logger_name = rcl_node_get_logger_name(node);
  if (NULL == logger_name) {
    RCL_SET_ERROR_MSG("Logger name was null.");
    return RCL_RET_ERROR;
  }
  if (rcutils_hash_map_key_exists(&__logger_map, &logger_name)) {
    RCL_SET_ERROR_MSG("Logger already initialized for node.");
    return RCL_RET_ALREADY_INIT;
  }

  // Create a new Log message publisher on the node
  const rosidl_message_type_support_t * type_support =
    rosidl_typesupport_c__get_message_type_support_handle__rcl_interfaces__msg__Log();
  rcl_publisher_options_t options = rcl_publisher_get_default_options();
  new_entry.publisher = rcl_get_zero_initialized_publisher();
  status =
    rcl_publisher_init(&new_entry.publisher, node, type_support, ROSOUT_TOPIC_NAME, &options);

  // Add the new publisher to the map
  if (RCL_RET_OK == status) {
    new_entry.node = node;
    status = rcutils_hash_map_set(&__logger_map, &logger_name, &new_entry);
    if (RCL_RET_OK != status) {
      RCL_SET_ERROR_MSG("Failed to add publisher to map.");
      // We failed to add to the map so destroy the publisher that we created
      rcl_ret_t fini_status = rcl_publisher_fini(&new_entry.publisher, new_entry.node);
      // ignore the return status in favor of the failure from set
      RCL_UNUSED(fini_status);
    }
  }

  return status;
}

rcl_ret_t rcl_logging_rosout_fini_publisher_for_node(
  rcl_node_t * node)
{
  RCL_LOGGING_ROSOUT_VERIFY_INITIALIZED
  rosout_map_entry_t entry;
  const char * logger_name = NULL;
  rcl_ret_t status = RCL_RET_OK;

  // Verify input and make sure it's initialized
  RCL_CHECK_ARGUMENT_FOR_NULL(node, RCL_RET_INVALID_ARGUMENT);
  logger_name = rcl_node_get_logger_name(node);
  if (NULL == logger_name) {
    return RCL_RET_ERROR;
  }
  if (!rcutils_hash_map_key_exists(&__logger_map, &logger_name)) {
    return RCL_RET_NOT_INIT;
  }

  // fini the publisher and remove the entry from the map
  status = rcutils_hash_map_get(&__logger_map, &logger_name, &entry);
  if (RCL_RET_OK == status) {
    status = rcl_publisher_fini(&entry.publisher, entry.node);
  }
  if (RCL_RET_OK == status) {
    status = rcutils_hash_map_unset(&__logger_map, &logger_name);
  }

  return status;
}

void rcl_logging_rosout_output_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * log_str)
{
  rosout_map_entry_t entry;
  rcl_ret_t status = RCL_RET_OK;
  if (!__is_initialized) {
    return;
  }
  status = rcutils_hash_map_get(&__logger_map, &name, &entry);
  if (RCL_RET_OK == status) {
    rcl_interfaces__msg__Log * log_message = rcl_interfaces__msg__Log__create();
    if (NULL != log_message) {
      log_message->stamp.sec = (timestamp / 1000000000);
      log_message->stamp.nanosec = (timestamp % 1000000000);
      log_message->level = severity;
      log_message->line = location->line_number;
      rosidl_generator_c__String__assign(&log_message->name, name);
      rosidl_generator_c__String__assign(&log_message->msg, log_str);
      rosidl_generator_c__String__assign(&log_message->file, location->file_name);
      rosidl_generator_c__String__assign(&log_message->function, location->function_name);
      status = rcl_publish(&entry.publisher, log_message);

      rcl_interfaces__msg__Log__destroy(log_message);
    }
  }
}


#ifdef __cplusplus
}
#endif

#endif  // RCL__LOGGING_H_
