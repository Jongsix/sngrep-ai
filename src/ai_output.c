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
 * @file ai_output.c
 * @author Claude Code <claude-code@anthropic.com>
 *
 * @brief AI Agent JSON output module implementation
 */

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ai_output.h"
#include "sip.h"
#include "util.h"
#include "packet.h"

ai_output_ctx_t *
ai_output_init(FILE *output)
{
    ai_output_ctx_t *ctx = sng_malloc(sizeof(ai_output_ctx_t));
    if (!ctx)
        return NULL;

    ctx->output = output ? output : stdout;
    ctx->batch_id = 0;
    ctx->batch_count = 0;
    ctx->total_messages = 0;
    ctx->enabled = true;
    ctx->batch_buffer = sng_malloc(sizeof(sip_msg_t *) * AI_OUTPUT_BATCH_SIZE);

    if (!ctx->batch_buffer) {
        sng_free(ctx);
        return NULL;
    }

    return ctx;
}

void
ai_output_destroy(ai_output_ctx_t *ctx)
{
    if (!ctx)
        return;

    // Flush any remaining messages
    if (ctx->batch_count > 0) {
        ai_output_flush(ctx);
    }

    if (ctx->batch_buffer) {
        sng_free(ctx->batch_buffer);
    }

    sng_free(ctx);
}

void
ai_output_json_escape(const char *input, char *output, int output_size)
{
    if (!input) {
        if (output && output_size > 0)
            output[0] = '\0';
        return;
    }

    int i = 0, o = 0;
    while (input[i] && o < output_size - 2) {
        switch (input[i]) {
            case '"':
                if (o < output_size - 3) {
                    output[o++] = '\\';
                    output[o++] = '"';
                }
                break;
            case '\\':
                if (o < output_size - 3) {
                    output[o++] = '\\';
                    output[o++] = '\\';
                }
                break;
            case '\n':
                if (o < output_size - 3) {
                    output[o++] = '\\';
                    output[o++] = 'n';
                }
                break;
            case '\r':
                if (o < output_size - 3) {
                    output[o++] = '\\';
                    output[o++] = 'r';
                }
                break;
            case '\t':
                if (o < output_size - 3) {
                    output[o++] = '\\';
                    output[o++] = 't';
                }
                break;
            default:
                output[o++] = input[i];
                break;
        }
        i++;
    }
    output[o] = '\0';
}

