#include <QtGui>
#include <QtNetwork>
#include <QRegExpValidator>
#include <QFile>
#include <QDir>
#include <QTest>
#include <QDebug>
#include "base64.h"

#include "mailsender.h"

#pragma warning(push)
#pragma warning(disable:4996)


const char BOUNDARY_TEXT[] = "__MESSAGE__ID__54yg6f6h6y456345";
static const int PayloadSize = 65536;
static const int SMTP_PORT = 25;
static const QString SMTP_HOSTNAME = "smtp.foo.com";
static const QString MAIL_FROM = "from@foo.com";
static const QString MAIL_TO = "to@bar.com";
static const QString USER_NAME = "sender_username";
static const QString PASSWORD = "sender_password";

const char HEADER[] =
"EHLO %1\r\n"
"AUTH LOGIN\r\n%2\r\n%3\r\n" //+ BASE64 USER + BASE64 PASS
"MAIL FROM:%6<%4>\r\n"
"RCPT TO:<%5>\r\n"
"DATA\r\n";

const char BODY_HEADER[] =
"FROM:%5<%1>\r\n"
"TO:<%2>\r\n"
"SUBJECT: %3\r\n"
"DATE: %4\r\n"
//"X-Mailer: shadowstar's mailer\r\n"
"MIME-Version: 1.0\r\n"
"Content-type: multipart/mixed; boundary=\"#BOUNDARY#\"\r\n"
//"Content-type: multipart/alternative;\r\n"
"This is a multi-part message in MIME format.\r\n"
"\r\n--#BOUNDARY#\r\n"
"Content-Type: text/html; charset=gb2312\r\n"
"Content-Transfer-Encoding: 8bit\r\n"
//"Content-Transfer-Encoding: quoted-printable\r\n"
"\r\n";

const char ATT_HEADER[] =
"\r\n--#BOUNDARY#\r\n"
//"Content-Type: application/octet-stream;"
"Content-Type: application/x-msdownload;"
"name=%1\r\n"
"Content-Disposition: attachment; filename=%1\r\n"
"Content-Transfer-Encoding: base64\r\n"
"\r\n";

//---------------------------------------------------------------------------
static int ANSIToBase64(const char *szInANSI, int nInLen, char *szOutBase64, int nOutLen);

MailSender::MailSender(QWidget *parent)
    : QWidget(parent)
{
    // Initialize variables
    m_hostname = SMTP_HOSTNAME;
    m_port = SMTP_PORT;
    setUsername(USER_NAME);
    setPassword(PASSWORD);
    m_from = MAIL_FROM;
    m_sender = "MAXINT";
    m_to = MAIL_TO;
    m_subject = "你好";
    mailBodyFile = "";

    emailLabel = new QLabel(tr("Your Email:"));
    emailLineEdit = new QLineEdit(MAIL_TO);
    emailLineEdit->setValidator(new QRegExpValidator(
        QRegExp("(?:[0-9a-z-.]*){1}@(?:[0-9a-z-]*\\.){1,2}[a-z]{2,3}"), this));

    stateLabel = new QLabel(tr("Ready"));
    progressBar = new QProgressBar;
    sendButton = new QPushButton(tr("&Send"));
    sendButton->setDefault(true);
    //sendButton->setEnabled(false);

    cancelButton = new QPushButton(tr("&Cancel"));
    cancelButton->setEnabled(false);

    connect(emailLineEdit, SIGNAL(textChanged(const QString)), this, SLOT(enableSendButton()));
    connect(sendButton, SIGNAL(clicked()), this, SLOT(smtpStart()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelSending()));
    connect(&tcpClient, SIGNAL(connected()), this, SLOT(smtpConnect()));
    connect(&tcpClient, SIGNAL(disconnected()), this, SLOT(smtpDisconnect()));
    connect(&tcpClient, SIGNAL(bytesWritten(qint64)), this, SLOT(updateProgress(qint64)));
    connect(&tcpClient, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(displayError(QAbstractSocket::SocketError)));
    //connect(&tcpClient, SIGNAL(disconnected()), progressBar, SLOT(reset()));
    //connect(cancelButton, SIGNAL(clicked()), progressBar, SLOT(reset()));

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addWidget(sendButton);
    hLayout->addWidget(cancelButton);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(emailLabel, 0, 0);
    mainLayout->addWidget(emailLineEdit, 0, 1);
    mainLayout->addWidget(progressBar, 1, 0, 1, 2);
    mainLayout->addWidget(stateLabel, 2, 0, 1, 2);
    mainLayout->addLayout(hLayout, 3, 0, 1, 2);

    this->setLayout(mainLayout);
}

