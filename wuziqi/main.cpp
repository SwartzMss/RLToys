#include <QApplication>
#include "GameWindow.h"
#include "HttpServer.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

	GameBoard board;
	GameWindow window(&board);
	window.show();

	HttpServer server(&board, &window);

	return app.exec();
}
