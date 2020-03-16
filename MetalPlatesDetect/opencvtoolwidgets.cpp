#include "opencvtoolwidgets.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QFileDialog>
#include <QSettings>
#include <QRadioButton>

// opencv includes
#include <opencv2/bgsegm.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

OpencvBaseToolWidget::OpencvBaseToolWidget(QWidget *parent) : QWidget(parent)
{
    fEnabled = true;
    QVBoxLayout *vl = new QVBoxLayout;
    QHBoxLayout *hl = new QHBoxLayout;
    QCheckBox *cb = new QCheckBox("On/Off");
    cb->setChecked(true);
    hl->addWidget(cb);
    fMainLayout = new QVBoxLayout;
    QWidget *w = new QWidget;
    w->setLayout(fMainLayout);
    vl->addLayout(hl);
    vl->addWidget(w);
    setLayout(vl);

    connect(cb, SIGNAL(toggled(bool)), this, SLOT(enableChanged(bool)));
}

void OpencvBaseToolWidget::enableChanged(bool v)
{
    fEnabled = v;
    foreach (QObject *obj, fMainLayout->parentWidget()->children()) {
        if (obj->isWidgetType())
            ((QWidget*)obj)->setEnabled(v);
    }
}

OpencvBackgroundSubtractorToolWidget::OpencvBackgroundSubtractorToolWidget(QWidget *parent) : OpencvBaseToolWidget(parent)
{
    setObjectName("BackgroundSubtractorToolWidget");

    QVBoxLayout* mainLayout = (QVBoxLayout*)layout();
    algoComboBox = new QComboBox();
    algoComboBox->addItem("MOG2");
    algoComboBox->addItem("KNN");
    algoComboBox->addItem("CNT");
    algoComboBox->addItem("GMG");
    algoComboBox->addItem("GSOC");
    algoComboBox->addItem("LSBP");
    algoComboBox->addItem("MOG");

    fAlgo = MOG2;

    QHBoxLayout *hl = new QHBoxLayout;
    hl->addWidget(new QLabel("Algorithm:"));
    hl->addWidget(algoComboBox);
    hl->addStretch();
    mainLayout->addLayout(hl);

    hl = new QHBoxLayout;
    fHistory = new QSpinBox;
    fHistory->setMinimum(-1);
    fHistory->setMaximum(1000);
    fHistory->setValue(500);
    fHistory->setPrefix("History: ");
    hl->addWidget(fHistory);
    hl->addStretch();
    mainLayout->addLayout(hl);

    hl = new QHBoxLayout;
    fThreshold = new QDoubleSpinBox;
    fThreshold->setMinimum(0);
    fThreshold->setMaximum(1000);
    fThreshold->setValue(16);
    fThreshold->setPrefix("Threshold: ");
    hl->addWidget(fThreshold);
    hl->addStretch();
    mainLayout->addLayout(hl);

    hl = new QHBoxLayout;
    fDetectShadows = new QCheckBox("Detect shadows");
    fDetectShadows->setChecked(true);
    hl->addWidget(fDetectShadows);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    int minPixelStability = 15,
    hl = new QHBoxLayout;
    fMinPixelStability = new QSpinBox;
    fMinPixelStability->setMinimum(0);
    fMinPixelStability->setMaximum(10000);
    fMinPixelStability->setValue(15);
    fMinPixelStability->setPrefix("MinPixelStability: ");
    hl->addWidget(fMinPixelStability);
    hl->addStretch();
    mainLayout->addLayout(hl);

    //    int maxPixelStability = 15*60,
    hl = new QHBoxLayout;
    fMaxPixelStability = new QSpinBox;
    fMaxPixelStability->setMinimum(0);
    fMaxPixelStability->setMaximum(10000);
    fMaxPixelStability->setValue(15*60);
    fMaxPixelStability->setPrefix("MaxPixelStability: ");
    hl->addWidget(fMaxPixelStability);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    bool useHistory = true,
    hl = new QHBoxLayout;
    fUseHistory = new QCheckBox("use history");
    fUseHistory->setChecked(true);
    hl->addWidget(fUseHistory);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    bool isParallel = true
    hl = new QHBoxLayout;
    fIsParallel = new QCheckBox("is parallel");
    fIsParallel->setChecked(true);
    hl->addWidget(fIsParallel);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    int initializationFrames=120,
    hl = new QHBoxLayout;
    fInitializationFrames = new QSpinBox;
    fInitializationFrames->setMinimum(0);
    fInitializationFrames->setMaximum(1000);
    fInitializationFrames->setValue(120);
    fInitializationFrames->setPrefix("Initialization frames: ");
    hl->addWidget(fInitializationFrames);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    double decisionThreshold=0.8
    hl = new QHBoxLayout;
    fDecisionThreshold = new QDoubleSpinBox;
    fDecisionThreshold->setMinimum(0);
    fDecisionThreshold->setMaximum(1000);
    fDecisionThreshold->setValue(0.8);
    fDecisionThreshold->setPrefix("Decision threshold: ");
    hl->addWidget(fDecisionThreshold);
    hl->addStretch();
    mainLayout->addLayout(hl);

    createGSOCWidgets();
    createLSBPWidgets();
    createMOGWidgets();

    hl = new QHBoxLayout;
    fBgButton = new QPushButton("Load background");
    fBgLabel = new QLabel("");
    fBgLabel->setMinimumHeight(64);
    fBgLabel->setMinimumWidth(64);
    fBgLabel->setAlignment(Qt::AlignCenter);
    hl->addWidget(fBgButton);
    hl->addWidget(fBgLabel);
    hl->addStretch();
    mainLayout->addLayout(hl);

    mainLayout->addStretch();
    connect(algoComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(algoChanged(int)));
    connect(fBgButton, SIGNAL(clicked(bool)), this, SLOT(openBgImage()));
    algoChanged(algoComboBox->currentIndex());
}

