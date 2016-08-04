#ifndef MainWindow_H
#define MainWindow_H

#include <QMainWindow>
#include <QtWidgets>
#include <QImage>
#include "NiftiImage.h"

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"

#define CORONAL 0
#define SAGITTAL 1
#define AXIAL 2

struct DicomInfo
{
	Float32 coordX, coordY, coordZ;
	Float32 angleX, angleY, angleZ;
};

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

	// MRI image
	bool loadImageFile(const QString &);
	NiftiImage *img = NULL;
	float * imgvol = NULL;

	void setDefaultIntensity();
	float intensity;
	void drawPlane(int planeType);

	// DICOM
	void findDicomFiles();
};



#endif
