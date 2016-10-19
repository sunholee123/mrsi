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
	DicomInfo& operator-(const DicomInfo& dcminfo) {
		coordFH -= dcminfo.coordFH; coordAP -= dcminfo.coordAP; coordRL -= dcminfo.coordRL;
		angleFH -= dcminfo.angleFH; angleAP -= dcminfo.angleAP; angleRL -= dcminfo.angleRL;
		return *this;
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
	void openSlab();
	void openLCM();
	void valueUpdateCor(int value);
	void valueUpdateSag(int value);
	void valueUpdateAxi(int value);
	void openSlabMask();
	void makeSlabMask();

private:
	QWidget *mainWidget;
	int planeSize = 300;
	QLabel *plane[3];
	QLabel *sliceInfoText[3];
	QSpinBox *sliceSpinBox[3];
	int sliceNum[3]; // coronal, sagittal, axial slice
	QGridLayout *viewerLayout;
	QGridLayout *ctrlLayout;
	QVBoxLayout *lcmLayout;
	QTextEdit *lcmInfo;
	
	void createActions();
	void setLCMLayout();
	
	// MRI image
	NiftiImage *img = NULL;
	vec3df imgvol;
	QImage T1Images[3];
	QString imgFileName;
	float intensity;

	bool loadImageFile(const QString &);
	void setDefaultIntensity();
	void setSliceNum();
	void arr1Dto3D(NiftiImage *image, int imageType);

	// DICOM image
	DicomInfo T1, MRSI;
	void findDicomFiles();

	// Slab image
	NiftiImage *slab = NULL;
	QImage slabImages[3];
	vec3df slabvol;

	bool loadSlab(const QString &);

	// LCM info
	TableInfo ***tables = NULL;
	QStringList metaList;

	bool loadLCMInfo(QStringList filepaths);
	TableInfo parseTable(string filename);
	void presentLCMInfo();

	// Draw and update planes
	bool overlay = false;
	bool voxelPick = false;
	float selectedVoxel = -1;

	void drawPlane(int planeType);
	void overlayImage(QImage base, QImage overlay, int planeType);
	void initImages(int planeType, int imageType);

	// Slab - voxel picking (single voxle selection yet)
	bool eventFilter(QObject *watched, QEvent *e);
	float getSlabVoxelValue(int x, int y, int planeType);
	void changeVoxelValues(float value, bool on);

	// Slab - mask (voxel quality check)
	NiftiImage *mask = NULL;
	vec3df maskvol;
	QImage maskImages[3];
	//bool mask = false;

	bool loadSlabMask(const QString &);
	void voxelQualityCheck(string metabolite, int sd, float fwhm, int snr);
	void saveSlabMask(string metabolite);
	QString getMaskFileName();

	// Slab
	void makeSlab();
	QString getSlabFileName();

	// Slab - transformation (not fully implemented yet!!!)
	vec3df transformation3d(vec3df imgvol, float coordFH, float coordAP, float coordRL , float angleFH, float angleAP, float angleRL);
	float deg2rad(float degree);

	float* arr3Dto1D(NiftiImage *image, vec3df imagevol);
	bool saveImageFile(string filename, NiftiImage *image, vec3df data);	
};

#endif
