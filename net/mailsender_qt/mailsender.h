#ifndef MAILSENDER_H
#define MAILSENDER_H

#include <QWidget>
#include <QTcpSocket>

//QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QPushButton;
class QProgressBar;
class QPushButton;
class QTcpSocket;
//QT_END_NAMESPACE

#define BUFFER_SIZE 10240	  // SendData and RecvData buffers sizes
#define DELAY_IN_MS 10			// delay between send and recv functions
#define MSG_SIZE_IN_MB 20		// the maximum size of the message with all attachments

enum CSmtpError
{
	CSMTP_NO_ERROR = 0,
	CSMTP_WSA_STARTUP = 100, // WSAGetLastError()
	CSMTP_WSA_VER,
	CSMTP_WSA_SEND,
	CSMTP_WSA_RECV,
	CSMTP_WSA_CONNECT,
	CSMTP_WSA_GETHOSTBY_NAME_ADDR,
	CSMTP_WSA_INVALID_SOCKET,
	CSMTP_WSA_HOSTNAME,
	CSMTP_BAD_IPV4_ADDR,
	CSMTP_UNDEF_MSG_HEADER = 200,
	CSMTP_UNDEF_MAILFROM,
	CSMTP_UNDEF_SUBJECT,
	CSMTP_UNDEF_RECIPENTS,
	CSMTP_UNDEF_LOGIN,
	CSMTP_UNDEF_PASSWORD,
	CSMTP_UNDEF_RECIPENT_MAIL,
	CSMTP_COMMAND_MAIL_FROM = 300,
	CSMTP_COMMAND_EHLO,
	CSMTP_COMMAND_AUTH_LOGIN,
	CSMTP_COMMAND_DATA,
	CSMTP_COMMAND_QUIT,
	CSMTP_COMMAND_RCPT_TO,
	CSMTP_MSG_BODY_ERROR,
	CSMTP_CONNECTION_CLOSED = 400, // by server
	CSMTP_SERVER_NOT_READY, // remote server
	CSMTP_FILE_NOT_EXIST,
	CSMTP_MSG_TOO_BIG,
	CSMTP_BAD_LOGIN_PASS,
	CSMTP_UNDEF_XYZ_RESPOMSE,
	CSMTP_LACK_OF_MEMORY
};

class MailSender : public QWidget
{
	Q_OBJECT

public:
	MailSender(QWidget *parent = 0);
	~MailSender();

	void setHostName(const QString& hostname);
	void setPort(int port);
	void setSender(const QString& name);
	void setFrom(const QString& from);
	void setDefaultTo(const QString& to);
	void setUsername(const QString& username, bool isBase64 = false);
	void setPassword(const QString& passwd, bool isBase64 = false);

	void addAttachment(const QString& attFileName) { attachmentFiles << attFileName; };
	void setMailContentFile(const QString& contFileName) { mailBodyFile = contFileName; };
	CSmtpError getLastError();
	friend char* GetErrorText(CSmtpError);

public slots:
	void smtpStart();

private:
	void formatMessage();
	void formatMailContent();
	void formatAttachment();
	int smtpXYZdigits();
	bool sendData();
	bool receiveData();

private slots:
	void smtpConnect();
	void smtpDisconnect();
	void quitServer();
	void updateProgress(qint64 numBytes);
	void displayError(QAbstractSocket::SocketError socketError);
	void enableSendButton();
	void cancelSending();

private:
	QLabel *emailLabel;
	QLineEdit *emailLineEdit;
	QProgressBar *progressBar;
	QLabel *stateLabel;
	QPushButton *sendButton;
	QPushButton *cancelButton;

	QTcpSocket tcpClient;
	int totalBytes;
	int bytesToWrite;
	int bytesWritten;
	QString m_hostname;
	int m_port;
	QString m_username;
	QString m_password;
	QString m_from;
	QString m_sender;
	QString m_to;
	QString m_subject;
	QString m_header;
	QString m_body;
	//QString message;

	CSmtpError m_oError;
	bool m_bCon;
	QString mailBodyFile;
	QStringList attachmentFiles;
	QString sendBuf;
	//QByteArray recvBuf;
	char recvBuf[BUFFER_SIZE];
}; 

#endif // MAILSENDER_H
