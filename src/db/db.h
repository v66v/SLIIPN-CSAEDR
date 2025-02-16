#pragma once
#include "global.h"

#if DB_IMPL == 0
#include "sqlite.h"
#elif DB_IMPL == 1
#include "sqlite_client.h"
#endif
