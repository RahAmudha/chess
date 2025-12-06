#pragma once

#include <assert.h>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <vector>
#include "Bitboard.h"

constexpr int WHITE = +1;
constexpr int BLACK = -1;
// Define a constant for the maximum depth of your AI.
constexpr int MAX_DEPTH = 24;
// Define constants for ranks and files
constexpr uint64_t NotAFile(0xFEFEFEFEFEFEFEFEULL); // A file mask
constexpr uint64_t NotHFile(0x7F7F7F7F7F7F7F7FULL); // H file mask
constexpr uint64_t Rank3(0x0000000000FF0000ULL); // Rank 3 mask
constexpr uint64_t Rank6(0x0000FF0000000000ULL); // Rank 6 mask

enum AllBitBoards
{
    WHITE_PAWNS,
    WHITE_KNIGHTS,
    WHITE_BISHOPS,
    WHITE_ROOKS,
    WHITE_QUEENS,
    WHITE_KING,
    WHITE_ALL_PIECES,
    BLACK_PAWNS,
    BLACK_KNIGHTS,
    BLACK_BISHOPS,
    BLACK_ROOKS,
    BLACK_QUEENS,
    BLACK_KING,
    BLACK_ALL_PIECES,
    OCCUPANCY,
    EMPTY_SQUARES,
    e_numBitboards
};

enum MoveFlags {
    EnPassant = 0x01, // 0000 0001
    IsCapture = 0x02, // 0000 0010
    KingSideCastle = 0x04, // 0000 0100
    QueenSideCastle = 0x08, // 0000 1000
    IsPromotion = 0x10 // 0001 0000
};

constexpr int WhiteKingSide  = 0x1;
constexpr int WhiteQueenSide = 0x2;
constexpr int BlackKingSide  = 0x4;
constexpr int BlackQueenSide = 0x8;

#pragma pack(push, 1)
struct BitMove {
    unsigned char from;
    unsigned char to;
    unsigned char piece;
    unsigned char flags;

    BitMove(int from, int to, ChessPiece piece, int flags = 0)
        : from(from), to(to), piece(piece), flags(flags) { }
        
    BitMove() : from(0), to(0), piece(NoPiece), flags(0) { }
    
    bool operator==(const BitMove& other) const {
        return from == other.from && 
               to == other.to && 
               piece == other.piece &&
               flags == other.flags;
    }

    void print() const {
        std::cout << "BitMove { "
                << "from=" << (int)from << ", "
                << "to=" << (int)to << ", "
                << "piece=" << (int)piece << ", "
                << "flags=" << (int)flags
                << " }" << std::endl;
    }
};
#pragma pack(pop)

struct alignas(32) GameStateData {
    char state[64];                 // persisitent
    int flags;
    char color;                     // BLACK or WHITE

    GameStateData() : flags(12)
        , color(WHITE) {
        std::memset(state, '0', sizeof(state));
    }
    GameStateData(const GameStateData&) = default;
    GameStateData& operator=(const GameStateData&) = default;

     // Print the board in 8x8 format
    void printBoard() const {
        std::cout << "  +-----------------+" << std::endl;
        for (int rank = 7; rank >= 0; --rank) {
            std::cout << rank + 1 << " | ";
            for (int file = 0; file < 8; ++file) {
                char piece = state[rank * 8 + file];
                std::cout << piece << ' ';
            }
            std::cout << '|' << std::endl;
        }
        std::cout << "  +-----------------+" << std::endl;
        std::cout << "    a b c d e f g h" << std::endl;
        std::cout << "Color to move: " << (color == WHITE ? "White" : "Black") << std::endl;
        std::cout << "Castling Rights: "
                  << ((flags & KingSideCastle) ? "K" : "")
                  << ((flags & QueenSideCastle) ? "Q" : "")
                  << std::endl;
    }
};

class GameState : public GameStateData {
public:
    GameStateData stateStack[MAX_DEPTH];
    int stackPtr = 0;

    uint64_t _zobristHash[2]; // when one hash value is made, the other is made as well because it's just a xor of the first by the color bit
    BitBoard _bitboards[e_numBitboards];
    BitBoard _attackBitBoard;

    GameState() : stackPtr(0) { }

    void init(const char* newState, char player);

    inline void printBitMove(const BitMove& move) {
        std::cout << "BitMove { "
                << "from=" << (int)move.from << ", "
                << "to=" << (int)move.to << ", "
                << "piece=" << (int)move.piece << ", "
                << "flags=" << (int)move.flags
                << " }" << std::endl;
    }

