// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef EVENT_TASK_BARE_METAL_STATIC_H_
#define EVENT_TASK_BARE_METAL_STATIC_H_

#include "event_task_bare_metal.h"


/* Internal functions declared to allow for static initialization. */
int event_task_bare_metal_lock (const struct event_task *task);
int event_task_bare_metal_unlock (const struct event_task *task);
int event_task_bare_metal_get_event_context (const struct event_task *task,
	struct event_task_context **context);
int event_task_bare_metal_notify (const struct event_task *task,
	const struct event_task_handler *handler);


/**
 * Constant initializer for the event task API
 */
#define	EVENT_TASK_BARE_METAL_API_INIT  { \
		.lock = event_task_bare_metal_lock, \
		.unlock = event_task_bare_metal_unlock, \
		.get_event_context = event_task_bare_metal_get_event_context, \
		.notify = event_task_bare_metal_notify \
	}


/**
 * Initialize a static instance of a bare-metal event handler task.  This does not initialize the
 * task state.  This can be a constant instance.
 *
 * There is no validation done on the arguments.
 *
 * @param state_ptr Variable context for the task.
 * @param system_ptr The manager for system operations.
 * @param handlers_list The list of event handlers that can be used with this task instance.
 * @param count The number of event handlers in the list.
 */
#define	event_task_bare_metal_static_init(state_ptr, system_ptr, handlers_list, count)	{ \
		.base = EVENT_TASK_BARE_METAL_API_INIT, \
		.state = state_ptr, \
		.system = system_ptr, \
		.handlers = handlers_list, \
		.num_handlers = count \
	}


#endif /* EVENT_TASK_BARE_METAL_STATIC_H_ */
