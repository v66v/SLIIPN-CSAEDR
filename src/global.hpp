#pragma once
#include "global.h"
#include "post_office/post_office.hpp"
#include "safeguard/safeguard.hpp"
#include "external/global.h"

extern post_office_t post_office;
extern safeguard_t safeguard;
extern size_t keep_alive_period;
extern int crashed;
extern mem_external ext;
