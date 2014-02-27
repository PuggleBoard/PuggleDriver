/*
	 -------------------------------------------------------------------------

	 This file is part of the Puggle Data Conversion and Processing System
	 Copyright (C) 2013 Puggle

	 -------------------------------------------------------------------------

	 Written in 2013 by: Yogi Patel <yapatel@gatech.edu>

	 To the extent possible under law, the author(s) have dedicated all copyright
	 and related and neighboring rights to this software to the public domain
	 worldwide. This software is distributed without any warranty.

	 You should have received a copy of the CC Public Domain Dedication along with
	 this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.
	 */

#ifndef DEBUG_H
#define DEBUG_H

#include <execinfo.h>
#include <stdio.h>

//! Prints a backtrace to standard error.
static inline void PRINT_BACKTRACE(void) {
    int buffer_size;
    void *buffer[256];

    buffer_size = backtrace(buffer,sizeof(buffer));
    fprintf(stderr,"Backtrace:\n");
    backtrace_symbols_fd(buffer,buffer_size,2);
}

#define ERROR_MSG(fmt,args...) do { fprintf(stderr,"%s:%d:",__FILE__,__LINE__); fprintf(stderr,fmt,## args); } while(0)

#ifdef DEBUG

#define DEBUG_MSG(fmt,args...) do { fprintf(stderr,"%s:%d:",__FILE__,__LINE__); fprintf(stderr,fmt,## args); } while(0)

#else /* !DEBUG */

//! Prints debug messages to standard error.
/*!
 * When compiled without the DEBUG flag messages compile out.
 */
#define DEBUG_MSG(fmt,args...) do { ; } while(0)

#endif /* DEBUG */

#endif /* DEBUG_H */
