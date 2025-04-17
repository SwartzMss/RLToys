#pragma once
#include <QObject>
#include <vector>

// 定义全局棋盘大小常量
constexpr int BOARD_SIZE = 8;

class GameBoard : public QObject {
	Q_OBJECT
public:
	explicit GameBoard(QObject* parent = nullptr);
	void reset();
	bool place(int x, int y, int color);
	int get(int x, int y) const;
	int getWinner() const;
	const std::vector<std::vector<int>>& data() const { return board; }
	bool isFull() const;

private:
	std::vector<std::vector<int>> board;
	int winner = 0;
	bool checkFive(int x, int y, int color) const;
};
