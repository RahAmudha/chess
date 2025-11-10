#include "Chess.h"
#include <limits>
#include <cmath>

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

// square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
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
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;
    if (pieceColor == currentPlayer) return true;
    return false;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
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
