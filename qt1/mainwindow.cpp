#include <QtWidgets>
#include "MainWindow.h"
MainWindow::MainWindow()
{
	// main widget
	mainwidget = new QWidget;
	mainwidget->setBackgroundRole(QPalette::Dark);
	mainwidget->setLayout(mainLayout);
	setCentralWidget(mainwidget);

	for (int i = 0; i < 3; i++)
	{
		plane[i] = new QLabel();
		plane[i]->setFixedWidth(500);
		plane[i]->setFixedHeight(500);
		//plane[i]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		plane[i]->setScaledContents(true);
	}
	
	mainLayout = new QGridLayout;
	mainLayout->addWidget(plane[CORONAL], 0, 0);
	mainLayout->addWidget(plane[SAGITTAL], 0, 1);
	mainLayout->addWidget(plane[AXIAL], 1, 0);

	// spinboxes for controlling slices
	for (int i = 0; i < 3; i++)
	{
		sliceSpinBox[i] = new QSpinBox();
		sliceSpinBox[i]->setRange(0, 180);
		sliceNum[i] = 90;
		sliceSpinBox[i]->setValue(sliceNum[i]);
	}
	connect(sliceSpinBox[0], SIGNAL(valueChanged(int)), this, SLOT(valueUpdateCor(int)));
	connect(sliceSpinBox[1], SIGNAL(valueChanged(int)), this, SLOT(valueUpdateSag(int)));
	connect(sliceSpinBox[2], SIGNAL(valueChanged(int)), this, SLOT(valueUpdateAxi(int)));

	// controllers layout
	ctrlLayout = new QGridLayout;
	sliceInfoText[AXIAL] = new QLabel("<font color='black'>Axial slice:</font>");
	sliceInfoText[SAGITTAL] = new QLabel("<font color='black'>Sagittal slice:</font>");
	sliceInfoText[CORONAL] = new QLabel("<font color ='black'>Coronal slice:</font>");
	for (int i = 0; i < 3; i++)
	{
		sliceInfoText[i]->setVisible(true);
		ctrlLayout->addWidget(sliceInfoText[i], i, 0);
		ctrlLayout->addWidget(sliceSpinBox[i], i, 1);
		
	}
	mainLayout->addLayout(ctrlLayout, 1, 1);

	createActions();
}

MainWindow::~MainWindow()
{
	if(img != NULL)
		delete img;
	if (imgvol != NULL)
		delete [] imgvol;
}

void MainWindow::setDefaultIntensity()
{
	// find maximum value
	float maxval = 0;
	for (int i = 0; i < img->nx()*img->ny()*img->nz(); i++)
	{
		if (imgvol[i] > maxval)
			maxval = imgvol[i];
	}
	//intensity = 256 / maxval;
	intensity = 300 / maxval;
}

void MainWindow::drawPlane(int planeType)
{
	int width, height;
	switch(planeType)
	{
	case CORONAL:	width = img->nx();	height = img->nz();	break;
	case SAGITTAL:	width = img->ny();	height = img->nz();	break;
	case AXIAL:		width = img->nx();	height = img->ny();	break;
	}

	QImage slice(width, height, QImage::Format_RGB32);
	QRgb value;
	float val;
	for (int i = 0; i<width; i++)
		for (int j = 0; j<height; j++)
		{
			switch (planeType)
			{
			case CORONAL:	val = imgvol[i + (j * width * height) + (width * sliceNum[planeType])];	break;
			case SAGITTAL:	val = imgvol[(i * img->nx()) + (j * img->nx() * height) + sliceNum[planeType]];	break;
			case AXIAL:		val = imgvol[i + (j * width) + (width * height * sliceNum[planeType])];	break;
			}
			val = val * intensity;
			value = qRgb(val, val, val);
			slice.setPixel(i, height-j, value);
		}
	plane[planeType]->setPixmap(QPixmap::fromImage(slice));
}

bool MainWindow::loadFile(const QString &fileName)
{
	// load mri image
	string filename = fileName.toStdString();
	
	img = new NiftiImage(filename, 'r');
	imgvol = img->readAllVolumes<float>();

	setDefaultIntensity();
	drawPlane(CORONAL);
	drawPlane(SAGITTAL);
	drawPlane(AXIAL);

	const QString message = tr("Opened \"%1\\").arg(QDir::toNativeSeparators(fileName));
	statusBar()->showMessage(message);
	
	return true;
}

void MainWindow::open()
{
	QFileDialog dialog(this, tr("Open File"));
	dialog.setNameFilter(tr("Nifti files (*.nii.gz *.nii *.hdr)"));
	while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

void MainWindow::createActions()
{
	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

	QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &MainWindow::open);
	openAct->setShortcut(QKeySequence::Open);

	fileMenu->addSeparator();

	QAction *exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
	exitAct->setShortcut(tr("Ctrl+Q"));

	QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

	QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

	viewMenu->addSeparator();

	QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
}

void MainWindow::valueUpdateCor(int value)
{
	sliceNum[CORONAL] = value;
	drawPlane(CORONAL);
}
void MainWindow::valueUpdateSag(int value)
{
	sliceNum[SAGITTAL] = value;
	drawPlane(SAGITTAL);
}
void MainWindow::valueUpdateAxi(int value)
{
	sliceNum[AXIAL] = value;
	drawPlane(AXIAL);
}