#pragma once

#include "Game.h"
#include "Grid.h"
#include "Bitboard.h"
#include "GameState.h"
#include "../Application.h"

constexpr int pieceSize = 80;

class Chess : public Game
{
public:
    Chess();
    ~Chess();

    void setUpBoard() override;

    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    bool actionForEmptyHolder(BitHolder &holder) override;

    void endTurn() override;

    void stopGame() override;

    Player *checkForWinner() override;
    bool checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;

    Grid* getGrid() override { return _grid; }

    // Negamax Functions
    BitMove findBestMove(int depth);
    int negamax(int depth, int alpha, int beta);
    int evaluate();

    static constexpr int PIECE_VALUES[7] = { 0, 100, 320, 330, 500, 900, 0 }; 

    void updateAI() override;
    bool gameHasAI() override { return true; }

private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;

    Grid* _grid;
    GameState* _gameState;
    BitMove _lastMove;
};