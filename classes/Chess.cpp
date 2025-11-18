#include "Chess.h"
#include "Bitboard.h"
#include <cstdint>
#include <limits>
#include <cmath>

BitboardElement lastGeneratedMoves;

Chess::Chess()
{
    _grid = new Grid(8, 8);
}

Chess::~Chess()
{
    delete _grid;
}

char Chess::pieceNotation(int x, int y) const
{
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    Bit *bit = _grid->getSquare(x, y)->bit();
    char notation = '0';
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit* bit = new Bit();
    int newPiece = (piece | playerNumber << 7);
    bit->setGameTag(newPiece);
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    startGame();
}


void Chess::FENtoBoard(const std::string& fen) {
    // convert a FEN string to a board
    // FEN is a space delimited string with 6 fields
    // 1: piece placement (from white's perspective)

    int index = 0;
    int count = 0;

    Bit *bit = nullptr;

    for (;fen[index] != ' ' && index < int(fen.size()); ++index){}
    for (;fen[index] != '/' && index >= 0; --index){} index++;

    // I just really wanted to use this function lol
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        bit = nullptr;
        if (count == 0){
            WACK: // Truly
            if (fen[index] == ' ' || index >= int(fen.size())) {
                for (;fen[index] != '/' && index >= 0; --index){};
                for (--index; fen[index] != '/' && index >= 0; --index){};
                ++index;
            }

            switch (fen[index]){

                // Move the index but stay on the same square!
                // Move the index to the start of the previous row
                case '/': 
                    for (--index; fen[index] != '/' && index >= 0; --index){} 
                    for (--index; fen[index] != '/' && index >= 0; --index){} 
                    ++index; goto WACK;

                // Black Pieces
                case 'p': bit = PieceForPlayer(1, Pawn); break;
                case 'n': bit = PieceForPlayer(1, Knight); break;
                case 'b': bit = PieceForPlayer(1, Bishop); break;
                case 'r': bit = PieceForPlayer(1, Rook); break;
                case 'q': bit = PieceForPlayer(1, Queen); break;
                case 'k': bit = PieceForPlayer(1, King); break;
                
                // White Pieces
                case 'P': bit = PieceForPlayer(0, Pawn); break;
                case 'N': bit = PieceForPlayer(0, Knight); break;
                case 'B': bit = PieceForPlayer(0, Bishop); break;
                case 'R': bit = PieceForPlayer(0, Rook); break;
                case 'Q': bit = PieceForPlayer(0, Queen); break;
                case 'K': bit = PieceForPlayer(0, King); break;

                // Keep count of how many empty spaces
                case '8': count = 7; break;
                case '7': count = 6; break;
                case '6': count = 5; break;
                case '5': count = 4; break;
                case '4': count = 3; break;
                case '3': count = 2; break;
                case '2': count = 1; break;
                case '1': break;

                default: break;
            }

            if (bit){
                bit->setPosition(square->getPosition());
                square->setBit(bit);
            }

            ++index;

        } else {
            --count;
        }
    });

    // NOT PART OF THIS ASSIGNMENT BUT OTHER THINGS THAT CAN BE IN A FEN STRING
    // ARE BELOW
    // 2: active color (W or B)
    for (;fen[index] != ' ' && index < int(fen.size()); ++index){}
    ++index;

    if (index > int(fen.size())) return;
    
    int currentPlayer;
    if (fen[index] != '-') {
        currentPlayer = fen[index] == 'b';
    }

    // Created custom function to set the current turn number (This does sets the full moves and the current player)

    // 3: castling availability (KQkq or -)
    for (;fen[index] != ' ' && index < int(fen.size()); ++index){}
    ++index;

    if (index > int(fen.size())) return;

    if (fen[index] != '-'){

        for (;fen[index] != ' ' && index < int(fen.size()); ++index){
            switch (fen[index]){
                
                // Will implement later
                case 'w': break;
                case 'b': break;
                case 'W': break;
                case 'B': break;
                default: break;
            }
        }

    }

    // 4: en passant target square (in algebraic notation, or -)
    int col, row;
    ++index;

    if (index > int(fen.size())) return;

    if (fen[index] != '-'){
        col = fen[index] - 'a';
        ++index;
        row = fen[index] - '0';
    }
    
    // Will store these values somewhere

    // 5: halfmove clock (number of halfmoves since the last capture or pawn advance)
    for (;fen[index] != ' ' && index < int(fen.size()); ++index){}
    ++index;

    if (index > int(fen.size())) return;

    int tenh, oneh, halfmove=-1;
    if (fen[index] != '-'){
        tenh = fen[index] - '0';
        ++index;
        if (index < int(fen.size() || fen[index] != ' ')){
            oneh = fen[index] - '0';
            halfmove = tenh*10 + oneh;
        } else {
            halfmove = tenh;
        }
    }

    // Step 6: Full Moves
    for (;fen[index] != ' ' && index < int(fen.size()); ++index){}
    ++index;

    if (index > int(fen.size())) return;

    int tenf, onef, fullmove=-1;
    if (fen[index] != '-'){
        tenf = fen[index] - '0';
        ++index;
        if (index < int(fen.size())){
            onef = fen[index] - '0';
            fullmove = tenf*10 + onef;
        } else {
            fullmove = tenf;
        }
    }

    setCurrentTurnNo(fullmove + currentPlayer);
}

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{   
    ChessSquare* source = dynamic_cast<ChessSquare*>(&src);

    ChessPiece piece = (ChessPiece)(bit.gameTag());
    std::cout << piece << ' ' << Pawn << std::endl;
    
    int index = _grid->getIndex(source->getColumn(), source->getRow());

    // Ask the bitboard engine for a mask of legal moves
    BitboardElement moves = getLegalMovesFor(piece, source, index);

    // Save these moves so GUI can highlight them
    lastGeneratedMoves = moves;

    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber();
    int pieceColor = bit.gameTag() >> 7;
    if (pieceColor == currentPlayer) return true;
    return false;
}

