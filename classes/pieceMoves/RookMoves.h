#pragma once

#include "SlidingMoves.h"

class RookMoves : public SlidingMoves
{
public:
    RookMoves() {
        setMagicBits();
        initMagics();  // Base class function uses mask() and attacksOnTheFly()
    }

    virtual inline int getAttackBits() const override { return 12; };
    virtual void setMagicBits() override;

private:
    // Only override the functions that differ for rooks
    uint64_t mask(int sq) override;
    uint64_t attacksOnTheFly(int sq, uint64_t occ) override;
};