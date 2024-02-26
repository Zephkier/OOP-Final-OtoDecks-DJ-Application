#include <JuceHeader.h>
#include "PlaylistComponent.h"

PlaylistComponent::PlaylistComponent(juce::Colour _leftColour, juce::Colour _rightColour)
  : leftColour(_leftColour),
    rightColour(_rightColour)
{
  buttons.add(&importTrackButton);
  buttons.add(&importLibraryButton);
  buttons.add(&replaceLibraryButton);
  buttons.add(&exportLibraryButton);

  formatManager.registerBasicFormats();

  addAndMakeVisible(searchEditor);
  searchEditor.addListener(this);
  searchEditor.setTextToShowWhenEmpty("Search tracks", juce::Colours::white.withAlpha(0.5f));
  searchEditor.setJustification(juce::Justification::centredLeft);

  for (auto button : buttons)
  {
    addAndMakeVisible(button);
    button->addListener(this);
  }

  // Column width values are merely here for show, actual width values are set at PlaylistComponent::resized() below
  addAndMakeVisible(tableComponent);
  tableComponent.setModel(this);
  tableComponent.getHeader().addColumn("Track Title", 1, 20);
  tableComponent.getHeader().addColumn("Duration", 2, 10);
  tableComponent.getHeader().addColumn("Load into Left Deck", 3, 10);
  tableComponent.getHeader().addColumn("Load into Right Deck", 4, 10);
  tableComponent.getHeader().addColumn("Remove", 5, 10);

  // For persisting playlist (whether or not user wants to save/export library)
  readIncomingLibraryAndUpdateTable(juce::File::getCurrentWorkingDirectory().getChildFile("persisted-playlist.txt"));
}

PlaylistComponent::~PlaylistComponent()
{
  // For persisting playlist (whether or not user wants to save/export library)
  exportAndPersistCurrentPlaylist();

  tableComponent.setModel(nullptr);
}

void PlaylistComponent::paint(juce::Graphics& g)
{
  // Background
  g.fillAll(myBlackLight);

  // Buttons above tableComp
  searchEditor.setColour(juce::TextEditor::backgroundColourId, myBlack);
  for (auto button : buttons)
  {
    button->setColour(juce::TextButton::buttonColourId, myBlack);
    button->setConnectedEdges(juce::TextButton::ConnectedOnLeft | juce::TextButton::ConnectedOnRight);
  }

  // tableComp's background
  tableComponent.setColour(juce::TableListBox::backgroundColourId, myBlackLight);
  
  // tableComp's column headers
  tableComponent.getHeader().setColour(juce::TableHeaderComponent::backgroundColourId, myBlack);
  tableComponent.getHeader().setColour(juce::TableHeaderComponent::highlightColourId, myBlack);
  tableComponent.getHeader().setColour(juce::TableHeaderComponent::outlineColourId, myBlack);
  tableComponent.getHeader().setColour(juce::TableHeaderComponent::textColourId, juce::Colours::white);
}

void PlaylistComponent::resized()
{
  // Buttons above tableComp
  double buttonWidth = getWidth() / static_cast<double>(6);
  double buttonHeight = getHeight() / static_cast<double>(8);
  searchEditor.setBounds(0, 0, buttonWidth * 2, buttonHeight);
  importTrackButton.setBounds(searchEditor.getX() + searchEditor.getWidth(), 0, buttonWidth, buttonHeight);
  importLibraryButton.setBounds(importTrackButton.getX() + importTrackButton.getWidth(), 0, buttonWidth, buttonHeight);
  replaceLibraryButton.setBounds(importLibraryButton.getX() + importLibraryButton.getWidth(), 0, buttonWidth, buttonHeight);
  exportLibraryButton.setBounds(replaceLibraryButton.getX() + replaceLibraryButton.getWidth(), 0, buttonWidth, buttonHeight);

  // tableComp itself
  tableComponent.setBounds(0, searchEditor.getY() + searchEditor.getHeight(), getWidth(), (getHeight() - buttonHeight));
  double widthPart = getWidth() / static_cast<double>(6);
  double removeColWidth = widthPart / 2;
  double scrollbarWidth = 10; // This is a rough estimate due to being unable to get Juce's default scrollbar width
  tableComponent.getHeader().setColumnWidth(1, widthPart * 2);
  tableComponent.getHeader().setColumnWidth(2, widthPart * 1);
  tableComponent.getHeader().setColumnWidth(3, ((widthPart * 3) - removeColWidth) / 2);
  tableComponent.getHeader().setColumnWidth(4, ((widthPart * 3) - removeColWidth) / 2);
  tableComponent.getHeader().setColumnWidth(5, removeColWidth - scrollbarWidth);
}

