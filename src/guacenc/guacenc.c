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

#include "config.h"

#include "encode.h"
#include "guacenc.h"
#include "log.h"
#include "parse.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include <getopt.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {

    /* Load defaults */
    bool force = false;
    int width = GUACENC_DEFAULT_WIDTH;
    int height = GUACENC_DEFAULT_HEIGHT;
    int bitrate = GUACENC_DEFAULT_BITRATE;
    char * codec = "mpeg4";
    char * output= NULL;
    char * input = NULL;

    /* Parse arguments */
    int opt;
    while ((opt = getopt(argc, argv, "s:r:f:i:o:c:")) != -1) {

        /* -s: Dimensions (WIDTHxHEIGHT) */
        if (opt == 's') {
            if (guacenc_parse_dimensions(optarg, &width, &height)) {
                guacenc_log(GUAC_LOG_ERROR, "Invalid dimensions.");
                goto invalid_options;
            }
        }

        /* -r: Bitrate (bits per second) */
        else if (opt == 'r') {
            if (guacenc_parse_int(optarg, &bitrate)) {
                guacenc_log(GUAC_LOG_ERROR, "Invalid bitrate.");
                goto invalid_options;
            }
        }

        /* -f: Force */
        else if (opt == 'f')
            force = true;

        /* -i: Input file */
        else if (opt == 'i')
            input = optarg;

        /* -o: Output file */
        else if (opt == 'o')
            output = optarg;

        /* -c: libavcodec name */
        else if (opt == 'c')
            codec = optarg;

        /* Invalid option */
        else {
            goto invalid_options;
        }

    }

    /* Log start */
    guacenc_log(GUAC_LOG_INFO, "Guacamole video encoder (guacenc) "
            "version " VERSION);


    if (input == NULL && output == NULL) {
        return guacenc_batch_mode(width, height, bitrate, argc, argv, optind, force);
    }

    if (allowed_codec(codec) < 0) {
        goto invalid_codec;
    }

    if (input == NULL) {
        guacenc_log(GUAC_LOG_INFO, "No input file specified. Nothing to do.");
        return 0;
    }

    if (output == NULL) {
        guacenc_log(GUAC_LOG_ERROR, "No output file specified. Cannot continue.");
    }

    return guacenc_inout_mode(width, height, bitrate, input, output, codec, force);

    /* Display usage and exit with error if options are invalid */
    invalid_options:

    fprintf(stderr, "USAGE: \n"
            " BATCH MODE:\n"
            " %s"
            " [-s WIDTHxHEIGHT]"
            " [-r BITRATE]"
            " [-f]"
            " [FILE]...\n\n"
            " SINGLE FILE MODE:\n"
            " %s"
            " [-s WIDTHxHEIGHT]"
            " [-r BITRATE]"
            " [-i INPUT_FILE]"
            " [-o OUTPUT_FILE]"
            " [-c FFMPEG_CODEC]"
            " [-f]\n"
            , argv[0], argv[0]);


    return 1;

    invalid_codec:

    error_codecs();
    return 1;

}

int guacenc_batch_mode(int width, int height, int bitrate,
        int argc, char* argv[], int optind,
        bool force) {

    int i;

    /* Prepare libavcodec */
    avcodec_register_all();

    /* Prepare libavformat */
    av_register_all();

    /* Track total number of failures */
    int total_files = argc - optind;
    int failures = 0;

    /* Abort if no files given */
    if (total_files <= 0) {
        guacenc_log(GUAC_LOG_INFO, "No input files specified. Nothing to do.");
        return 0;
    }

    guacenc_log(GUAC_LOG_INFO, "%i input file(s) provided.", total_files);

    guacenc_log(GUAC_LOG_INFO, "Video will be encoded at %ix%i "
            "and %i bps.", width, height, bitrate);

    /* Encode all input files */
    for (i = optind; i < argc; i++) {

        /* Get current filename */
        const char* path = argv[i];

        /* Generate output filename */
        char out_path[4096];
        int len = snprintf(out_path, sizeof(out_path), "%s.m4v", path);

        /* Do not write if filename exceeds maximum length */
        if (len >= sizeof(out_path)) {
            guacenc_log(GUAC_LOG_ERROR, "Cannot write output file for \"%s\": "
                    "Name too long", path);
            continue;
        }

        /* Attempt encoding, log granular success/failure at debug level */
        if (guacenc_encode(path, out_path, "mpeg4",
                width, height, bitrate, force)) {
            failures++;
            guacenc_log(GUAC_LOG_DEBUG,
                    "%s was NOT successfully encoded.", path);
        }
        else
            guacenc_log(GUAC_LOG_DEBUG, "%s was successfully encoded.", path);

    }

    /* Warn if at least one file failed */
    if (failures != 0)
        guacenc_log(GUAC_LOG_WARNING, "Encoding failed for %i of %i file(s).",
                failures, total_files);

    /* Notify of success */
    else
        guacenc_log(GUAC_LOG_INFO, "All files encoded successfully.");

    /* Encoding complete */
    return 0;
}

int guacenc_inout_mode(int width, int height, int bitrate,
        char* input, char* output, char* codec, bool force) {
    /* Prepare libavcodec */
    avcodec_register_all();

    /* Prepare libavformat */
    av_register_all();


    guacenc_log(GUAC_LOG_INFO, "Video will be encoded at %ix%i "
            "and %i bps.", width, height, bitrate);


    /* Do not write if filename exceeds maximum length */
    if (strlen(output) >= 4096) {
        guacenc_log(GUAC_LOG_ERROR, "Cannot write output file for \"%s\": "
                "Name too long", output);
        return 1;
    }

    /* Attempt encoding, log granular success/failure at debug level */
    if (guacenc_encode(input, output, codec,
            width, height, bitrate, force)) {
        guacenc_log(GUAC_LOG_DEBUG,
                "%s was NOT successfully encoded.", input);
    }
    else
        guacenc_log(GUAC_LOG_DEBUG, "%s was successfully encoded.", input);


    /* Encoding complete */
    return 0;
}

int allowed_codec(char* codec) {
    char* allowed_codecs[] = { GUACENC_ALLOWED_CODECS };
    int size = sizeof(allowed_codecs) / sizeof(allowed_codecs[0]);
    int i;

    for (i = 0; i < size; i++) {
        if (strcmp(codec, allowed_codecs[i]) == 0) {
            return 0;
        }
    }
    return -1;
}

void error_codecs() {
    fprintf(stderr, "ERROR: unsupported codec specified. List of supported codecs:\n");
    char* allowed_codecs[] = { GUACENC_ALLOWED_CODECS };
    int size = sizeof(allowed_codecs) / sizeof(allowed_codecs[0]);
    int i;
    for (i = 0; i < size; i ++) {
        fprintf(stderr, "%s ", allowed_codecs[i]);
    }
    fprintf(stderr, "\n");
}

