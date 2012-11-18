#ifndef CBFUNC_H
#define CBFUNC_H

#define CB_FUNC1(__ret_type__, __func_name__, __arg1) \
	__ret_type__ CB_ ## __func_name__ (arg1) {
#endif // CBFUNC_H