#ifndef AI_OUTPUT_TEST_MODE
int
ai_output_msg_to_json(sip_msg_t *msg, char *buffer, int buffer_size)
{
    if (!msg || !buffer || buffer_size <= 0)
        return -1;

    char escaped[AI_OUTPUT_BUFFER_SIZE];
    int len = 0;
    struct timeval tv;
    char timestamp[64];
    const char *payload;
    sip_call_t *call = msg_get_call(msg);

    // Get message timestamp
    tv = msg_get_time(msg);
    struct tm *tm_info = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", tm_info);
    snprintf(timestamp + strlen(timestamp), sizeof(timestamp) - strlen(timestamp),
             ".%03ldZ", tv.tv_usec / 1000);

    // Start JSON object
    len += snprintf(buffer + len, buffer_size - len, "    {\n");

    // Add index
    len += snprintf(buffer + len, buffer_size - len,
                   "      \"index\": %d,\n", msg->index);

    // Add timestamp
    len += snprintf(buffer + len, buffer_size - len,
                   "      \"timestamp\": \"%s\",\n", timestamp);

    // Add call-id
    if (call && call->callid) {
        ai_output_json_escape(call->callid, escaped, sizeof(escaped));
        len += snprintf(buffer + len, buffer_size - len,
                       "      \"call_id\": \"%s\",\n", escaped);
    }

    // Add method/response
    if (msg_is_request(msg)) {
        const char *method = sip_method_str(msg->reqresp);
        len += snprintf(buffer + len, buffer_size - len,
                       "      \"method\": \"%s\",\n", method ? method : "UNKNOWN");
        len += snprintf(buffer + len, buffer_size - len,
                       "      \"response_code\": null,\n");
    } else {
        len += snprintf(buffer + len, buffer_size - len,
                       "      \"method\": null,\n");
        len += snprintf(buffer + len, buffer_size - len,
                       "      \"response_code\": %d,\n", msg->reqresp);
    }

    // Add From/To
    if (msg->sip_from) {
        ai_output_json_escape(msg->sip_from, escaped, sizeof(escaped));
        len += snprintf(buffer + len, buffer_size - len,
                       "      \"from\": \"%s\",\n", escaped);
    } else {
        len += snprintf(buffer + len, buffer_size - len,
                       "      \"from\": null,\n");
    }

    if (msg->sip_to) {
        ai_output_json_escape(msg->sip_to, escaped, sizeof(escaped));
        len += snprintf(buffer + len, buffer_size - len,
                       "      \"to\": \"%s\",\n", escaped);
    } else {
        len += snprintf(buffer + len, buffer_size - len,
                       "      \"to\": null,\n");
    }

    // Add addresses
    if (msg->packet) {
        char src_addr[128], dst_addr[128];
        snprintf(src_addr, sizeof(src_addr), "%s:%u",
                 msg->packet->src.ip, msg->packet->src.port);
        snprintf(dst_addr, sizeof(dst_addr), "%s:%u",
                 msg->packet->dst.ip, msg->packet->dst.port);

        len += snprintf(buffer + len, buffer_size - len,
                       "      \"src_addr\": \"%s\",\n", src_addr);
        len += snprintf(buffer + len, buffer_size - len,
                       "      \"dst_addr\": \"%s\",\n", dst_addr);
    }

    // Add CSeq
    len += snprintf(buffer + len, buffer_size - len,
                   "      \"cseq\": %u,\n", msg->cseq);

    // Add payload
    payload = msg_get_payload(msg);
    if (payload) {
        ai_output_json_escape(payload, escaped, sizeof(escaped));
        len += snprintf(buffer + len, buffer_size - len,
                       "      \"payload\": \"%s\"\n", escaped);
    } else {
        len += snprintf(buffer + len, buffer_size - len,
                       "      \"payload\": null\n");
    }

    // Close JSON object
    len += snprintf(buffer + len, buffer_size - len, "    }");

    return len;
}
#endif /* AI_OUTPUT_TEST_MODE */

void
ai_output_flush(ai_output_ctx_t *ctx)
{
    if (!ctx || ctx->batch_count == 0)
        return;

    struct timeval tv;
    struct tm *tm_info;
    char timestamp[64];
#ifndef AI_OUTPUT_TEST_MODE
    int i;
#endif

    // Get current time for batch
    gettimeofday(&tv, NULL);
    tm_info = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", tm_info);
    snprintf(timestamp + strlen(timestamp), sizeof(timestamp) - strlen(timestamp),
             ".%03ldZ", tv.tv_usec / 1000);

    // Output batch header
    fprintf(ctx->output, "{\n");
    fprintf(ctx->output, "  \"batch_id\": %d,\n", ++ctx->batch_id);
    fprintf(ctx->output, "  \"timestamp\": \"%s\",\n", timestamp);
    fprintf(ctx->output, "  \"packet_count\": %d,\n", ctx->batch_count);
    fprintf(ctx->output, "  \"messages\": [\n");

#ifndef AI_OUTPUT_TEST_MODE
    // Output each message
    for (i = 0; i < ctx->batch_count; i++) {
        char msg_json[AI_OUTPUT_BUFFER_SIZE];
        if (ai_output_msg_to_json(ctx->batch_buffer[i], msg_json,
                                   sizeof(msg_json)) > 0) {
            fprintf(ctx->output, "%s", msg_json);
            if (i < ctx->batch_count - 1) {
                fprintf(ctx->output, ",\n");
            } else {
                fprintf(ctx->output, "\n");
            }
        }
    }
#endif

    // Close batch
    fprintf(ctx->output, "  ]\n");
    fprintf(ctx->output, "}\n");
    fflush(ctx->output);

    // Reset batch
    ctx->batch_count = 0;
}

void
ai_output_add_message(ai_output_ctx_t *ctx, sip_msg_t *msg)
{
    if (!ctx || !msg || !ctx->enabled)
        return;

    // Add to batch buffer
    ctx->batch_buffer[ctx->batch_count++] = msg;
    ctx->total_messages++;

    // Flush if batch is full
    if (ctx->batch_count >= AI_OUTPUT_BATCH_SIZE) {
        ai_output_flush(ctx);
    }
}
