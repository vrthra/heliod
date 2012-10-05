/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright 2008 Sun Microsystems, Inc. All rights reserved.
 *
 * THE BSD LICENSE
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution. 
 *
 * Neither the name of the  nor the names of its contributors may be
 * used to endorse or promote products derived from this software without 
 * specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FCGIERRORTYPES_H
#define FCGIERRORTYPES_H

#define MAX_STRING_LEN 256

typedef enum {
    NO_FCGI_ERROR = 0,

    //config related errors
    NO_APP_BIND_PATH,
    NO_BIND_PATH,
    INVALID_BIND_PATH,

    //Stub related errors
    SEMAPHORE_OPEN_ERROR,
    STUB_PID_FILE_CREATE_FAILURE,
    STUB_NOT_RESPONDING,
    STUB_START_FAILURE,
    STUB_BIND_ERROR,
    STUB_LISTEN_ERROR,
    STUB_ACCEPT_ERROR,
    STUB_SOCKET_CREATION_FAILURE,
    STUB_CONNECT_FAILURE,
    STUB_STAT_FAILURE,
    STUB_NO_PERM,
    PIPE_CREATE_FAILURE,
    STUB_FORK_ERROR,
    STUB_EXEC_FAILURE,
    REQUEST_SEND_FAILURE,
    REQUEST_MEMORY_ALLOCATION_FAILURE,
    BUILD_REQ_ERROR,
    ERROR_RESPONSE,
    UNKNOWN_STUB_REQ_TYPE,
    STUB_SERVER_START_REQ_FAILURE,
    STUB_OVER_LOAD_REQ_FAILURE,
    STUB_POLL_ERROR,

    //stub request related errors
    REQUEST_THREAD_CREATE_FAILURE,
    REQUEST_READ_ERROR,
    INCOMPLETE_HEADER,
    INVALID_REQUEST_TYPE,
    REQUEST_MISSING_OR_INVALID_PARAM,
    PROC_EXISTS,
    SERVER_SOCKET_CREATION_FAILURE,
    SET_RLIMIT_FAILURE,
    SET_NICE_FAILURE,
    INVALID_PARAM_VALUE,
    INVALID_USER,
    INVALID_GROUP,
    SET_GROUP_FAILURE,
    SET_USER_FAILURE,
    SET_CHDIR_FAILURE,
    NO_PERMISSION,
    STAT_FAILURE,
    NOT_EXEC_OWNER,
    NO_EXEC_PERMISSION,
    WRITE_OTHER_PERMISSION,
    CHILD_EXEC_FAILURE,
    SET_CHROOT_FAILURE,
    CHILD_FORK_FAILURE,
    PROC_DOES_NOT_EXIST,
    SERVER_BIND_ERROR,
    SERVER_LISTEN_ERROR,
    REQUEST_INCOMPLETE_HEADER,

    //parser related errors
    INVALID_VERSION,
    INVALID_TYPE,
    INVALID_RECORD,
    INVALID_HTTP_HEADER,
    CANT_MPX,
    OVER_LOADED,
    UNKNOWN_ROLE,

    //protocol related errors
    FCGI_INVALID_SERVER,
    FCGI_INVALID_RESPONSE,
    FCGI_NO_BUFFER_SPACE,
    FCGI_FILTER_FILE_OPEN_ERROR,
    FCGI_NO_AUTHORIZATION


} PluginError;

const char *getErrorString(PluginError error);
PRBool shouldRetry(PluginError error);

#endif // FCGIERRORTYPES_H