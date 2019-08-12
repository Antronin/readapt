#include "ruby.h"
#include "ruby/debug.h"

static int isWindows;
static VALUE zero;
static VALUE rangeWithoutFirst;
static VALUE gsub1;
static VALUE gsub2;

static int
checkIfWindows()
{
    VALUE regexp, result;
    
    regexp = rb_reg_new("/cygwin|mswin|mingw|bccwin|wince|emx/", 37, 0);
    result = rb_reg_match(regexp, rb_str_new_cstr(RUBY_PLATFORM));
    return result == Qnil ? 0 : 1;
}

static VALUE
normalize_path(VALUE str)
{
    VALUE letter, path;

    if (isWindows)
    {
        letter = rb_funcall(str, rb_intern("[]"), 1, zero);
        path = rb_funcall(str, rb_intern("[]"), 1, rangeWithoutFirst);
        return rb_str_plus(
            rb_funcall(letter, rb_intern("upcase"), 0),
            rb_funcall(path, rb_intern("gsub"), 2, gsub1, gsub2)
        );
    }
    return str;
}

static VALUE
normalize_path_s(VALUE self, VALUE str)
{
    return normalize_path(str);
}

void initialize_normalize(VALUE m_Readapt)
{
    isWindows = checkIfWindows();
    zero = INT2NUM(0);
    gsub1 = rb_str_new_cstr("\\");
    gsub2 = rb_str_new_cstr("/");
    rangeWithoutFirst = rb_funcall(rb_cRange, rb_intern("new"), 2, INT2NUM(1), INT2NUM(-1));
    rb_define_singleton_method(m_Readapt, "normalize_path", normalize_path_s, 1);
}
