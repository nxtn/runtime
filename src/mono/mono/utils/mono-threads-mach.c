/*
 * mono-threads-mach.c: Low-level threading, mach version
 *
 * Author:
 *	Rodrigo Kumpera (kumpera@gmail.com)
 *
 * (C) 2011 Novell, Inc
 */

#include "config.h"

#if defined(__MACH__)

#include <mono/utils/mach-support.h>
#include <mono/utils/mono-compiler.h>
#include <mono/utils/mono-semaphore.h>
#include <mono/utils/mono-threads.h>
#include <mono/utils/hazard-pointer.h>
#include <mono/metadata/gc-internal.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/threads-types.h>

#include <pthread.h>
#include <errno.h>

void
mono_threads_init_platform (void)
{	
}

void
mono_threads_core_interrupt (MonoThreadInfo *info)
{
	thread_abort_safely (info->native_handle);
}

void
mono_threads_core_self_suspend (MonoThreadInfo *info)
{
	kern_return_t kern_ret;
	gboolean ret;

	g_assert (info);

	ret = mono_threads_get_runtime_callbacks ()->thread_state_init_from_sigctx (&info->suspend_state, NULL);
	g_assert (ret);

	/* we must unlock only after context is captured. */
	LeaveCriticalSection (&info->suspend_lock);

	kern_ret = thread_suspend (info->native_handle);
	g_assert (kern_ret == KERN_SUCCESS);
}

gboolean
mono_threads_core_suspend (MonoThreadInfo *info)
{
	kern_return_t ret;
	g_assert (info);

	ret = thread_suspend (info->native_handle);
	if (ret != KERN_SUCCESS)
		return FALSE;
	return mono_threads_get_runtime_callbacks ()->
		thread_state_init_from_handle (&info->suspend_state, mono_thread_info_get_tid (info), info->native_handle);
}

gboolean
mono_threads_core_resume (MonoThreadInfo *info)
{
	kern_return_t ret;
	ret = thread_resume (info->native_handle);
	return ret == KERN_SUCCESS;
}

void
mono_threads_platform_register (MonoThreadInfo *info)
{
	info->native_handle = mach_thread_self ();
}

void
mono_threads_platform_free (MonoThreadInfo *info)
{
	mach_port_deallocate (current_task (), info->native_handle);
}

#endif
