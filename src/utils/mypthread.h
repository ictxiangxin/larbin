/*
 *   Larbin - is a web crawler
 *   Copyright (C) 2013  ictxiangxin
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <pthread.h>

#include "options.h"

#ifdef THREAD_OUTPUT

#define mypthread_cond_init(x,y) pthread_cond_init(x,y)
#define mypthread_cond_destroy(x) pthread_cond_destroy(x)
#define mypthread_cond_wait(c,x,y) while (c) { pthread_cond_wait(x,y); }
#define mypthread_cond_broadcast(x) pthread_cond_broadcast(x)

#define mypthread_mutex_init(x,y) pthread_mutex_init(x,y)
#define mypthread_mutex_destroy(x) pthread_mutex_destroy(x)
#define mypthread_mutex_lock(x) pthread_mutex_lock(x)
#define mypthread_mutex_unlock(x) pthread_mutex_unlock(x)

#else

#define mypthread_cond_init(x,y) ((void) 0)
#define mypthread_cond_destroy(x) ((void) 0)
#define mypthread_cond_wait(c,x,y) ((void) 0)
#define mypthread_cond_broadcast(x) ((void) 0)

#define mypthread_mutex_init(x,y) ((void) 0)
#define mypthread_mutex_destroy(x) ((void) 0)
#define mypthread_mutex_lock(x) ((void) 0)
#define mypthread_mutex_unlock(x) ((void) 0)

#endif // THREAD_OUTPUT

typedef void* (*StartFun) (void *);
pthread_t startThread (StartFun run, void *arg);

#endif // MYTHREAD_H
