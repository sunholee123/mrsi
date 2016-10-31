#include "MainWindow.h"

MainWindow::MainWindow()
{
	// main widget
	mainWidget = new QWidget();
	mainWidget->setBackgroundRole(QPalette::Dark);
	setCentralWidget(mainWidget);
	QHBoxLayout *mainLayout = new QHBoxLayout();
	mainWidget->setLayout(mainLayout);
	this->move(100, 50);

	for (int i = 0; i < 3; i++)
	{
		plane[i] = new QLabel();
		plane[i]->setFixedWidth(planeSize);
		plane[i]->setFixedHeight(planeSize);
		plane[i]->setAlignment(Qt::AlignHCenter);
		plane[i]->setStyleSheet("background-color: black;");
		plane[i]->installEventFilter(this);
	}
	
	viewerLayout = new QGridLayout;
	viewerLayout->addWidget(plane[CORONAL], 0, 0);
	viewerLayout->addWidget(plane[SAGITTAL], 0, 1);
	viewerLayout->addWidget(plane[AXIAL], 1, 0);
	 
	// spinboxes for controlling slices
	for (int i = 0; i < 3; i++)
	{
		sliceSpinBox[i] = new QSpinBox();
		sliceSpinBox[i]->setRange(1, 1);
		sliceNum[i] = 1;
		sliceSpinBox[i]->setValue(sliceNum[i]);
	}
	intensitySpinBox = new QSpinBox();
	intensitySpinBox->setRange(0, 9999);
	intensity = 0;
	intensitySpinBox->setValue(intensity);

	connect(sliceSpinBox[0], SIGNAL(valueChanged(int)), this, SLOT(valueUpdateCor(int)));
	connect(sliceSpinBox[1], SIGNAL(valueChanged(int)), this, SLOT(valueUpdateSag(int)));
	connect(sliceSpinBox[2], SIGNAL(valueChanged(int)), this, SLOT(valueUpdateAxi(int)));
	connect(intensitySpinBox, SIGNAL(valueChanged(int)), this, SLOT(valueUpdateIntensity(int)));

	// controllers layout
	ctrlLayout = new QGridLayout;
	sliceInfoText[AXIAL] = new QLabel("<font color='black'>Axial slice:</font>");
	sliceInfoText[SAGITTAL] = new QLabel("<font color='black'>Sagittal slice:</font>");
	sliceInfoText[CORONAL] = new QLabel("<font color ='black'>Coronal slice:</font>");
	int i;
	for (i = 0; i < 3; i++)
	{
		sliceInfoText[i]->setVisible(true);
		ctrlLayout->addWidget(sliceInfoText[i], i, 0);
		ctrlLayout->addWidget(sliceSpinBox[i], i, 1);
	}
	intensityText = new QLabel("<font color = 'black'>Intensity: </font>");
	ctrlLayout->addWidget(intensityText, i, 0);
	ctrlLayout->addWidget(intensitySpinBox, i, 1);
	
	viewerLayout->addLayout(ctrlLayout, 1, 1);
	createActions();

	// LCModel info layout
	lcmLayout = new QVBoxLayout;
	setLCMLayout();
	mainLayout->addLayout(viewerLayout);
	mainLayout->addLayout(lcmLayout);
}

MainWindow::~MainWindow()
{
	if (img != NULL)
	{
		img->close();
		delete img;
	}
	if (slab != NULL)
	{
		slab->close();
		delete slab;
	}
	if (!T1vol.empty())
		T1vol = vec3df();
	if (!slabvol.empty())
		slabvol = vec3df();
}

// not fully implemented : problem with height, add buttons etc.
void MainWindow::setLCMLayout() {
	if (metaList.isEmpty()) {
		lcmInfoBox = new QGroupBox(tr("MRSI Chemical Information"));
		lcmInfoBox->setAlignment(Qt::AlignHCenter);
		lcmInfoBox->setFixedWidth(300);

		QLabel *lcmMessage = new QLabel("<font color='black'>Please Load LCM process result files</font>");
		lcmMessage->setAlignment(Qt::AlignCenter);

		QVBoxLayout *vbox = new QVBoxLayout;
		vbox->addWidget(lcmMessage);
		lcmInfoBox->setLayout(vbox);

		lcmLayout->addWidget(lcmInfoBox);
	}
	else { // LCM info loaded
		lcmInfoBox->setHidden(true);
		
		QGroupBox *metabolitesBox = new QGroupBox(tr("select metabolites need to be analyzed"));
		metabolitesBox->setAlignment(Qt::AlignHCenter);
		metabolitesBox->setFixedWidth(300);
		
		QGridLayout *gbox = new QGridLayout;
		QButtonGroup *group = new QButtonGroup();
		group->setExclusive(false);
		int j = 0;
		for (int i = 0; i < metaList.size(); i++) {
			if (i % 2 == 0) {
				j += 1;
			}
			QCheckBox *metaBox = new QCheckBox();
			metaBox->setText(metaList[i]);
			gbox->addWidget(metaBox,j,i%2);
			group->addButton(metaBox);
		}
		connect(group, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(updateMetaChecked(QAbstractButton*)));

		QPushButton *calAvgConButton = new QPushButton("Calculate Avg. Conc.");
		gbox->addWidget(calAvgConButton, j + 1, 0);
		connect(calAvgConButton, SIGNAL(released()), this, SLOT(calAvgButtonClicked()));
		/*
		// partial volume correction button
		QPushButton *pvcButton = new QPushButton("Partial Vol. Corr.");
		gbox->addWidget(pvcButton, j + 1, 1);
		connect(pvcButton, SIGNAL(released()), this, SLOT(calPVC()));
		*/

		metabolitesBox->setLayout(gbox);

		lcmInfo = new QTextEdit;
		lcmInfo->setReadOnly(true);

		lcmLayout->addWidget(metabolitesBox);
		lcmLayout->addWidget(lcmInfo);
	}
}

void MainWindow::setEnabledT1DepMenus(bool enabled)
{
	overlaySlabAct->setEnabled(enabled);
	openSlabMaskAct->setEnabled(enabled);
	slabMenu->setEnabled(enabled);
}