MailSender::~MailSender()
{

}

void MailSender::setHostName(const QString& hostname)
{
    m_hostname = hostname;
}

void MailSender::setPort(int port)
{
    m_port = port;
}

void MailSender::setUsername(const QString& username, bool isBase64 /* = false */)
{
    if (isBase64)
    {
        m_username = username;
    }
    else
    {
        m_username = QString(base64_encode(reinterpret_cast<const unsigned char*>(username.toStdString().c_str()),username.size()).c_str());
    }
}

void MailSender::setPassword(const QString& passwd, bool isBase64 /* = false */)
{
    if (isBase64)
    {
        m_password = passwd;
    }
    else
    {
        m_password = QString(base64_encode(reinterpret_cast<const unsigned char*>(passwd.toStdString().c_str()),passwd.size()).c_str());
    }
}

void MailSender::setFrom(const QString& from)
{
    m_from = from;
}

void MailSender::setDefaultTo(const QString& to)
{
    m_to = to;
    emailLineEdit->setText(m_to);
}

void MailSender::setSender(const QString& name)
{
    m_sender = name;
}

void MailSender::smtpStart()
{
    m_bCon = false;
    m_oError = CSMTP_NO_ERROR;
    disconnect(&tcpClient, SIGNAL(readyRead()), this, SLOT(quitServer()));

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif

    m_to = emailLineEdit->text();
    formatMessage();

    tcpClient.connectToHost(m_hostname, m_port);
    stateLabel->setText(tr("Connecting SMTP server...."));
    progressBar->reset();
    sendButton->setEnabled(false);
    cancelButton->setEnabled(true);

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

}

void MailSender::smtpConnect()
{
    stateLabel->setText(tr("Connecting SMTP server successfully!"));
    m_bCon = true;

    QTest::qWait(DELAY_IN_MS);
    if (!receiveData())
    {
        return;
    }
    switch(smtpXYZdigits())
    {
    case 220:
        break;
    default:
        m_oError = CSMTP_SERVER_NOT_READY;
        return;
        break;
    }

    // EHLO <SP> <domain> <CRLF>
    sendBuf = QString("HELO %1\r\n").arg(QHostInfo::localHostName());
    if (!sendData())
    {
        return;
    }
    QTest::qWait(DELAY_IN_MS);
    if (!receiveData())
    {
        return;
    }
    switch(smtpXYZdigits())
    {
    case 250:
        break;
    default:
        m_oError = CSMTP_COMMAND_EHLO;
        return;
        break;
    }

    // AUTH <SP> LOGIN <CRLF>
    sendBuf = QString("AUTH LOGIN\r\n");
    if (!sendData())
    {
        return;
    }
    QTest::qWait(DELAY_IN_MS);
    if (!receiveData())
    {
        return;
    }
    switch(smtpXYZdigits())
    {
    case 334:
        break;
    default:
        m_oError = CSMTP_COMMAND_AUTH_LOGIN;
        return;
        break;
    }

    // send login:
    sendBuf = QString("%1\r\n").arg(m_username);
    if (!sendData())
    {
        return;
    }
    QTest::qWait(DELAY_IN_MS);
    if (!receiveData())
    {
        return;
    }
    switch(smtpXYZdigits())
    {
    case 334:
        break;
    default:
        m_oError = CSMTP_UNDEF_XYZ_RESPOMSE;
        return;
        break;
    }

    // send password
    sendBuf = QString("%1\r\n").arg(m_password);
    if (!sendData())
    {
        return;
    }
    QTest::qWait(DELAY_IN_MS);
    if (!receiveData())
    {
        return;
    }
    switch(smtpXYZdigits())
    {
    case 235:
        break;
    case 535:
        m_oError = CSMTP_BAD_LOGIN_PASS;
        return;
    default:
        m_oError = CSMTP_UNDEF_XYZ_RESPOMSE;
        return;
    }

    // MAIL <SP> FROM:<reverse-path> <CRLF>
    sendBuf = QString("MAIL FROM:<%1>\r\n").arg(m_from);
    if (!sendData())
    {
        return;
    }
    QTest::qWait(DELAY_IN_MS);
    if (!receiveData())
    {
        return;
    }
    switch(smtpXYZdigits())
    {
    case 250:
        break;
    default:
        m_oError = CSMTP_COMMAND_MAIL_FROM;
        return;
        break;
    }

    // RCPT <SP> TO:<forward-path> <CRLF>
    sendBuf = QString("RCPT TO:<%1>\r\n").arg(m_to);
    if(!sendData())
        return;
    QTest::qWait(DELAY_IN_MS);
    if(!receiveData())
        return;

    switch(smtpXYZdigits())
    {
    case 250:
        break;
    default:
        m_oError = CSMTP_COMMAND_RCPT_TO;
        return;
    }

    // DATA <CRLF>
    sendBuf = QString("DATA\r\n");
    if(!sendData())
        return;
    QTest::qWait(DELAY_IN_MS);
    if(!receiveData())
        return;

    switch(smtpXYZdigits())
    {
    case 354:
        break;
    default:
        m_oError = CSMTP_COMMAND_DATA;
        return;
    }

    // send data
    sendBuf = m_body;

    // <CRLF> . <CRLF>
    sendBuf += QString("\r\n.\r\n");
    if(!sendData())
        return;

    connect(&tcpClient, SIGNAL(readyRead()), this, SLOT(quitServer()));
}

