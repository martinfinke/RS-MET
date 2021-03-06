#include "rojue_ImageSavingDialog.h"
using namespace rojue;

//-------------------------------------------------------------------------------------------------
// construction/destruction:

ImageSavingDialog::ImageSavingDialog(CoordinateSystemOld *owner, int defaultWidth, int defaultHeight, 
                                     const String &defaultFormat, const File &defaultTargetFile)
{
  setSize(384, 52);
  ownerSystem = owner;

  int x, y, w, h;
  x = 0;
  y = 0;
  w = getWidth();
  h = getHeight();

  targetFileLabel = new Label(String(T("Target File")), String(T("File:")) );
  targetFileLabel->setBounds(4, 4, 32, 20);
  addAndMakeVisible(targetFileLabel);

  browseButton = new TextButton(String(T("Browse")));
  browseButton->setBounds(w-48-4, 4, 48, 20);
  browseButton->addListener(this);
  addAndMakeVisible(browseButton);

  x = targetFileLabel->getRight();
  w = browseButton->getX() - targetFileLabel->getRight();
  //targetFileEditLabel = new Label(String(T("Target File")), String::empty );
  targetFileEditLabel = new Label(String(T("Target File")), defaultTargetFile.getFullPathName() );
  targetFileEditLabel->setColour(Label::backgroundColourId, Colours::white);
  targetFileEditLabel->setColour(Label::outlineColourId, Colours::black);
  targetFileEditLabel->setEditable(true);
  targetFileEditLabel->setBounds(x+4, 4, w-8, 20);
  addAndMakeVisible(targetFileEditLabel);

  x = 0;
  y = targetFileEditLabel->getBottom();
  pixelWidthLabel = new Label(String(T("Pixel width")), String(T("Width:")) );
  pixelWidthLabel->setBounds(x+4, y+4, 48, 20);
  addAndMakeVisible(pixelWidthLabel);

  x = pixelWidthLabel->getRight();
  //pixelWidthEditLabel = new Label(String(T("Pixel width")), String(owner->getWidth()) );
  pixelWidthEditLabel = new Label(String(T("Pixel width")), String(defaultWidth) );
  pixelWidthEditLabel->setBounds(x+4, y+4, 48, 20);
  pixelWidthEditLabel->setColour(Label::backgroundColourId, Colours::white);
  pixelWidthEditLabel->setColour(Label::outlineColourId, Colours::black);
  pixelWidthEditLabel->setEditable(true);
  pixelWidthEditLabel->addListener(this);
  addAndMakeVisible(pixelWidthEditLabel);

  x = pixelWidthEditLabel->getRight();
  pixelHeightLabel = new Label(String(T("Pixel height")), String(T("Height:")) );
  pixelHeightLabel->setBounds(x+4, y+4, 52, 20);
  addAndMakeVisible(pixelHeightLabel);

  x = pixelHeightLabel->getRight();
  //pixelHeightEditLabel = new Label(String(T("Pixel height")), String(owner->getHeight()) );
  pixelHeightEditLabel = new Label(String(T("Pixel height")), String(defaultHeight) );
  pixelHeightEditLabel->setBounds(x+4, y+4, 48, 20);
  pixelHeightEditLabel->setColour(Label::backgroundColourId, Colours::white);
  pixelHeightEditLabel->setColour(Label::outlineColourId, Colours::black);
  pixelHeightEditLabel->setEditable(true);
  pixelHeightEditLabel->addListener(this);
  addAndMakeVisible(pixelHeightEditLabel);

  x = pixelHeightEditLabel->getRight();
  formatLabel = new Label(String(T("Format")), String(T("Format:")) );
  formatLabel->setBounds(x+4, y+4, 52, 20);
  addAndMakeVisible(formatLabel);

  x = formatLabel->getRight();
  w = browseButton->getX() - x;
  formatComboBox = new ComboBox(String(T("formatComboBox")));
  formatComboBox->setBounds(x+4, y+4, w-8, 20);
  formatComboBox->addItem(String(T("svg")),        1);
  formatComboBox->addItem(String(T("png")),        2);
  //formatComboBox->setSelectedId(2, true);
  if( defaultFormat == String(T("svg")) )
    formatComboBox->setSelectedId(1, true);
  else if( defaultFormat == String(T("png")) )
    formatComboBox->setSelectedId(2, true);
  else
    formatComboBox->setSelectedId(2, true);
  addAndMakeVisible(formatComboBox);

  saveButton = new TextButton(String(T("Save")));
  saveButton->setBounds(browseButton->getX(), browseButton->getY()+24, 48, 20);
  saveButton->addListener(this);
  addAndMakeVisible(saveButton);

  // initialize the target file field:
  File   thisExeAsFile         = File::getSpecialLocation(File::currentExecutableFile);
  File   thisDirectoryAsFile   = thisExeAsFile.getParentDirectory();
  String thisDirectoryAsString = thisDirectoryAsFile.getFullPathName();
  File   targetFile            = File(thisDirectoryAsString + String(T("/ExportedImage")) );
  if( defaultTargetFile != File::nonexistent )
    targetFile = defaultTargetFile;
  targetFileEditLabel->setText(targetFile.getFullPathName() , false);
}