/***** widget menu actions *****/
void MainWindow::createActions()
{
	QMenu *fileMenu = menuBar()->addMenu(tr("File"));
	slabMenu = menuBar()->addMenu(tr("Slab"));
	QMenu *helpMenu = menuBar()->addMenu(tr("Help"));

	QAction *openImgAct = fileMenu->addAction(tr("Open T1 Image"), this, &MainWindow::open);
	overlaySlabAct = fileMenu->addAction(tr("Overlay Slab"), this, &MainWindow::openSlab);
	openSlabMaskAct = fileMenu->addAction(tr("Overlay Slab Mask"), this, &MainWindow::openSlabMask);
	QAction *exitAct = fileMenu->addAction(tr("Exit"), this, &QWidget::close);
	
	QAction *openDicomAct = slabMenu->addAction(tr("Create Slab Image from DICOM files"), this, &MainWindow::loadDicom);
	QAction *makeSlabMaskAct = slabMenu->addAction(tr("Create Slab Mask Image"), this, &MainWindow::makeSlabMask);
	QAction *loadLCMInfoAct = slabMenu->addAction(tr("Load LCM Info"), this, &MainWindow::openLCM);
	QAction *loadSegImgsAct = slabMenu->addAction(tr("Load FSLVBM Segmented Images"), this, &MainWindow::loadSegImgs);
	
	// Some menus and actions are disabled until the T1 image is fully loaded
	setEnabledT1DepMenus(false);

	// future work: add help action
	fileMenu->addSeparator();
	slabMenu->addSeparator();
}

void MainWindow::open()
{
	QFileDialog dialog(this, tr("Open File"));
	dialog.setNameFilter(tr("Nifti files (*.nii.gz *.nii *.hdr)"));
	while (dialog.exec() == QDialog::Accepted && !loadImageFile(dialog.selectedFiles().first())) {}

	// if the slab file exists, then load it
	// future work: optimization (duplication of drawing parts)
	QFileInfo f(getSlabFileName());
	if (f.exists() && f.isFile())
		loadSlab(getSlabFileName());

	// if the lcm data file exists, then load it
	f = QFileInfo(getLCMFileName());
	if (f.exists() && f.isFile())
		readLCMData();
}

void MainWindow::openSlab() {
	QFileDialog dialog(this, tr("Open File"));
	dialog.setNameFilter(tr("Nifti files (*.nii.gz *.nii *.hdr)"));
	while (dialog.exec() == QDialog::Accepted && !loadSlab(dialog.selectedFiles().first())) {}
}

void MainWindow::openSlabMask() {
	QFileDialog dialog(this, tr("Open File"));
	dialog.setNameFilter(tr("Nifti files (*.nii.gz *.nii *.hdr)"));
	while (dialog.exec() == QDialog::Accepted && !loadSlabMask(dialog.selectedFiles().first())) {}
}

void MainWindow::loadDicom()
{
	QFileDialog dialog(this, tr("Select Directory"));
	dialog.setFileMode(QFileDialog::Directory);
	if (dialog.exec() == QDialog::Accepted) {
		if (findDicomFiles(dialog.selectedFiles().first()))
		{
			makeSlab();
			loadSlab(getSlabFileName());
		}
	}
}

void MainWindow::loadSegImgs()
{
	// get T1 directory name
	QFileInfo f(imgFileName);
	
	// load gm, wm, csf images
	QString gmFileName = f.absolutePath() + "/struc/" + f.baseName() + "_struc_GM.nii.gz";
	QString wmFileName = f.absolutePath() + "/struc/" + f.baseName() + "_struc_brain_pve_2.nii.gz";
	QString csfFileName = f.absolutePath() + "/struc/" + f.baseName() + "_struc_brain_pve_0.nii.gz";

	vec3df gmvol, wmvol, csfvol;
	NiftiImage gmimg = NiftiImage(gmFileName.toStdString(), 'r');
	NiftiImage wmimg = NiftiImage(wmFileName.toStdString(), 'r');
	NiftiImage csfimg = NiftiImage(csfFileName.toStdString(), 'r');
	gmvol = getImgvol(&gmimg);
	wmvol = getImgvol(&wmimg);
	csfvol = getImgvol(&csfimg);

	// calculate PVC value automatically
	calPVC(gmvol, wmvol, csfvol);
}

void MainWindow::makeSlabMask() {
	// future work: if no LCM data loaded, then popup message
	QDialog dialog(this);
	QFormLayout form(&dialog);

	form.addRow(new QLabel("<center>Values for Quality Check</center>"));

	QComboBox *metabolites = new QComboBox();
	metabolites->addItems(metaList);
	form.addRow("Metabolite", metabolites);

	QLineEdit *sdInput = new QLineEdit(&dialog);
	sdInput->setValidator(new QIntValidator(0, 100, this));
	sdInput->setText("20");
	form.addRow("SD(%)", sdInput);

	QLineEdit *fwhmInput = new QLineEdit(&dialog);
	fwhmInput->setValidator(new QDoubleValidator(0, 10, 2, this));
	fwhmInput->setText("0.2");
	form.addRow("FWHM", fwhmInput);

	QLineEdit *snrInput = new QLineEdit(&dialog);
	snrInput->setValidator(new QIntValidator(0, 10, this));
	form.addRow("SNR(optional)", snrInput);

	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
	form.addRow(&buttonBox);
	QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
	QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

	if (dialog.exec() == QDialog::Accepted) {
		string metabolite = metabolites->currentText().toStdString();
		int sd = sdInput->text().toInt();
		float fwhm = fwhmInput->text().toFloat();
		int snr = -1;
		if (!snrInput->text().isEmpty())
			snr = snrInput->text().toInt();

		voxelQualityCheck(metabolite, sd, fwhm, snr);
		saveSlabMask(metabolite);
	}
}

void MainWindow::openLCM() {
	QFileDialog dialog(this, tr("Select Directory"));
	dialog.setFileMode(QFileDialog::Directory);
	if (dialog.exec() == QDialog::Accepted && loadLCMInfo(dialog.selectedFiles().first()))
		saveLCMData();
}

