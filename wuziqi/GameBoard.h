#pragma once
#include <QObject>
#include <vector>

class GameBoard : public QObject {
	Q_OBJECT
public:
	explicit GameBoard(QObject* parent = nullptr);
	void reset();
	bool place(int x, int y, int color);
	int get(int x, int y) const;
	int getWinner() const;
	const std::vector<std::vector<int>>& data() const { return board; }

private:
	std::vector<std::vector<int>> board;
	int winner = 0;
	bool checkFive(int x, int y, int color) const;
};
