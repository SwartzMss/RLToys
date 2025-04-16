#pragma once
#include <QTcpServer>
#include <QTcpSocket>
#include "GameBoard.h"
#include "GameWindow.h"

class HttpServer : public QTcpServer {
	Q_OBJECT
public:
	HttpServer(GameBoard* board, GameWindow* window, QObject* parent = nullptr);
protected:
	void incomingConnection(qintptr socketDescriptor) override;

private:
	GameBoard* board;
	GameWindow* window;
	void handleRequest(QTcpSocket* socket);
};