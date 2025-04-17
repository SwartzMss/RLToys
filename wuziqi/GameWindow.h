#pragma once
#include <QWidget>
#include "GameBoard.h"

class QLabel;
class QPushButton;
class QComboBox;

class GameWindow : public QWidget {
	Q_OBJECT
public:
	explicit GameWindow(GameBoard* board, QWidget* parent = nullptr);
	int getCurrentPlayer() const;
	bool isGameStarted() const { return gameStarted; }
	bool makeMove(int x, int y, int color);
protected:
	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;

private:
	GameBoard* board;
	int gridSize = 40;
	int currentPlayer = 1;
	bool gameStarted = false;

	int blackWinCount = 0;
	int whiteWinCount = 0;
	int drawCount = 0;

	QLabel* statusLabel = nullptr;
	QLabel* currentPlayerLabel = nullptr;
	QComboBox* blackMode = nullptr;
	QComboBox* whiteMode = nullptr;
	QPushButton* startBtn = nullptr;
	QPushButton* endBtn = nullptr;

	void checkWin();
	void updateStatusLabel();
	void updateCurrentPlayerLabel();
	void updateGameControls();
};