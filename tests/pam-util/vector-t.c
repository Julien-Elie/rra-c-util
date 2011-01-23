/*
 * PAM utility vector library test suite.
 *
 * Written by Russ Allbery <rra@stanford.edu>
 * Copyright 2001, 2002, 2004, 2005 Russ Allbery <rra@stanford.edu>
 * Copyright 2009, 2010, 2011
 *     The Board of Trustees of the Leland Stanford Junior University
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <config.h>
#include <portable/system.h>

#include <sys/wait.h>

#include <pam-util/vector.h>
#include <tests/tap/basic.h>


int
main(void)
{
    struct vector *vector, *ovector, *copy;
    const char cstring[] = "This is a\ttest.  ";
    char *string;
    char buffer[BUFSIZ];
    const char * const env[] = { buffer, NULL };
    pid_t child;
    size_t i;

    plan(60);

    vector = vector_new();
    ok(vector != NULL, "vector_new returns non-NULL");
    ok(vector_add(vector, cstring), "vector_add succeeds");
    is_int(1, vector->count, "vector_add increases count");
    ok(vector->strings[0] != cstring, "...and allocated new memory");
    ok(vector_resize(vector, 4), "vector_resize succeeds");
    is_int(4, vector->allocated, "vector_resize works");
    ok(vector_add(vector, cstring), "vector_add #2");
    ok(vector_add(vector, cstring), "vector_add #3");
    ok(vector_add(vector, cstring), "vector_add #4");
    is_int(4, vector->allocated, "...and no reallocation when adding strings");
    is_int(4, vector->count, "...and the count matches");
    is_string(cstring, vector->strings[0], "added the right string");
    is_string(cstring, vector->strings[1], "added the right string");
    is_string(cstring, vector->strings[2], "added the right string");
    is_string(cstring, vector->strings[3], "added the right string");
    ok(vector->strings[1] != vector->strings[2], "each pointer is different");
    ok(vector->strings[2] != vector->strings[3], "each pointer is different");
    ok(vector->strings[3] != vector->strings[0], "each pointer is different");
    ok(vector->strings[0] != cstring, "each pointer is different");
    copy = vector_copy(vector);
    ok(copy != NULL, "vector_copy returns non-NULL");
    is_int(4, copy->count, "...and has right count");
    is_int(4, copy->allocated, "...and has right allocated count");
    for (i = 0; i < 4; i++) {
        is_string(cstring, copy->strings[i], "...and string %d is right", i);
        ok(copy->strings[i] != vector->strings[i],
           "...and pointer %d is different", i);
    }
    vector_free(copy);
    vector_clear(vector);
    is_int(0, vector->count, "vector_clear works");
    is_int(4, vector->allocated, "...but doesn't free the allocation");
    string = strdup(cstring);
    if (string == NULL)
        sysbail("cannot allocate memory");
    ok(vector_add(vector, cstring), "vector_add succeeds");
    ok(vector_add(vector, string), "vector_add succeeds");
    is_int(2, vector->count, "added two strings to the vector");
    ok(vector->strings[1] != string, "...and the pointers are different");
    ok(vector_resize(vector, 1), "vector_resize succeeds");
    is_int(1, vector->count, "vector_resize shrinks the vector");
    ok(vector->strings[0] != cstring, "...and the pointer is different");
    vector_free(vector);
    free(string);

    vector = vector_split_multi("foo, bar, baz", ", ", NULL);
    ok(vector != NULL, "vector_split_multi returns non-NULL");
    is_int(3, vector->count, "vector_split_multi returns right count");
    is_string("foo", vector->strings[0], "...first string");
    is_string("bar", vector->strings[1], "...second string");
    is_string("baz", vector->strings[2], "...third string");
    ovector = vector;
    vector = vector_split_multi("", ", ", vector);
    ok(vector != NULL, "reuse of vector doesn't return NULL");
    ok(vector == ovector, "...and reuses the same vector pointer");
    is_int(0, vector->count, "vector_split_multi reuse with empty string");
    is_int(3, vector->allocated, "...and doesn't free allocation");
    vector = vector_split_multi(",,,  foo,   ", ", ", vector);
    ok(vector != NULL, "reuse of vector doesn't return NULL");
    is_int(1, vector->count, "vector_split_multi with extra separators");
    is_string("foo", vector->strings[0], "...first string");
    vector = vector_split_multi(", ,  ", ", ", vector);
    is_int(0, vector->count, "vector_split_multi with only separators");
    vector_free(vector);

    vector = vector_new();
    ok(vector_add(vector, "/bin/sh"), "vector_add succeeds");
    ok(vector_add(vector, "-c"), "vector_add succeeds");
    snprintf(buffer, sizeof(buffer), "echo ok %lu - vector_exec", testnum++);
    ok(vector_add(vector, buffer), "vector_add succeeds");
    child = fork();
    if (child < 0)
        sysbail("unable to fork");
    else if (child == 0)
        if (vector_exec("/bin/sh", vector) < 0)
            sysdiag("unable to exec /bin/sh");
    waitpid(child, NULL, 0);
    vector_free(vector);

    vector = vector_new();
    ok(vector_add(vector, "/bin/sh"), "vector_add succeeds");
    ok(vector_add(vector, "-c"), "vector_add succeeds");
    ok(vector_add(vector, "echo ok $NUMBER - vector_exec_env"),
       "vector_add succeeds");
    snprintf(buffer, sizeof(buffer), "NUMBER=%lu", testnum++);
    child = fork();
    if (child < 0)
        sysbail("unable to fork");
    else if (child == 0)
        if (vector_exec_env("/bin/sh", vector, env) < 0)
            sysdiag("unable to exec /bin/sh");
    waitpid(child, NULL, 0);
    vector_free(vector);

    return 0;
}