int PlaylistComponent::getNumRows()
{
  return fileTitles.size();
}

void PlaylistComponent::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
  // Alternating row colours
  if (rowNumber % 2 == 0) g.fillAll(myBlackLighter);
  else g.fillAll(myBlackLightest);

  // Selected row has black background and white border
  if (rowIsSelected)
  {
    double scrollbarWidth = 10; // This is rough estimate due to being unable to get Juce's default scrollbar width
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.drawRect(0, 0, width - scrollbarWidth, height, 2);
  }
}

// For 'columnId' of 1 and 2
void PlaylistComponent::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
  // Row's font colour
  g.setColour(juce::Colours::white);

  // "Track Title" column
  if (columnId == 1) g.drawText(fileTitles[rowNumber], 2, 0, width - 4, height, juce::Justification::centredLeft, true);

  // "Duration" column
  if (columnId == 2) g.drawText(formatDoubleToMMSS(fileDurations[rowNumber]), 2, 0, width - 4, height, juce::Justification::centredLeft, true);
}

// For 'columnId' of 3, 4 and 5 (for buttons within tableComp, aka. 'Load' and 'X' button)
juce::Component* PlaylistComponent::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate)
{
  // "Load into Left Deck" column
  if (columnId == 3)
  {
    if (existingComponentToUpdate == nullptr)
    {
      // Setup
      juce::TextButton* btn = new juce::TextButton{ "Load" };
      btn->addListener(this);
      juce::String id{ std::to_string(rowNumber) };
      btn->setComponentID(id);
      
      // Set its look
      btn->setConnectedEdges(juce::TextButton::ConnectedOnRight);
      btn->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
      btn->setColour(juce::TextButton::textColourOffId, leftColour.brighter());

      // Load functionality (connected to MainComponent::timerCallback())
      btn->onClick = [this, rowNumber]()
        {
          if (rowNumber >= 0 && rowNumber < fileTitles.size())
          {
            deckNumber = 1;
            fileURL = fileURLs[rowNumber];
            loadToDeck = true;
          }
        };

      // End
      existingComponentToUpdate = btn;
    }
  }

  // "Load into Right Deck" column
  if (columnId == 4)
  {
    if (existingComponentToUpdate == nullptr)
    {
      // Setup
      juce::TextButton* btn = new juce::TextButton{ "Load" };
      btn->addListener(this);
      juce::String id{ std::to_string(rowNumber) };
      btn->setComponentID(id);

      // Set its look
      btn->setConnectedEdges(juce::TextButton::ConnectedOnLeft | juce::TextButton::ConnectedOnRight);
      btn->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
      btn->setColour(juce::TextButton::textColourOffId, rightColour.brighter());

      // Load functionality (connected to MainComponent::timerCallback())
      btn->onClick = [this, rowNumber]()
        {
          if (rowNumber >= 0 && rowNumber < fileTitles.size())
          {
            deckNumber = 2;
            fileURL = fileURLs[rowNumber];
            loadToDeck = true;
          }
        };

      // End
      existingComponentToUpdate = btn;
    }
  }
  
  // "Remove" column
  if (columnId == 5)
  {
    if (existingComponentToUpdate == nullptr)
    {
      // Setup
      juce::TextButton* btn = new juce::TextButton{ "X" };
      btn->addListener(this);
      juce::String id{ std::to_string(rowNumber) };
      btn->setComponentID(id);

      // Set its look
      btn->setConnectedEdges(juce::TextButton::ConnectedOnLeft);
      btn->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
      btn->setColour(juce::TextButton::textColourOffId, juce::Colours::red.brighter());

      // Remove functionality
      btn->onClick = [this, rowNumber]()
        {
          if (rowNumber >= 0 && rowNumber < fileTitles.size())
          {
            fileURLs.erase(fileURLs.begin() + rowNumber);
            fileTitles.erase(fileTitles.begin() + rowNumber);
            fileDurations.erase(fileDurations.begin() + rowNumber);

            // Store updated array after removing
            fileURLsCopy = fileURLs;
            fileTitlesCopy = fileTitles;
            fileDurationsCopy = fileDurations;

            tableComponent.updateContent();
          }
        };
      // End
      existingComponentToUpdate = btn;
    }
  }

  return existingComponentToUpdate;
}