BitboardElement Chess::getLegalMovesFor(ChessPiece piece, ChessSquare* src, int index){
    uint64_t whitePieces = 0;
    uint64_t blackPieces = 0;
    _grid->forEachSquare([&](ChessSquare* square, int x, int y){
        if (!square->bit()) return;
        int index = _grid->getIndex(x, y);
        int pieceColor = square->bit()->gameTag() >> 7;
        if (pieceColor) {
            blackPieces |= 1ULL << index;
        } else {
            whitePieces |= 1ULL << index;
        }
    });

    bool isWhite = (src->bit()->gameTag() >> 7) == 0;
    uint64_t currentPos = 1ULL << index;

    const uint64_t currentPieces = isWhite ? whitePieces : blackPieces;
    const uint64_t opponentPieces = isWhite ? blackPieces : whitePieces;

    const uint64_t startRank = isWhite ? 0x000000000000FF00ULL : 0x00FF000000000000ULL;

    const uint64_t notAFile = 0xfefefefefefefefeULL;
    const uint64_t notHFile = 0x7f7f7f7f7f7f7f7fULL;

    const uint64_t occupied = whitePieces | blackPieces;

    switch(piece & 127){
        case Pawn: {

            const uint64_t oneStep  = isWhite ? (currentPos << 8) : (currentPos >> 8);
            const uint64_t twoStep  = isWhite ? (currentPos << 16) : (currentPos >> 16);

            const uint64_t captureLeft  = isWhite ? ((currentPos << 7) & notAFile)
                                                : ((currentPos >> 9) & notHFile);
            const uint64_t captureRight = isWhite ? ((currentPos << 9) & notHFile)
                                                : ((currentPos >> 7) & notAFile);
            
            currentPos |= (captureLeft & opponentPieces) | (captureRight & opponentPieces);

            if (oneStep & occupied) break;

            currentPos |= oneStep;
            if ((currentPos & startRank) && !(twoStep & occupied)) currentPos |= twoStep;

            break;
        }

        case Knight: {
            const uint64_t notABFile = 0xfcfcfcfcfcfcfcfcULL;
            const uint64_t notGHFile = 0x3f3f3f3f3f3f3f3fULL;

            const uint64_t knightMoves =  (currentPos & notAFile) << 15  | 
                                    (currentPos & notHFile) << 17  |
                                    (currentPos & notABFile) << 6  |
                                    (currentPos & notGHFile) << 10 |
                                    (currentPos & notHFile) >> 15  |
                                    (currentPos & notAFile) >> 17  |
                                    (currentPos & notGHFile) >> 6  |
                                    (currentPos & notABFile) >> 10 ;

            currentPos |= knightMoves;
            
            currentPos &= ~currentPieces;

            break;
        }

        case Bishop : { // Will work on something more efficient later
            uint64_t moves = 0ULL;

            uint64_t pos = currentPos;

            uint64_t temp = pos;
            while (temp & notHFile) {
                temp <<= 9;
                if (temp & currentPieces) break;
                moves |= temp;
                if (temp & opponentPieces) break; 
            }

            temp = pos;
            while (temp & notAFile) {
                temp <<= 7;
                if (temp & currentPieces) break;
                moves |= temp;
                if (temp & opponentPieces) break; 
            }

            temp = pos;
            while (temp & notHFile) {
                temp >>= 7;
                if (temp & currentPieces) break;
                moves |= temp;
                if (temp & opponentPieces) break; 
            }

            temp = pos;
            while (temp & notAFile) {
                temp >>= 9;
                if (temp & currentPieces) break;
                moves |= temp;
                if (temp & opponentPieces) break; 
            }

            currentPos = moves;
        }

        case Rook : {
            uint64_t moves = 0ULL;

            uint64_t pos = currentPos;

            uint64_t temp = pos;
            while (temp & notHFile) {
                temp <<= 1;
                if (temp & occupied) {
                    moves |= temp & opponentPieces;
                    break;
                }
                moves |= temp;
            }

            temp = pos;
            while (temp & notAFile) {
                temp >>= 1;
                if (temp & occupied) {
                    moves |= temp & opponentPieces;
                    break;
                }
                moves |= temp;
            }

            temp = pos;
            while ((temp >> 8) != 0) {
                temp >>= 8;
                if (temp & occupied) {
                    moves |= temp & opponentPieces;
                    break;
                }
                moves |= temp;
            }

            temp = pos;
            while ((temp << 8) != 0) {
                temp <<= 8;
                if (temp & occupied) {
                    moves |= temp & opponentPieces;
                    break;
                }
                moves |= temp;
            }

            currentPos = moves;
        }

        case King : {
            uint64_t moves = 0ULL;

            uint64_t east  = (currentPos & notHFile) << 1;
            uint64_t west  = (currentPos & notAFile) >> 1;
            uint64_t north = currentPos << 8;
            uint64_t south = currentPos >> 8;

            uint64_t ne = (currentPos & notHFile) << 9;
            uint64_t nw = (currentPos & notAFile) << 7; 
            uint64_t se = (currentPos & notHFile) >> 7;
            uint64_t sw = (currentPos & notAFile) >> 9;

            moves |= east | west | north | south | ne | nw | se | sw;

            moves &= ~currentPieces;

            currentPos = moves;
        }

    }

    BitboardElement(currentPos).printBitboard();

    return BitboardElement(currentPos);
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    ChessSquare* source = dynamic_cast<ChessSquare*>(&src);
    ChessSquare* destination = dynamic_cast<ChessSquare*>(&dst);

    return true;
}


void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

std::string Chess::initialStateString()
{
    return stateString();
}

std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation( x, y );
        }
    );
    return s;}

void Chess::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
        } else {
            square->setBit(nullptr);
        }
    });
}
