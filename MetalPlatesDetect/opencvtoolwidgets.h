#ifndef OPENCVTOOLWIDGETS_H
#define OPENCVTOOLWIDGETS_H

#include <QWidget>
#include <opencv2/core.hpp>

class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QPushButton;
class QLabel;
class QComboBox;
class QSettings;
class QRadioButton;

class OpencvBaseToolWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OpencvBaseToolWidget(QWidget *parent = nullptr);
    QLayout *layout() const { return fMainLayout; }
    bool toolIsEnabled() const { return fEnabled; }
    virtual void loadSettings(QSettings*) = 0;
    virtual void saveSettings(QSettings*) = 0;

public slots:
    virtual void process(cv::Mat *src, cv::Mat *dst) = 0;

private:
    QLayout *fMainLayout;
    bool fEnabled;

private slots:
    void enableChanged(bool);
};

class OpencvBackgroundSubtractorToolWidget : public OpencvBaseToolWidget
{
    Q_OBJECT
public:
    explicit OpencvBackgroundSubtractorToolWidget(QWidget *parent = nullptr);
    void loadSettings(QSettings*) override;
    void saveSettings(QSettings*) override;

public slots:
    void process(cv::Mat *src, cv::Mat *dst) override;

private slots:
    void createGSOCWidgets();
    void createLSBPWidgets();
    void createMOGWidgets();
    void algoChanged(int);
    void openBgImage();

private:
    enum ALGO {
        MOG2 = 0,
        KNN = 1,
        CNT = 2,
        GMG = 3,
        GSOC = 4,
        LSBP = 5,
        MOG = 6
    };
    QSpinBox*       fHistory;
    QDoubleSpinBox* fThreshold;
    QCheckBox*      fDetectShadows;

    QSpinBox*       fMinPixelStability;
    QSpinBox*       fMaxPixelStability;
    QCheckBox*      fUseHistory;
    QCheckBox*      fIsParallel;

    QSpinBox*       fInitializationFrames;
    QDoubleSpinBox* fDecisionThreshold;

    QCheckBox*      fMotionCompensation;
    QSpinBox*       fNSamples;
    QDoubleSpinBox* fReplaceRate;
    QDoubleSpinBox* fPropagationRate;
    QSpinBox*       fHitsThreshold;
    QDoubleSpinBox* fAlpha;
    QDoubleSpinBox* fBeta;
    QDoubleSpinBox* fBlinkingSupressionDecay;
    QDoubleSpinBox* fBlinkingSupressionMultiplier;
    QDoubleSpinBox* fNoiseRemovalThresholdFacBG;
    QDoubleSpinBox* fNoiseRemovalThresholdFacFG;

    QSpinBox*       fLSBPRadius;
    QDoubleSpinBox* fTlower;
    QDoubleSpinBox* fTupper;
    QDoubleSpinBox* fTinc;
    QDoubleSpinBox* fTdec;
    QDoubleSpinBox* fRscale;
    QDoubleSpinBox* fRincdec;
    QSpinBox*       fLSBPthreshold;
    QSpinBox*       fMinCount;

    QSpinBox*       fNmixtures;
    QDoubleSpinBox* fBackgroundRatio;
    QDoubleSpinBox* fNoiseSigma;

    cv::Mat bgImage;
    QPushButton* fBgButton;
    QLabel* fBgLabel;

    ALGO fAlgo;
    QComboBox *algoComboBox;

    void updateWidget(ALGO algo);
};

class OpencvSeparateChannelsToolWidget : public OpencvBaseToolWidget
{
    Q_OBJECT
public:
    explicit OpencvSeparateChannelsToolWidget(QWidget *parent = nullptr);
    void loadSettings(QSettings*) override;
    void saveSettings(QSettings*) override;

public slots:
    void process(cv::Mat *src, cv::Mat *dst) override;

private:
    enum MODE {
        modeBGR,
        modeBGR2XYZ,  // COLOR_BGR2XYZ
        modeBGR2Lab,  // COLOR_BGR2Lab
        modeBGR2YUV,  // COLOR_BGR2YUV
        modeBGR2HLS,  // COLOR_BGR2HLS
        modeBGR2Luv,  // COLOR_BGR2Luv
        modeBGR2HSV,  // COLOR_BGR2HSV
        modeBGR2YCrCb // COLOR_BGR2YCrCb
        // COLOR_BGR2GRAY
    };

    QComboBox *modeComboBox;
    MODE mode;
    QLabel *channel1;
    QLabel *channel2;
    QLabel *channel3;
    QLabel *labelChannel1;
    QLabel *labelChannel2;
    QLabel *labelChannel3;
    QRadioButton *outputChannel1;
    QRadioButton *outputChannel2;
    QRadioButton *outputChannel3;
    QSpinBox *imageSize;
    QCheckBox *colored;

    cv::Mat originImage;
    cv::Mat channel1Image;
    cv::Mat channel2Image;
    cv::Mat channel3Image;

    void updateWidget(MODE mode);
    void updateLabel(QLabel *label, cv::Mat *mat);
    std::vector<cv::Mat> coloredSeparatedChannels(std::vector<cv::Mat> channels);
    cv::Mat rgb2mode(cv::Mat mat);
    cv::Mat mode2rgb(cv::Mat mat);

private slots:
    void separate();
    void modeChanged(int);
    void imageSizeChanged(int);
};

#endif // OPENCVTOOLWIDGETS_H