void MailSender::quitServer()
{
    if(!receiveData())
        return;

    switch(smtpXYZdigits())
    {
    case 250:
        break;
    default:
        m_oError = CSMTP_MSG_BODY_ERROR;
        return;
    }

    // QUIT <CRLF>
    sendBuf = QString("QUIT\r\n");
    if(!sendData())
        return;
    QTest::qWait(DELAY_IN_MS);
    if(!receiveData())
        return;

    switch(smtpXYZdigits())
    {
    case 221:
        break;
    default:
        m_oError = CSMTP_COMMAND_QUIT;
        return;
    }

    m_bCon = false;
}

void MailSender::updateProgress(qint64 numBytes)
{
    bytesWritten += (int)numBytes;
    //if (bytesToWrite > 0)
    //  bytesToWrite -= (int)tcpClient.write(QByteArray(qMin(bytesToWrite, PayloadSize), '@'));

    progressBar->setMaximum(totalBytes<bytesWritten ? bytesWritten : totalBytes);
    progressBar->setValue(bytesWritten);
    QString status = QString(tr("Transmitting...\t%1KB / %2KB")).arg(bytesWritten / 1024).arg(totalBytes / 1024);
    stateLabel->setText(status);
}

void MailSender::displayError(QAbstractSocket::SocketError socketError)
{
    if (socketError == QTcpSocket::RemoteHostClosedError)
        return;

    QMessageBox::information(this, tr("Network error"),
        tr("The following error occurred: %1.")
        .arg(tcpClient.errorString()));

    tcpClient.close();
    progressBar->reset();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
}

void MailSender::formatMessage()
{
    m_header = QString(HEADER)
        .arg(QHostInfo::localHostName())
        .arg(USER_NAME)
        .arg(PASSWORD)
        .arg(m_from)
        .arg(m_to)
        .arg(m_sender);

    m_body = QString(BODY_HEADER)
        .arg(m_from)
        .arg(m_to)
        .arg(m_subject)
        .arg(QDateTime::currentDateTime().toString("yyyy-M-d hh:mm:ss"))
        .arg(m_sender);

    formatMailContent();
    formatAttachment();
    //message = m_header + m_body;

    bytesWritten = 0;
    //bytesToWrite = totalBytes - (int)tcpClient.write(QByteArray(PayloadSize, '@'));
    totalBytes = (m_header + m_body).size();
    progressBar->setMaximum(totalBytes);
}

void MailSender::formatMailContent()
{
    QFile file(mailBodyFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "mail content is empty!";
        return;
    }
    QString content = file.readAll();
    m_body += content;
    file.close();
}

void MailSender::formatAttachment()
{
    if (!attachmentFiles.isEmpty())
    {
        int     x;
        FILE    *fp;
        char    *aatt = new char[0x400000];
        if (aatt == NULL)
        {
            return;
        }
        char    *batt = new char[0x555556];
        if (batt == NULL)
        {
            return;
        }
        for (int i=0; i<attachmentFiles.size(); ++i)
        {
            QFileInfo fileinfo(attachmentFiles[i]);
            QString filename = fileinfo.absoluteFilePath();
            fp = fopen(filename.toAscii(), "rb");
            if (!fp)
            {
                continue;
            }
            fseek(fp, 0, SEEK_END);
            x = ftell(fp);
            if (x > 0x400000)
                x = 0;
            rewind(fp);
            fread(aatt, x, 1, fp);
            fclose(fp);
            x = ANSIToBase64(aatt, x, batt, 0x555556);
            m_body += QString(ATT_HEADER).arg(fileinfo.fileName());
            m_body += QString::fromAscii(batt, strlen(batt));
        }
        delete []aatt;
        delete []batt;
        m_body += QString("\r\n--#BOUNDARY#\r\n");
    }
}
void MailSender::smtpDisconnect()
{
    tcpClient.disconnectFromHost();
    //if (m_bCon)
    //{
    //  stateLabel->setText(tr("Mail sending error! Please resend the mail!"));
    //}
    if (m_oError != CSMTP_NO_ERROR)
    {
        stateLabel->setText(tr("[ERROR]:") + GetErrorText(m_oError));
    }
    else
    {
        stateLabel->setText(tr("Mail sending finished!"));
    }
    //progressBar->reset();
    cancelButton->setEnabled(false);
    sendButton->setEnabled(true);
    m_body.clear();
    m_header.clear();
}

