/* Copyright (C) 1995-2014 Free Software Foundation, Inc.

This file is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This file is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */
.text
.syntax unified
.thumb
.align 1
.global __gnu_thumb1_case_uqi
.type __gnu_thumb1_case_uqi,%function
.thumb_func
__gnu_thumb1_case_uqi:
 push {r1}
 mov r1, lr
 lsrs r1, r1, #1
 lsls r1, r1, #1
 ldrb r1, [r1, r0]
 lsls r1, r1, #1
 add lr, lr, r1
 pop {r1}
 bx lr
.size __gnu_thumb1_case_uqi, .-__gnu_thumb1_case_uqi
.global __gnu_thumb1_case_uhi
.type __gnu_thumb1_case_uhi,%function
.thumb_func
__gnu_thumb1_case_uhi:
 push {r0, r1}
 mov r1, lr
 lsrs r1, r1, #1
 lsls r0, r0, #1
 lsls r1, r1, #1
 ldrh r1, [r1, r0]
 lsls r1, r1, #1
 add lr, lr, r1
 pop {r0, r1}
 bx lr
.size __gnu_thumb1_case_uhi, .-__gnu_thumb1_case_uhi
.section .note.GNU-stack,"",%progbits