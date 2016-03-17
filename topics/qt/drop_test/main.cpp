#include <QApplication>

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>
#include <QImageReader>

class Widget : public QWidget
{
public:
    Widget(QWidget* parent = 0, Qt::WindowFlags f = 0) : QWidget(parent, f)
    {
        setWindowTitle("Qt Image Drag and Drop Test (by maxint)");
        setupUi();
    }

    void setupUi()
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        QPushButton* btn = new QPushButton("Open image...");
        dropLabel = new QLabel("Drop image file to here!");
        imageView = new QLabel("[Image show here]");

        layout->addWidget(btn);
        layout->addWidget(dropLabel);
        layout->addWidget(imageView);

        dropLabel->setMinimumHeight(60);
        dropLabel->setStyleSheet("border:1px solid rgb(255, 0, 0); ");

        QObject::connect(btn, &QPushButton::clicked, [this](){
            QString fileName = QFileDialog::getOpenFileName(this, "Open Image", "", "Images (*.png *.xpm *.jpg)");
            if (fileName.isEmpty()) return;
            imageView->setPixmap(QPixmap(fileName));
            this->adjustSize();
        });

        dropLabel->setAcceptDrops(true);
        dropLabel->installEventFilter(this);

        setLayout(layout);
    }

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (obj == dropLabel)
        {
            if (event->type() == QEvent::DragEnter)
            {
                QDragEnterEvent* e = static_cast<QDragEnterEvent*>(event);
                qDebug() << "DragEnter: " << e->mimeData()->text();
                if (e->mimeData()->hasFormat("text/uri-list"))
                {
                    QList<QUrl> urls = e->mimeData()->urls();
                    if (!urls.isEmpty())
                    {
                        if (!QImageReader::imageFormat(urls.first().toLocalFile()).isEmpty())
                            e->acceptProposedAction();
                    }
                }
                return true;
            }
            else if (event->type() == QEvent::Drop)
            {
                QDropEvent* e = static_cast<QDropEvent*>(event);
                qDebug() << "Drop: " << e->mimeData()->text();
                QList<QUrl> urls = e->mimeData()->urls();
                if (!urls.isEmpty())
                {
                    QString fileName = urls.first().toLocalFile();
                    imageView->setPixmap(QPixmap(fileName));
                    this->adjustSize();
                }
                return true;
            }
        }
        return QWidget::eventFilter(obj, event);
    }

private:
    QLabel* dropLabel;
    QLabel* imageView;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;
    w.show();

    return a.exec();
}
