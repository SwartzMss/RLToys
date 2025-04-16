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
	setFixedSize(640, 760);
	std::srand(std::time(nullptr));

	startBtn = new QPushButton("Start Game", this);
	startBtn->setGeometry(20, 640, 120, 30);

	endBtn = new QPushButton("End Game", this);
	endBtn->setGeometry(160, 640, 120, 30);
	endBtn->setEnabled(false);

	statusLabel = new QLabel(this);
	statusLabel->setGeometry(20, 680, 300, 60);
	updateStatusLabel();

	blackMode = new QComboBox(this);
	blackMode->addItems({ "Manual", "Random", "AI" });
	blackMode->setGeometry(20, 710, 120, 25);

	whiteMode = new QComboBox(this);
	whiteMode->addItems({ "Manual", "Random", "AI" });
	whiteMode->setGeometry(160, 710, 120, 25);

	// 创建定时器
	moveTimer = new QTimer(this);
	moveTimer->setInterval(500);  // 500ms 检查一次
	connect(moveTimer, &QTimer::timeout, this, &GameWindow::checkAndMakeMove);

	// 创建网络管理器
	networkManager = new QNetworkAccessManager(this);
	connect(networkManager, &QNetworkAccessManager::finished, this, &GameWindow::onAiMoveResponse);

	connect(startBtn, &QPushButton::clicked, this, [=]() {
		gameStarted = true;
		updateGameControls();
		board->reset();
		currentPlayer = 1;
		update();
		moveTimer->start();  // 开始定时器
	});

	connect(endBtn, &QPushButton::clicked, this, [=]() {
		gameStarted = false;
		updateGameControls();
		// 等待一小段时间，确保所有待处理的 AI 落子都被处理
		QTimer::singleShot(1000, this, [=]() {
			moveTimer->stop();  // 延迟停止定时器
		});
	});
}