void OpencvBackgroundSubtractorToolWidget::loadSettings(QSettings *settings)
{
    settings->beginGroup(objectName());
    {
        fAlgo = (ALGO)(settings->value("algorithm", MOG2).toInt());
        fHistory->setValue(         settings->value("History",          500).toInt());
        fThreshold->setValue(       settings->value("Threshold",        16).toDouble());
        fDetectShadows->setChecked( settings->value("DetectShadows",    true).toBool());

        fMinPixelStability->setValue(   settings->value("MinPixelStability",    15).toInt());
        fMaxPixelStability->setValue(   settings->value("MaxPixelStability",    15*60).toInt());
        fUseHistory->setChecked(        settings->value("UseHistory",           true).toBool());
        fIsParallel->setChecked(        settings->value("IsParallel",           true).toBool());

        fInitializationFrames->setValue(settings->value("InitializationFrames", 120).toInt());
        fDecisionThreshold->setValue(   settings->value("DecisionThreshold",    0.8).toDouble());

        fMotionCompensation->setChecked(        settings->value("MotionCompensation",           true).toBool());
        fNSamples->setValue(                    settings->value("NSamples",                     20).toInt());
        fReplaceRate->setValue(                 settings->value("ReplaceRate",                  0.003).toDouble());
        fPropagationRate->setValue(             settings->value("PropagationRate",              0.01).toDouble());
        fHitsThreshold->setValue(               settings->value("HitsThreshold",                32).toInt());
        fAlpha->setValue(                       settings->value("Alpha",                        0.01).toDouble());
        fBeta->setValue(                        settings->value("Beta",                         0.0022).toDouble());
        fBlinkingSupressionDecay->setValue(     settings->value("BlinkingSupressionDecay",      0.1).toDouble());
        fBlinkingSupressionMultiplier->setValue(settings->value("BlinkingSupressionMultiplier", 0.01).toDouble());
        fNoiseRemovalThresholdFacBG->setValue(  settings->value("NoiseRemovalThresholdFacBG",   0.0004).toDouble());
        fNoiseRemovalThresholdFacFG->setValue(  settings->value("NoiseRemovalThresholdFacFG",   0.0008).toDouble());

        fLSBPRadius->setValue(      settings->value("LSBPRadius",       16).toInt());
        fTlower->setValue(          settings->value("Tlower",           2.0).toDouble());
        fTupper->setValue(          settings->value("Tupper",           32.0).toDouble());
        fTinc->setValue(            settings->value("Tinc",             1).toDouble());
        fTdec->setValue(            settings->value("Tdec",             0.05).toDouble());
        fRscale->setValue(          settings->value("Rscale",           10).toDouble());
        fRincdec->setValue(         settings->value("Rincdec",          10).toDouble());
        fLSBPthreshold->setValue(   settings->value("LSBPthreshold",    8).toInt());
        fMinCount->setValue(        settings->value("MinCount",         2).toInt());

        fNmixtures->setValue(       settings->value("Nmixtures",        5).toInt());
        fBackgroundRatio->setValue( settings->value("BackgroundRatio",  0.7).toDouble());
        fNoiseSigma->setValue(      settings->value("NoiseSigma",       0).toDouble());
    }
    settings->endGroup();

    algoComboBox->setCurrentIndex(fAlgo);
}

void OpencvBackgroundSubtractorToolWidget::saveSettings(QSettings *settings)
{
    settings->beginGroup(objectName());
    settings->setValue("algorithm", fAlgo);
    settings->setValue("History", fHistory->value());
    settings->setValue("Threshold", fThreshold->value());
    settings->setValue("DetectShadows", fDetectShadows->isChecked());

    settings->setValue("MinPixelStability", fMinPixelStability->value());
    settings->setValue("MaxPixelStability", fMaxPixelStability->value());
    settings->setValue("UseHistory", fUseHistory->isChecked());
    settings->setValue("IsParallel", fIsParallel->isChecked());

    settings->setValue("InitializationFrames", fInitializationFrames->value());
    settings->setValue("DecisionThreshold", fDecisionThreshold->value());

    settings->setValue("MotionCompensation", fMotionCompensation->isChecked());
    settings->setValue("NSamples", fNSamples->value());
    settings->setValue("ReplaceRate", fReplaceRate->value());
    settings->setValue("PropagationRate", fPropagationRate->value());
    settings->setValue("HitsThreshold", fHitsThreshold->value());
    settings->setValue("Alpha", fAlpha->value());
    settings->setValue("Beta", fBeta->value());
    settings->setValue("BlinkingSupressionDecay", fBlinkingSupressionDecay->value());
    settings->setValue("BlinkingSupressionMultiplier", fBlinkingSupressionMultiplier->value());
    settings->setValue("NoiseRemovalThresholdFacBG", fNoiseRemovalThresholdFacBG->value());
    settings->setValue("NoiseRemovalThresholdFacFG", fNoiseRemovalThresholdFacFG->value());

    settings->setValue("LSBPRadius", fLSBPRadius->value());
    settings->setValue("Tlower", fTlower->value());
    settings->setValue("Tupper", fTupper->value());
    settings->setValue("Tinc", fTinc->value());
    settings->setValue("Tdec", fTdec->value());
    settings->setValue("Rscale", fRscale->value());
    settings->setValue("Rincdec", fRincdec->value());
    settings->setValue("LSBPthreshold", fLSBPthreshold->value());
    settings->setValue("MinCount", fMinCount->value());

    settings->setValue("Nmixtures", fNmixtures->value());
    settings->setValue("BackgroundRatio", fBackgroundRatio->value());
    settings->setValue("NoiseSigma", fNoiseSigma->value());
    settings->endGroup();
}

