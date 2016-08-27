/* Copyright (C) 2016-2016 cmj. All right reserved. */
#ifndef __ZERG_H__
#  define __ZERG_H__

#include <string>

#include "utils.h"
#include "zerg/ir.h"

class Zerg : public IR {
	public:
		Zerg(std::string src, std::string dst);
};

#endif /* __ZERG_H__ */
