/* vi: set sw=4 ts=4: */

#include "monkeycall.h"
#include "include/internal.h"
#include "include/symbol-table.h"
#include "include/machine.h"
#include "include/symbol-table.h"
#include "include/translate.h"

int
mkc_go(mkc_session * session)	{
	mkc_symbol_function function;
	int r;

	r = mkc_symbol_enter_brace(session);
	if (r)	{
		return -1;
	}

	function.tree = session->tree;

	r = __mkc_translate_function(session, &function);
	if (r)	{
		mkc_symbol_exit_brace(session);
		return -1;
	}

	r = mkc_machine(
			session
			, function.tree->sequence
			, function.stack_depth
			);

	mkc_symbol_exit_brace(session);

	return r;
}

/* eof */