ImageSavingDialog::~ImageSavingDialog()
{
  deleteAllChildren();
}

//-------------------------------------------------------------------------------------------------
// inquiry:

int ImageSavingDialog::getSelectedPixelWidth()
{
  int w = pixelWidthEditLabel->getText().getIntValue();
  if( w > 1 )
    return w;
  else
    return 1;
}

int ImageSavingDialog::getSelectedPixelHeight()
{
  int h = pixelHeightEditLabel->getText().getIntValue();
  if( h > 1 )
    return h;
  else
    return 1;
}

//-------------------------------------------------------------------------------------------------
// callbacks:

void ImageSavingDialog::buttonClicked(juce::Button *buttonThatWasClicked)
{
  if( buttonThatWasClicked == browseButton )
  {
    FileChooser chooser(String(T("Select target file")), 
      File(targetFileEditLabel->getText()), String::empty, false);
    if(chooser.browseForFileToSave(false))
    {
      File newTargetFile = chooser.getResult();
      targetFileEditLabel->setText(newTargetFile.getFullPathName() , false);
      saveNow();
    }
  }
  else if( buttonThatWasClicked == saveButton )
  {
    saveNow();
  }
}

void ImageSavingDialog::comboBoxChanged(juce::ComboBox *comboBoxThatHasChanged)
{

}

void ImageSavingDialog::labelTextChanged(juce::Label *labelThatHasChanged)
{

}

//-------------------------------------------------------------------------------------------------
// saving:

void ImageSavingDialog::saveNow()
{
  int w = getSelectedPixelWidth();
  int h = getSelectedPixelHeight();
  File fileToSaveTo = File(targetFileEditLabel->getText());

  if( formatComboBox->getText() == String(T("png")) )
  {
    // append the proper extension, if not already there:
    if( !fileToSaveTo.hasFileExtension(String(T("png"))) )
      fileToSaveTo = fileToSaveTo.withFileExtension(String(T("png")));

    // ask user for overwriting when the file already exists:
    if( fileToSaveTo.existsAsFile() )
    {
      bool overwrite = AlertWindow::showOkCancelBox(
        AlertWindow::WarningIcon, 
        String(T("Overwrite Warning")),                        
        String(T("File already exists. Overwrite?")),
        String(T("yes")),
        String(T("no"))  );

      if( overwrite )
        fileToSaveTo.deleteFile();
      else
        return;
    }

    Image* im = ownerSystem->getPlotAsImage(w, h);

    // create a PNGImagefileFormat object:
    PNGImageFormat pngFormat;

    // create the file output stream:
    FileOutputStream fileStream(fileToSaveTo);

    bool success = false;
    success = pngFormat.writeImageToStream(*im, fileStream);

    delete im;
  }
  else if( formatComboBox->getText() == String(T("svg")) )
  {
    // append the proper extension, if not already there:
    if( !fileToSaveTo.hasFileExtension(String(T("svg"))) )
      fileToSaveTo = fileToSaveTo.withFileExtension(String(T("svg")));

    // ask user for overwriting when the file already exists:
    if( fileToSaveTo.existsAsFile() )
    {
      bool overwrite = AlertWindow::showOkCancelBox(
        AlertWindow::WarningIcon, 
        String(T("Overwrite Warning")),                        
        String(T("File already exists. Overwrite?")),
        String(T("yes")),
        String(T("no"))  );

      if( overwrite )
        fileToSaveTo.deleteFile();
      else
        return;
    }

    // create a dummy-image - this has the side-effect to make the svg-drawing inside the 
    // CoordinatSystem to have the right dimensions - elegantize this someday...
    //const Image* dummyImage = ownerSystem->getPlotAsImage(w, h);
    //delete dummyImage;

    XmlElement* theSVG = ownerSystem->getPlotAsSVG(w, h);

    String myXmlDoc = theSVG->createDocument(String::empty);
    fileToSaveTo.create();
    fileToSaveTo.appendText(myXmlDoc);

    delete theSVG;
  }
}




