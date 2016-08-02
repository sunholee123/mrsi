#ifndef MainWindow_H
#define MainWindow_H

#include <QMainWindow>
#include <QImage>
#include "NiftiImage.h"

#define CORONAL 0
#define SAGITTAL 1
#define AXIAL 2

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
	bool loadFile(const QString &);

	private slots:
	void open();
	void valueUpdateCor(int value);
	void valueUpdateSag(int value);
	void valueUpdateAxi(int value);


private:
	void createActions();
	
	QWidget *mainwidget;

	// coronal, sagittal, axial
	QLabel *plane[3];
	QLabel *sliceInfoText[3];
	QSpinBox *sliceSpinBox[3];
	int sliceNum[3]; // coronal, sagittal, axial slice

	QGridLayout *mainLayout;
	QGridLayout *ctrlLayout;

	// MRI image
	NiftiImage *img = NULL;
	float * imgvol = NULL;

	void setDefaultIntensity();
	float intensity;
	void drawPlane(int planeType);

};

#endif
