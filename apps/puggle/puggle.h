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

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

typedef void *Task;
int initiate(void);
void shutdown(void);
int createTask(Task *);
void deleteTask(Task);
int setPeriod(Task, long long);
void sleepTimestep(Task);
bool isRealtime(void);

void *module_thread(void *);

/*!
 * Returns the current CPU time in nanoseconds. In general
 *   this is really only useful for determining the time
 *   between two events.
 *
 * \return The current CPU time.
 */
long long getTime(void);
