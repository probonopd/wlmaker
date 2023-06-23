# Copyright 2023 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

CMAKE_MINIMUM_REQUIRED(VERSION 3.13)

FIND_PROGRAM(
  WAYLAND_SCANNER_EXECUTABLE
  NAMES wayland-scanner
  REQUIRED)
IF(NOT WAYLAND_SCANNER_EXECUTABLE)
  MESSAGE(FATAL_ERROR "'wayland-scanner' executable not found.")
ENDIF()

PKG_GET_VARIABLE(PROTOCOL_DIR wayland-protocols pkgdatadir)

ADD_CUSTOM_COMMAND(
  OUTPUT wlm-input-activity-v1-client.h
  COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header ${CMAKE_CURRENT_SOURCE_DIR}/../protocols/wlm-input-activity-v1.xml wlm-input-activity-v1-client.h
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../protocols/wlm-input-activity-v1.xml
  VERBATIM)

ADD_CUSTOM_COMMAND(
  OUTPUT wlm-input-activity-v1-server.h
  COMMAND ${WAYLAND_SCANNER_EXECUTABLE} server-header ${CMAKE_CURRENT_SOURCE_DIR}/../protocols/wlm-input-activity-v1.xml wlm-input-activity-v1-server.h
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../protocols/wlm-input-activity-v1.xml
  VERBATIM)

ADD_LIBRARY(
  wlm_protocol_headers
  OBJECT
  wlm-input-activity-v1-client.h
  wlm-input-activity-v1-server.h)
