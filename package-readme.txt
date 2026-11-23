# dcmedit

dcmedit is a DICOM editor for Windows, Linux and macOS.

## Qt

This app was created with Qt, which can be downloaded here https://github.com/qt.

## Troubleshooting

If an error is displayed about missing MSVCP140.dll when launching dcmedit, it means that
Microsoft Visual C++ Redistributable isn't installed. Download and install the latest version
here https://docs.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170.

## Opening .dcm files

On Windows, dcmedit registers itself as a program that can open .dcm files for the current
user when it is started. After dcmedit has been started once, files can be opened with:

- a double click, if no other program is associated with the .dcm extension yet,
- "Open with" in the context menu of the file,
- "Open with dcmedit" in the context menu of the file.

The registration is stored in the registry of the current user (no administrator rights are
needed) and it is refreshed every time dcmedit is started, so moving the application to
another folder is not a problem.

## Private tags

Information about how to use private tags and dictionaries can be found here:
https://support.dcmtk.org/docs/file_datadict.html
https://github.com/DCMTK/dcmtk/blob/master/dcmdata/data/private.dic

The basic steps are:
1. Create or find a text file that lists the private tags in the following format:
(gggg,"private creator name",eeee)	VR	TagName	VM	Private
2. Set the environment variable DCMDICTPATH to the path of the file.
