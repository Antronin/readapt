#include "ruby.h"
#include "ruby/debug.h"
#include "frame.h"
#include "normalize.h"

static VALUE c_Frame;

void frame_free(void *data)
{
    frame_t *frm = data;

    free(frm->file);
    free(frm);
}

static size_t
frame_size(const void *data)
{
    return sizeof(frame_t);
}

static const rb_data_type_t frame_type = {
    .wrap_struct_name = "frame_data",
    .function = {
        .dmark = NULL,
        .dfree = frame_free,
        .dsize = frame_size,
    },
    .data = NULL,
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE frame_allocate_s(VALUE self)
{
    VALUE obj;
    frame_t *data = malloc(sizeof(frame_t));
    obj = TypedData_Wrap_Struct(self, &frame_type, data);
    data->file = NULL;
    data->line = 0;
    data->binding_id = 0;
    return obj;
}

VALUE frame_allocate()
{
    return frame_allocate_s(c_Frame);
}

frame_t *frame_data_from_tracepoint(VALUE tracepoint)
{
    frame_t *data;
    VALUE tmp, bnd;
    rb_trace_arg_t *tracearg;
    char *file;
    int line;
    long binding_id;

    data = malloc(sizeof(frame_t));
    tracearg = rb_tracearg_from_tracepoint(tracepoint);
    tmp = rb_tracearg_path(tracearg);
    file = (tmp == Qnil ? NULL : normalize_path_new_cstr(StringValueCStr(tmp)));
    line = NUM2INT(rb_tracearg_lineno(tracearg));
    bnd = rb_tracearg_binding(tracearg);
    binding_id = NUM2LONG(rb_obj_id(bnd));

    data->file = file;
    data->line = line;
    data->binding_id = binding_id;
    data->stack = 0;

    return data;
}

VALUE frame_initialize_m(VALUE self, VALUE file, VALUE line, VALUE binding_id, VALUE stack)
{
    frame_t *data;
    TypedData_Get_Struct(self, frame_t, &frame_type, data);
    if (file == Qnil)
    {
        data->file = NULL;
    }
    else
    {
        data->file = normalize_path_new_cstr(StringValueCStr(file));
    }
    data->line = NUM2INT(line);
    data->binding_id = NUM2LONG(binding_id);
    data->stack = NUM2INT(stack);
    return self;
}

VALUE frame_new_from_data(frame_t *data)
{
    VALUE obj;

    obj = frame_allocate();
    frame_initialize_m(
        obj,
        rb_str_new_cstr(data->file),
        INT2NUM(data->line),
        LONG2NUM(data->binding_id),
        INT2NUM(data->stack));

    return obj;
}

VALUE frame_file_m(VALUE self)
{
    frame_t *data;
    VALUE str = Qnil;

    TypedData_Get_Struct(self, frame_t, &frame_type, data);
    if (data->file)
    {
        str = rb_str_new_cstr(data->file);
        rb_obj_freeze(str);
    }
    return str;
}

VALUE frame_line_m(VALUE self)
{
    frame_t *data;
    TypedData_Get_Struct(self, frame_t, &frame_type, data);
    return INT2NUM(data->line);
}

VALUE frame_binding_id_m(VALUE self)
{
    frame_t *data;
    TypedData_Get_Struct(self, frame_t, &frame_type, data);
    return LONG2NUM(data->binding_id);
}

VALUE frame_stack_m(VALUE self)
{
    frame_t *data;
    TypedData_Get_Struct(self, frame_t, &frame_type, data);
    return INT2NUM(data->stack);
}

void initialize_frame(VALUE m_Readapt)
{
    c_Frame = rb_define_class_under(m_Readapt, "Frame", rb_cData);
    rb_define_alloc_func(c_Frame, frame_allocate_s);
    rb_define_method(c_Frame, "initialize", frame_initialize_m, 4);
    rb_define_method(c_Frame, "file", frame_file_m, 0);
    rb_define_method(c_Frame, "line", frame_line_m, 0);
    rb_define_method(c_Frame, "binding_id", frame_binding_id_m, 0);
    rb_define_method(c_Frame, "stack", frame_stack_m, 0);
}
