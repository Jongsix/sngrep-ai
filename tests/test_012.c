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
 * @file test_012.c
 * @author Claude Code <claude-code@anthropic.com>
 *
 * @brief Test AI output module functionality
 */

#include "config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "../src/ai_output.h"

/**
 * @brief Test JSON escape function
 */
void test_json_escape() {
    char output[256];

    // Test normal string
    ai_output_json_escape("Hello World", output, sizeof(output));
    assert(strcmp(output, "Hello World") == 0);

    // Test string with quotes
    ai_output_json_escape("He said \"Hello\"", output, sizeof(output));
    assert(strcmp(output, "He said \\\"Hello\\\"") == 0);

    // Test string with backslash
    ai_output_json_escape("Path\\to\\file", output, sizeof(output));
    assert(strcmp(output, "Path\\\\to\\\\file") == 0);

    // Test string with newline
    ai_output_json_escape("Line1\nLine2", output, sizeof(output));
    assert(strcmp(output, "Line1\\nLine2") == 0);

    // Test string with tab
    ai_output_json_escape("Col1\tCol2", output, sizeof(output));
    assert(strcmp(output, "Col1\\tCol2") == 0);

    printf("✓ JSON escape tests passed\n");
}

/**
 * @brief Test AI output context initialization
 */
void test_ai_output_init() {
    ai_output_ctx_t *ctx = ai_output_init(NULL);

    assert(ctx != NULL);
    assert(ctx->output == stdout);
    assert(ctx->batch_id == 0);
    assert(ctx->batch_count == 0);
    assert(ctx->total_messages == 0);
    assert(ctx->enabled == true);
    assert(ctx->batch_buffer != NULL);

    ai_output_destroy(ctx);

    printf("✓ AI output init tests passed\n");
}

/**
 * @brief Test batch management
 */
void test_batch_management() {
    char tmpfile[] = "/tmp/ai_output_test_XXXXXX";
    int fd = mkstemp(tmpfile);
    assert(fd != -1);
    FILE *fp = fdopen(fd, "w+");
    assert(fp != NULL);

    ai_output_ctx_t *ctx = ai_output_init(fp);
    assert(ctx != NULL);

    // Initially batch should be empty
    assert(ctx->batch_count == 0);

    // Test flush on empty batch (should not crash and should not increment)
    ai_output_flush(ctx);

    // Verify batch_id NOT incremented for empty batch
    assert(ctx->batch_id == 0);

    ai_output_destroy(ctx);
    fclose(fp);
    unlink(tmpfile);

    printf("✓ Batch management tests passed\n");
}

/**
 * @brief Test NULL safety
 */
void test_null_safety() {
    char output[256];

    // Test NULL input to JSON escape
    ai_output_json_escape(NULL, output, sizeof(output));
    assert(output[0] == '\0');

    // Test NULL context destroy (should not crash)
    ai_output_destroy(NULL);

    printf("✓ NULL safety tests passed\n");
}

int main(int argc, char *argv[]) {
    printf("Running AI Output Module Tests\n");
    printf("================================\n\n");
    printf("Testing core AI output functions (basic unit tests)\n");
    printf("Note: Message formatting tests will be done in integration tests\n\n");

    test_json_escape();
    test_ai_output_init();
    test_batch_management();
    test_null_safety();

    printf("\n================================\n");
    printf("All core tests passed! ✓\n");
    printf("Integration tests will run with full sngrep binary\n");

    return 0;
}
