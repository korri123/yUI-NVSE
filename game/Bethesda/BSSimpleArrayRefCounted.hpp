#pragma once

#include "NiRefObject.hpp"
#include "BSSimpleArray.hpp"

template <typename T_Data>
class BSSimpleArrayRefCounted : public BSSimpleArray<T_Data>, public NiRefObject {
public:
};