void GameWindow::requestAiMove() {
	// 获取当前棋盘状态
	QJsonArray boardData;
	for (int y = 0; y < 15; ++y) {
		QJsonArray row;
		for (int x = 0; x < 15; ++x) {
			row.append(board->get(x, y));
		}
		boardData.append(row);
	}

	QJsonObject requestData;
	requestData["board"] = boardData;
	requestData["current_player"] = currentPlayer;

	QNetworkRequest request(QUrl("http://localhost:8080/move"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	QJsonDocument doc(requestData);
	networkManager->post(request, doc.toJson());
}

void GameWindow::onAiMoveResponse(QNetworkReply* reply) {
	if (reply->error() != QNetworkReply::NoError) {
		qDebug() << "Error:" << reply->errorString();
		reply->deleteLater();
		return;
	}

	QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
	QJsonObject response = doc.object();

	if (response["success"].toBool()) {
		int x = response["x"].toInt();
		int y = response["y"].toInt();
		if (board->place(x, y, currentPlayer)) {
			checkWin();
			currentPlayer = 3 - currentPlayer;
			update();
		}
	}

	reply->deleteLater();
}

void GameWindow::paintEvent(QPaintEvent*) {
	QPainter painter(this);
	for (int i = 0; i < 15; ++i) {
		painter.drawLine(gridSize / 2, gridSize / 2 + i * gridSize,
			gridSize / 2 + 14 * gridSize, gridSize / 2 + i * gridSize);
		painter.drawLine(gridSize / 2 + i * gridSize, gridSize / 2,
			gridSize / 2 + i * gridSize, gridSize / 2 + 14 * gridSize);
	}

	for (int y = 0; y < 15; ++y) {
		for (int x = 0; x < 15; ++x) {
			int piece = board->get(x, y);
			if (piece == 0) continue;
			QPoint center(gridSize / 2 + x * gridSize, gridSize / 2 + y * gridSize);
			painter.setBrush(piece == 1 ? Qt::black : Qt::white);
			painter.drawEllipse(center, gridSize / 2 - 4, gridSize / 2 - 4);
		}
	}
}

void GameWindow::checkAndMakeMove() {
	if (!gameStarted || board->getWinner() != 0) {
		return;
	}

	int mode = (currentPlayer == 1 ? blackMode->currentIndex() : whiteMode->currentIndex());
	
	// 如果是手动模式，直接返回
	if (mode == 0) {
		return;
	}

	// 如果是 AI 模式，检查对手是否是手动模式
	if (mode == 2) {
		int opponentMode = (currentPlayer == 1 ? whiteMode->currentIndex() : blackMode->currentIndex());
		if (opponentMode == 0) {
			// 对手是手动模式，停止定时器，等待手动落子
			moveTimer->stop();
			return;
		}
	}

	// 执行落子
	if (mode == 1) {  // Random 模式
		std::vector<std::pair<int, int>> empty;
		for (int y = 0; y < 15; ++y)
			for (int x = 0; x < 15; ++x)
				if (board->get(x, y) == 0)
					empty.emplace_back(x, y);

		if (!empty.empty()) {
			auto [x, y] = empty[rand() % empty.size()];
			if (board->place(x, y, currentPlayer)) {
				checkWin();
				currentPlayer = 3 - currentPlayer;
				update();
			}
		}
	}
	else if (mode == 2) {  // AI 模式
		requestAiMove();  // 请求 AI 落子
	}
}

void GameWindow::mousePressEvent(QMouseEvent* event)
{
	// 1. 如果游戏尚未开始，弹窗并返回
	if (!gameStarted) {
		QMessageBox::warning(this, "Warning", "Please start the game first!");
		return;
	}

	// 2. 获取当前玩家模式（假设 0=手动, 1=网络, 2=AI）
	int currentMode = (currentPlayer == 1) ? blackMode->currentIndex() : whiteMode->currentIndex();
	// 若当前是 AI 或网络模式，则用户点击无效，直接返回
	if (currentMode != 0) {
		return;
	}

	// 3. 计算点击坐标在棋盘中的 (x, y) 索引
	int x = event->pos().x() / gridSize;
	int y = event->pos().y() / gridSize;

	// 若坐标不在合法范围内，直接返回
	if (x < 0 || x >= 15 || y < 0 || y >= 15) {
		return;
	}

	// 4. 尝试落子
	if (board->place(x, y, currentPlayer)) {
		// 检查胜负
		checkWin();

		// 切换玩家
		currentPlayer = 3 - currentPlayer;
		update();

		// 5. 如果切换后轮到的玩家是 AI，就启动定时器让 AI 下子
		int opponentMode = (currentPlayer == 1) ? blackMode->currentIndex() : whiteMode->currentIndex();
		if (opponentMode == 2) {
			moveTimer->start();
		}
	}
}

bool GameWindow::makeMove(int x, int y, int color)
{
	// 1. 尝试放到棋盘
	bool success = board->place(x, y, color);
	if (!success)
		return false;

	// 2. 如果成功，做后续流程
	//    - 检查胜负 (若你想先判断是谁下，把 color 传进去做五连判断)
	//    - 切换当前玩家 
	//    - 更新UI 
	//    - 如果下一个玩家是AI，就启动定时器
	checkWin(); // 需要的话

	currentPlayer = 3 - currentPlayer; //1->2,2->1

	// 判断对手模式
	int nextMode = (currentPlayer == 1) ? blackMode->currentIndex() : whiteMode->currentIndex();
	if (nextMode == 2) {
		moveTimer->start();
	}

	update();
	return true;
}

void GameWindow::checkWin() {
	int w = board->getWinner();
	if (w == 1 || w == 2) {
		if (w == 1) ++blackWinCount;
		if (w == 2) ++whiteWinCount;
		updateStatusLabel();
	}
}

void GameWindow::updateStatusLabel() {
	QString text = QString("Black Wins: %1\nWhite Wins: %2")
		.arg(blackWinCount)
		.arg(whiteWinCount);

	if (statusLabel) {
		// 1) 确保是 PlainText（如果之前没有设置过也可以不用写）
		statusLabel->setTextFormat(Qt::PlainText);
		// 2) 若需要自动换行，可开启
		statusLabel->setWordWrap(true);
		// 3) 设置文本
		statusLabel->setText(text);
		// 4) 让它根据内容自动调大小（可选）
		statusLabel->adjustSize();
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
