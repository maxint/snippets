#include <QApplication>

#include <QWidget>
#include <QTcpSocket>
#include "mailsender.h"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	MailSender mailSender;
	mailSender.show();
	mailSender.setHostName("zjuem.zju.edu.cn");
	mailSender.setPort(25);
	mailSender.setUsername("bWF4aW50", true);
	mailSender.setPassword("Y2hpbmE=", true);
	mailSender.setDefaultTo("lnychina@gmail.com");
	mailSender.setMailContentFile("test.html");
	//mailSender.setMailContentFile("main.cpp");
	//mailSender.addAttachment("facelift.flv");
	mailSender.addAttachment("f:\\mac.png");
	//mailSender.addAttachment("f:\\mac02.png");
	//mailSender.addAttachment("test.html");

	return app.exec();
}