void MailSender::enableSendButton()
{
    sendButton->setEnabled(QRegExp("(?:[0-9a-z-.]{1,}){1}@(?:[0-9a-z-]{1,}\\.){1,2}[a-z]{2,3}").exactMatch(emailLineEdit->text()));
}

void MailSender::cancelSending()
{
    tcpClient.abort();
    cancelButton->setEnabled(false);
    stateLabel->setText(tr("Mail sending canceled!" ));
}

int MailSender::smtpXYZdigits()
{
    if(recvBuf == NULL)
    //if (recvBuf.isEmpty())
        return 0;
    return (recvBuf[0]-'0')*100 + (recvBuf[1]-'0')*10 + recvBuf[2]-'0';
}

bool MailSender::sendData()
{
    if (!sendBuf.isEmpty())
    {
        qint64 res = tcpClient.write(sendBuf.toAscii(), sendBuf.size());
        if (res == 0)
        {
            return false;
        }
        return true;
    }
    return false;
}

bool MailSender::receiveData()
{
    if (!tcpClient.bytesAvailable())
    {
        if (!tcpClient.waitForReadyRead())
        {
            return false;
        }
        //qDebug() << tcpClient.bytesAvailable();
    }
    int res;

    if (recvBuf == NULL)
    {
        return false;
    }
    res = tcpClient.read(recvBuf, BUFFER_SIZE);
    if (res == 0)
    {
        return false;
    }
    recvBuf[res] = '\0';

    qDebug() << recvBuf;

    //recvBuf = tcpClient.readAll();
    //if (recvBuf.isEmpty())
    //{
    //  return false;
    //}

    return true;
}

CSmtpError MailSender::getLastError()
{
    return m_oError;
}

//////////////////////////////////////////////////////////////////////
// Friends
//////////////////////////////////////////////////////////////////////

char* GetErrorText(CSmtpError ErrorId)
{
    switch(ErrorId)
    {
    case CSMTP_NO_ERROR:
        return "";
    case CSMTP_WSA_STARTUP:
        return "Unable m_to initialise winsock2.";
    case CSMTP_WSA_VER:
        return "Wrong version of the winsock2.";
    case CSMTP_WSA_SEND:
        return "Function send() failed.";
    case CSMTP_WSA_RECV:
        return "Function recv() failed.";
    case CSMTP_WSA_CONNECT:
        return "Function connect failed.";
    case CSMTP_WSA_GETHOSTBY_NAME_ADDR:
        return "Functions gethostbyname() or gethostbyaddr() failed.";
    case CSMTP_WSA_INVALID_SOCKET:
        return "Invalid winsock2 socket.";
    case CSMTP_WSA_HOSTNAME:
        return "Function hostname() failed.";
    case CSMTP_BAD_IPV4_ADDR:
        return "Improper IPv4 address.";
    case CSMTP_UNDEF_MSG_HEADER:
        return "Undefined message m_header.";
    case CSMTP_UNDEF_MAILFROM:
        return "Undefined m_from is the mail.";
    case CSMTP_UNDEF_SUBJECT:
        return "Undefined message m_subject.";
    case CSMTP_UNDEF_RECIPENTS:
        return "Undefined at least one reciepent.";
    case CSMTP_UNDEF_RECIPENT_MAIL:
        return "Undefined recipent mail.";
    case CSMTP_UNDEF_LOGIN:
        return "Undefined user login.";
    case CSMTP_UNDEF_PASSWORD:
        return "Undefined user password.";
    case CSMTP_COMMAND_MAIL_FROM:
        return "Server returned error after sending MAIL FROM.";
    case CSMTP_COMMAND_EHLO:
        return "Server returned error after sending EHLO.";
    case CSMTP_COMMAND_AUTH_LOGIN:
        return "Server returned error after sending AUTH LOGIN.";
    case CSMTP_COMMAND_DATA:
        return "Server returned error after sending DATA.";
    case CSMTP_COMMAND_QUIT:
        return "Server returned error after sending QUIT.";
    case CSMTP_COMMAND_RCPT_TO:
        return "Server returned error after sending RCPT TO.";
    case CSMTP_MSG_BODY_ERROR:
        return "Error in message m_body";
    case CSMTP_CONNECTION_CLOSED:
        return "Server has closed the connection.";
    case CSMTP_SERVER_NOT_READY:
        return "Server is not ready.";
    case CSMTP_FILE_NOT_EXIST:
        return "File not exist.";
    case CSMTP_MSG_TOO_BIG:
        return "Message is too big.";
    case CSMTP_BAD_LOGIN_PASS:
        return "Bad login or password.";
    case CSMTP_UNDEF_XYZ_RESPOMSE:
        return "Undefined xyz SMTP response.";
    case CSMTP_LACK_OF_MEMORY:
        return "Lack of memory.";
    default:
        return "Undefined error id.";
    }
}