// For buttons above tableComp
void PlaylistComponent::buttonClicked(juce::Button* button)
{
  if (button == &importTrackButton)
  {
    // Create juce::FileChooser, specify file types allowed
    juce::FileChooser importTrack{ "Add audio files to current library", juce::File(), "*.mp3,.*wav,*.aac,*.flac,*.ogg" };

    if (importTrack.browseForMultipleFilesToOpen())
    {
      // Get array of files
      juce::Array<juce::File> chosenFiles = importTrack.getResults();

      // Go through every file in array
      for (const auto& chosenFile : chosenFiles)
      {
        readIncomingFileAndUpdateTable(chosenFile);
      }
    }
  }

  if (button == &importLibraryButton || button == &replaceLibraryButton)
  {
    // Set browserMessage for juce::FileChooser
    juce::String browserMessage;
    if (button == &replaceLibraryButton) browserMessage = "Replace current library with incoming library";
    else browserMessage = "Add libraries to current library";

    // Create juce::FileChooser, specify file types allowed
    juce::FileChooser importer{ browserMessage, juce::File(), "*.txt" };

    // If user decides to import/replace library, then do the following
    if (importer.browseForMultipleFilesToOpen())
    {
      // If replacing, then backup and reset vectors
      if (button == &replaceLibraryButton)
      {
        // Save a backup copy (in case user clicks 'cancel')
        fileURLsCopy = fileURLs;
        fileTitlesCopy = fileTitles;
        fileDurationsCopy = fileDurations;
       
        // Clear file-related vectors to clear tableComp
        fileURLs.clear();
        fileTitles.clear();
        fileDurations.clear();

        // Refresh searchInput
        searchEditor.setText("");

        // Update table after clearing vector
        tableComponent.updateContent();
      }
      
      // To import library, get array of .txt files
      juce::Array<juce::File> chosenLibraries = importer.getResults();
      
      // Go through every .txt file in array
      for (const auto& chosenLibrary : chosenLibraries)
      {
        readIncomingLibraryAndUpdateTable(chosenLibrary);
      }
    }

    // Else if user clicks 'cancel' in file explorer window, then do nothing
  }

  if (button == &exportLibraryButton)
  {
    // Create juce::FileChooser, specify file types allowed
    juce::FileChooser exportLibrary{ "Save current library", juce::File(), "*.txt" };

    // If user clicks 'save' in file explorer window, then do the following
    if (exportLibrary.browseForFileToSave(false))
    {
      // Get path of where to save file
      juce::File exportPath = exportLibrary.getResult();

      // If file already exists, indicate to user
      if (exportPath.existsAsFile())
      {
        bool result = juce::AlertWindow::showOkCancelBox(
          juce::AlertWindow::WarningIcon,
          "File already exists",
          "Do you want to overwrite the existing file?",
          "Yes",   // Returns true  when clicked
          "Cancel" // Returns false when clicked
        );
        
        // Delete file as a way of "overwriting" content, then continue with function
        if (result) exportPath.deleteFile();
        else
        {
          // Indicate to user nothing happened, and exit out of function
          juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::NoIcon,
            "Export cancelled",
            ""
          );
          return;
        }
      }

      // ----- Exporting process ----- //
      // Create file output stream
      juce::FileOutputStream fileOutputStream(exportPath);
      
      if (fileOutputStream.openedOk())
      {
        // Loop through CURRENT fileURLs vector
        for (const auto& URL : fileURLs)
        {
          // Write to file with a line break
          fileOutputStream << juce::String(URL) << "\n";
        }
        // Indicate to user
        juce::AlertWindow::showMessageBoxAsync(
          juce::AlertWindow::InfoIcon,
          "Export successful",
          "Library exported to " + exportPath.getFullPathName()
        );
      }
    }

    // Else if user clicks 'cancel', then do nothing
  }
}