/***** load T1 image *****/
bool MainWindow::loadImageFile(const QString &fileName)
{
	// load mri image
	if (overlay == true)
	{
		overlay = false;
//		delete slab;
	}
	if (img != NULL)
	{
		img->close();
		delete img;
	}

	imgFileName = fileName;
	string filename = fileName.toStdString();
	img = new NiftiImage(filename, 'r');
	T1vol = getImgvol(img);

	setDefaultIntensity();
	setSliceNum();

	drawPlane(CORONAL);
	drawPlane(SAGITTAL);
	drawPlane(AXIAL);

	const QString message = tr("Opened \"%1\\").arg(QDir::toNativeSeparators(fileName));
	statusBar()->showMessage(message);
	setEnabledT1DepMenus(true);

	return true;
}

void MainWindow::setDefaultIntensity()
{
	T1MaxVal = getMaxVal(T1vol);
	intensity = 300 / T1MaxVal;
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

vec3df MainWindow::getImgvol(NiftiImage *image) {
	const size_t dimX = image->nx();
	const size_t dimY = image->ny();
	const size_t dimZ = image->nz();

	float *array1D = image->readAllVolumes<float>();
	vec3df array3D;

	array3D.resize(dimX);
	for (int i = 0; i < dimX; i++) {
		array3D[i].resize(dimY);
		for (int j = 0; j < dimY; j++) {
			array3D[i][j].resize(dimZ);
		}
	}

	for (int i = 0; i < dimX; i++) {
		for (int j = 0; j < dimY; j++) {
			for (int k = 0; k < dimZ; k++) {
				array3D[i][j][k] = array1D[i + j*dimX + k*dimX*dimY];
			}
		}
	}
	delete array1D;
	return array3D;
}

/***** load Dicom image *****/
bool MainWindow::findDicomFiles(QString dir)
{
	QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);
	if (!it.hasNext())
	{
		QMessageBox::StandardButton msg;
		msg = QMessageBox::critical(this, "Error!", "Can't find DICOM files.", QMessageBox::Ok);
		return false;
	}

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

		status = fileformat.loadFile(it.next().toStdString().c_str());
		if (status.good()) {
			if (fileformat.getDataset()->findAndGetSequence(DcmTag(0x2001, 0x105f, "Philips Imaging DD 001"), sqi, false, false).good()
				&& fileformat.getDataset()->findAndGetOFString(DCM_SeriesDescription, seriesDescription).good()) {
				if (!T1flag && seriesDescription.compare("T1_SAG_MPRAGE_1mm_ISO") == 0)
				{
					item = sqi->getItem(0);
					item->findAndGetFloat32(DcmTag(0x2005, 0x1078), T1.coordAP, 0, false).good();
					item->findAndGetFloat32(DcmTag(0x2005, 0x1079), T1.coordFH, 0, false).good();
					item->findAndGetFloat32(DcmTag(0x2005, 0x107a), T1.coordRL, 0, false).good();
					item->findAndGetFloat32(DcmTag(0x2005, 0x1071), T1.angleAP, 0, false).good();
					item->findAndGetFloat32(DcmTag(0x2005, 0x1072), T1.angleFH, 0, false).good();
					item->findAndGetFloat32(DcmTag(0x2005, 0x1073), T1.angleRL, 0, false).good();
					T1flag = true;
				}
				else if (!MRSIflag && seriesDescription.compare("3SL_SECSI_TE19") == 0)
				{
					item = sqi->getItem(0);
					item->findAndGetFloat32(DcmTag(0x2005, 0x1078), MRSI.coordAP, 0, false).good();
					item->findAndGetFloat32(DcmTag(0x2005, 0x1079), MRSI.coordFH, 0, false).good();
					item->findAndGetFloat32(DcmTag(0x2005, 0x107a), MRSI.coordRL, 0, false).good();
					item->findAndGetFloat32(DcmTag(0x2005, 0x1071), MRSI.angleAP, 0, false).good();
					item->findAndGetFloat32(DcmTag(0x2005, 0x1072), MRSI.angleFH, 0, false).good();
					item->findAndGetFloat32(DcmTag(0x2005, 0x1073), MRSI.angleRL, 0, false).good();

					// mrsi voxel size, slab
					MRSIflag = true;
				}
				if (T1flag && MRSIflag)
					return true;
			}
		}
		else
			it.next();
	}
	return false;
}

/***** load Slab image *****/
bool MainWindow::loadSlab(const QString &fileName) {
	if (slab != NULL)
	{
		slab->close();
		delete slab;
	}

	string filename = fileName.toStdString();
	slab = new NiftiImage(filename, 'r');
	slabvol = getImgvol(slab);

	overlay = true;

	drawPlane(CORONAL);
	drawPlane(SAGITTAL);
	drawPlane(AXIAL);

	return true;
}

/***** load LCM info *****/
bool MainWindow::loadLCMInfo(QString dir)
{
	QStringList filepaths;
	QDirIterator it(dir, QStringList() << "*.table", QDir::Files, QDirIterator::Subdirectories);
	if (!it.hasNext())
	{
		QMessageBox::StandardButton msg;
		msg = QMessageBox::critical(this, "Error!", "Can't find *.table files.", QMessageBox::Ok);
		return false;
	}

	tables = new TableInfo**[3];
	for (int i = 0; i < 3; i++) {
		tables[i] = new TableInfo*[32];
		for (int j = 0; j < 32; j++) {
			tables[i][j] = new TableInfo[32];
		}
	}
		QTime myTimer;
		myTimer.start();
	while (it.hasNext())
	{
		it.next();
		string filename = it.fileName().toStdString();
		string filepath = it.filePath().toStdString();	// path + name
		size_t index1 = filename.find("_");
		size_t index2 = filename.find("-");
		size_t index3 = filename.find(".");
		int x = filename.at(index1 - 1) - '0';
		int y = stoi(filename.substr(index1 + 1, index2 - 1));
		int z = stoi(filename.substr(index2 + 1, index3 - 1));
		tables[x - 1][y - 1][z - 1] = parseTable(filepath);
	}
		statusBar()->showMessage(QString::number(myTimer.elapsed()));
	setLCMLayout();
	return true;
}

