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

/* DO NOT EDIT THIS FILE - it is automatically generated */

typedef struct _NtError {
	int ErrorNumber;
	char *ErrorString;
	struct _NtError *next;
} NtError;

NtError NtErrorStrings[] = {
{ 10004 , "WSAEINTR" },
{ 10009 , "WSAEBADF" },
{ 10013 , "WSAEACCES" },
{ 10014 , "WSAEFAULT" },
{ 10022 , "WSAEINVAL" },
{ 10024 , "WSAEMFILE" },
{ 10035 , "WSAEWOULDBLOCK" },
{ 10036 , "WSAEINPROGRESS" },
{ 10037 , "WSAEALREADY" },
{ 10038 , "WSAENOTSOCK" },
{ 10039 , "WSAEDESTADDRREQ" },
{ 10040 , "WSAEMSGSIZE" },
{ 10041 , "WSAEPROTOTYPE" },
{ 10042 , "WSAENOPROTOOPT" },
{ 10043 , "WSAEPROTONOSUPPORT" },
{ 10044 , "WSAESOCKTNOSUPPORT" },
{ 10045 , "WSAEOPNOTSUPP" },
{ 10046 , "WSAEPFNOSUPPORT" },
{ 10047 , "WSAEAFNOSUPPORT" },
{ 10048 , "WSAEADDRINUSE" },
{ 10049 , "WSAEADDRNOTAVAIL" },
{ 10050 , "WSAENETDOWN" },
{ 10051 , "WSAENETUNREACH" },
{ 10052 , "WSAENETRESET" },
{ 10053 , "WSAECONNABORTED" },
{ 10054 , "WSAECONNRESET" },
{ 10055 , "WSAENOBUFS" },
{ 10056 , "WSAEISCONN" },
{ 10057 , "WSAENOTCONN" },
{ 10058 , "WSAESHUTDOWN" },
{ 10059 , "WSAETOOMANYREFS" },
{ 10060 , "WSAETIMEDOUT" },
{ 10061 , "WSAECONNREFUSED" },
{ 10062 , "WSAELOOP" },
{ 10063 , "WSAENAMETOOLONG" },
{ 10064 , "WSAEHOSTDOWN" },
{ 10065 , "WSAEHOSTUNREACH" },
{ 10066 , "WSAENOTEMPTY" },
{ 10067 , "WSAEPROCLIM" },
{ 10068 , "WSAEUSERS" },
{ 10069 , "WSAEDQUOT" },
{ 10070 , "WSAESTALE" },
{ 10071 , "WSAEREMOTE" },
{ 10101 , "WSAEDISCON" },
{ 10091 , "WSASYSNOTREADY" },
{ 10092 , "WSAVERNOTSUPPORTED" },
{ 10093 , "WSANOTINITIALISED" },
{ 11001 , "WSAHOST_NOT_FOUND" },
{ 11002 , "WSATRY_AGAIN" },
{ 11003 , "WSANO_RECOVERY" },
{ 11004 , "WSANO_DATA" },
{ 0, NULL }
};