void PlaylistComponent::textEditorTextChanged(juce::TextEditor& editor)
{
  // Create another copy but for filtered items only
  std::vector<std::string> fileURLsFiltered;
  std::vector<std::string> fileTitlesFiltered;
  std::vector<double> fileDurationsFiltered;

  // Go through every track in array
  juce::String searchInput = searchEditor.getText().trim();
  for (int i = 0; i < fileTitlesCopy.size(); ++i)
  {
    // If there is input and substring matches, then populate filtered arrays
    if (searchInput.isNotEmpty() && juce::String(fileTitlesCopy[i]).containsIgnoreCase(searchInput))
    {
      fileURLsFiltered.push_back(fileURLsCopy[i]);
      fileTitlesFiltered.push_back(fileTitlesCopy[i]);
      fileDurationsFiltered.push_back(fileDurationsCopy[i]);
    }

    // Else do nothing
  }

  // If there is input, then show filter array's items only
  if (searchInput.isNotEmpty())
  {
    fileURLs = fileURLsFiltered;
    fileTitles = fileTitlesFiltered;
    fileDurations = fileDurationsFiltered;
  }

  // Else if there is NO input, then show original items (upon importing library)
  else
  {
    fileURLs = fileURLsCopy;
    fileTitles = fileTitlesCopy;
    fileDurations = fileDurationsCopy;
  }

  // Update table whenever text editor changes
  tableComponent.updateContent();
}

bool PlaylistComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
  return true;
}

void PlaylistComponent::filesDropped(const juce::StringArray& files, int x, int y)
{
  for (const auto& file : files)
  {
    // Although 'file' is already a URL path, convert it to a juce::File for simplicity's sake when reusing helper functions
    juce::File droppedFile(file);

    // If dropped files are audio files, then do the following
    bool cond1 = isIncomingFileOfValidType(droppedFile, { ".mp3", ".wav", ".aac", ".flac", ".ogg" });
    if (cond1) readIncomingFileAndUpdateTable(droppedFile);

    // If dropped files are library (.txt) files, then do the following
    bool cond2 = isIncomingFileOfValidType(droppedFile, { ".txt" });
    if (cond2) readIncomingLibraryAndUpdateTable(droppedFile);

    // Alert user if they drop a non-valid file
    if (!cond1 && !cond2)
    {
      juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::WarningIcon,
        "Unable to drop the file type: " + droppedFile.getFileExtension().toLowerCase(),
        "Only...\n\n - audio file types (eg. mp3, wav, aac, flac, ogg)\n - library file types (eg. txt)\n\nare allowed!"
      );
    }
  }
}

juce::String PlaylistComponent::formatDoubleToMMSS(double durationInSeconds)
{
  int min = static_cast<int>(durationInSeconds) / 60;
  int sec = static_cast<int>(durationInSeconds) % 60;
  return juce::String::formatted("%02d:%02d", min, sec);
}

void PlaylistComponent::readIncomingFileAndUpdateTable(juce::File incomingFile)
{
  // 1. Extract and push file path url to its vector (used in 'if (button == &exportLibraryButton)' to save current library)
  juce::String url = incomingFile.getFullPathName();
  fileURLs.push_back(url.toStdString());
  
  // 2. Extract and push file name to its vector (displayed in PlaylistComponent::paintCell())
  fileTitles.push_back(incomingFile.getFileNameWithoutExtension().toStdString());
  
  // 3. Create file reader to extract, calculate, and push file duration to its vector (displayed in PlaylistComponent::paintCell())
  juce::AudioFormatReader* reader = formatManager.createReaderFor(incomingFile);
  if (reader)
  {
    double duration = reader->lengthInSamples / reader->sampleRate;
    fileDurations.push_back(duration);
  }
  
  // Update backup copy (for PlaylistComponent::textEditorTextChanged() purposes below)
  fileURLsCopy = fileURLs;
  fileTitlesCopy = fileTitles;
  fileDurationsCopy = fileDurations;
  
  // Refresh searchInput (for PlaylistComponent::textEditorTextChanged() purposes below)
  searchEditor.setText("");

  // Update table after reading audio files
  tableComponent.updateContent();
}