TableInfo MainWindow::parseTable(string filename) {
	TableInfo table;
	table.isAvailable = false;

	char line[255];
	ifstream myfile(filename);
	if (myfile.is_open()) {
		char* token = NULL;
		char s[] = " \t";
		while (myfile.getline(line, 255))
		{
			if (strstr(line, "Conc."))
			{
				while (myfile.getline(line, 255))
				{
					Metabolite metainfo;
					token = strtok(line, s);
					if (token == NULL)	// the last of metabolite parts
						break;
					metainfo.conc = stof(token);
					token = strtok(NULL, s);
					metainfo.sd = stoi(token);
					token = strtok(NULL, s);
					// exception check (1.7E+03+MM17, 0.000-MM17,...)
					QString tempstr = token;
					QStringList t;
					t = tempstr.split(QRegExp("[0-9][+-]"));
					if (t.length() == 2)
					{
						//metainfo.ratio = stof(t[0].toStdString());
						metainfo.ratio = stof(tempstr.left(t[0].length() + 1).toStdString());
						metainfo.name = t[1].toStdString();
					}
					else
					{
						metainfo.ratio = stof(token);
						token = strtok(NULL, s);
						metainfo.name = token;
					}
					metainfo.qc = true;
					table.metaInfo[metainfo.name] = metainfo;
					if (metaList.empty() || !metaList.contains(QString::fromStdString(metainfo.name))) { // To-do: call routine just once 
						metaList.push_back(QString::fromStdString(metainfo.name));
					}
				}
			}
			else if (strstr(line, "FWHM"))
			{
				token = strtok(line, "FWHM = ");
				table.fwhm = stof(token);
				token = strtok(NULL, " ppm    S/N =   ");
				table.snr = stoi(token);
				table.isAvailable = true;
			}
		}
		myfile.close();
	}
	return table;
}

void MainWindow::presentLCMInfo() {
	QString info_str;
	map<string, Metabolite>::iterator metaPos;

	coord abc = n2abc(selectedVoxel);
	TableInfo temp = tables[abc.a][abc.b][abc.c];
	
	info_str.append("<qt><style>.mytable{ border-collapse:collapse; }");
	info_str.append(".mytable th, .mytable td { border:5px solid black; }</style>");
	info_str.append("<table class=\"mytable\"><tr><th>Metabolite</th><th>Conc.</th><th>%SD</th><th>/Cr</th></tr>");
	for (metaPos = temp.metaInfo.begin(); metaPos != temp.metaInfo.end(); ++metaPos) {
		string s1 = "<tr><td>" + metaPos->first + "</td>";
		string s2 = "<td>" + to_string(metaPos->second.conc) + "</td>";
		string s3 = "<td>" + to_string(metaPos->second.sd) + "</td>";
		string s4 = "<td>" + to_string(metaPos->second.ratio) + "</td></tr>";
		info_str.append(QString::fromStdString(s1 + s2 + s3 + s4));
	}
	/*
	for (int i = 0; i < 35; i++) {
		string s1 = "<tr><td>" + tables[a - 1][b - 1][c - 1].metaInfo[i][3] + "</td>";
		string s2 = "<td>" + tables[a - 1][b - 1][c - 1].metaInfo[i][0] + "</td>";
		string s3 = "<td>" + tables[a - 1][b - 1][c - 1].metaInfo[i][1] + "</td>";
		string s4 = "<td>" + tables[a - 1][b - 1][c - 1].metaInfo[i][2] + "</td></tr>";
		info_str.append(QString::fromStdString(s1 + s2 + s3 + s4));
	}
	*/
	info_str.append("</table></qt>");
	
	lcmInfo->setText(QString::fromStdString("slab: " + to_string(abc.a +1) + ", " + to_string(abc.b+1) + ", " + to_string(abc.c+1) + " (" + to_string(selectedVoxel) + ")"));
	lcmInfo->append(info_str);
	lcmInfo->append("\n\nFWHM: " + QString::number(temp.fwhm));
	lcmInfo->append("SNR: " + QString::number(temp.snr));
}

