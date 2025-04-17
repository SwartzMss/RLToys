#include "GameBoard.h"

GameBoard::GameBoard(QObject* parent) : QObject(parent), board(BOARD_SIZE, std::vector<int>(BOARD_SIZE, 0)) {}

void GameBoard::reset() {
	for (auto& row : board) row.assign(BOARD_SIZE, 0);
	winner = 0;
}

bool GameBoard::place(int x, int y, int color) {
	if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE || board[y][x] != 0 || winner != 0)
		return false;
	board[y][x] = color;
	if (checkFive(x, y, color)) {
		winner = color;
	} else if (isFull()) {
		winner = 3;  // 3表示平局
	}
	return true;
}

int GameBoard::get(int x, int y) const {
	return board[y][x];
}

int GameBoard::getWinner() const {
	return winner;
}

bool GameBoard::isFull() const {
	for (const auto& row : board) {
		for (int cell : row) {
			if (cell == 0) return false;
		}
	}
	return true;
}

bool GameBoard::checkFive(int x, int y, int color) const {
	const int dx[] = { 1, 0, 1, 1 };
	const int dy[] = { 0, 1, 1, -1 };
	for (int d = 0; d < 4; ++d) {
		int count = 1;
		for (int i = 1; i < 5; ++i) {
			int nx = x + i * dx[d], ny = y + i * dy[d];
			if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[ny][nx] == color)
				count++;
			else break;
		}
		for (int i = 1; i < 5; ++i) {
			int nx = x - i * dx[d], ny = y - i * dy[d];
			if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[ny][nx] == color)
				count++;
			else break;
		}
		if (count >= 5) return true;
	}
	return false;
}
