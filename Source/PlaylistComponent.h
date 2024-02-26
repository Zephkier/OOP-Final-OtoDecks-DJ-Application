#pragma once
#include <JuceHeader.h>
#include <vector>
#include <string>

class PlaylistComponent : public juce::Component,
                          public juce::TableListBoxModel,
                          public juce::Button::Listener,
                          public juce::TextEditor::Listener,
                          public juce::FileDragAndDropTarget
{
public:
  PlaylistComponent(juce::Colour _leftColour,
                    juce::Colour _rightColour);
  ~PlaylistComponent() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  int getNumRows() override;
  void paintRowBackground(juce::Graphics &, int rowNumber, int width, int height, bool rowIsSelected) override;
  void paintCell(juce::Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
  juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;

  void buttonClicked(juce::Button* button) override;
  void textEditorTextChanged(juce::TextEditor& editor) override;

  // Made public to be accessed in MainComponent (for tableComp's 'Load' button to work)
  bool loadToDeck = false;
  int deckNumber = 1;
  std::string fileURL = "";

  bool isInterestedInFileDrag(const juce::StringArray& files) override;
  void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
  // ----- General variables ----- //
  juce::Array<juce::Button*> buttons;
  juce::Colour myBlack = juce::Colour::fromRGB(20, 20, 20);         // Used in buttons above tableComp
  juce::Colour myBlackLight = juce::Colour::fromRGB(40, 40, 40);    // Used in tableComp's background
  juce::Colour myBlackLighter = juce::Colour::fromRGB(60, 60, 60);  // Used in tableComp's alternating rows
  juce::Colour myBlackLightest = juce::Colour::fromRGB(80, 80, 80); // Used in tableComp's alternating rows
  juce::Colour leftColour;                                          // Used in tableComp's 'Load' button
  juce::Colour rightColour;                                         // Used in tableComp's 'Load' button

  // ----- Components ----- //
  juce::TextEditor searchEditor{ "Search for tracks" };
  juce::TextButton importTrackButton { "Add Track" };
  juce::TextButton importLibraryButton { "Add Library" };
  juce::TextButton replaceLibraryButton{ "Replace Library" };
  juce::TextButton exportLibraryButton { "Save Library" };
  juce::TableListBox tableComponent;

  // ----- For 'importTrackButton' to work ----- //
  juce::AudioFormatManager formatManager;
  std::vector<std::string> fileURLs;
  std::vector<std::string> fileTitles;
  std::vector<double> fileDurations;

  // A copy for original items (called when importing library for 'searchEditor' purposes)
  std::vector<std::string> fileURLsCopy;
  std::vector<std::string> fileTitlesCopy;
  std::vector<double> fileDurationsCopy;

  // ----- General helper and refactored functions ----- //
  juce::String formatDoubleToMMSS(double durationInSeconds);
  void readIncomingFileAndUpdateTable(juce::File incomingFile);
  void readIncomingLibraryAndUpdateTable(juce::File incomingLibrary);
  bool isIncomingFileOfValidType(const juce::File& incomingFile, juce::StringArray validFileTypes);

  // ----- For persisting playlist (whether or not user wants to save/export library) ----- //
  void exportAndPersistCurrentPlaylist();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
};
