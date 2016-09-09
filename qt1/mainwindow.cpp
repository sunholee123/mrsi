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
		plane[i]->setFixedWidth(300);
		plane[i]->setFixedHeight(300);
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

	// LCModel info layout
	lcmLayout = new QVBoxLayout;
	QLabel *lcmInfoTitle = new QLabel("<font color='black'>LCModel Info</font>");
	lcmInfoTitle->setFixedWidth(500);
	lcmInfoTitle->setAlignment(Qt::AlignCenter);
	lcmLayout->addWidget(lcmInfoTitle, 0, 0);
	// Chemical info presentation -- need to know number of chemicals
	mainLayout->addLayout(lcmLayout, 0, 2);	
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
	/*for (int i = 0; i < img->nx()*img->ny()*img->nz(); i++)
	{
		if (imgvol[i] > maxval)
			maxval = imgvol[i];
	}
	*/

	// find maximum value
	float maxval = 0;
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


void MainWindow::drawPlane(int planeType){
	int width, height;
	switch(planeType){
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
			switch (planeType){
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
	arr1Dto3D(img, t1image);

	//imgvol = img->readAllVolumes<float>();
	//arr1Dto3D(img->readAllVolumes<float>());

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

void MainWindow::openSlab() {
	QFileDialog dialog(this, tr("Open File"));
	dialog.setNameFilter(tr("Nifti files (*.nii.gz *.nii *.hdr)"));
	while (dialog.exec() == QDialog::Accepted && !loadSlab(dialog.selectedFiles().first())) {}
}

bool MainWindow::loadSlab(const QString &fileName) {
	string filename = fileName.toStdString();
	slab = new NiftiImage(filename, 'r');
	arr1Dto3D(slab, slabimage);

	overlaySlab(CORONAL);
	overlaySlab(SAGITTAL);
	overlaySlab(AXIAL);
	overlay = true;

	return true;
}

void MainWindow::overlaySlab(int planeType) {
	int width, height;
	switch (planeType) {
	case CORONAL:	width = slab->nx();	height = slab->nz();	break;
	case SAGITTAL:	width = slab->ny();	height = slab->nz();	break;
	case AXIAL:		width = slab->nx();	height = slab->ny();	break;
	}

	QImage base = plane[planeType]->pixmap()->toImage();
	QImage overlay(width, height, QImage::Format_ARGB32);
	QImage result(width, height, QImage::Format_ARGB32_Premultiplied);
	QPainter painter(&result);

	QRgb value;
	float val;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++)
		{
			switch (planeType) {
			case CORONAL: val = slabvol[i][sliceNum[planeType]][j]; break;
			case SAGITTAL: val = slabvol[sliceNum[planeType]][i][j]; break;
			case AXIAL: val = slabvol[i][j][sliceNum[planeType]]; break;
			}
			if (val == -1) { value = qRgba(0, 0, 0, 0); }
			else { value = qRgba(val*intensity, val*intensity, 0, 255); }
			overlay.setPixel(i, height - j, value);
		}
	}

	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(result.rect(), Qt::transparent);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.drawImage(0, 0, base);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.setOpacity(0.5);
	painter.drawImage(0, 0, overlay);
	painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
	painter.fillRect(result.rect(), Qt::white);
	painter.end();

	plane[planeType]->setPixmap(QPixmap::fromImage(result));
}

void MainWindow::createActions()
{
	QMenu *fileMenu = menuBar()->addMenu(tr("File"));

	QAction *openImgAct = fileMenu->addAction(tr("Open Image File"), this, &MainWindow::open);
	QAction *openDicomAct = fileMenu->addAction(tr("Open Dicom File"), this, &MainWindow::loadDicom);
	QAction *overlaySlabAct = fileMenu->addAction(tr("Overlay Slab"), this, &MainWindow::openSlab);

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
	if (overlay == true) { overlaySlab(CORONAL); }
}
void MainWindow::valueUpdateSag(int value)
{
	sliceNum[SAGITTAL] = value - 1;
	drawPlane(SAGITTAL);
	if (overlay == true) { overlaySlab(SAGITTAL); }
}
void MainWindow::valueUpdateAxi(int value)
{
	sliceNum[AXIAL] = value - 1;
	drawPlane(AXIAL);
	if (overlay == true) { overlaySlab(AXIAL); }
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
	//makeSlab();
}

void MainWindow::makeSlab() {
	float mrsiVoxelSizeX = 6.875;
	float mrsiVoxelSizeY = 6.875;
	float mrsiVoxelSizeZ = 15;

	int mrsiSlabNum = 3;
	int mrsiVoxelNumX = 32;
	int mrsiVoxelNumY = 32;

}

void MainWindow::arr1Dto3D(NiftiImage *image, int imageType) {
	const size_t dimX = image->nx();
	const size_t dimY = image->ny();
	const size_t dimZ = image->nz();

	float *array1D = image->readAllVolumes<float>();
	std::vector<std::vector<std::vector<float>>> imagevol;

	imagevol.resize(dimX);
	for (int i = 0; i < dimX; i++) {
		imagevol[i].resize(dimY);
		for (int j = 0; j < dimY; j++) {
			imagevol[i][j].resize(dimZ);
		}
	}

	for (int i = 0; i < dimX; i++) {
		for (int j = 0; j < dimY; j++) {
			for (int k = 0; k < dimZ; k++) {
				imagevol[i][j][k] = array1D[i + j*dimX + k*dimX*dimY];
			}
		}
	}

	switch (imageType) {
		case t1image: imgvol = imagevol; break;
		case slabimage: slabvol = imagevol; break;
	}
}
// build error - temporary comment out

vector<vector<vector<float>>> MainWindow::transformation3d(vector<vector<vector<float>>> imagevol, float cx, float cy, float cz, float ax, float ay, float az)
{
	const size_t dimX = imagevol.size();
	const size_t dimY = imagevol[0].size();
	const size_t dimZ = imagevol[0][0].size();

	Affine3f rx = Affine3f(AngleAxisf(deg2rad(ax), Vector3f(1, 0, 0)));
	Affine3f ry = Affine3f(AngleAxisf(deg2rad(ay), Vector3f(0, 1, 0)));
	Affine3f rz = Affine3f(AngleAxisf(deg2rad(az), Vector3f(0, 0, 1)));
	Affine3f r = rx * ry * rz;
	Affine3f t1(Translation3f(Vector3f(-round(dimX / 2), -round(dimY / 2), -round(dimZ / 2))));
	Affine3f t2(Translation3f(Vector3f(round(dimX/2), round(dimY/2), round(dimZ/2))));
	Matrix4f m = (t2 * r * t1).matrix();
	Matrix4f m_inv = m.inverse();

	std::vector<std::vector<std::vector<float>>> rotvol;
	rotvol = imagevol;

	MatrixXf imgcoord(4, 1);
	MatrixXf newcoord(4, 1);
	int x, y, z;
//	QTime myTimer;
//	myTimer.start();
	for (int i = 0; i < dimX; i++)
	{
		for (int j = 0; j < dimY; j++)
		{
			for (int k = 0; k < dimZ; k++)
			{
				newcoord << i, j, k, 1;
				imgcoord = m_inv * newcoord;
				x = round(imgcoord(0));
				y = round(imgcoord(1));
				z = round(imgcoord(2));
				if ((x < dimX) && (y < dimY) && (z < dimZ) && (x >= 0) && (y >= 0) && (z >= 0))
					rotvol[i][j][k] = imagevol[x][y][z];
				else
					rotvol[i][j][k] = 0;
			}
		}
	}
//	statusBar()->showMessage(QString::number(myTimer.elapsed()));
	return rotvol;
}

float MainWindow::deg2rad(float degree)
{
	return degree * M_PI / 180;
}

std::vector<std::vector<std::vector<float>>> MainWindow::makeDefaultSlab(std::vector<std::vector<std::vector<float>>> imagevol)
{
	//imgvol = transformation3d(imgvol, 0, 0, 0, 10, 10, 10);
}