void OpencvBackgroundSubtractorToolWidget::process(cv::Mat *src, cv::Mat *dst)
{
    cv::Ptr<cv::BackgroundSubtractor> pBackSub;
    switch (fAlgo) {
    case CNT:{
        int minPixelStability = fMinPixelStability->value();
        int maxPixelStability = fMaxPixelStability->value();
        bool useHistory = fUseHistory->isChecked();
        bool isParallel = fIsParallel->isChecked();
        pBackSub = cv::bgsegm::createBackgroundSubtractorCNT(minPixelStability, useHistory, maxPixelStability, isParallel);
    }
        break;
    case GMG:{
        int initializationFrames = fInitializationFrames->value();
        double decisionThreshold = fDecisionThreshold->value();
        pBackSub = cv::bgsegm::createBackgroundSubtractorGMG(initializationFrames, decisionThreshold);
    }
        break;
    case GSOC:{
        int mc = fMotionCompensation->isChecked() ? cv::bgsegm::LSBP_CAMERA_MOTION_COMPENSATION_LK : cv::bgsegm::LSBP_CAMERA_MOTION_COMPENSATION_NONE;
        int nSamples = fNSamples->value();
        float replaceRate = fReplaceRate->value();
        float propagationRate = fPropagationRate->value();
        int hitsThreshold = fHitsThreshold->value();
        float alpha = fAlpha->value();
        float beta = fBeta->value();
        float blinkingSupressionDecay = fBlinkingSupressionDecay->value();
        float blinkingSupressionMultiplier = fBlinkingSupressionMultiplier->value();
        float noiseRemovalThresholdFacBG = fNoiseRemovalThresholdFacBG->value();
        float noiseRemovalThresholdFacFG = fNoiseRemovalThresholdFacFG->value();
        pBackSub = cv::bgsegm::createBackgroundSubtractorGSOC(mc, nSamples, replaceRate, propagationRate, hitsThreshold, alpha, beta, blinkingSupressionDecay, blinkingSupressionMultiplier, noiseRemovalThresholdFacBG, noiseRemovalThresholdFacFG);
    }
        break;
    case LSBP:{
        int mc = fMotionCompensation->isChecked() ? cv::bgsegm::LSBP_CAMERA_MOTION_COMPENSATION_LK : cv::bgsegm::LSBP_CAMERA_MOTION_COMPENSATION_NONE;
        int nSamples = fNSamples->value();
        float noiseRemovalThresholdFacBG = fNoiseRemovalThresholdFacBG->value();
        float noiseRemovalThresholdFacFG = fNoiseRemovalThresholdFacFG->value();
        int LSBPRadius = fLSBPRadius->value();
        double Tlower = fTlower->value();
        double Tupper = fTupper->value();
        double Tinc = fTinc->value();
        double Tdec = fTdec->value();
        double Rscale = fRscale->value();
        double Rincdec = fRincdec->value();
        int LSBPthreshold = fLSBPthreshold->value();
        int minCount = fMinCount->value();
        pBackSub = cv::bgsegm::createBackgroundSubtractorLSBP(mc, nSamples, LSBPRadius, Tlower, Tupper, Tinc, Tdec, Rscale, Rincdec, noiseRemovalThresholdFacBG, noiseRemovalThresholdFacFG, LSBPthreshold, minCount);
    }
        break;
    case MOG:{
        int history = fHistory->value();
        int nmixtures = fNmixtures->value();
        double backgroundRatio = fBackgroundRatio->value();
        double noiseSigma = fNoiseSigma->value();
        pBackSub = cv::bgsegm::createBackgroundSubtractorMOG(history, nmixtures, backgroundRatio, noiseSigma);
    }
        break;
    case KNN: {int history = fHistory->value();
        double varThreshold = fThreshold->value();
        bool detectShadows = fDetectShadows->isChecked();
        pBackSub = cv::createBackgroundSubtractorKNN(history, varThreshold, detectShadows);
    }
        break;
    case MOG2:
    default:{
        int history = fHistory->value();
        double varThreshold = fThreshold->value();
        bool detectShadows = fDetectShadows->isChecked();
        pBackSub = cv::createBackgroundSubtractorMOG2(history, varThreshold, detectShadows);
    }
        break;
    }

    cv::Mat fgMask;

    //update the background model
    pBackSub->apply(bgImage, fgMask);
    pBackSub->apply(*src, fgMask);
    fgMask.copyTo(*dst);
}