    inline void pushMove(const BitMove& move) {
        pushState(); // Save full state including flags

        printBoard();

        unsigned char fromPiece = state[move.from];
        unsigned char toPiece   = state[move.to];

        state[move.from] = '0';
        state[move.to] = fromPiece;

        // -------------------------------
        // Update castling rights
        // -------------------------------
        // King moves => lose both castling rights for that side
        if (fromPiece == 'K') flags &= ~(WhiteKingSide | WhiteQueenSide);
        if (fromPiece == 'k') flags &= ~(BlackKingSide | BlackQueenSide);

        // Rook moves or capture => lose that rook's castling right
        if (move.from == 0 || move.to == 0) flags &= ~WhiteQueenSide;
        if (move.from == 7 || move.to == 7) flags &= ~WhiteKingSide;
        if (move.from == 56 || move.to == 56) flags &= ~BlackQueenSide;
        if (move.from == 63 || move.to == 63) flags &= ~BlackKingSide;

        // -------------------------------
        // Castling moves
        // -------------------------------
        if (move.flags & KingSideCastle) {
            if (fromPiece == 'K') { 
                state[5] = state[7]; state[7] = '0'; 
                _bitboards[WHITE_ROOKS] &= ~(1ULL << 7);
                _bitboards[WHITE_ROOKS] |=  1ULL << 5;
            }       // White
            if (fromPiece == 'k') { 
                state[61] = state[63]; state[63] = '0'; 
                _bitboards[BLACK_ROOKS] &= ~(1ULL << 63);
                _bitboards[BLACK_ROOKS] |=  1ULL << 61;
            }     // Black
        }
        if (move.flags & QueenSideCastle) {
            if (fromPiece == 'K') { 
                state[3] = state[0]; state[0] = '0'; 
                _bitboards[WHITE_ROOKS] &= ~(1ULL << 0);
                _bitboards[WHITE_ROOKS] |=  1ULL << 3;
            }       // White
            if (fromPiece == 'k') { 
                state[59] = state[56]; state[56] = '0'; 
                _bitboards[BLACK_ROOKS] &= ~(1ULL << 56);
                _bitboards[BLACK_ROOKS] |=  1ULL << 59;
            }     // Black
        }

        // -------------------------------
        // En passant capture
        // -------------------------------
        if (move.flags & EnPassant) {
            if (fromPiece == 'P') state[move.to - 8] = '0';  // White
            else state[move.to + 8] = '0';                   // Black
        }

        // -------------------------------
        // Promotion
        // -------------------------------
        if (move.flags & IsPromotion) {
            state[move.to] = (fromPiece == 'P') ? 'Q' : 'q';
        }

        // -------------------------------
        // Switch turn
        // -------------------------------
        color = (color == WHITE) ? BLACK : WHITE;

        // Do NOT reset flags here!
    }

    inline void pushState() {
        assert(stackPtr < MAX_DEPTH);
        stateStack[stackPtr++] = static_cast<const GameStateData&>(*this);
    }
    inline void popState() {
        assert(stackPtr > 0);
        static_cast<GameStateData&>(*this) = stateStack[--stackPtr];
    }

    std::vector<BitMove> generateAllMoves();
    void shutdown();
private:
    const BitBoard generatePawnAttacks(const BitBoard pawns, char color);
    uint64_t generatePawnAttacksBitBoard(int square, char color);
    
    void generateKnightMoves(std::vector<BitMove>& moves, BitBoard knightBoard, uint64_t occupancy);
    void generateKingMoves(std::vector<BitMove>& moves, BitBoard kingBoard, uint64_t occupancy);
    void generateRooksMoves(std::vector<BitMove>& moves, BitBoard bishopBoard, uint64_t occupancy, uint64_t friendlies);
    void generateQueensMoves(std::vector<BitMove>& moves, BitBoard bishopBoard, uint64_t occupancy, uint64_t friendlies);

    void generateBishopMoves(std::vector<BitMove>& moves, BitBoard bishopBoard, uint64_t occupancy, uint64_t friendlies);
    void generatePawnMoveList(std::vector<BitMove>& moves, const BitBoard pawns, const BitBoard emptySquares, const BitBoard enemyPieces, char color);
    void addPawnBitboardMovesToList(std::vector<BitMove>& moves, const BitBoard bitboard, const int shift);
    bool isSquareAttacked(int square, char attackerColor, const BitBoard (&boards)[e_numBitboards]);
    void filterOutIllegalMoves(std::vector<BitMove>& moves);

};