//---------------------------------------------------------------------------
const int  BASE64_MAXLINE = 76;
const char EOL[] = "\r\n";
const char BASE64_TAB[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz0123456789+/";

int ANSIToBase64(const char *szInANSI, int nInLen, char *szOutBase64, int nOutLen)
{
    //Input Parameter validation
    if ((szInANSI == NULL) || (nInLen == 0) || (szOutBase64 == NULL) || (nOutLen == 0))
        return 0;
    if (nOutLen < (nInLen*4/3 + 1 + nInLen*4/3/BASE64_MAXLINE*2 + 1 + 4))
        return 0;

    //Set up the parameters prior m_to the main encoding loop
    int nInPos  = 0;
    int nOutPos = 0;
    int nLineLen = 0;
    int c1, c2, c3;
    int i;

    // Get three characters at a time m_from the input buffer and encode them
    for (i=0; i<nInLen/3; ++i)
    {
        //Get the next 2 characters
        c1 = szInANSI[nInPos++] & 0xFF;
        c2 = szInANSI[nInPos++] & 0xFF;
        c3 = szInANSI[nInPos++] & 0xFF;

        //Encode into the 4 6 bit characters
        szOutBase64[nOutPos++] = BASE64_TAB[c1 >> 2];
        szOutBase64[nOutPos++] = BASE64_TAB[((c1 << 4) | (c2 >> 4)) & 0x3F];
        szOutBase64[nOutPos++] = BASE64_TAB[((c2 << 2) | (c3 >> 6)) & 0x3F];
        szOutBase64[nOutPos++] = BASE64_TAB[c3 & 0x3F];
        nLineLen += 4;

        //Handle the case where we have gone over the max line boundary
        if (nLineLen > BASE64_MAXLINE - 4)
        {
            szOutBase64[nOutPos++] = EOL[0];
            szOutBase64[nOutPos++] = EOL[1];
            nLineLen = 0;
        }
    }

    // Encode the remaining one or two characters in the input buffer
    switch (nInLen % 3)
    {
    case 0:
        {
            szOutBase64[nOutPos++] = EOL[0];
            szOutBase64[nOutPos++] = EOL[1];
            break;
        }
    case 1:
        {
            c1 = szInANSI[nInPos] & 0xFF;
            szOutBase64[nOutPos++] = BASE64_TAB[(c1 & 0xFC) >> 2];
            szOutBase64[nOutPos++] = BASE64_TAB[((c1 & 0x03) << 4)];
            szOutBase64[nOutPos++] = '=';
            szOutBase64[nOutPos++] = '=';
            szOutBase64[nOutPos++] = EOL[0];
            szOutBase64[nOutPos++] = EOL[1];
            break;
        }
    case 2:
        {
            c1 = szInANSI[nInPos++] & 0xFF;
            c2 = szInANSI[nInPos] & 0xFF;
            szOutBase64[nOutPos++] = BASE64_TAB[(c1 & 0xFC) >> 2];
            szOutBase64[nOutPos++] = BASE64_TAB[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
            szOutBase64[nOutPos++] = BASE64_TAB[((c2 & 0x0F) << 2)];
            szOutBase64[nOutPos++] = '=';
            szOutBase64[nOutPos++] = EOL[0];
            szOutBase64[nOutPos++] = EOL[1];
            break;
        }
    default:
        {
            return 0;
        }
    }

    szOutBase64[nOutPos] = 0;

    return nOutPos;
}

#pragma warning(pop)