/***** draw and update planes *****/
void MainWindow::drawPlane(int planeType){
	initImages(planeType, t1image);

	if (!maskvol.empty())	// overlay mask image
	{
		initImages(planeType, maskimage);
		overlayImage(T1Images[planeType], maskImages[planeType], planeType);
	}
	else if (overlay == true)	// overlay slab image
	{
		initImages(planeType, slabimage);
		overlayImage(T1Images[planeType], slabImages[planeType], planeType);
	}
	else	// T1 image only
		plane[planeType]->setPixmap(QPixmap::fromImage(T1Images[planeType].scaled(planeSize, planeSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
}

void MainWindow::overlayImage(QImage base, QImage overlay, int planeType) {
	QImage result(base.width(), base.height(), QImage::Format_ARGB32_Premultiplied);
	QPainter painter(&result);

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

	result = result.scaled(planeSize, planeSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	plane[planeType]->setPixmap(QPixmap::fromImage(result));
}

void MainWindow::initImages(int planeType, int imageType) {
	QRgb value;
	int width, height;
	float val;

	if (imageType == t1image) {
		switch (planeType) {
		case CORONAL: width = img->nx(); height = img->nz(); break;
		case SAGITTAL: width = img->ny(); height = img->nz(); break;
		case AXIAL:	width = img->nx(); height = img->ny(); break;
		}
	
		T1Images[planeType] = QImage(width, height, QImage::Format_RGB32);
		T1Images[planeType].fill(qRgb(0, 0, 0));
		 
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				switch (planeType) {
				case CORONAL: val = T1vol[i][sliceNum[planeType]][j]; break;
				case SAGITTAL: val = T1vol[sliceNum[planeType]][i][j]; break;
				case AXIAL: val = T1vol[i][j][sliceNum[planeType]]; break;
				}
				value = qRgb(val*intensity, val*intensity, val*intensity);
				T1Images[planeType].setPixel(i, height - j, value);
			}
		}
	}
	else if (imageType == slabimage) {
		switch (planeType) {
		case CORONAL: width = slab->nx(); height = slab->nz(); break;
		case SAGITTAL: width = slab->ny(); height = slab->nz(); break;
		case AXIAL:	width = slab->nx(); height = slab->ny(); break;
		}

		slabImages[planeType] = QImage(width, height, QImage::Format_ARGB32);
		slabImages[planeType].fill(qRgba(0, 0, 0, 255));

		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				switch (planeType) {
				case CORONAL: val = slabvol[i][sliceNum[planeType]][j]; break;
				case SAGITTAL: val = slabvol[sliceNum[planeType]][i][j]; break;
				case AXIAL: val = slabvol[i][j][sliceNum[planeType]]; break;
				}
				if (val == 0)
					value = qRgba(0, 0, 0, 0);
				else if (val == selectedVoxel)
					value = qRgba(val*intensity, 0, 0, 255);
				else
					value = qRgba(val*intensity, val*intensity, 0, 255);
				slabImages[planeType].setPixel(i, height - j, value);
			}
		}
	}
	else if (imageType == maskimage) {
		switch (planeType) {
		case CORONAL: width = mask->nx(); height = mask->nz(); break;
		case SAGITTAL: width = mask->ny(); height = mask->nz(); break;
		case AXIAL:	width = mask->nx(); height = mask->ny(); break;
		}

		maskImages[planeType] = QImage(width, height, QImage::Format_ARGB32);
		maskImages[planeType].fill(qRgba(0, 0, 0, 0));

		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				switch (planeType) {
				case CORONAL: val = maskvol[i][sliceNum[planeType]][j]; break;
				case SAGITTAL: val = maskvol[sliceNum[planeType]][i][j]; break;
				case AXIAL: val = maskvol[i][j][sliceNum[planeType]]; break;
				}

				if (val == 1) { value = qRgba(255,255,0,255); }
				else { value = qRgba(0, 0, 0, 0); }
				maskImages[planeType].setPixel(i, height - j, value);
			}
		}
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

void MainWindow::valueUpdateIntensity(int value)
{
	intensity = 300.0 /value;
	drawPlane(CORONAL);
	drawPlane(SAGITTAL);
	drawPlane(AXIAL);
}

/***** slab voxel picking *****/
bool MainWindow::eventFilter(QObject *watched, QEvent *e) {
	if (e->type() == QEvent::MouseButtonPress) {
		QMouseEvent *event = (QMouseEvent*)e;

		if (event->button() == Qt::LeftButton) {
			if (overlay) { // get mouse event only when slab overlayed
				if ((QLabel*)watched == plane[CORONAL] || (QLabel*)watched == plane[SAGITTAL] || (QLabel*)watched == plane[AXIAL]) { // and mouse event occured in plane
					int x = event->x();
					int y = event->y();
					int planeType;
					float val;

					if ((QLabel*)watched == plane[CORONAL]) { planeType = CORONAL; }
					else if ((QLabel*)watched == plane[SAGITTAL]) { planeType = SAGITTAL; }
					else { planeType = AXIAL; }

					int width1 = slabImages[planeType].width();
					int height1 = slabImages[planeType].height();
					int width2 = slabImages[planeType].scaled(planeSize, planeSize, Qt::KeepAspectRatio, Qt::SmoothTransformation).width();
					int height2 = slabImages[planeType].scaled(planeSize, planeSize, Qt::KeepAspectRatio, Qt::SmoothTransformation).height();
					int margin1 = 0;
					int margin2 = 0;

					if (width2 < planeSize) { margin1 = (planeSize - width2) / 2; }
					if (height2 < planeSize) { margin2 = (planeSize - height2) / 2; }

					int a = (x - margin1)*width1 / width2;
					int b = (y - margin2)*height1 / height2;
					if ((a > 0) && (a < width1) && (b > 0) && (b < height1))
					{
						val = getSlabVoxelValue(a, b, planeType);
						if (selectedVoxel == val)
							selectedVoxel = 0;
						else
							selectedVoxel = val;
					}
				}
			}
		}
	}
	else if (e->type() == QEvent::MouseButtonRelease) {
		if (overlay) { // get mouse event only when slab overlayed
			if ((QLabel*)watched == plane[CORONAL] || (QLabel*)watched == plane[SAGITTAL] || (QLabel*)watched == plane[AXIAL]) { // and mouse event occured in plane
				drawPlane(CORONAL);
				drawPlane(SAGITTAL);
				drawPlane(AXIAL);

				if (tables != NULL && selectedVoxel)
					presentLCMInfo();
			}
		}
	}
	return false;
}

float MainWindow::getSlabVoxelValue(int x, int y, int planeType) {
	int width = slabImages[planeType].width();
	int height = slabImages[planeType].height();
	float val;

	switch (planeType) {
	case CORONAL: val = slabvol[x][sliceNum[planeType]][height - y];  break;
	case SAGITTAL: val = slabvol[sliceNum[planeType]][x][height - y]; break;
	case AXIAL: val = slabvol[x][height - y][sliceNum[planeType]]; break;
	}

	return val;
}

void MainWindow::changeVoxelValues(float value, bool on) {
	if (value != -1) { // change value only for slab voxel
		float temp;

		for (int p = 0; p < 3; p++) { // update voxel value for every plane
			for (int i = 0; i < slabImages[p].width(); i++) {
				for (int j = 0; j < slabImages[p].height(); j++) {
					if (p == CORONAL)		{ temp = slabvol[i][sliceNum[p]][j]; }
					else if (p == SAGITTAL)	{ temp = slabvol[sliceNum[p]][i][j]; }
					else if (p == AXIAL)	{ temp = slabvol[i][j][sliceNum[p]]; }

					if (temp == value) {
						if (on == true) { slabImages[p].setPixelColor(i, slabImages[p].height() - j, qRgba(temp*intensity, 0, 0, 255)); }
						else { slabImages[p].setPixelColor(i, slabImages[p].height() - j, qRgba(temp*intensity, temp*intensity, 0, 255)); }
					}
				}
			}
		}
	}
}

