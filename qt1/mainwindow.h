#ifndef MainWindow_H
#define MainWindow_H

#include <QMainWindow>
#include <QtWidgets>
#include <QImage>
#include <QMouseEvent>
#include "NiftiImage.h"
#include <vector>

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"

#include "Eigen/Eigen"

#define CORONAL 0
#define SAGITTAL 1
#define AXIAL 2

#define t1image 0
#define slabimage 1
#define maskimage 2

struct DicomInfo
{
	Float32 coordFH, coordAP, coordRL;
	Float32 angleFH, angleAP, angleRL;

	friend DicomInfo operator-(const DicomInfo& a, const DicomInfo& b) {
		DicomInfo result;
		result.coordFH = a.coordFH - b.coordFH;
		result.coordAP = a.coordAP - b.coordAP;
		result.coordRL = a.coordRL - b.coordRL;
		result.angleFH = a.angleFH - b.angleFH;
		result.angleAP = a.angleAP - b.angleAP;
		result.angleRL = a.angleRL - b.angleRL;
		return result;
	}
};
typedef vector<vector<vector<float>>> vec3df; // vector - 3d - float

struct Metabolite {
	string name;
	float conc;
	int sd;
	float ratio;
	bool qc;
};

struct TableInfo {
	//string metaInfo[35][4];
	//string fwhm, snr;
	map<string, Metabolite> metaInfo;
	float fwhm;
	int snr;
	float pvc;
	bool isAvailable = false;
};

struct coord
{
	int a, b, c;
};

class QAction;
class QLabel;
class QMenu;
class QGridLayout;
class QSpinBox;
class QFileInfo;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();

	private slots:
	void open();
	void loadDicom();
	void loadSegImgs();
	void openSlab();
	void openLCM();
	void valueUpdateCor(int value);
	void valueUpdateSag(int value);
	void valueUpdateAxi(int value);
	void openSlabMask();
	void makeSlabMask();
	void valueUpdateIntensity(int value);
	void updateMetaChecked(QAbstractButton*);

private:
	QWidget *mainWidget;
	int planeSize = 500;
	QLabel *plane[3];
	QLabel *sliceInfoText[3];
	QSpinBox *sliceSpinBox[3];
	int sliceNum[3]; // coronal, sagittal, axial slice
	QGridLayout *viewerLayout;
	QGridLayout *ctrlLayout;
	QVBoxLayout *lcmLayout;
	QGroupBox *lcmInfoBox;

	QTextEdit *lcmInfo;

	QLabel *intensityText;
	//QDoubleSpinBox *intensitySpinBox;
	QSpinBox *intensitySpinBox;

	void createActions();
	void setLCMLayout();
	
	// menu & actions
	QMenu *slabMenu;
	QAction *overlaySlabAct;
	QAction *openSlabMaskAct;
	void setEnabledT1DepMenus(bool);

	// T1 image
	NiftiImage *img = NULL;
	vec3df T1vol;
	QImage T1Images[3];
	QString imgFileName;
	float intensity;
	float T1MaxVal;

	bool loadImageFile(const QString &);
	void setDefaultIntensity();
	float getMaxVal(vec3df imagevol);
	void setSliceNum();
	vec3df getImgvol(NiftiImage *image);

	// DICOM image
	DicomInfo T1, MRSI;
	bool findDicomFiles(QString dir);

	// Slab image
	NiftiImage *slab = NULL;
	QImage slabImages[3];
	vec3df slabvol;

	bool loadSlab(const QString &);

	// LCM info
	TableInfo ***tables = NULL;
	QStringList metaList;

	bool loadLCMInfo(QString dir); //bool loadLCMInfo(QStringList filepaths);
	TableInfo parseTable(string filename);
	void presentLCMInfo();
	void saveLCMData();

	// Draw and update planes
	bool overlay = false;
	float selectedVoxel = 0;

	void drawPlane(int planeType);
	void overlayImage(QImage base, QImage overlay, int planeType);
	void initImages(int planeType, int imageType);

	// Slab - voxel picking (single voxle selection yet)
	bool eventFilter(QObject *watched, QEvent *e);
	float getSlabVoxelValue(int x, int y, int planeType);
	void changeVoxelValues(float value, bool on);

	// Mask (voxel quality check)
	NiftiImage *mask = NULL;
	vec3df maskvol;
	QImage maskImages[3];
	//bool mask = false;

	bool loadSlabMask(const QString &);
	void voxelQualityCheck(string metabolite, int sd, float fwhm, int snr);
	void saveSlabMask(string metabolite);
	QString getMaskFileName(string metabolite);

	// statistics
	float calAvgConc(string metabolite);

	// Slab
	void makeSlab();
	QString getSlabFileName();

	// Slab - transformation
	vec3df transformation3d(vec3df imgvol, float coordFH, float coordAP, float coordRL , float angleFH, float angleAP, float angleRL);
	float deg2rad(float degree);

	float* arr3Dto1D(NiftiImage *image, vec3df imagevol);
	bool saveImageFile(string filename, NiftiImage *image, vec3df data);

	// Partial volume correction
	void calPVC(vec3df gmvol, vec3df wmvol, vec3df csfvol);

	coord n2abc(int n);
};

#endif