void OpencvBackgroundSubtractorToolWidget::createGSOCWidgets()
{
    QVBoxLayout* mainLayout = (QVBoxLayout*)layout();

//    int mc = cv::bgsegm::LSBP_CAMERA_MOTION_COMPENSATION_NONE;
    QHBoxLayout *hl = new QHBoxLayout;
    fMotionCompensation = new QCheckBox("Motion compensation");
    fMotionCompensation->setChecked(true);
    hl->addWidget(fMotionCompensation);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    int nSamples = 20;
    hl = new QHBoxLayout;
    fNSamples = new QSpinBox;
    fNSamples->setMinimum(0);
    fNSamples->setMaximum(1000);
    fNSamples->setValue(20);
    fNSamples->setPrefix("N samples: ");
    hl->addWidget(fNSamples);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    float replaceRate = 0.003f;
    hl = new QHBoxLayout;
    fReplaceRate = new QDoubleSpinBox;
    fReplaceRate->setMinimum(0);
    fReplaceRate->setMaximum(1000);
    fReplaceRate->setDecimals(3);
    fReplaceRate->setValue(0.003);
    fReplaceRate->setPrefix("Replace rate: ");
    hl->addWidget(fReplaceRate);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    float propagationRate = 0.01f;
    hl = new QHBoxLayout;
    fPropagationRate = new QDoubleSpinBox;
    fPropagationRate->setMinimum(0);
    fPropagationRate->setMaximum(1000);
    fPropagationRate->setValue(0.01);
    fPropagationRate->setPrefix("Propagation rate: ");
    hl->addWidget(fPropagationRate);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    int hitsThreshold = 32;
    hl = new QHBoxLayout;
    fHitsThreshold = new QSpinBox;
    fHitsThreshold->setMinimum(0);
    fHitsThreshold->setMaximum(1000);
    fHitsThreshold->setValue(32);
    fHitsThreshold->setPrefix("Hits threshold: ");
    hl->addWidget(fHitsThreshold);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    float alpha = 0.01f;
    hl = new QHBoxLayout;
    fAlpha = new QDoubleSpinBox;
    fAlpha->setMinimum(0);
    fAlpha->setMaximum(1000);
    fAlpha->setValue(0.01);
    fAlpha->setPrefix("Alpha: ");
    hl->addWidget(fAlpha);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    float beta = 0.0022f;
    hl = new QHBoxLayout;
    fBeta = new QDoubleSpinBox;
    fBeta->setMinimum(0);
    fBeta->setMaximum(1000);
    fBeta->setDecimals(4);
    fBeta->setValue(0.0022);
    fBeta->setPrefix("Beta: ");
    hl->addWidget(fBeta);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    float blinkingSupressionDecay = 0.1f;
    hl = new QHBoxLayout;
    fBlinkingSupressionDecay = new QDoubleSpinBox;
    fBlinkingSupressionDecay->setMinimum(0);
    fBlinkingSupressionDecay->setMaximum(1000);
    fBlinkingSupressionDecay->setValue(0.1);
    fBlinkingSupressionDecay->setPrefix("Blinking supression decay: ");
    hl->addWidget(fBlinkingSupressionDecay);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    float blinkingSupressionMultiplier = 0.1f;
    hl = new QHBoxLayout;
    fBlinkingSupressionMultiplier = new QDoubleSpinBox;
    fBlinkingSupressionMultiplier->setMinimum(0);
    fBlinkingSupressionMultiplier->setMaximum(1000);
    fBlinkingSupressionMultiplier->setValue(0.1);
    fBlinkingSupressionMultiplier->setPrefix("Blinking supression multiplier: ");
    hl->addWidget(fBlinkingSupressionMultiplier);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    float noiseRemovalThresholdFacBG = 0.0004f;
    hl = new QHBoxLayout;
    fNoiseRemovalThresholdFacBG = new QDoubleSpinBox;
    fNoiseRemovalThresholdFacBG->setMinimum(0);
    fNoiseRemovalThresholdFacBG->setMaximum(0.4999);
    fNoiseRemovalThresholdFacBG->setDecimals(4);
    fNoiseRemovalThresholdFacBG->setValue(0.0004);
    fNoiseRemovalThresholdFacBG->setSingleStep(0.0001);
    fNoiseRemovalThresholdFacBG->setPrefix("Noise removal threshold BG: ");
    hl->addWidget(fNoiseRemovalThresholdFacBG);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    float noiseRemovalThresholdFacFG = 0.0008f;
    hl = new QHBoxLayout;
    fNoiseRemovalThresholdFacFG = new QDoubleSpinBox;
    fNoiseRemovalThresholdFacFG->setMinimum(0);
    fNoiseRemovalThresholdFacFG->setMaximum(0.4999);
    fNoiseRemovalThresholdFacFG->setDecimals(4);
    fNoiseRemovalThresholdFacFG->setValue(0.0008);
    fNoiseRemovalThresholdFacFG->setSingleStep(0.0001);
    fNoiseRemovalThresholdFacFG->setPrefix("Noise removal threshold FG: ");
    hl->addWidget(fNoiseRemovalThresholdFacFG);
    hl->addStretch();
    mainLayout->addLayout(hl);
}

