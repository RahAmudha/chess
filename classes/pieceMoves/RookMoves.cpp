#include "RookMoves.h"

// Override: rook-specific mask
uint64_t RookMoves::mask(int sq) {
    uint64_t mask = 0ULL;
    int r = sq / 8;
    int f = sq % 8;

    for (int rr = r + 1; rr <= 6; rr++)
        mask |= 1ULL << (rr*8+f);

    for (int rr = r - 1; rr >= 1; rr--)
        mask |= 1ULL << (rr*8+f);

    for (int ff = f + 1; ff <= 6; ff++)
        mask |= 1ULL << (r*8+ff);

    for (int ff = f - 1; ff >= 1; ff--)
        mask |= 1ULL << (r*8+ff);

    return mask;
}

// Override: rook-specific attack generation
uint64_t RookMoves::attacksOnTheFly(int sq, uint64_t occ) {
    uint64_t attacks = 0ULL;
    int r = sq / 8;
    int f = sq % 8;

    for (int rr = r + 1; rr < 8; rr++) {
        attacks |= 1ULL << (rr*8 + f);
        if (occ & (1ULL << (rr*8 + f))) break;
    }

    for (int rr = r - 1; rr >= 0; rr--) {
        attacks |= 1ULL << (rr*8 + f);
        if (occ & (1ULL << (rr*8 + f))) break;
    }

    for (int ff = f + 1; ff < 8; ff++) {
        attacks |= 1ULL << (r*8 + ff);
        if (occ & (1ULL << (r*8 + ff))) break;
    }

    for (int ff = f - 1; ff >= 0; ff--) {
        attacks |= 1ULL << (r*8 + ff);
        if (occ & (1ULL << (r*8 + ff))) break;
    }

    return attacks;
}

// Override: set rook-specific magic bits
void RookMoves::setMagicBits() {
    const uint64_t rookMagics[64] = {
        0x8a80104000800020ULL, 0x140002000100040ULL, 0x2801880a0017001ULL,  0x100081001000420ULL,
        0x200020010080420ULL,  0x3001c0002010008ULL, 0x8480008002000100ULL, 0x2080088004402900ULL,
        0x800098204000ULL, 0x2024401000200040ULL, 0x100802000801000ULL,  0x120800800801000ULL,
        0x208808088000400ULL, 0x2802200800400ULL, 0x2200800100020080ULL, 0x801000060821100ULL,
        0x80044006422000ULL, 0x100808020004000ULL, 0x12108a0010204200ULL, 0x140848010000802ULL,
        0x481828014002800ULL, 0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
        0x100400080208000ULL, 0x2040002120081000ULL, 0x21200680100081ULL,   0x20100080080080ULL,
        0x2000a00200410ULL, 0x20080800400ULL, 0x80088400100102ULL,   0x80004600042881ULL,
        0x4040008040800020ULL, 0x440003000200801ULL, 0x4200011004500ULL,    0x188020080040080ULL,
        0x14800401802800ULL, 0x2080040080800200ULL, 0x124080204001001ULL,  0x200046502000484ULL,
        0x480400080088020ULL, 0x1000422010034000ULL, 0x30200100110040ULL,   0x100021010009ULL,
        0x2002080100110004ULL, 0x202008004008002ULL, 0x20020004010100ULL,   0x2048440040820001ULL,
        0x101002200408200ULL, 0x40802000401080ULL, 0x4008142004410100ULL, 0x2060820c0120200ULL,
        0x1001004080100ULL, 0x20c020080040080ULL, 0x293562010020080ULL,  0x44440041009200ULL,
        0x280001040802101ULL, 0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
        0x20030a0244872ULL, 0x12001008414402ULL, 0x2006104900a0804ULL,  0x1004081002402ULL
    };

    memcpy(magics, rookMagics, sizeof(magics));
}


