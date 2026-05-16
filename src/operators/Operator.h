#pragma once

#include <optional>
#include "operators/block.h"

class Operator {
public:
    virtual ~Operator() = default;
    virtual std::optional<Block> Next() = 0;
};