void OpencvBackgroundSubtractorToolWidget::createLSBPWidgets()
{
    QVBoxLayout* mainLayout = (QVBoxLayout*)layout();

//    int mc = LSBP_CAMERA_MOTION_COMPENSATION_NONE;
//    int nSamples = 20;

//    int LSBPRadius = 16;
    QHBoxLayout *hl = new QHBoxLayout;
    fLSBPRadius = new QSpinBox;
    fLSBPRadius->setMinimum(0);
    fLSBPRadius->setMaximum(1000);
    fLSBPRadius->setValue(16);
    fLSBPRadius->setPrefix("LSBP radius: ");
    hl->addWidget(fLSBPRadius);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    float Tlower = 2.0f;
    hl = new QHBoxLayout;
    fTlower = new QDoubleSpinBox;
    fTlower->setMinimum(0);
    fTlower->setMaximum(1000);
    fTlower->setValue(2.0);
    fTlower->setPrefix("T lower: ");
    hl->addWidget(fTlower);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    float Tupper = 32.0f;
    hl = new QHBoxLayout;
    fTupper = new QDoubleSpinBox;
    fTupper->setMinimum(0);
    fTupper->setMaximum(1000);
    fTupper->setValue(32.0);
    fTupper->setPrefix("T upper: ");
    hl->addWidget(fTupper);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    float Tinc = 1.0f;
    hl = new QHBoxLayout;
    fTinc = new QDoubleSpinBox;
    fTinc->setMinimum(0);
    fTinc->setMaximum(1000);
    fTinc->setValue(1.0);
    fTinc->setPrefix("T inc: ");
    hl->addWidget(fTinc);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    float Tdec = 0.05f;
    hl = new QHBoxLayout;
    fTdec = new QDoubleSpinBox;
    fTdec->setMinimum(0);
    fTdec->setMaximum(1000);
    fTdec->setValue(0.05);
    fTdec->setPrefix("T dec: ");
    hl->addWidget(fTdec);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    float Rscale = 10.0f;
    hl = new QHBoxLayout;
    fRscale = new QDoubleSpinBox;
    fRscale->setMinimum(0);
    fRscale->setMaximum(1000);
    fRscale->setValue(10.0);
    fRscale->setPrefix("R scale: ");
    hl->addWidget(fRscale);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    float Rincdec = 0.005f;
    hl = new QHBoxLayout;
    fRincdec = new QDoubleSpinBox;
    fRincdec->setMinimum(0);
    fRincdec->setMaximum(1000);
    fRincdec->setDecimals(3);
    fRincdec->setValue(10.0);
    fRincdec->setPrefix("R inc dec: ");
    hl->addWidget(fRincdec);
    hl->addStretch();
    mainLayout->addLayout(hl);
//    float noiseRemovalThresholdFacBG = 0.0004f;
//    float noiseRemovalThresholdFacFG = 0.0008f;

//    int LSBPthreshold = 8;
    hl = new QHBoxLayout;
    fLSBPthreshold = new QSpinBox;
    fLSBPthreshold->setMinimum(0);
    fLSBPthreshold->setMaximum(1000);
    fLSBPthreshold->setValue(8);
    fLSBPthreshold->setPrefix("LSBP threshold: ");
    hl->addWidget(fLSBPthreshold);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    int minCount = 2;
    hl = new QHBoxLayout;
    fMinCount = new QSpinBox;
    fMinCount->setMinimum(0);
    fMinCount->setMaximum(1000);
    fMinCount->setValue(2);
    fMinCount->setPrefix("Min count: ");
    hl->addWidget(fMinCount);
    hl->addStretch();
    mainLayout->addLayout(hl);
}

void OpencvBackgroundSubtractorToolWidget::createMOGWidgets()
{
    QVBoxLayout* mainLayout = (QVBoxLayout*)layout();

//    int history=200;

//    int nmixtures=5;
    QHBoxLayout *hl = new QHBoxLayout;
    fNmixtures = new QSpinBox;
    fNmixtures->setMinimum(0);
    fNmixtures->setMaximum(1000);
    fNmixtures->setValue(5);
    fNmixtures->setPrefix("N mixtures: ");
    hl->addWidget(fNmixtures);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    double backgroundRatio=0.7;
    hl = new QHBoxLayout;
    fBackgroundRatio = new QDoubleSpinBox;
    fBackgroundRatio->setMinimum(0);
    fBackgroundRatio->setMaximum(1000);
    fBackgroundRatio->setValue(0.7);
    fBackgroundRatio->setPrefix("Background ratio: ");
    hl->addWidget(fBackgroundRatio);
    hl->addStretch();
    mainLayout->addLayout(hl);

//    double noiseSigma=0;
    hl = new QHBoxLayout;
    fNoiseSigma = new QDoubleSpinBox;
    fNoiseSigma->setMinimum(0);
    fNoiseSigma->setMaximum(1000);
    fNoiseSigma->setValue(0.0);
    fNoiseSigma->setPrefix("Noise sigma: ");
    hl->addWidget(fNoiseSigma);
    hl->addStretch();
    mainLayout->addLayout(hl);
}

void OpencvBackgroundSubtractorToolWidget::algoChanged(int algoIndex)
{
    fAlgo = (ALGO)algoIndex;
    updateWidget((ALGO)algoIndex);
}

