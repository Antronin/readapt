#ifndef THREADS_H_
#define THREADS_H_

typedef struct thread_reference_struct {
	long id;
	int depth;
	int cursor;
	char *prev_file;
	long prev_line;
	ID control;
} thread_reference_t;

void initialize_threads();
VALUE thread_current_reference();
VALUE thread_reference(VALUE);
VALUE thread_reference_id(VALUE);
VALUE thread_add_reference(VALUE);
VALUE thread_delete_reference(VALUE);
void thread_reference_set_prev_file(thread_reference_t *thr, char *file);
thread_reference_t *thread_reference_pointer(VALUE);
void thread_pause();
void thread_reset();

#endif
