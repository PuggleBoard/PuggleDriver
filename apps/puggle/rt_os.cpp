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

#include <debug.h>
#include <pthread.h>
#include <rt.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <native/task.h>
#include <native/timer.h>

typedef struct {
	long long period;
	RT_TASK task;
} xenomai_task_t;

static bool init_rt = false;
static pthread_key_t is_rt_key;

int RT::OS::initiate(void) {
	rt_timer_set_mode(TM_ONESHOT);

	// overrie blocking limit on memory
	struct rlimit rlim = { RLIM_INFINITY, RLIM_INFINITY };
	setrlimit(RLIMIT_MEMLOCK,&rlim);

	if(mlockall(MCL_CURRENT | MCL_FUTURE)) {
		ERROR_MSG("RTOS:OS::initiate : failed to lock memory.\n");
		return -EPERM;
	}

	pthread_key_create(&is_rt_key,0);
	init_rt = true;

	return 0;
}

void RT::OS::shutdown(void) {
	pthread_key_delete(is_rt_key);
}

int RT::OS::createTask(RT::OS::Task *task,void *(*entry)(void *),void *arg,int prio) {
	int retval = 0;
	xenomai_task_t *t = new xenomai_task_t;
	int priority = 99;

	// Invert priority, default prio=0 but max priority for xenomai task is 99
	if ((prio >=0) && (prio <=99))
		priority -= prio;

	if((retval = rt_task_create(&t->task,"Pugggle RT Thread",0,priority,T_FPU|T_JOINABLE))) {
		ERROR_MSG("RT::OS::createTask : failed to create task\n");
		return retval;
	}

	t->period = -1;

	*task = t;
	pthread_setspecific(is_rt_key,reinterpret_cast<const void *>(t));

	if((retval = rt_task_start(&t->task,reinterpret_cast<void(*)(void *)>(entry),arg))) {
		ERROR_MSG("RT::OS::createTask : failed to start task\n");
		return retval;
	}

	return 0;
}

void RT::OS::deleteTask(RT::OS::Task task) {
	xenomai_task_t *t = reinterpret_cast<xenomai_task_t *>(task);
	rt_task_delete(&t->task);
}

	bool RT::OS::isRealtime(void) {
		if(init_rt && pthread_getspecific(is_rt_key))
			return true;
		return false;
	}

long long RT::OS::getTime(void) {
	return rt_timer_tsc2ns(rt_timer_tsc());
}

int RT::OS::setPeriod(RT::OS::Task task,long long period) {
	xenomai_task_t *t = reinterpret_cast<xenomai_task_t *>(task);
	t->period = period;
	return rt_task_set_periodic(&t->task,TM_NOW,period);
}

void RT::OS::sleepTimestep(RT::OS::Task task) {
	rt_task_wait_period(0);
}