void OpencvBackgroundSubtractorToolWidget::openBgImage()
{
    QString bgFileName = QFileDialog::getOpenFileName( nullptr, "Background image", ".", "Images (*.png *.jpg *.jpeg)" );
    if( !bgFileName.isEmpty() ) {
        bgImage = cv::imread( bgFileName.toStdString(), cv::IMREAD_UNCHANGED );
//        QImage resultImg;
//        resultImg = QImage( bgImage.data, bgImage.cols, bgImage.rows, bgImage.step, QImage::Format_RGB888 ).copy();
//        fBgButton->setIcon(QIcon(bgFileName));
        QPixmap pix;
        pix.load(bgFileName);
        pix = pix.scaled(64,64,Qt::KeepAspectRatio, Qt::SmoothTransformation);
        fBgLabel->setPixmap(pix);
    }
}

void OpencvBackgroundSubtractorToolWidget::updateWidget(OpencvBackgroundSubtractorToolWidget::ALGO algo)
{
    fHistory->setVisible(false);
    fThreshold->setVisible(false);
    fDetectShadows->setVisible(false);
    fMinPixelStability->setVisible(false);
    fMaxPixelStability->setVisible(false);
    fUseHistory->setVisible(false);
    fIsParallel->setVisible(false);
    fInitializationFrames->setVisible(false);
    fDecisionThreshold->setVisible(false);

    fMotionCompensation->setVisible(false);
    fNSamples->setVisible(false);
    fReplaceRate->setVisible(false);
    fPropagationRate->setVisible(false);
    fHitsThreshold->setVisible(false);
    fAlpha->setVisible(false);
    fBeta->setVisible(false);
    fBlinkingSupressionDecay->setVisible(false);
    fBlinkingSupressionMultiplier->setVisible(false);
    fNoiseRemovalThresholdFacBG->setVisible(false);
    fNoiseRemovalThresholdFacFG->setVisible(false);

    fLSBPRadius->setVisible(false);
    fTlower->setVisible(false);
    fTupper->setVisible(false);
    fTinc->setVisible(false);
    fTdec->setVisible(false);
    fRscale->setVisible(false);
    fRincdec->setVisible(false);
    fLSBPthreshold->setVisible(false);
    fMinCount->setVisible(false);

    fNmixtures->setVisible(false);
    fBackgroundRatio->setVisible(false);
    fNoiseSigma->setVisible(false);

    switch (algo) {
    case CNT:
        fMinPixelStability->setVisible(true);
        fMaxPixelStability->setVisible(true);
        fUseHistory->setVisible(true);
        fIsParallel->setVisible(true);
        break;
    case GMG:
        fInitializationFrames->setVisible(true);
        fDecisionThreshold->setVisible(true);
        break;
    case GSOC:
        fMotionCompensation->setVisible(true);
        fNSamples->setVisible(true);
        fReplaceRate->setVisible(true);
        fPropagationRate->setVisible(true);
        fHitsThreshold->setVisible(true);
        fAlpha->setVisible(true);
        fBeta->setVisible(true);
        fBlinkingSupressionDecay->setVisible(true);
        fBlinkingSupressionMultiplier->setVisible(true);
        fNoiseRemovalThresholdFacBG->setVisible(true);
        fNoiseRemovalThresholdFacFG->setVisible(true);
        break;
    case LSBP:
        fMotionCompensation->setVisible(true);
        fNSamples->setVisible(true);
        fNoiseRemovalThresholdFacBG->setVisible(true);
        fNoiseRemovalThresholdFacFG->setVisible(true);
        fLSBPRadius->setVisible(true);
        fTlower->setVisible(true);
        fTupper->setVisible(true);
        fTinc->setVisible(true);
        fTdec->setVisible(true);
        fRscale->setVisible(true);
        fRincdec->setVisible(true);
        fLSBPthreshold->setVisible(true);
        fMinCount->setVisible(true);
        break;
    case MOG:
        fHistory->setVisible(true);
        fNmixtures->setVisible(true);
        fBackgroundRatio->setVisible(true);
        fNoiseSigma->setVisible(true);
        break;
    case MOG2:
    case KNN:
    default:
        fHistory->setVisible(true);
        fThreshold->setVisible(true);
        fDetectShadows->setVisible(true);
        break;
    }
}

