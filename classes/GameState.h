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
constexpr int MAX_DEPTH = 100;
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
    unsigned char state[64];
    uint64_t bitboards[e_numBitboards];
    unsigned int flags;
    char color;
    int halfmoveClock;
    int fullmoveNumber;

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

        applyMove(move);
    }

    inline void applyMove(const BitMove &move) {
        unsigned char fromPiece = state[move.from];
        unsigned char toPiece   = state[move.to];

        // Remove castling rights if a king or corner rook moves/captured
        auto clearCastlingOnKingMove = [&](unsigned char piece) {
            if (piece == 'K') flags &= ~(WhiteKingSide | WhiteQueenSide);
            if (piece == 'k') flags &= ~(BlackKingSide | BlackQueenSide);
        };
        auto clearCastlingOnRookSquare = [&](int sq) {
            // white a1=0, h1=7 ; black a8=56, h8=63
            if (sq == 0)  flags &= ~WhiteQueenSide;
            if (sq == 7)  flags &= ~WhiteKingSide;
            if (sq == 56) flags &= ~BlackQueenSide;
            if (sq == 63) flags &= ~BlackKingSide;
        };

        // Save masks
        uint64_t fromMask = 1ULL << move.from;
        uint64_t toMask   = 1ULL << move.to;

        // If a rook or king moves, clear castling rights appropriately
        clearCastlingOnKingMove(fromPiece);
        if (fromPiece == 'R' || fromPiece == 'r') clearCastlingOnRookSquare(move.from);

        // If capturing a rook on its original square, clear opponent's castling right
        if (toPiece == 'R' || toPiece == 'r') clearCastlingOnRookSquare(move.to);

        // Handle en-passant capture (remove captured pawn)
        if (move.flags & EnPassant) {
            int capturedSquare = (fromPiece == 'P') ? (move.to - 8) : (move.to + 8);
            state[capturedSquare] = '0';
            // remove captured pawn from bitboards (opponent pawn)
            if (fromPiece == 'P') _bitboards[BLACK_PAWNS] &= ~(1ULL << capturedSquare);
            else                  _bitboards[WHITE_PAWNS] &= ~(1ULL << capturedSquare);
        }

        // Handle normal capture: remove captured piece bit from its bitboard
        if (move.flags & IsCapture && !(move.flags & EnPassant)) {
            switch (toPiece) {
                case 'P': _bitboards[WHITE_PAWNS]   &= ~toMask; break;
                case 'N': _bitboards[WHITE_KNIGHTS] &= ~toMask; break;
                case 'B': _bitboards[WHITE_BISHOPS] &= ~toMask; break;
                case 'R': _bitboards[WHITE_ROOKS]   &= ~toMask; break;
                case 'Q': _bitboards[WHITE_QUEENS]  &= ~toMask; break;
                case 'K': _bitboards[WHITE_KING]    &= ~toMask; break;
                case 'p': _bitboards[BLACK_PAWNS]   &= ~toMask; break;
                case 'n': _bitboards[BLACK_KNIGHTS] &= ~toMask; break;
                case 'b': _bitboards[BLACK_BISHOPS] &= ~toMask; break;
                case 'r': _bitboards[BLACK_ROOKS]   &= ~toMask; break;
                case 'q': _bitboards[BLACK_QUEENS]  &= ~toMask; break;
                case 'k': _bitboards[BLACK_KING]    &= ~toMask; break;
                default: break;
            }
        }

        // Move piece in state array
        state[move.from] = '0';
        state[move.to]   = fromPiece;

        // Handle castling rook moves in state and bitboards
        if (move.flags & KingSideCastle) {
            if (fromPiece == 'K') {
                // white: rook 7 -> 5
                state[7] = '0';
                state[5] = 'R';
                _bitboards[WHITE_ROOKS] &= ~(1ULL << 7);
                _bitboards[WHITE_ROOKS] |=  1ULL << 5;
            } else if (fromPiece == 'k') {
                // black: rook 63 -> 61
                state[63] = '0';
                state[61] = 'r';
                _bitboards[BLACK_ROOKS] &= ~(1ULL << 63);
                _bitboards[BLACK_ROOKS] |=  1ULL << 61;
            }
        }
        if (move.flags & QueenSideCastle) {
            if (fromPiece == 'K') {
                // white: rook 0 -> 3
                state[0] = '0';
                state[3] = 'R';
                _bitboards[WHITE_ROOKS] &= ~(1ULL << 0);
                _bitboards[WHITE_ROOKS] |=  1ULL << 3;
            } else if (fromPiece == 'k') {
                // black: rook 56 -> 59
                state[56] = '0';
                state[59] = 'r';
                _bitboards[BLACK_ROOKS] &= ~(1ULL << 56);
                _bitboards[BLACK_ROOKS] |=  1ULL << 59;
            }
        }

        // Promotion: convert pawn -> queen in state (already set above) and move bit from pawn board to queen board
        if (move.flags & IsPromotion) {
            // state[move.to] already changed below in caller earlier; ensure bitboards reflect new piece
            if (fromPiece == 'P') {
                // remove pawn from white pawn board, add to white queens
                _bitboards[WHITE_PAWNS] &= ~fromMask; // fromMask was pawn original square
                _bitboards[WHITE_PAWNS] &= ~toMask; // safety
                _bitboards[WHITE_QUEENS] |= toMask;
            } else if (fromPiece == 'p') {
                _bitboards[BLACK_PAWNS] &= ~fromMask;
                _bitboards[BLACK_PAWNS] &= ~toMask;
                _bitboards[BLACK_QUEENS] |= toMask;
            }
        } else {
            // No promotion: move the piece bit from its board to target board
            auto updateBitboard = [&](int boardIndex) {
                _bitboards[boardIndex] &= ~fromMask;  // remove from origin
                _bitboards[boardIndex] |=  toMask;    // add at destination
            };

            switch (fromPiece) {
                case 'P': updateBitboard(WHITE_PAWNS);   break;
                case 'N': updateBitboard(WHITE_KNIGHTS); break;
                case 'B': updateBitboard(WHITE_BISHOPS); break;
                case 'R': updateBitboard(WHITE_ROOKS);   break;
                case 'Q': updateBitboard(WHITE_QUEENS);  break;
                case 'K': updateBitboard(WHITE_KING);    break;
                case 'p': updateBitboard(BLACK_PAWNS);   break;
                case 'n': updateBitboard(BLACK_KNIGHTS); break;
                case 'b': updateBitboard(BLACK_BISHOPS); break;
                case 'r': updateBitboard(BLACK_ROOKS);   break;
                case 'q': updateBitboard(BLACK_QUEENS);  break;
                case 'k': updateBitboard(BLACK_KING);    break;
                default: break;
            }
        }

        // Recompute aggregated bitboards to keep everything consistent
        _bitboards[WHITE_ALL_PIECES].setData(
            _bitboards[WHITE_PAWNS].getData()
            | _bitboards[WHITE_KNIGHTS].getData()
            | _bitboards[WHITE_BISHOPS].getData()
            | _bitboards[WHITE_ROOKS].getData()
            | _bitboards[WHITE_QUEENS].getData()
            | _bitboards[WHITE_KING].getData()
        );

        _bitboards[BLACK_ALL_PIECES].setData(
            _bitboards[BLACK_PAWNS].getData()
            | _bitboards[BLACK_KNIGHTS].getData()
            | _bitboards[BLACK_BISHOPS].getData()
            | _bitboards[BLACK_ROOKS].getData()
            | _bitboards[BLACK_QUEENS].getData()
            | _bitboards[BLACK_KING].getData()
        );

        _bitboards[OCCUPANCY].setData(
            _bitboards[WHITE_ALL_PIECES].getData() | _bitboards[BLACK_ALL_PIECES].getData()
        );

        // Finally switch side to move
        color = (color == WHITE) ? BLACK : WHITE;
    }


    inline void undoMove(const BitMove &move, unsigned char capturedPiece = '0') {
        unsigned char movedPiece = state[move.to];

        // Undo move on board
        state[move.from] = movedPiece;
        state[move.to]   = capturedPiece;

        // Undo castling
        if (move.flags & KingSideCastle) {
            if (movedPiece == 'K') { state[5] = '0'; state[7] = 'R'; }
            if (movedPiece == 'k') { state[61] = '0'; state[63] = 'r'; }
        }
        if (move.flags & QueenSideCastle) {
            if (movedPiece == 'K') { state[3] = '0'; state[0] = 'R'; }
            if (movedPiece == 'k') { state[59] = '0'; state[56] = 'r'; }
        }

        // Undo en passant
        if (move.flags & EnPassant) {
            int capturedSquare = (movedPiece == 'P') ? move.to - 8 : move.to + 8;
            state[capturedSquare] = (movedPiece == 'P') ? 'p' : 'P';
        }

        // Undo promotion
        if (move.flags & IsPromotion) {
            state[move.from] = (movedPiece == 'Q') ? 'P' : 'p';
        }

        // Undo bitboards
        uint64_t fromMask = 1ULL << move.from;
        uint64_t toMask   = 1ULL << move.to;

        auto revertBitboard = [&](int boardIndex) {
            _bitboards[boardIndex] &= ~toMask;
            _bitboards[boardIndex] |= fromMask;
        };

        switch (movedPiece) {
            case 'P': revertBitboard(WHITE_PAWNS); break;
            case 'N': revertBitboard(WHITE_KNIGHTS); break;
            case 'B': revertBitboard(WHITE_BISHOPS); break;
            case 'R': revertBitboard(WHITE_ROOKS);   break;
            case 'Q': revertBitboard(WHITE_QUEENS);  break;
            case 'K': revertBitboard(WHITE_KING);    break;
            case 'p': revertBitboard(BLACK_PAWNS);   break;
            case 'n': revertBitboard(BLACK_KNIGHTS); break;
            case 'b': revertBitboard(BLACK_BISHOPS); break;
            case 'r': revertBitboard(BLACK_ROOKS);   break;
            case 'q': revertBitboard(BLACK_QUEENS);  break;
            case 'k': revertBitboard(BLACK_KING);    break;
        }

        // Restore captured piece in bitboards
        if (move.flags & IsCapture) {
            if (capturedPiece != '0') {
                uint64_t capMask = 1ULL << move.to;
                switch (capturedPiece) {
                    case 'P': _bitboards[WHITE_PAWNS] |= capMask; break;
                    case 'N': _bitboards[WHITE_KNIGHTS] |= capMask; break;
                    case 'B': _bitboards[WHITE_BISHOPS] |= capMask; break;
                    case 'R': _bitboards[WHITE_ROOKS]   |= capMask; break;
                    case 'Q': _bitboards[WHITE_QUEENS]  |= capMask; break;
                    case 'K': _bitboards[WHITE_KING]    |= capMask; break;
                    case 'p': _bitboards[BLACK_PAWNS]   |= capMask; break;
                    case 'n': _bitboards[BLACK_KNIGHTS] |= capMask; break;
                    case 'b': _bitboards[BLACK_BISHOPS] |= capMask; break;
                    case 'r': _bitboards[BLACK_ROOKS]   |= capMask; break;
                    case 'q': _bitboards[BLACK_QUEENS]  |= capMask; break;
                    case 'k': _bitboards[BLACK_KING]    |= capMask; break;
                }
            }
        }

        // Switch turn back
        color = (color == WHITE) ? BLACK : WHITE;
    }

    inline void pushState() {
        assert(stackPtr < MAX_DEPTH);

        // Copy full GameStateData part
        GameStateData& slot = stateStack[stackPtr++];
        std::memcpy(slot.state, state, sizeof(state));
        std::memcpy(slot.bitboards, _bitboards, sizeof(_bitboards));
        slot.flags = flags;
        slot.color = color;
        slot.halfmoveClock = halfmoveClock;
        slot.fullmoveNumber = fullmoveNumber;
    }
    inline void popState() {
        assert(stackPtr > 0);
        GameStateData& slot = stateStack[--stackPtr];

        // Restore all fields
        std::memcpy(state, slot.state, sizeof(state));
        std::memcpy(_bitboards, slot.bitboards, sizeof(_bitboards));
        flags = slot.flags;
        color = slot.color;
        halfmoveClock = slot.halfmoveClock;
        fullmoveNumber = slot.fullmoveNumber;
    }

    std::vector<BitMove> generateAllMoves();
    void shutdown();
    bool isKingInCheck();
    void rebuildBitboards();
    
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
