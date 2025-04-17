#include "GameWindow.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QTimer>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <cstdlib>
#include <ctime>

GameWindow::GameWindow(GameBoard* board, QWidget* parent)
	: QWidget(parent), board(board) {
	setFixedSize(400, 600);  // 减小窗口高度
	std::srand(std::time(nullptr));

	startBtn = new QPushButton("Start Game", this);
	startBtn->setGeometry(20, 480, 120, 30);

	endBtn = new QPushButton("End Game", this);
	endBtn->setGeometry(160, 480, 120, 30);
	endBtn->setEnabled(false);

	statusLabel = new QLabel(this);
	statusLabel->setGeometry(20, 420, 300, 50);  // 上移状态标签
	updateStatusLabel();

	// 添加当前玩家标签
	currentPlayerLabel = new QLabel(this);
	currentPlayerLabel->setGeometry(20, 380, 300, 30);
	currentPlayerLabel->setText("Current Player: -");
	currentPlayerLabel->setStyleSheet("font-weight: bold;");

	blackMode = new QComboBox(this);
	blackMode->addItems({ "Manual", "AI" });  // 移除Random选项
	blackMode->setGeometry(20, 550, 120, 25);

	whiteMode = new QComboBox(this);
	whiteMode->addItems({ "Manual", "AI" });  // 移除Random选项
	whiteMode->setGeometry(160, 550, 120, 25);

	connect(startBtn, &QPushButton::clicked, this, [=]() {
		gameStarted = true;
		updateGameControls();
		board->reset();
		currentPlayer = 1;
		updateCurrentPlayerLabel();
		update();
	});

	connect(endBtn, &QPushButton::clicked, this, [=]() {
		gameStarted = false;
		updateGameControls();
		currentPlayerLabel->setText("Current Player: -");
	});
}


void GameWindow::paintEvent(QPaintEvent*) {
	QPainter painter(this);
	for (int i = 0; i < BOARD_SIZE; ++i) {
		painter.drawLine(gridSize / 2, gridSize / 2 + i * gridSize,
			gridSize / 2 + (BOARD_SIZE - 1) * gridSize, gridSize / 2 + i * gridSize);
		painter.drawLine(gridSize / 2 + i * gridSize, gridSize / 2,
			gridSize / 2 + i * gridSize, gridSize / 2 + (BOARD_SIZE - 1) * gridSize);
	}

	for (int y = 0; y < BOARD_SIZE; ++y) {
		for (int x = 0; x < BOARD_SIZE; ++x) {
			int piece = board->get(x, y);
			if (piece == 0) continue;
			QPoint center(gridSize / 2 + x * gridSize, gridSize / 2 + y * gridSize);
			painter.setBrush(piece == 1 ? Qt::black : Qt::white);
			painter.drawEllipse(center, gridSize / 2 - 4, gridSize / 2 - 4);
		}
	}
}

void GameWindow::mousePressEvent(QMouseEvent* event)
{
	// 1. 如果游戏尚未开始，弹窗并返回
	if (!gameStarted) {
		QMessageBox::warning(this, "Warning", "Please start the game first!");
		return;
	}

	// 2. 获取当前玩家模式（0=手动, 1=AI）
	int currentMode = (currentPlayer == 1) ? blackMode->currentIndex() : whiteMode->currentIndex();
	// 若当前是 AI 模式，则用户点击无效，直接返回
	if (currentMode == 1) {
		return;
	}

	// 3. 计算点击坐标在棋盘中的 (x, y) 索引
	int x = event->pos().x() / gridSize;
	int y = event->pos().y() / gridSize;

	// 若坐标不在合法范围内，直接返回
	if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
		return;
	}

	// 4. 尝试落子
	if (board->place(x, y, currentPlayer)) {
		// 检查胜负
		checkWin();

		// 切换玩家
		currentPlayer = 3 - currentPlayer;
		updateCurrentPlayerLabel();  // 更新当前玩家标签
		update();
	}
}

bool GameWindow::makeMove(int x, int y, int color)
{
	// 1. 尝试放到棋盘
	bool success = board->place(x, y, color);
	if (!success)
		return false;

	checkWin();

	currentPlayer = 3 - currentPlayer; //1->2,2->1

	update();
	return true;
}

void GameWindow::checkWin() {
	int w = board->getWinner();
	if (w == 1 || w == 2) {
		if (w == 1) ++blackWinCount;
		if (w == 2) ++whiteWinCount;
		updateStatusLabel();
	} else if (w == 3) {
		++drawCount;  // 增加平局计数
		updateStatusLabel();
	}
}

void GameWindow::updateStatusLabel() {
	QString text = QString("Black Wins: %1\nWhite Wins: %2\nDraws: %3")
		.arg(blackWinCount)
		.arg(whiteWinCount)
		.arg(drawCount);


	if (statusLabel) {
		statusLabel->setTextFormat(Qt::PlainText);
		statusLabel->setWordWrap(true);
		statusLabel->setText(text);
		statusLabel->adjustSize();
	}
}

void GameWindow::updateCurrentPlayerLabel() {
	if (gameStarted) {
		QString player = currentPlayer == 1 ? "Black" : "White";
		currentPlayerLabel->setText(QString("Current Player: %1").arg(player));
		currentPlayerLabel->update();  // 添加这行，强制立即更新标签
	}
}

void GameWindow::updateGameControls() {
	startBtn->setEnabled(!gameStarted);
	endBtn->setEnabled(gameStarted);
	blackMode->setEnabled(!gameStarted);
	whiteMode->setEnabled(!gameStarted);
}

int GameWindow::getCurrentPlayer() const {
	return currentPlayer;
}
