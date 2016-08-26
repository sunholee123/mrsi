#include "MainWindow.h"
MainWindow::MainWindow()
{
	// main widget
	mainWidget = new QWidget();
	mainWidget->setBackgroundRole(QPalette::Dark);
	setCentralWidget(mainWidget);

	for (int i = 0; i < 3; i++)
	{
		plane[i] = new QLabel();
		plane[i]->setFixedWidth(500);
		plane[i]->setFixedHeight(500);
		//plane[i]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		plane[i]->setScaledContents(true);
	}
	
	mainLayout = new QGridLayout;
	mainWidget->setLayout(mainLayout);
	mainLayout->addWidget(plane[CORONAL], 0, 0);
	mainLayout->addWidget(plane[SAGITTAL], 0, 1);
	mainLayout->addWidget(plane[AXIAL], 1, 0);
	 
	// spinboxes for controlling slices
	for (int i = 0; i < 3; i++)
	{
		sliceSpinBox[i] = new QSpinBox();
		sliceSpinBox[i]->setRange(1, 1);
		sliceNum[i] = 1;
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
	/*if (imgvol != NULL)
		delete [] imgvol;*/
	if (!imgvol.empty())
		imgvol = std::vector<std::vector<std::vector<float>>>();
}

void MainWindow::setDefaultIntensity()
{
	// find maximum value
	float maxval = 0;
	/*for (int i = 0; i < img->nx()*img->ny()*img->nz(); i++)
	{
		if (imgvol[i] > maxval)
			maxval = imgvol[i];
	}
	*/

	for (int i = 0; i < img->nx(); i++) {
		for (int j = 0; j < img->ny(); j++) {
			for (int k = 0; k < img->nz(); k++) {
				if (imgvol[i][j][k] > maxval) { maxval = imgvol[i][j][k]; }
			}
		}
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
			/*case CORONAL:	val = imgvol[i + (j * width * height) + (width * sliceNum[planeType])];	break;
			case SAGITTAL:	val = imgvol[(i * img->nx()) + (j * img->nx() * height) + sliceNum[planeType]];	break;
			case AXIAL:		val = imgvol[i + (j * width) + (width * height * sliceNum[planeType])];	break;*/
			case CORONAL: val = imgvol[i][sliceNum[planeType]][j]; break;
			case SAGITTAL: val = imgvol[sliceNum[planeType]][i][j]; break;
			case AXIAL: val = imgvol[i][j][sliceNum[planeType]]; break;
			}
			val = val * intensity;
			value = qRgb(val, val, val);
			slice.setPixel(i, height-j, value);
		}
	plane[planeType]->setPixmap(QPixmap::fromImage(slice));
}

bool MainWindow::loadImageFile(const QString &fileName)
{
	// load mri image
	string filename = fileName.toStdString();
	
	img = new NiftiImage(filename, 'r');
	//imgvol = img->readAllVolumes<float>();
	arr1Dto3D(img->readAllVolumes<float>());
	
	setDefaultIntensity();
	setSliceNum();
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
	while (dialog.exec() == QDialog::Accepted && !loadImageFile(dialog.selectedFiles().first())) {}
}

void MainWindow::loadDicom()
{
	findDicomFiles();
}

void MainWindow::createActions()
{
	QMenu *fileMenu = menuBar()->addMenu(tr("File"));

	QAction *openImgAct = fileMenu->addAction(tr("Open Image File"), this, &MainWindow::open);
	QAction *openDicomAct = fileMenu->addAction(tr("Open Dicom File"), this, &MainWindow::loadDicom);
	fileMenu->addSeparator();
	QAction *exitAct = fileMenu->addAction(tr("Exit"), this, &QWidget::close);
}

void MainWindow::setSliceNum()
{
	int sliceNumMax[3];
	sliceNumMax[CORONAL] = img->ny();
	sliceNumMax[SAGITTAL] = img->nx();
	sliceNumMax[AXIAL] = img->nz();

	for (int i = 0; i < 3; i++)
	{
		sliceNum[i] = sliceNumMax[i] / 2;
		sliceSpinBox[i]->setRange(1, sliceNumMax[i]);
		sliceSpinBox[i]->setValue(sliceNum[i]);
	}
}

void MainWindow::valueUpdateCor(int value)
{
	sliceNum[CORONAL] = value - 1;
	drawPlane(CORONAL);
}
void MainWindow::valueUpdateSag(int value)
{
	sliceNum[SAGITTAL] = value - 1;
	drawPlane(SAGITTAL);
}
void MainWindow::valueUpdateAxi(int value)
{
	sliceNum[AXIAL] = value - 1;
	drawPlane(AXIAL);
}

