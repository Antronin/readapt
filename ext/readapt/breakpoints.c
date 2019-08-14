#include "ruby.h"
#include "hash_table.h"

static VALUE m_Breakpoints;
ht_hash_table *ht;

void breakpoints_set(char *file, long *lines)
{

}

static VALUE breakpoints_set_s(VALUE self, VALUE file, VALUE lines)
{
    long length = rb_array_len(lines);
    long *ll;
    long i;

    rb_funcall(rb_stderr, rb_intern("puts"), 2, rb_str_new_cstr("Length of set array:"), LONG2NUM(length));
    ll = malloc(sizeof(long) * length);
    for (i = 0; i < length; i++)
    {
        ll[i] = NUM2LONG(rb_ary_entry(lines, i));
    }
    ht_insert(ht, StringValueCStr(file), ll, length);
    free(ll);
    return Qnil;
}

void breakpoints_delete(char *file)
{

}

static VALUE breakpoints_delete_s(VALUE self, VALUE file)
{
    return Qnil;
}

int breakpoints_match(char *file, long line)
{
    ht_long_array *lines;
    long i;

    lines = ht_search(ht, file);
    if (lines != NULL)
    {
        for (i = 0; i < lines->size; i++)
        {
            if (lines->items[i] == line)
            {
                return 1;
            }
        }
    }
    return 0;
}

static VALUE breakpoints_match_s(VALUE self, VALUE file, VALUE line)
{
    return breakpoints_match(StringValueCStr(file), NUM2LONG(line)) == 0 ? Qfalse : Qtrue;
}

void initialize_breakpoints(VALUE m_Readapt)
{
    m_Breakpoints = rb_define_module_under(m_Readapt, "Breakpoints");
    rb_define_singleton_method(m_Breakpoints, "set", breakpoints_set_s, 2);
    rb_define_singleton_method(m_Breakpoints, "delete", breakpoints_delete_s, 1);
    rb_define_singleton_method(m_Breakpoints, "match", breakpoints_match_s, 2);

    ht = ht_new(); // TODO Need to free?
}