/***** slab mask making (voxel quality check) *****/
bool MainWindow::loadSlabMask(const QString &fileName) {
	if (mask != NULL) { delete mask; }

	string filename = fileName.toStdString();
	mask = new NiftiImage(filename, 'r');
	maskvol = getImgvol(mask);

	if (overlay == false) { overlay = true; }
	
	drawPlane(CORONAL);
	drawPlane(SAGITTAL);
	drawPlane(AXIAL);

	return true;
}

void MainWindow::voxelQualityCheck(string metabolite, int sd, float fwhm, int snr) {
	if (sd == -1 || fwhm == -1) {
		// exception -- not available sd, fwhm values 
	}
	else if (tables == NULL) {
		// exception -- LCM info did not load
	}
	else {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 32; j++) {
				for (int k = 0; k < 32; k++) {
					if (tables[i][j][k].isAvailable)
					{
						map<string, Metabolite>::iterator tempPos;
						tempPos = tables[i][j][k].metaInfo.find(metabolite);
						if (tempPos != tables[i][j][k].metaInfo.end()) {
							if (tempPos->second.sd > sd || tables[i][j][k].fwhm > fwhm || tables[i][j][k].snr < snr)
								tempPos->second.qc = false;
							else
								tempPos->second.qc = true;
						}
						else {
							// exception -- metabolite not found
						}
					}
				}
			}
		}
		lcmInfo->append("slab table qc value all changed");
	}
}

void MainWindow::saveSlabMask(string metabolite) {
	if (slabvol.empty()) { lcmInfo->setText("slabvol is empty, cannot init mask image size"); }

	vec3df imagevol = slabvol;
	const size_t dimX = slabvol.size();
	const size_t dimY = slabvol[0].size();
	const size_t dimZ = slabvol[0][0].size();

	for (int i = 0; i < dimX; i++) {
		for (int j = 0; j < dimY; j++) {
			for (int k = 0; k < dimZ; k++) {
				if (slabvol[i][j][k] != 0)
				{
					coord abc = n2abc(slabvol[i][j][k]);
					if (tables[abc.a][abc.b][abc.c].fwhm == -1)
						imagevol[i][j][k] = 0;
					else
					{
						map<string, Metabolite>::iterator tempPos = tables[abc.a][abc.b][abc.c].metaInfo.find(metabolite);
						if (tempPos != tables[abc.a][abc.b][abc.c].metaInfo.end()) {
							if (tempPos->second.qc && tables[abc.a][abc.b][abc.c].isAvailable)
							{
								imagevol[i][j][k] = 1;
								//imagevol[i][j][k] = tempPos->second.conc;
							}
							else
								imagevol[i][j][k] = 0;
						}
					}
				}
			}
		}
	}
	
	saveImageFile(getMaskFileName(metabolite).toStdString(), img, imagevol);
	lcmInfo->append("save: slab mask image");
}

/***** statistics *****/
void MainWindow::updateMetaChecked(QAbstractButton* button) {
	if (button->isChecked()) {
		//lcmInfo->append(button->text());
		selMetaList.push_back(button->text());
	}
	else {
		//lcmInfo->append("unchecked");
		int index = selMetaList.indexOf(button->text(), 0);
		selMetaList.removeAt(index);
	}
}

void MainWindow::calAvgButtonClicked() {
	if (selMetaList.empty()) {
		lcmInfo->setText("Please select metabolites");
	}
	else {
		QString infotext = "Average concentration of selected metabolites\n";
		for (int i = 0; i < selMetaList.size(); i++) {
			string metabolite = selMetaList[i].toStdString();
			float avg = calAvgConc(metabolite);
			string text = metabolite + ": " + std::to_string(avg)+"\n";
			infotext.append(QString::fromStdString(text));			
		}
		lcmInfo->setText(infotext);
	}
}

float MainWindow::calAvgConc(string metabolite) {
	float sum = 0;
	int count = 0;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 32; j++) {
			for (int k = 0; k < 32; k++) {
				map<string, Metabolite>::iterator tempPos;
				tempPos = tables[i][j][k].metaInfo.find(metabolite);
				if (tempPos != tables[i][j][k].metaInfo.end()) {
					if (tempPos->second.qc && tables[i][j][k].isAvailable){ 
						count++; 
						sum += tempPos->second.conc * tables[i][j][k].pvc;
					}
				}
			}
		}
	}

	return (sum/count);
}

void MainWindow::calPVC(vec3df gmvol, vec3df wmvol, vec3df csfvol)
{
	struct segvals	{
		float gm, wm, csf;
	};
	// initialize gm, wm and csf values
	segvals ***s;
	s = new segvals**[3];
	for (int i = 0; i < 3; i++) {
		s[i] = new segvals*[32];
		for (int j = 0; j < 32; j++) {
			s[i][j] = new segvals[32];
			for (int k = 0; k < 32; k++) {
				s[i][j][k].gm = 0;
				s[i][j][k].wm = 0;
				s[i][j][k].csf = 0;
			}
		}
	}
	// get segmentation information by voxel
	const size_t dimX = slabvol.size();
	const size_t dimY = slabvol[0].size();
	const size_t dimZ = slabvol[0][0].size();

	for (int i = 0; i < dimX; i++) {
		for (int j = 0; j < dimY; j++) {
			for (int k = 0; k < dimZ; k++) {
				if (slabvol[i][j][k] != 0)
				{
					coord abc = n2abc(slabvol[i][j][k]);
					s[abc.a][abc.b][abc.c].gm += gmvol[i][j][k];
					s[abc.a][abc.b][abc.c].wm += wmvol[i][j][k];
					s[abc.a][abc.b][abc.c].csf += csfvol[i][j][k];
				}
			}
		}
	}
	// calculate volume fractions
	float mrsiVoxSizeX = 6.875;
	float mrsiVoxSizeY = 6.875;
	float mrsiVoxSizeZ = 15;
	float mrsiVoxVolume = mrsiVoxSizeX * mrsiVoxSizeY * mrsiVoxSizeZ;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 32; j++) {
			for (int k = 0; k < 32; k++) {
				float total = s[i][j][k].gm + s[i][j][k].wm + s[i][j][k].csf;
				// qc: pvc = 0 if an mrsi voxel is not included in the brain
				if (total < (mrsiVoxVolume * 0.8))	// 80%
				{
					tables[i][j][k].isAvailable = false;
					continue;
				}
				float f_gm = s[i][j][k].gm / total;
				float f_wm = s[i][j][k].wm / total;
				float f_csf = s[i][j][k].csf / total;
				// calculate partial volume corection values
				tables[i][j][k].pvc = (43300 * f_gm + 35880 * f_wm + 55556 * f_csf) / ((1 - f_csf) * 35880);
			}
		}
	}
}

