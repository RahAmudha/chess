#include "SlidingMoves.h"

SlidingMoves::SlidingMoves(){
    initMagics();
}

uint64_t SlidingMoves::getMoves(int sq, uint64_t occupancy) {
    uint64_t occ = occupancy & masks[sq];
    uint64_t idx;
    if (indexBits[sq] == 0) {
        idx = 0;
    } else {
        idx = (occ * magics[sq]) >> (64 - indexBits[sq]);
        idx &= ((1ULL << indexBits[sq]) - 1ULL);
    }
    return attackTable[sq][idx];
}

uint64_t SlidingMoves::mask(int sq) {
    return 0ULL;
}

uint64_t SlidingMoves::attacksOnTheFly(int sq, uint64_t occ) {
    return 0ULL;
}

std::vector<uint64_t> SlidingMoves::getSubsets(uint64_t mask) {
    std::vector<uint64_t> subsets;
    uint64_t subset = 0ULL;

    while (true) {
        subsets.push_back(subset);
        subset = (subset - mask) & mask;
        if (subset == 0) break;
    }

    return subsets;
}

void SlidingMoves::initMagics() {
    attackTable.clear();
    attackTable.resize(64);

    for (int sq = 0; sq < 64; sq++) {
        masks[sq] = mask(sq);
        indexBits[sq] = popcount(masks[sq]);

        size_t table_size = (indexBits[sq] == 0) ? 1ULL : (1ULL << indexBits[sq]);
        attackTable[sq].assign(table_size, 0ULL);

        std::vector<uint64_t> subsets = getSubsets(masks[sq]);
        for (uint64_t occ : subsets) {
            uint64_t index = 0;
            if (indexBits[sq] != 0)
                index = (occ * magics[sq]) >> (64 - indexBits[sq]);

            attackTable[sq][index] = attacksOnTheFly(sq, occ);
        }
    }
}

void SlidingMoves::setMagicBits() {
 
}

