#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <random>
#include <iostream>

class SlidingMoves
{
    public:
        SlidingMoves();
        void initMagics();

        inline bool onBoard(int r, int f) { return r >= 0 && r < 8 && f >= 0 && f < 8; }
        virtual inline int getAttackBits() const { return 0; }
        virtual void setMagicBits();
        uint64_t magics[64] = {
            0x40040844404084ULL, 0x2004208a004208ULL, 0x10190041080202ULL, 0x108060845042010ULL,
            0x581104180800210ULL, 0x2112080446200010ULL, 0x1080820820060210ULL, 0x3c0808410220200ULL,
            0x4050404440404ULL, 0x21001420088ULL, 0x24d0080801082102ULL, 0x1020a0a020400ULL,
            0x40308200402ULL, 0x4011002100800ULL, 0x401484104104005ULL, 0x801010402020200ULL,
            0x400210c3880100ULL, 0x404022024108200ULL, 0x810018200204102ULL, 0x4002801a02003ULL,
            0x85040820080400ULL, 0x810102c808880400ULL, 0xe900410884800ULL, 0x8002020480840102ULL,
            0x220200865090201ULL, 0x2010100a02021202ULL, 0x152048408022401ULL, 0x20080002081110ULL,
            0x4001001021004000ULL, 0x800040400a011002ULL, 0xe4004081011002ULL, 0x1c004001012080ULL,
            0x8004200962a00220ULL, 0x8422100208500202ULL, 0x2000402200300c08ULL, 0x8646020080080080ULL,
            0x80020a0200100808ULL, 0x2010004880111000ULL, 0x623000a080011400ULL, 0x42008c0340209202ULL,
            0x209188240001000ULL, 0x400408a884001800ULL, 0x110400a6080400ULL, 0x1840060a44020800ULL,
            0x90080104000041ULL, 0x201011000808101ULL, 0x1a2208080504f080ULL, 0x8012020600211212ULL,
            0x500861011240000ULL, 0x180806108200800ULL, 0x4000020e01040044ULL, 0x300000261044000aULL,
            0x802241102020002ULL, 0x20906061210001ULL, 0x5a84841004010310ULL, 0x4010801011c04ULL,
            0xa010109502200ULL, 0x4a02012000ULL, 0x500201010098b028ULL, 0x8040002811040900ULL,
            0x28000010020204ULL, 0x6000020202d0240ULL, 0x8918844842082200ULL, 0x4010011029020020ULL
        };


        uint64_t getMoves(int sq, uint64_t occupancy);

    private:
    
        // Global Vars
        uint64_t masks[64];
            
        std::vector<std::vector<uint64_t>> attackTable;
        int indexBits[64];

        // Helper Functions

        // These two functions determine which piece it is (hence virtual lol)
        virtual uint64_t mask(int sq);
        virtual uint64_t attacksOnTheFly(int sq, uint64_t occ);

        std::vector<uint64_t> getSubsets(uint64_t mask);

        // Helper Functions for the Helper Functions
        inline int popcount(uint64_t x) { return __builtin_popcountll(x); }
        inline int lsb(uint64_t x) { return __builtin_ctzll(x); }
        inline uint64_t setBit(uint64_t bb, int sq) { return bb | (1ULL << sq); }
        inline uint64_t random64(std::mt19937_64 &rng) { return rng(); }
};
