#pragma once

/**/
#include "__macro.hpp"
/**/

#if defined(_MSC_VER)

#include <sal.h>

#else

#define _At_(...)
#define _Success_(expr)
#define _Return_type_success_(expr)
#define _On_failure_(...)
#define _Always_(...)

#define _Reserved_
#define _Const_
#define _In_
#define _In_opt_
#define _Out_
#define _Out_opt_
#define _Inout_
#define _Inout_opt_

#define _Outptr_
#define _Outptr_opt_
#define _Outref_
#define _Outref_result_maybenull_

#define _Ret_notnull_
#define _Ret_maybenull_
#define _Ret_null_

#define _Null_
#define _Notnull_
#define _Maybenull_

#define _Result_nullonfailure_

#endif
