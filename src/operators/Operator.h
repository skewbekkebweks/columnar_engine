#pragma once

#include <optional>
#include "operators/Block.h"

class Operator {
public:
    virtual ~Operator() = default;
    virtual std::optional<Block> Next() = 0;
};
