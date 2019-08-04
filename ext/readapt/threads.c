#include "ruby.h"
#include "ruby/debug.h"
#include "threads.h"

static VALUE threads;

void thread_reference_free(void* data)
{
	free(data);
}

size_t thread_reference_size(const void* data)
{
	return sizeof(thread_reference_t);
}

static const rb_data_type_t thread_reference_type = {
	.wrap_struct_name = "thread_reference",
	.function = {
		.dmark = NULL,
		.dfree = thread_reference_free,
		.dsize = thread_reference_size,
	},
	.data = NULL,
	.flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

void initialize_threads()
{
	threads = rb_hash_new();
	rb_global_variable(&threads);
}

VALUE thread_reference_new(VALUE thr)
{
	thread_reference_t *data = malloc(sizeof(thread_reference_t));
	VALUE obj = TypedData_Make_Struct(rb_cData, thread_reference_t, &thread_reference_type, data);
	data->id = NUM2LONG(rb_funcall(thr, rb_intern("object_id"), 0));
	data->depth = 0;
	data->cursor = 0;
	data->control = rb_intern("continue");
	return obj;
}

thread_reference_t *thread_reference_pointer(VALUE ref)
{
	thread_reference_t *ptr;
    TypedData_Get_Struct(ref, thread_reference_t, &thread_reference_type, ptr);
    return ptr;
}

VALUE thread_current_reference()
{
	return thread_reference(rb_thread_current());
}

VALUE thread_reference(VALUE thr)
{
	return rb_hash_aref(threads, thr);	
}

VALUE thread_add_reference(VALUE thr)
{
	VALUE ref;

	ref = thread_reference_new(thr);
	rb_hash_aset(threads, thr, ref);
	return ref;
}

void thread_reset()
{
	rb_funcall(threads, rb_intern("clear"), 0);
}