OpencvSeparateChannelsToolWidget::OpencvSeparateChannelsToolWidget(QWidget *parent) : OpencvBaseToolWidget(parent)
{
    setObjectName("SeparateChannelsToolWidget");

    QVBoxLayout* mainLayout = (QVBoxLayout*)layout();
    modeComboBox = new QComboBox();
    modeComboBox->addItem("BGR");
    modeComboBox->addItem("BGR2XYZ");
    modeComboBox->addItem("BGR2Lab");
    modeComboBox->addItem("BGR2YUV");
    modeComboBox->addItem("BGR2HLS");
    modeComboBox->addItem("BGR2Luv");
    modeComboBox->addItem("BGR2HSV");
    modeComboBox->addItem("BGR2YCrCb");

    mode = modeBGR;

    QHBoxLayout *hl = new QHBoxLayout;
    hl->addWidget(new QLabel("Mode:"));
    hl->addWidget(modeComboBox);
    hl->addStretch();
    mainLayout->addLayout(hl);

    hl = new QHBoxLayout;
    colored = new QCheckBox("Colored");
    hl->addWidget(colored);
    hl->addStretch();
    mainLayout->addLayout(hl);

    hl = new QHBoxLayout;
    QPushButton *separateButton = new QPushButton("Separate");
    hl->addWidget(separateButton);
    hl->addStretch();
    mainLayout->addLayout(hl);

    hl = new QHBoxLayout;
    imageSize = new QSpinBox();
    imageSize->setMinimum(64);
    imageSize->setMaximum(64 + 32 * 10);
    imageSize->setSingleStep(32);
    imageSize->setValue(128);
    imageSize->setPrefix("Image size: ");
    hl->addWidget(imageSize);
    hl->addStretch();
    mainLayout->addLayout(hl);

    channel1 = new QLabel;
    channel2 = new QLabel;
    channel3 = new QLabel;
    labelChannel1 = new QLabel;
    labelChannel2 = new QLabel;
    labelChannel3 = new QLabel;

    channel1->setMinimumHeight(128);
    channel1->setMinimumWidth(128);
    channel2->setMinimumHeight(128);
    channel2->setMinimumWidth(128);
    channel3->setMinimumHeight(128);
    channel3->setMinimumWidth(128);

    channel1->setMaximumHeight(128);
    channel1->setMaximumWidth(128);
    channel2->setMaximumHeight(128);
    channel2->setMaximumWidth(128);
    channel3->setMaximumHeight(128);
    channel3->setMaximumWidth(128);

    channel1->setFrameShape(QFrame::Box);
    channel2->setFrameShape(QFrame::Box);
    channel3->setFrameShape(QFrame::Box);

    outputChannel1 = new QRadioButton("Output");
    outputChannel2 = new QRadioButton("Output");
    outputChannel3 = new QRadioButton("Output");

    hl = new QHBoxLayout;
    hl->addWidget(labelChannel1);
    hl->addWidget(outputChannel1);
    hl->addStretch();
    mainLayout->addLayout(hl);
    mainLayout->addWidget(channel1);

    hl = new QHBoxLayout;
    hl->addWidget(labelChannel2);
    hl->addWidget(outputChannel2);
    hl->addStretch();
    mainLayout->addLayout(hl);
    mainLayout->addWidget(channel2);

    hl = new QHBoxLayout;
    hl->addWidget(labelChannel3);
    hl->addWidget(outputChannel3);
    hl->addStretch();
    mainLayout->addLayout(hl);
    mainLayout->addWidget(channel3);

    mainLayout->addStretch();
    connect(modeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged(int)));
    connect(separateButton, SIGNAL(clicked(bool)), this, SLOT(separate()));
    connect(imageSize, SIGNAL(valueChanged(int)), this, SLOT(imageSizeChanged(int)));
    modeChanged(modeComboBox->currentIndex());
}

void OpencvSeparateChannelsToolWidget::loadSettings(QSettings *settings)
{
    settings->beginGroup(objectName());
    mode = (MODE)(settings->value("Mode", 0).toInt());
    imageSize->setValue(settings->value("ImageSize", 128).toInt());
    colored->setChecked(settings->value("Colored", false).toBool());
    settings->endGroup();

    modeComboBox->setCurrentIndex(mode);
    imageSizeChanged(imageSize->value());
}

void OpencvSeparateChannelsToolWidget::saveSettings(QSettings *settings)
{
    settings->beginGroup(objectName());
    settings->setValue("Mode", mode);
    settings->setValue("ImageSize", imageSize->value());
    settings->setValue("Colored", colored->isChecked());
    settings->endGroup();
}

void OpencvSeparateChannelsToolWidget::process(cv::Mat *src, cv::Mat *dst)
{
    src->copyTo(*dst);
    src->copyTo(originImage);
    separate();
}

void OpencvSeparateChannelsToolWidget::updateWidget(OpencvSeparateChannelsToolWidget::MODE mode)
{
    switch (mode) {
    case modeBGR: {
        labelChannel1->setText("B");
        labelChannel2->setText("G");
        labelChannel3->setText("R");
        break;
    }
    case modeBGR2Lab: {
        labelChannel1->setText("L");
        labelChannel2->setText("a");
        labelChannel3->setText("b");
    }
        break;
    case modeBGR2YUV: {
        labelChannel1->setText("Y");
        labelChannel2->setText("U");
        labelChannel3->setText("V");
    }
        break;
    case modeBGR2HLS: {
        labelChannel1->setText("H");
        labelChannel2->setText("L");
        labelChannel3->setText("S");
    }
        break;
    case modeBGR2Luv: {
        labelChannel1->setText("L");
        labelChannel2->setText("u");
        labelChannel3->setText("v");
    }
        break;
    case modeBGR2HSV: {
        labelChannel1->setText("H");
        labelChannel2->setText("S");
        labelChannel3->setText("V");
    }
        break;
    case modeBGR2YCrCb: {
        labelChannel1->setText("Y");
        labelChannel2->setText("Cr");
        labelChannel3->setText("Cb");
    }
        break;
    case modeBGR2XYZ:
    default: {
        labelChannel1->setText("X");
        labelChannel2->setText("Y");
        labelChannel3->setText("Z");
    }
        break;
    }
}

void OpencvSeparateChannelsToolWidget::updateLabel(QLabel *label, cv::Mat *mat)
{
    QImage resultImg;
    if (colored->isChecked())
        resultImg = QImage( mat->data, mat->cols, mat->rows, mat->step, QImage::Format_RGB888 ).copy().rgbSwapped();
    else
        resultImg = QImage( mat->data, mat->cols, mat->rows, mat->step, QImage::Format_Grayscale8 ).copy();

    label->setPixmap(
        QPixmap::fromImage( resultImg ).scaled(
            label->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        )
                );
}

