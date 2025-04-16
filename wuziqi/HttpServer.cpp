#include "HttpServer.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>

HttpServer::HttpServer(GameBoard* b, GameWindow* w, QObject* parent)
	: QTcpServer(parent), board(b), window(w) {
	listen(QHostAddress::Any, 8080);
}

void HttpServer::incomingConnection(qintptr socketDescriptor) {
	QTcpSocket* socket = new QTcpSocket(this);
	socket->setSocketDescriptor(socketDescriptor);
	connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
		handleRequest(socket);
		});
	connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

void HttpServer::handleRequest(QTcpSocket* socket) {
	QByteArray data = socket->readAll();
	QString request(data);
	if (request.isEmpty()) return;

	if (request.startsWith("POST /move")) {
		int bodyStart = request.indexOf("\r\n\r\n");
		if (bodyStart == -1) return;
		QByteArray body = data.mid(bodyStart + 4);
		QJsonDocument doc = QJsonDocument::fromJson(body);
		int x = doc["x"].toInt();
		int y = doc["y"].toInt();
		int color = doc["color"].toInt();

		bool success = false;
		if (window) {
			success = window->makeMove(x, y, color);
		}

		QJsonObject res;
		res["success"] = success;
		res["winner"] = board->getWinner();
		QJsonDocument jsonDoc(res);
		QByteArray response = jsonDoc.toJson();

		QByteArray full;
		full.append("HTTP/1.1 200 OK\r\n");
		full.append("Content-Type: application/json\r\n");
		full.append("Content-Length: " + QByteArray::number(response.size()) + "\r\n\r\n");
		full.append(response);
		socket->write(full);
	}
	else if (request.startsWith("POST /reset")) {
		board->reset();

		QByteArray response = "reset";
		QByteArray full;
		full.append("HTTP/1.1 200 OK\r\n");
		full.append("Content-Type: text/plain\r\n");
		full.append("Content-Length: " + QByteArray::number(response.size()) + "\r\n\r\n");
		full.append(response);
		socket->write(full);
	}
	else if (request.startsWith("GET /get_board")) {
		QJsonArray rows;
		for (const auto& row : board->data()) {
			QJsonArray line;
			for (int v : row) line.append(v);
			rows.append(line);
		}
		QJsonObject res;
		res["board"] = rows;
		res["winner"] = board->getWinner();
		QJsonDocument jsonDoc(res);
		QByteArray response = jsonDoc.toJson();

		QByteArray full;
		full.append("HTTP/1.1 200 OK\r\n");
		full.append("Content-Type: application/json\r\n");
		full.append("Content-Length: " + QByteArray::number(response.size()) + "\r\n\r\n");
		full.append(response);
		socket->write(full);
	}
	else if (request.startsWith("GET /can_move")) {
		QJsonObject res;
		res["can_move"] = board->getWinner() == 0;
		res["current_player"] = window->getCurrentPlayer();  // 👈 你需要新增个 getter 方法
		QJsonDocument doc(res);
		QByteArray body = doc.toJson();

		QByteArray full;
		full.append("HTTP/1.1 200 OK\r\n");
		full.append("Content-Type: application/json\r\n");
		full.append("Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n");
		full.append(body);
		socket->write(full);
	}
	else {
		QByteArray response = "404 Not Found";
		QByteArray full;
		full.append("HTTP/1.1 404 Not Found\r\n");
		full.append("Content-Type: text/plain\r\n");
		full.append("Content-Length: " + QByteArray::number(response.size()) + "\r\n\r\n");
		full.append(response);
		socket->write(full);
	}

	socket->flush();
	socket->disconnectFromHost();
}