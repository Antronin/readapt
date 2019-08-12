#include "ruby.h"
#include "ruby/debug.h"
#include "monitor.h"
#include "normalize.h"

static VALUE m_Readapt;

void Init_readapt()
{
	m_Readapt = rb_define_module("Readapt");

	initialize_normalize(m_Readapt);
	initialize_monitor(m_Readapt);
}
