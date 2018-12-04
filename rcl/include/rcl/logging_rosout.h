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

#ifndef RCL__LOGGING_ROSOUT_H_
#define RCL__LOGGING_ROSOUT_H_

#include "rcl/allocator.h"
#include "rcl/error_handling.h"
#include "rcl/node.h"
#include "rcl/types.h"
#include "rcl/visibility_control.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// Initializes the rcl_logging_rosout features
/**
 * Calling this will initialize the rcl_logging_rosout features. This function must be called
 * before any other rcl_logging_rosout_* functions can be called.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] allocator The allocator used for metadata related to the rcl_logging_rosout features
 * \return `RCL_RET_OK` if the rcl_logging_rosout features are successfully initialized, or
 * \return `RCL_RET_ALREADY_INIT` if the feature has already initialized, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_LOCAL
RCL_WARN_UNUSED
rcl_ret_t
rcl_logging_rosout_init(
  const rcl_allocator_t * allocator);

/// Uninitializes the rcl_logging_rosout features
/**
 * Calling this will set the rcl_logging_rosout features into the an unitialized state that is
 * functionally the same as before rcl_logging_rosout_init was called.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \return `RCL_RET_OK` if the rcl_logging_rosout feature was successfully unitialized, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_LOCAL
RCL_WARN_UNUSED
rcl_ret_t
rcl_logging_rosout_fini();

/// Creates a rosout publisher for a node and registers to be used by the logging system
/**
 * Calling this for an rcl_node_t will create a new publisher on that node that will be
 * used by the logging system to publish all log messages from that Node's logger.
 *
 * If a publisher already exists for this node then a new publisher will NOT be created.
 *
 * It is expected that after creating a rosout publisher with this function that
 * rcl_logging_destroy_rosout_publisher_for_node() will be called for the node to cleanup
 * the publisher while the Node is still valid.
 *
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] node a valid rcl_node_t that the publisher will be created on
 * \return `RCL_RET_OK` if the publisher was created successfully, or
 * \return `RCL_RET_ALREADY_INIT` if the publisher has already exists, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_LOCAL
RCL_WARN_UNUSED
rcl_ret_t
rcl_logging_rosout_init_publisher_for_node(
  rcl_node_t * node);

/// Deregisters a rosout publisher for a node and cleans up all allocated resources for the publisher
/**
 * Calling this for an rcl_node_t will destroy the rosout publisher on that node and remove it from
 * the logging system so that no more Log messages are published to this function.
 *
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param[in] node a valid rcl_node_t that the publisher will be created on
 * \return `RCL_RET_OK` if the publisher was created successfully, or
 * \return `RCL_RET_NOT_INIT` if the publisher does not exist for the provided node, or
 * \return `RCL_RET_INVALID_ARGUMENT` if any arguments are invalid, or
 * \return `RCL_RET_BAD_ALLOC` if allocating memory failed, or
 * \return `RCL_RET_ERROR` if an unspecified error occurs.
 */
RCL_LOCAL
RCL_WARN_UNUSED
rcl_ret_t
rcl_logging_rosout_fini_publisher_for_node(
  rcl_node_t * node);

/// The output handler outputs log messages to rosout topics.
/**
 * When called with a logger name and log message this function will attempt to
 * find a rosout publisher correlated with the logger name and publish a Log
 * message out via that publisher. If there is no publisher directly correlated
 * with the logger then nothing will be done.
 *
 * This function is meant to be registered with the logging functions for rcutils
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param location The pointer to the location struct or NULL
 * \param severity The severity level
 * \param name The name of the logger, must be null terminated c string
 * \param timestamp The timestamp for when the log message was made
 * \param log_str The string to be logged
 */
RCL_PUBLIC
void rcl_logging_rosout_output_handler(
  const rcutils_log_location_t * location,
  int severity, const char * name, rcutils_time_point_value_t timestamp,
  const char * log_str);

#ifdef __cplusplus
}
#endif

#endif  // RCL__LOGGING_ROSOUT_H_