std::vector<cv::Mat> OpencvSeparateChannelsToolWidget::coloredSeparatedChannels(std::vector<cv::Mat> channels)
{
    std::vector<cv::Mat> separatedChannels;
    // создаем по одному изображению на каждый канал
    for ( int i = 0 ; i < 3 ; i++){
        cv::Mat zer = cv::Mat::zeros( channels[0].rows, channels[0].cols, channels[0].type());
        std::vector<cv::Mat> aux;
        for (int j=0; j < 3 ; j++) {
            if(j==i)
                aux.push_back(channels[i]);
            else
                aux.push_back(zer);
        }
        cv::Mat chann;
        merge(aux,chann);
        separatedChannels.push_back(chann);
    }
    return separatedChannels;
}

cv::Mat OpencvSeparateChannelsToolWidget::rgb2mode(cv::Mat mat)
{
    cv::Mat convertedImage;
    switch (mode) {
    case modeBGR:
        mat.copyTo(convertedImage);
        break;
    case modeBGR2Lab:
        cv::cvtColor(mat, convertedImage, cv::COLOR_BGR2Lab);
        break;
    case modeBGR2YUV:
        cv::cvtColor(mat, convertedImage, cv::COLOR_BGR2YUV);
        break;
    case modeBGR2HLS:
        cv::cvtColor(mat, convertedImage, cv::COLOR_BGR2HLS);
        break;
    case modeBGR2Luv:
        cv::cvtColor(mat, convertedImage, cv::COLOR_BGR2Luv);
        break;
    case modeBGR2HSV:
        cv::cvtColor(mat, convertedImage, cv::COLOR_BGR2HSV);
        break;
    case modeBGR2YCrCb:
        cv::cvtColor(mat, convertedImage, cv::COLOR_BGR2YCrCb);
        break;
    case modeBGR2XYZ:
    default: {
        cv::cvtColor(mat, convertedImage, cv::COLOR_BGR2XYZ);
    }
        break;
    }
    return convertedImage;
}

cv::Mat OpencvSeparateChannelsToolWidget::mode2rgb(cv::Mat mat)
{
    cv::Mat convertedImage;
    switch (mode) {
    case modeBGR:
        mat.copyTo(convertedImage);
        break;
    case modeBGR2Lab:
        cv::cvtColor(mat, convertedImage, cv::COLOR_Lab2BGR);
        break;
    case modeBGR2YUV:
        cv::cvtColor(mat, convertedImage, cv::COLOR_YUV2BGR);
        break;
    case modeBGR2HLS:
        cv::cvtColor(mat, convertedImage, cv::COLOR_HLS2BGR);
        break;
    case modeBGR2Luv:
        cv::cvtColor(mat, convertedImage, cv::COLOR_Luv2BGR);
        break;
    case modeBGR2HSV:
        cv::cvtColor(mat, convertedImage, cv::COLOR_HSV2BGR);
        break;
    case modeBGR2YCrCb:
        cv::cvtColor(mat, convertedImage, cv::COLOR_YCrCb2BGR);
        break;
    case modeBGR2XYZ:
    default: {
        cv::cvtColor(mat, convertedImage, cv::COLOR_XYZ2BGR);
    }
        break;
    }
    return convertedImage;
}

void OpencvSeparateChannelsToolWidget::separate()
{
    if (originImage.empty()) return;

    cv::Mat convertedImage = rgb2mode(originImage);

    std::vector<cv::Mat> image_planes(3);
    cv::split(convertedImage, image_planes);

    if (colored->isChecked()) {
        std::vector<cv::Mat> separatedChannels = coloredSeparatedChannels(image_planes);
        for (int i = 0; i < 3; i++) {
            if (mode == modeBGR2HSV || mode == modeBGR2HLS || mode == modeBGR2Lab || mode == modeBGR2Luv)
                image_planes[i] = convertedImage.clone();
            else
                image_planes[i] = mode2rgb(separatedChannels[i]);
        }
    }

    updateLabel( channel1, &image_planes[0] );
    updateLabel( channel2, &image_planes[1] );
    updateLabel( channel3, &image_planes[2] );

    channel1Image = image_planes[0].clone();
    channel2Image = image_planes[1].clone();
    channel3Image = image_planes[2].clone();

//    cv::imshow( "Foreground 0", image_planes[0] );
//    cv::imshow( "Foreground 1", image_planes[1] );
//    cv::imshow( "Foreground 2", image_planes[2] );
}

void OpencvSeparateChannelsToolWidget::modeChanged(int _mode)
{
    mode = (MODE)_mode;
    updateWidget(mode);
}

void OpencvSeparateChannelsToolWidget::imageSizeChanged(int)
{
    int size = imageSize->value();
    channel1->setMinimumHeight(size);
    channel1->setMinimumWidth(size);
    channel2->setMinimumHeight(size);
    channel2->setMinimumWidth(size);
    channel3->setMinimumHeight(size);
    channel3->setMinimumWidth(size);

    channel1->setMaximumHeight(size);
    channel1->setMaximumWidth(size);
    channel2->setMaximumHeight(size);
    channel2->setMaximumWidth(size);
    channel3->setMaximumHeight(size);
    channel3->setMaximumWidth(size);
}
