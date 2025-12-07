#include "Chess.h"
#include "Bitboard.h"
#include <cstdint>
#include <limits>
#include <cmath>

static const int MATE_SCORE = 1000000;
static const int INF = 10000000;

Chess::Chess()
{
    _grid = new Grid(8, 8);
    _gameState = new GameState();
}

Chess::~Chess()
{
    delete _grid;
    delete _gameState;
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

    _gameState->init(stateString().c_str(), WHITE);

    _gameState->rebuildBitboards();

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
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber();
    int pieceColor = bit.gameTag() >> 7;
    if (pieceColor == currentPlayer) return true;
    return false;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    ChessSquare* source = dynamic_cast<ChessSquare*>(&src);
    ChessSquare* destination = dynamic_cast<ChessSquare*>(&dst);

    int srcIndex = source->getColumn() + 8 * source->getRow();
    int dstIndex = destination->getColumn() + 8 * destination->getRow();

    auto moves = _gameState->generateAllMoves();

    for (auto &m : moves) {
        if (m.from == srcIndex && m.to == dstIndex){
            _lastMove = m;
            return true;
        }
    }
    return false;
}

// Added Castling 
void Chess::endTurn(){
    _gameOptions.currentTurnNo++;
	std::string startState = stateString();
	Turn *turn = new Turn;
	turn->_boardState = stateString();
	turn->_date = (int)_gameOptions.currentTurnNo;
	turn->_score = _gameOptions.score;
	turn->_gameNumber = _gameOptions.gameNumber;
	_turns.push_back(turn);

    _gameState->applyMove(_lastMove);

    Bit* rook;

    if (_lastMove.flags & MoveFlags::KingSideCastle) {
        if (_lastMove.piece == King) {

            if (_gameState->color == BLACK) {

                ChessSquare* from = _grid->getSquare(7, 0);
                ChessSquare* to   = _grid->getSquare(5, 0);

                rook = from->bit();

                rook->setPosition(to->getPosition());
                to->setBit(rook);
            }
            else {

                ChessSquare* from = _grid->getSquare(7, 7);
                ChessSquare* to   = _grid->getSquare(5, 7);

                rook = from->bit();

                rook->setPosition(to->getPosition());
                to->setBit(rook);
            }
        }
    }
    else if (_lastMove.flags & MoveFlags::QueenSideCastle) {
        if (_lastMove.piece == King) {

            if (_gameState->color == BLACK) {

                ChessSquare* from = _grid->getSquare(0, 0);
                ChessSquare* to   = _grid->getSquare(3, 0);

                rook = from->bit();

                rook->setPosition(to->getPosition());
                to->setBit(rook);
            }
            else {

                ChessSquare* from = _grid->getSquare(0, 7);
                ChessSquare* to   = _grid->getSquare(3, 7);

                rook = from->bit();

                rook->setPosition(to->getPosition());
                to->setBit(rook);
            }
        }
    }

	ClassGame::EndOfTurn();
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

Player* Chess::checkForWinner() {
    // Determine the player whose turn it is
    Player* currentPlayer = getCurrentPlayer(); // your method to get the player to move

    // Generate all legal moves for the current player
    std::vector<BitMove> moves = _gameState->generateAllMoves();

    // Check if king is in check
    bool inCheck = _gameState->isKingInCheck();

    if (moves.empty()) {
        if (inCheck) {
            // Current player has no moves and is in check → checkmate

            return getPlayerAt(1 - currentPlayer->playerNumber());
        } else {
            // Current player has no moves but not in check → stalemate
            return nullptr; // draw, no winner
        }
    }

    // If there are moves, no winner yet
    return nullptr;
}

bool Chess::checkForDraw()
{
    std::vector<BitMove> moves = _gameState->generateAllMoves();
    return moves.empty() && !_gameState->isKingInCheck();
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

static inline int popcount64(uint64_t x) {
    #if defined(__GNUG__) || defined(__clang__)
        return x ? __builtin_popcountll(x) : 0;
    #else
        int c=0; while (x) { x &= x-1; ++c; } return c;
    #endif
}

static inline bool isCaptureMove(const BitMove &m, const GameState *gs) {
    unsigned char target = gs->state[m.to];
    if (target == '0') return false;
    // capture if case differs (upper = white, lower = black)
    bool fromIsWhite = (gs->state[m.from] >= 'A' && gs->state[m.from] <= 'Z');
    bool toIsWhite   = (target >= 'A' && target <= 'Z');
    return fromIsWhite != toIsWhite;
}

int Chess::evaluate() {
    auto &bbs = _gameState->_bitboards;

    int whiteMaterial = 0;
    int blackMaterial = 0;

    // Using your exact values:
    whiteMaterial += popcount64(bbs[WHITE_PAWNS].getData())   * PIECE_VALUES[Pawn];
    whiteMaterial += popcount64(bbs[WHITE_KNIGHTS].getData()) * PIECE_VALUES[Knight];
    whiteMaterial += popcount64(bbs[WHITE_BISHOPS].getData()) * PIECE_VALUES[Bishop];
    whiteMaterial += popcount64(bbs[WHITE_ROOKS].getData())   * PIECE_VALUES[Rook];
    whiteMaterial += popcount64(bbs[WHITE_QUEENS].getData())  * PIECE_VALUES[Queen];

    blackMaterial += popcount64(bbs[BLACK_PAWNS].getData())   * PIECE_VALUES[Pawn];
    blackMaterial += popcount64(bbs[BLACK_KNIGHTS].getData()) * PIECE_VALUES[Knight];
    blackMaterial += popcount64(bbs[BLACK_BISHOPS].getData()) * PIECE_VALUES[Bishop];
    blackMaterial += popcount64(bbs[BLACK_ROOKS].getData())   * PIECE_VALUES[Rook];
    blackMaterial += popcount64(bbs[BLACK_QUEENS].getData())  * PIECE_VALUES[Queen];

    return whiteMaterial - blackMaterial; // White perspective
}

int Chess::negamax(int depth, int alpha, int beta) {
    // Leaf
    if (depth == 0) {
        // Ensure bitboards are synced (should be the case if we rebuilt after push)
        // return value from side-to-move perspective:
        int val = evaluate();
        return val * _gameState->color; // color is +1 white, -1 black
    }

    // Generate legal moves (generateAllMoves rebuilds bitboards for that node)
    std::vector<BitMove> moves = _gameState->generateAllMoves();

    if (moves.empty()) {
        // mate or stalemate
        if (_gameState->isKingInCheck()) {
            // Checkmate: large negative value for side to move.
            return - (MATE_SCORE + depth); // prefer shorter mate (depth as tiebreaker)
        } else {
            // Stalemate -> draw
            return 0;
        }
    }

    // Simple move ordering: captures first (stable)
    std::stable_sort(moves.begin(), moves.end(), [&](const BitMove &a, const BitMove &b){
        bool aCap = isCaptureMove(a, _gameState);
        bool bCap = isCaptureMove(b, _gameState);
        if (aCap != bCap) return aCap;           // captures first
        return a.from < b.from;                  // tie-breaker stable
    });

    int best = -INF;

    for (const auto &m : moves) {
        _gameState->pushMove(m);

        int score = -negamax(depth - 1, -beta, -alpha);

        _gameState->popState();

        if (score > best) best = score;
        if (score > alpha) alpha = score;
        if (alpha >= beta) {
            // cutoff
            break;
        }
    }

    return best;
}

BitMove Chess::findBestMove(int depth) {
    BitMove bestMove;
    int alpha = -INF;
    int beta  =  INF;
    int bestScore = -INF;

    // Root move generation (this will rebuild bitboards)
    std::vector<BitMove> moves = _gameState->generateAllMoves();
    if (moves.empty()) return bestMove;

    // Order root moves (captures first)
    std::stable_sort(moves.begin(), moves.end(), [&](const BitMove &a, const BitMove &b){
        bool aCap = isCaptureMove(a, _gameState);
        bool bCap = isCaptureMove(b, _gameState);
        if (aCap != bCap) return aCap;
        return a.from < b.from;
    });

    for (const auto &m : moves) {
        _gameState->pushMove(m);

        int val = -negamax(depth - 1, -beta, -alpha);

        _gameState->popState();

        if (val > bestScore) {
            bestScore = val;
            bestMove = m;
        }
        if (val > alpha) alpha = val;
    }

    return bestMove;
}

void Chess::updateAI() {
    Player* current = getCurrentPlayer();

    // If current player is human, do nothing.
    // Assuming player 0 = human, player 1 = AI.
    if (current->playerNumber() == 0)
        return;

    // Choose depth (tweak as needed)
    const int depth = 6;

    // Find best move using your negamax search
    BitMove best = findBestMove(depth);

    // If something is wrong and AI can't find a move, stop.
    if (best.from < 0 || best.to < 0)
        return;

    // Execute move on GUI board
    int fromX = best.from % 8;
    int fromY = best.from / 8;
    int toX   = best.to % 8;
    int toY   = best.to / 8;

    ChessSquare* source      = _grid->getSquare(fromX, fromY);
    ChessSquare* destination = _grid->getSquare(toX, toY);

    Bit* movingPiece = source->bit();
    if (!movingPiece) return; // safety check

    // Capture if needed
    if (destination->bit()) {
        destination->destroyBit();
    }

    // Move the piece visually
    destination->setBit(movingPiece);
    movingPiece->setPosition(destination->getPosition());
    source->setBit(nullptr);

    // Update engine internal state
    _lastMove = best;

    // End the turn
    endTurn();
}
