#ifndef MainWindow_H
#define MainWindow_H

#include <QMainWindow>
#include <QtWidgets>
#include <QImage>
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

struct DicomInfo // that naming is not uncertain...
{
	Float32 coordFH, coordAP, coordRL;
	Float32 angleFH, angleAP, angleRL;
};
typedef vector<vector<vector<float>>> vec3df; // vector - 3d - float

class QAction;
class QLabel;
class QMenu;
class QGridLayout;
class QSpinBox;

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
	void valueUpdateCor(int value);
	void valueUpdateSag(int value);
	void valueUpdateAxi(int value);


private:
	void createActions();
	
	QWidget *mainWidget;

	// coronal, sagittal, axial
	QLabel *plane[3];
	QLabel *sliceInfoText[3];
	QSpinBox *sliceSpinBox[3];
	int sliceNum[3]; // coronal, sagittal, axial slice
	void setSliceNum();

	QGridLayout *mainLayout;
	QGridLayout *ctrlLayout;
	QVBoxLayout *lcmLayout;
	// MRI image
	bool loadImageFile(const QString &);
	NiftiImage *img = NULL;
	vec3df imgvol;
	void arr1Dto3D(NiftiImage *image, int imageType);

	// Intensity
	void setDefaultIntensity();
	float intensity;

	// Draw image
	void drawPlane(int planeType);

	// DICOM
	DicomInfo T1, MRSI;
	void findDicomFiles();

	// Slab
	bool overlay = false;
	void makeSlab();
	bool loadSlab(const QString &);
	NiftiImage *slab = NULL;
	vec3df slabvol;
	void overlaySlab(int planeType);

	// Slab - transformation
	vec3df transformation3d(vec3df imgvol, float coordFH, float coordAP, float coordRL , float angleFH, float angleAP, float angleRL);
	float deg2rad(float degree);

	float* arr3Dto1D(NiftiImage *image, vec3df imagevol);
	bool saveImageFile(const char * filename, NiftiImage *image, vec3df data);

};



#endif
