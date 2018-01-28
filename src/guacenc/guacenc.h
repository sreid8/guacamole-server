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
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef GUACENC_H
#define GUACENC_H

#include "config.h"
#include <stdbool.h>

#include <guacamole/client.h>

/**
 * The width of the output video, in pixels, if no other width is given on the
 * command line. Note that different codecs will have different restrictions
 * regarding legal widths.
 */
#define GUACENC_DEFAULT_WIDTH 640

/**
 * The height of the output video, in pixels, if no other height is given on the
 * command line. Note that different codecs will have different restrictions
 * regarding legal heights.
 */
#define GUACENC_DEFAULT_HEIGHT 480

/**
 * The desired bitrate of the output video, in bits per second, if no other
 * bitrate is given on the command line.
 */
#define GUACENC_DEFAULT_BITRATE 2000000

/**
 * The allowed codecs for guacenc. More might work, but these are the ones that
 * have been confirmed to work properly
 */

#define GUACENC_ALLOWED_CODECS "libx264", "libx265", "libvpx", "mpeg4"

 /**
 * The default file name guacenc will use if stdin is used as input
 */
#define GUACENC_DEFAULT_FILENAME "output"

/**
 * The default log level below which no messages should be logged.
 */
#define GUACENC_DEFAULT_LOG_LEVEL GUAC_LOG_INFO

/**
 * Runs guacenc in the legacy batch mode
 *
 * @param width
 *     The width of the resulting video
 *
 * @param height
 *     The height of the resulting video
 *
 * @param bitrate
 *     The bitrate of the resulting video
 *
 * @param argc
 *     The argument count that guacenc was started with
 *
 * @param argv
 *     The argument vector that guacenc was started with
 *
 * @param optind
 *     The index at which the name of the input files starts
 *     in argv
 *
 * @param force
 *     Force encoding even files cannot be locked
 */
int guacenc_batch_mode(int width, int height, int bitrate,
        int argc, char* argv[], int optind,
        bool force);

/**
 * Runs guacenc with a single input file and a single output file
 *
 * @param width
 *     The width of the resulting video
 *
 * @param height
 *     The height of the resulting video
 *
 * @param bitrate
 *     The bitrate of the resulting video
 *
 * @param input
 *     The input file to be encoded
 *
 * @param output
 *     The path and name for the resulting encoded video file
 *
 * @param codec
 *     The codec the resulting video should use
 *
 * @param force
 *     Force encoding even files cannot be locked
 */
int guacenc_inout_mode(int width, int height, int bitrate,
        char* input, char* output, char* codec, bool force);

/**
 * Determines if a codec requested as a command line argument is allowed
 * by guacenc.
 *
 * @param codec
 *     The name of the codec requested as a command line argument
 *
 * @return
 *     0 if allowed, -1 if not allowed
 */
int allowed_codec(char* codec);

/**
 * Prints an error and lists the supported codecs to stderr.
 */
void error_codecs(void);

#endif

