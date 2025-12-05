/**************************************************************************
 **
 ** sngrep - SIP Messages flow viewer
 **
 ** Copyright (C) 2013-2018 Ivan Alonso (Kaian)
 ** Copyright (C) 2013-2018 Irontec SL. All rights reserved.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 ****************************************************************************/
/**
 * @file ai_output.h
 * @author Claude Code <claude-code@anthropic.com>
 *
 * @brief AI Agent JSON output module
 *
 * This module provides structured JSON output for AI agents to analyze
 * SIP traffic in real-time. Messages are batched and output in JSON format
 * with full details including headers, SDP, and payload.
 */

#ifndef __SNGREP_AI_OUTPUT_H
#define __SNGREP_AI_OUTPUT_H

#include "config.h"
#include <stdio.h>
#include <stdbool.h>
#include "sip_msg.h"
#include "sip_call.h"

//! AI output batch buffer size
#define AI_OUTPUT_BATCH_SIZE 10
#define AI_OUTPUT_BUFFER_SIZE 65536

/**
 * @brief AI Output context structure
 *
 * Manages batch buffering and JSON output generation
 * for AI agent consumption
 */
typedef struct ai_output_ctx {
    //! Output file handle (stdout by default)
    FILE *output;
    //! Batch counter
    int batch_id;
    //! Messages in current batch
    int batch_count;
    //! Batch buffer for messages
    sip_msg_t **batch_buffer;
    //! Total messages processed
    int total_messages;
    //! Enable AI output mode
    bool enabled;
} ai_output_ctx_t;

/**
 * @brief Initialize AI output context
 *
 * Sets up the AI output system with batch buffering
 *
 * @param output File handle for output (NULL for stdout)
 * @return Initialized AI output context
 */
ai_output_ctx_t *
ai_output_init(FILE *output);

/**
 * @brief Cleanup AI output context
 *
 * Flushes any pending messages and frees resources
 *
 * @param ctx AI output context
 */
void
ai_output_destroy(ai_output_ctx_t *ctx);

/**
 * @brief Add a SIP message to the batch
 *
 * Buffers the message and outputs when batch is full
 *
 * @param ctx AI output context
 * @param msg SIP message to output
 */
void
ai_output_add_message(ai_output_ctx_t *ctx, sip_msg_t *msg);

/**
 * @brief Force flush current batch
 *
 * Outputs all buffered messages immediately
 *
 * @param ctx AI output context
 */
void
ai_output_flush(ai_output_ctx_t *ctx);

/**
 * @brief Convert SIP message to JSON string
 *
 * Creates detailed JSON representation of a SIP message
 *
 * @param msg SIP message
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Length of JSON string or -1 on error
 */
int
ai_output_msg_to_json(sip_msg_t *msg, char *buffer, int buffer_size);

/**
 * @brief Escape string for JSON output
 *
 * @param input Input string
 * @param output Output buffer
 * @param output_size Size of output buffer
 */
void
ai_output_json_escape(const char *input, char *output, int output_size);

#endif /* __SNGREP_AI_OUTPUT_H */