void MainWindow::findDicomFiles()
{
	QString dir = "C:/New folder/20140212_CON000781_KSE_fu";
	QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);

	DcmFileFormat fileformat;
	OFCondition status;
	OFString seriesDescription;
	DcmSequenceOfItems *sqi;
	DcmItem *item;
	bool T1flag = false;
	bool MRSIflag = false;
	int counter = 0;
	while (it.hasNext())
	{
		counter++;
		if (counter % 100 == 0)
		{
			status = fileformat.loadFile(it.next().toStdString().c_str());
			if (status.good()) {
				if (fileformat.getDataset()->findAndGetSequence(DcmTag(0x2001, 0x105f, "Philips Imaging DD 001"), sqi, false, false).good()
					&& fileformat.getDataset()->findAndGetOFString(DCM_SeriesDescription, seriesDescription).good()) {
					if (!T1flag && seriesDescription.compare("T1_SAG_MPRAGE_1mm_ISO") == 0)
					{
						item = sqi->getItem(0);
						item->findAndGetFloat32(DcmTag(0x2005, 0x1078), T1.coordX, 0, false).good();
						item->findAndGetFloat32(DcmTag(0x2005, 0x1079), T1.coordY, 0, false).good();
						item->findAndGetFloat32(DcmTag(0x2005, 0x107a), T1.coordZ, 0, false).good();
						item->findAndGetFloat32(DcmTag(0x2005, 0x1071), T1.angleX, 0, false).good();
						item->findAndGetFloat32(DcmTag(0x2005, 0x1072), T1.angleY, 0, false).good();
						item->findAndGetFloat32(DcmTag(0x2005, 0x1073), T1.angleZ, 0, false).good();
						T1flag = true;
					}
					else if (!MRSIflag && seriesDescription.compare("3SL_SECSI_TE19") == 0)
					{
						item = sqi->getItem(0);
						item->findAndGetFloat32(DcmTag(0x2005, 0x1078), MRSI.coordX, 0, false).good();
						item->findAndGetFloat32(DcmTag(0x2005, 0x1079), MRSI.coordY, 0, false).good();
						item->findAndGetFloat32(DcmTag(0x2005, 0x107a), MRSI.coordZ, 0, false).good();
						item->findAndGetFloat32(DcmTag(0x2005, 0x1071), MRSI.angleX, 0, false).good();
						item->findAndGetFloat32(DcmTag(0x2005, 0x1072), MRSI.angleY, 0, false).good();
						item->findAndGetFloat32(DcmTag(0x2005, 0x1073), MRSI.angleZ, 0, false).good();

						// mrsi voxel size, slab
						// 
						MRSIflag = true;
					}
					if (T1flag && MRSIflag)
						break;
				}
			}
		}
		else
			it.next();

	}
	makeSlab();
}

void MainWindow::makeSlab()
{
	float mrsiVoxelSizeX = 6.875;
	float mrsiVoxelSizeY = 6.875;
	float mrsiVoxelSizeZ = 15;

	int mrsiSlabNum = 3;
	int mrsiVoxelNumX = 32;
	int mrsiVoxelNumY = 32;

	MatrixXf a(img->nx(), img->ny(), img->nz());
	for (int i = 0; i < img->nx; i++)
		for (int j = 0; j < img->ny; j++)
			for (int k = 0; k < img->nz; k++)
				a(i, j, k) = img[i][j][k];
	
	

	float ax = 0.1;
	float ay = 0.1;
	float az = 0.1;
	Affine3d rx = Affine3d(AngleAxisd(ax, Vector3d(1, 0, 0)));
	Affine3d ry = Affine3d(AngleAxisd(ay, Vector3d(0, 1, 0)));
	Affine3d rz = Affine3d(AngleAxisd(az, Vector3d(0, 0, 1)));
	Affine3d r = rx * ry * rz;
	Affine3d t(Translation3d(Vector3d(1, 1, 1)));
	Matrix4d m = (t * r).matrix();
	
	MatrixXf asdf = a*m;

}

void MainWindow::arr1Dto3D(float* array1D) {
	const size_t dimX = img->nx();
	const size_t dimY = img->ny();
	const size_t dimZ = img->nz();

	imgvol.resize(dimX);
	for (int i = 0; i < dimX; i++) {
		imgvol[i].resize(dimY);
		for (int j = 0; j < dimY; j++) {
			imgvol[i][j].resize(dimZ);
		}
	}

	for (int i = 0; i < dimX; i++) {
		for (int j = 0; j < dimY; j++) {
			for (int k = 0; k < dimZ; k++) {
				imgvol[i][j][k] = array1D[i + j*dimX + k*dimX*dimY];
			}
		}
	}
}

vector<vector<vector<float>>> MainWindow::rotation3d(vector<vector<vector<float>>> imgvol, float rx, float ry, float rz)
{
	vector<vector<vector<float>>> imgvol_new = imgvol;
	imgvol_new.clear();

	for (int i = 0; i < img->nx; i++)
	for (int j = 0; j < img->ny; j++)
	for (int k = 0; k < img->nz; k++)



	

}