vec3df MainWindow::transformation3d(vec3df imagevol, float coordAP, float coordFH, float coordRL, float angleAP, float angleFH, float angleRL)
{
	const size_t dimX = imagevol.size();
	const size_t dimY = imagevol[0].size();
	const size_t dimZ = imagevol[0][0].size();

	Affine3f rotRL = Affine3f(AngleAxisf(deg2rad(angleRL), Vector3f(-1, 0, 0))); // sagittal - clockwise
	Vector3f u1 = rotRL * Vector3f(0, 1, 0);
	Affine3f rotAP = Affine3f(AngleAxisf(deg2rad(angleAP), u1));	// coronal - clockwise
	Vector3f u2 = rotAP * Vector3f(0, 0, 1);
	Affine3f rotFH = Affine3f(AngleAxisf(deg2rad(angleFH), u2));	// axial - counterclockwise

	Affine3f r = rotFH * rotAP * rotRL;

	Affine3f t1(Translation3f(Vector3f(-round(dimX / 2), -round(dimY / 2), -round(dimZ / 2))));
	Affine3f t2(Translation3f(Vector3f(round(dimX / 2), round(dimY / 2), round(dimZ / 2))));

	// 1st param++ ==> slab moves to left
	// 2nd param++ ==> slab moves to front
	// 3rd param++ ==> slab moves to up
	Affine3f t3(Translation3f(Vector3f(coordRL, -coordAP, coordFH)));

	Matrix4f m = (t3 * t2 * r * t1).matrix();
	Matrix4f m_inv = m.inverse();

	vec3df rotvol;
	rotvol = imagevol;

	MatrixXf imgcoord(4, 1);
	MatrixXf newcoord(4, 1);
	int x, y, z;
//	QTime myTimer;
//	myTimer.start();
	for (int i = 0; i < dimX; i++) {
		for (int j = 0; j < dimY; j++) {
			for (int k = 0; k < dimZ; k++) {
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

void MainWindow::makeSlab()
{
/*
	code for reference
	sliceNumMax[SAGITTAL] = img->nx(); LR
	sliceNumMax[CORONAL] = img->ny(); AP
	sliceNumMax[AXIAL] = img->nz(); FH
*/
	float t1VoxSizeX = img->dx();
	float t1VoxSizeY = img->dy();
	float t1VoxSizeZ = img->dz();

	float mrsiVoxSizeX = 6.875;
	float mrsiVoxSizeY = 6.875;
	float mrsiVoxSizeZ = 15;

	int mrsiVoxNumX = 32;
	int mrsiVoxNumY = 32;
	int mrsiVoxNumZ = 3;

	float defSlabSizeX = mrsiVoxSizeX * mrsiVoxNumX / t1VoxSizeX;
	float defSlabSizeY = mrsiVoxSizeY * mrsiVoxNumY / t1VoxSizeY;
	float defSlabSizeZ = mrsiVoxSizeZ * mrsiVoxNumZ / t1VoxSizeZ;
	int maxLength = round(pow(pow(defSlabSizeX, 2) + pow(defSlabSizeY, 2) + pow(defSlabSizeZ, 2), 0.5));

	// slab image (full size)
	vec3df imagevol;
	imagevol.resize(maxLength);
	for (int i = 0; i < maxLength; i++) {
		imagevol[i].resize(maxLength);
		for (int j = 0; j < maxLength; j++) {
			imagevol[i][j].resize(maxLength);
		}
	}

	// fill default 
	int slabMid = maxLength / 2;
	int slabX = round(defSlabSizeX / 2);
	int slabY = round(defSlabSizeY / 2);
	int slabZ = round(defSlabSizeZ / 2);

	for (int i = 0; i < maxLength; i++) {
		for (int j = 0; j < maxLength; j++) {
			for (int k = 0; k < maxLength; k++)
			{
				imagevol[i][j][k] = 0;
			}
		}
	}
	int slabdiffX = slabMid - slabX;
	int slabdiffY = slabMid - slabY;
	int slabdiffZ = slabMid - slabZ;

	for (int i = slabdiffX; i < slabMid + slabX; i++) {
		for (int j = slabdiffY; j < slabMid + slabY; j++) {
			for (int k = slabdiffZ; k < slabMid + slabZ - 1; k++) // mrsiVoxNumZ�� Ȧ���� -1 �߰�...
			{
				int voxNumX = mrsiVoxNumX + 1 - round((i - slabdiffX) / mrsiVoxSizeX + 0.5); // left to right
				int voxNumY = mrsiVoxNumY + 1 - round((j - slabdiffY) / mrsiVoxSizeY + 0.5);
				int voxNumZ = round((k - slabdiffZ) / mrsiVoxSizeZ + 0.5); // bottom to top
				imagevol[i][j][k] = voxNumX + (voxNumY - 1) * mrsiVoxNumX + (voxNumZ - 1) * mrsiVoxNumX * mrsiVoxNumY;
			}
		}
	}
	
	// rotate and translate
	DicomInfo diff = MRSI - T1;
	imagevol = transformation3d(imagevol, diff.coordAP, diff.coordFH, diff.coordRL, diff.angleAP, diff.angleFH, diff.angleRL);
	
	// slab image (full size)
	vec3df slab;
	const size_t dimX = img->nx();
	const size_t dimY = img->ny();
	const size_t dimZ = img->nz();
	slab.resize(dimX);
	for (int i = 0; i < dimX; i++) {
		slab[i].resize(dimY);
		for (int j = 0; j < dimY; j++) {
			slab[i][j].resize(dimZ);
		}
	}

	// crop
	int diffX = slabMid - round(dimX/2);
	int diffY = slabMid - round(dimY/2);
	int diffZ = slabMid - round(dimZ/2);

	for (int i = 0; i < dimX; i++) {
		for (int j = 0; j < dimY; j++) {
			for (int k = 0; k < dimZ; k++)
			{
				slab[i][j][k] = imagevol[i + diffX][j + diffY][k + diffZ];
			}
		}
	}
	saveImageFile(getSlabFileName().toStdString(), img, slab);
}

float* MainWindow::arr3Dto1D(NiftiImage *image, vec3df imagevol) {
	const size_t dimX = imagevol.size();
	const size_t dimY = imagevol[0].size();
	const size_t dimZ = imagevol[0][0].size();

	float *array1D = image->readAllVolumes<float>();
	for (int i = 0; i < dimX; i++) {
		for (int j = 0; j < dimY; j++) {
			for (int k = 0; k < dimZ; k++) {
				array1D[i + j*dimX + k*dimX*dimY] = imagevol[i][j][k];
			}
		}
	}
	return array1D;
}

bool MainWindow::saveImageFile(string filename, NiftiImage *image, vec3df data)
{
	NiftiImage temp(*image);
	temp.open(filename, 'w');
	temp.writeAllVolumes<float>(arr3Dto1D(image, data));

	return true;
}

QString MainWindow::getSlabFileName()
{
	QFileInfo f(imgFileName);
	return (f.absolutePath() + "/" + f.baseName() + "_slab." + f.completeSuffix());
}

QString MainWindow::getMaskFileName(string metabolite)
{
	QFileInfo f(imgFileName);
	//return (f.absolutePath() + "/" + f.baseName() + "_mask." + f.completeSuffix()); // further work -- "_(metabolite name)"
	return (f.absolutePath() + "/" + f.baseName() + "_mask_" + QString::fromStdString(metabolite) + "." + f.completeSuffix());
}

float MainWindow::getMaxVal(vec3df imagevol)
{
	float maxval = 0;
	for (int i = 0; i < img->nx(); i++) {
		for (int j = 0; j < img->ny(); j++) {
			for (int k = 0; k < img->nz(); k++) {
				if (imagevol[i][j][k] > maxval) { maxval = imagevol[i][j][k]; }
			}
		}
	}
	return maxval;
}

coord MainWindow::n2abc(int n)
{
	coord temp;
	temp.a = (n - 1) / 1024;
	temp.b = fmod((n - 1), 1024) / 32;
	temp.c = fmod(fmod((n - 1), 1024), 32);
	return temp;
}

void MainWindow::saveLCMData()
{
	QFile file(getLCMFileName());
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::StandardButton msg;
		msg = QMessageBox::critical(this, "Error!", "LCM File Creation Failed.", QMessageBox::Ok);
		return;
	}
	QTextStream out(&file);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 32; j++) {
			for (int k = 0; k < 32; k++) {
				// need to optimize?
				out << (i + 1) << "\t" << (j + 1) << "\t" << (k + 1) << "\t";
				if (tables[i][j][k].isAvailable)
				{
					string str = "";
					map<string, Metabolite>::iterator metaPos;
					for (metaPos = tables[i][j][k].metaInfo.begin(); metaPos != tables[i][j][k].metaInfo.end(); metaPos++) {
						
						out << QString::fromStdString(metaPos->second.name) << "\t"
							<< metaPos->second.conc << "\t"
							<< metaPos->second.sd << "\t"
							<< metaPos->second.ratio << "\t";
					}
					out << tables[i][j][k].fwhm << "\t" << tables[i][j][k].snr << "\n";
				}
				else
				{
					out << "-1\n";
				}
			}
		}
	}
}

void MainWindow::readLCMData()
{
	QFile file(getLCMFileName());
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::StandardButton msg;
		msg = QMessageBox::critical(this, "Error!", "LCM File Open Failed.", QMessageBox::Ok);
		return;
	}
	QTextStream in(&file);

	if (tables == NULL)
	{
		tables = new TableInfo**[3];
		for (int i = 0; i < 3; i++) {
			tables[i] = new TableInfo*[32];
			for (int j = 0; j < 32; j++) {
				tables[i][j] = new TableInfo[32];
			}
		}
	}

	while (!in.atEnd())
	{
		QString tempstr = in.readLine();
		QStringList tokens;
		tokens = tempstr.split("\t");

		int a, b, c;
		a = tokens[0].toInt() - 1;
		b = tokens[1].toInt() - 1;
		c = tokens[2].toInt() - 1;

		if (tokens[3] == "-1")
			tables[a][b][c].isAvailable = false;
		else
		{
			tables[a][b][c].isAvailable = true;
			int tokenLen = tokens.length();
			int metaSize = (tokenLen - 3 - 2) / 4;
			for (int i = 0; i < metaSize; i++)
			{
				Metabolite metainfo;
				metainfo.name = tokens[3 + i * 4].toStdString();
				metainfo.conc = tokens[4 + i * 4].toFloat();
				metainfo.sd = tokens[5 + i * 4].toInt();
				metainfo.ratio = tokens[6 + i * 4].toFloat();
				metainfo.qc = true;
				tables[a][b][c].metaInfo[metainfo.name] = metainfo;
				if (metaList.empty() || !metaList.contains(QString::fromStdString(metainfo.name))) { // To-do: call routine just once
					metaList.push_back(QString::fromStdString(metainfo.name));
				}
			}
			tables[a][b][c].fwhm = tokens[tokenLen - 2].toFloat();
			tables[a][b][c].snr = tokens[tokenLen - 1].toInt();
		}
	}
	saveLCMData();	// test for equality
	setLCMLayout();
}

QString MainWindow::getLCMFileName()
{
	QFileInfo f(imgFileName);
	return f.absolutePath() + "/" + f.baseName() + ".lcm";
}