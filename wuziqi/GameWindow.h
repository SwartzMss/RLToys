#pragma once
#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "GameBoard.h"

class QLabel;
class QPushButton;
class QComboBox;
class QTimer;

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

private slots:
	void checkAndMakeMove();
	void onAiMoveResponse(QNetworkReply* reply);

private:
	GameBoard* board;
	int gridSize = 40;
	int currentPlayer = 1;
	bool gameStarted = false;

	int blackWinCount = 0;
	int whiteWinCount = 0;

	QLabel* statusLabel = nullptr;
	QComboBox* blackMode = nullptr;
	QComboBox* whiteMode = nullptr;
	QPushButton* startBtn = nullptr;
	QPushButton* endBtn = nullptr;
	QTimer* moveTimer = nullptr;
	QNetworkAccessManager* networkManager = nullptr;

	void checkWin();
	void updateStatusLabel();
	void updateGameControls();
	void requestAiMove();
};