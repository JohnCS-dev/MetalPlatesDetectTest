#include "testmetaldetectwindow.h"
#include "ui_testmetaldetectwindow.h"
#include "opencvtoolwidgets.h"
#include <QFileDialog>


#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

TestMetalDetectWindow::TestMetalDetectWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TestMetalDetectWindow),
    m_settings("config.ini", QSettings::IniFormat)
{
    ui->setupUi(this);

    lbView = new ScaledPixmap;
    int index = ui->verticalLayout->indexOf(ui->pbProcess);
    ui->verticalLayout->insertWidget(index, lbView);

    ui->toolBox->removeItem(0);

    fProcessList.append(new OpencvSeparateChannelsToolWidget());
    fResultViewList.append(cv::Mat());
    ui->toolBox->addItem(fProcessList.last(), "Separate Channels");
    ui->cbResultView->addItem("Separate channels");

    fProcessList.append(new OpencvBackgroundSubtractorToolWidget());
    fResultViewList.append(cv::Mat());
    ui->toolBox->addItem(fProcessList.last(), "Background subtractor");
    ui->cbResultView->addItem("Background subtractor");

    connect(ui->pbLoadOriginal, SIGNAL(clicked(bool)), this, SLOT(loadOriginal()));
    connect(ui->cbResultView, SIGNAL(currentIndexChanged(int)), this, SLOT(resultViewIndexChanged(int)));
    connect(ui->pbProcess, SIGNAL(clicked(bool)), this, SLOT(process()));

    loadSettings();
}

TestMetalDetectWindow::~TestMetalDetectWindow()
{
    saveSettings();

    delete ui;
}

void TestMetalDetectWindow::loadSettings()
{
    m_settings.beginGroup("TestMetalDetectWindow");
    ui->splitter->restoreState(m_settings.value("MainSplitter").toByteArray());
    fOriginalImagePath = m_settings.value("OriginalImagePath").toString();
    m_settings.endGroup();

    foreach (auto tool, fProcessList) {
        tool->loadSettings(&m_settings);
    }

    if (!fOriginalImagePath.isEmpty())
        loadOriginal(fOriginalImagePath);
}

void TestMetalDetectWindow::saveSettings()
{
    m_settings.beginGroup("TestMetalDetectWindow");
    m_settings.setValue("MainSplitter", ui->splitter->saveState());
    m_settings.setValue("OriginalImagePath", fOriginalImagePath);
    m_settings.endGroup();

    foreach (auto tool, fProcessList) {
        tool->saveSettings(&m_settings);
    }
}

void TestMetalDetectWindow::resizeEvent(QResizeEvent*)
{
    resultViewIndexChanged(ui->cbResultView->currentIndex());
}

void TestMetalDetectWindow::loadOriginal(QString path)
{
    fOriginalImage = cv::imread( path.toStdString(), cv::IMREAD_UNCHANGED );
    QImage resultImg = QImage( fOriginalImage.data, fOriginalImage.cols, fOriginalImage.rows, fOriginalImage.step, QImage::Format_RGB888 ).copy();
    lbView->setScaledPixmap(
        QPixmap::fromImage( resultImg ).scaled(
            lbView->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        )
    );
}

void TestMetalDetectWindow::loadOriginal()
{
    QString originalFileName = QFileDialog::getOpenFileName( nullptr, "Original image", ".", "Images (*.png *.jpg *.jpeg)" );
    if( !originalFileName.isEmpty() ) {
        fOriginalImagePath = originalFileName;
        loadOriginal(fOriginalImagePath);
    }
}

void TestMetalDetectWindow::resultViewIndexChanged(int index)
{
    QImage resultImg;
    if (!index) {
        resultImg = QImage( fOriginalImage.data, fOriginalImage.cols, fOriginalImage.rows, fOriginalImage.step, QImage::Format_RGB888 ).copy();
    }
    else {
        cv::Mat src = fResultViewList[index-1];
        resultImg = QImage( src.data, src.cols, src.rows, src.step, QImage::Format_Grayscale8 ).copy();
    }

    lbView->setScaledPixmap(
        QPixmap::fromImage( resultImg ).scaled(
            lbView->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        )
    );
}

void TestMetalDetectWindow::process()
{
    for (int i = 0; i < fProcessList.count(); i++) {
        auto processor = fProcessList[i];
        if (!processor->toolIsEnabled()) {
            fResultViewList[i] = i > 0 ? fResultViewList[i-1] : fOriginalImage;
            continue;
        }
        cv::Mat dst;
        if (!i)
            processor->process(&fOriginalImage, &dst);
        else {
            cv::Mat src = fResultViewList[i-1];
            processor->process(&src, &dst);
        }
        fResultViewList[i] = dst;
//        cv::imshow( "Background", dst );
//        cv::imshow( "Background1", fResultViewList[i] );
    }
    resultViewIndexChanged(ui->cbResultView->currentIndex());

//    cv::Mat img, img2;
//    img = fOriginalImage;
//    cv::cvtColor(fOriginalImage, img2, cv::COLOR_RGB2Lab/*COLOR_BGR2GRAY*/);

//    // Extract the L channel
//        std::vector<cv::Mat> lab_planes(3);
//        cv::split(img2, lab_planes);  // now we have the L image in lab_planes[0]

////    cv::Mat img3, img4;
//    cv::Ptr<cv::CLAHE> clane = cv::createCLAHE();
//    clane->setClipLimit(4);
//    cv::Mat dst;
//    clane->apply(lab_planes[0], dst);

//    // Merge the the color planes back into an Lab image
//    dst.copyTo(lab_planes[0]);
//    cv::merge(lab_planes, img2);

//    // convert back to RGB
//       cv::Mat image_clahe;
//       cv::cvtColor(img2, image_clahe, cv::COLOR_Lab2RGB);
//       cv::imshow( "CLAHE", img2 );

////    cv::cvtColor(img3, img4, cv::COLOR_Lab2RGB/*COLOR_GRAY2BGR*/);
//    cv::imshow( "CLAHE 2", image_clahe );
}
