#ifndef TESTMETALDETECTWINDOW_H
#define TESTMETALDETECTWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QPainter>
#include <opencv2/core.hpp>

class OpencvBaseToolWidget;
class ScaledPixmap;

namespace Ui {
class TestMetalDetectWindow;
}

class TestMetalDetectWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TestMetalDetectWindow(QWidget *parent = nullptr);
    ~TestMetalDetectWindow();
    void loadSettings();
    void saveSettings();

protected:
    void resizeEvent(QResizeEvent*) override;

private:
    Ui::TestMetalDetectWindow *ui;
    ScaledPixmap *lbView;
    QSettings m_settings;
    cv::Mat fOriginalImage;
    QString fOriginalImagePath;

    QList<OpencvBaseToolWidget*> fProcessList;
    QList<cv::Mat> fResultViewList;

    void loadOriginal(QString path);

private slots:
    void loadOriginal();
    void resultViewIndexChanged(int);
    void process();
};

class ScaledPixmap : public QWidget {
public:
    ScaledPixmap(QWidget *parent = 0) : QWidget(parent) {}
    void setScaledPixmap(const QPixmap &pixmap) {
        m_pixmap = pixmap;
        update();
    }
    QSize ScaledPixmap::sizeHint() const override {
        return m_pixmap.size();
    }
protected:
    void paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        if (false == m_pixmap.isNull()) {
            QSize widgetSize = size();
            QPixmap scaledPixmap = m_pixmap.scaled(widgetSize, Qt::KeepAspectRatio);
            QPoint center((widgetSize.width() - scaledPixmap.width())/2,
                          (widgetSize.height() - scaledPixmap.height())/2);
            painter.drawPixmap(center, scaledPixmap);
        }
        QWidget::paintEvent(event);
    }

private:
    QPixmap m_pixmap;
};

#endif // TESTMETALDETECTWINDOW_H