void PlaylistComponent::readIncomingLibraryAndUpdateTable(juce::File incomingLibrary)
{
  // Set boolean for alert window
  bool skipAllErrors = false;

  // Create file input stream
  juce::FileInputStream fileInputStream(incomingLibrary);

  // Go through every line in .txt file
  while (!fileInputStream.isExhausted())
  {
    // Extract individual line from .txt file
    juce::String line = fileInputStream.readNextLine();

    // Store .txt file's line as juce::File
    juce::File incomingFile(line);

    // If file exists, then do the following (to populate vectors)
    if (incomingFile.exists())
    {
      // 1. Push individual line (since every line is a path URL anyway)
      fileURLs.push_back(line.toStdString());

      // 2. Store as juce::File to extract, and push file name
      fileTitles.push_back(incomingFile.getFileNameWithoutExtension().toStdString());

      // 3. Create file reader to extract, calculate, and push file duration
      juce::AudioFormatReader* reader = formatManager.createReaderFor(incomingFile);
      if (reader)
      {
        double duration = reader->lengthInSamples / reader->sampleRate;
        fileDurations.push_back(duration);
      }
    }
    // Else if file does not exist (eg. error in .txt file), then show alert window
    else
    {
      if (!skipAllErrors)
      {
        // Create alert window
        juce::AlertWindow alertWindow(
          "Error with incoming library!",
          "Audio file does not exist. Skipping this file:\n\n" + line,
          juce::AlertWindow::WarningIcon
        );

        // addButton(<string>, <return value>)
        alertWindow.addButton("Okay", 1);
        alertWindow.addButton("Okay to All", 2);

        // Store button's return value from alert window into 'result'
        int result = alertWindow.runModalLoop();

        // If "Okay to All", do not show alert windows anymore
        if (result == 2) skipAllErrors = true;
      }
    }
  }

  // After everything, update backup copy (for PlaylistComponent::textEditorTextChanged() purposes below)
  fileURLsCopy = fileURLs;
  fileTitlesCopy = fileTitles;
  fileDurationsCopy = fileDurations;
  
  // Refresh searchInput (for PlaylistComponent::textEditorTextChanged() purposes below)
  searchEditor.setText("");
  
  // Update table after reading .txt file
  tableComponent.updateContent();
}

bool PlaylistComponent::isIncomingFileOfValidType(const juce::File& incomingFile, juce::StringArray validFileTypes)
{
  juce::StringArray allowedTypes = validFileTypes;
  return allowedTypes.contains(incomingFile.getFileExtension().toLowerCase());
}

void PlaylistComponent::exportAndPersistCurrentPlaylist()
{
  // ----- Copy-pasted and reworked from PlaylistComponent::buttonClicked()'s 'exportLibraryButton' above ----- //

  // Save persisted playlist in current directory (1. easy reference and 2. so it works in different computers)
  juce::File persistedPlaylistPath = juce::File::getCurrentWorkingDirectory().getChildFile("persisted-playlist.txt");

  // Note that it is being saved in "Builds/VisualStudio2022" (relative to OtoDecks.jucer)!
  // DBG("\nHEY!\nPlaylistComponent::exportAndPersistCurrentPlaylist() says:\n'persisted-playlist.txt' is found at: " << persistedPlaylistPath.getFullPathName() << "\n");

  // (Always) delete file as a way of "overwriting" content
  persistedPlaylistPath.deleteFile();

  // Create file output stream
  juce::FileOutputStream fileOutputStream(persistedPlaylistPath);

  if (fileOutputStream.openedOk())
  {
    // Loop through CURRENT fileURLs vector
    for (const auto& URL : fileURLs)
    {
      // Write to file with a line break
      fileOutputStream << juce::String(URL) << "\n";
    }
  }
}
