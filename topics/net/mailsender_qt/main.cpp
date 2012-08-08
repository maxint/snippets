#include <QApplication>

#include <QWidget>
#include <QTcpSocket>
#include "mailsender.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MailSender mailSender;
    mailSender.show();
    mailSender.setHostName("smtp.example.com");
    mailSender.setPort(25);
    mailSender.setUsername("foo");
    mailSender.setPassword("bar");
    mailSender.setSender("sender_name");
    mailSender.setFrom("from@example.com")
    mailSender.setDefaultTo("to@example.com");
    mailSender.setMailContentFile("test.html");
    mailSender.addAttachment("f:\\mac.png");

    return app.exec();
}
