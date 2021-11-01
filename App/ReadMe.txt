================================================================================
    MICROSOFT FOUNDATION CLASS LIBRARY : GuiTest3 Project Overview
===============================================================================

The application wizard has created this GuiTest3 application for
you.  This application not only demonstrates the basics of using the Microsoft
Foundation Classes but is also a starting point for writing your application.

This file contains a summary of what you will find in each of the files that
make up your GuiTest3 application.

GuiTest3.vcxproj
    This is the main project file for VC++ projects generated using an application wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    application wizard.

GuiTest3.vcxproj.filters
    This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the assciation between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

GuiTest3.h
    This is the main header file for the application.  It includes other
    project specific headers (including Resource.h) and declares the
    CGuiTest3App application class.

GuiTest3.cpp
    This is the main application source file that contains the application
    class CGuiTest3App.

GuiTest3.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Visual C++. Your project resources are in 1033.

res\GuiTest3.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file GuiTest3.rc.

res\GuiTest3.rc2
    This file contains resources that are not edited by Microsoft
    Visual C++. You should place all resources not editable by
    the resource editor in this file.

GuiTest3.reg
    This is an example .reg file that shows you the kind of registration
    settings the framework will set for you.  You can use this as a .reg
    file to go along with your application or just delete it and rely
    on the default RegisterShellFileTypes registration.


/////////////////////////////////////////////////////////////////////////////

For the main frame window:
    The project includes a standard MFC interface.

MainFrm.h, MainFrm.cpp
    These files contain the frame class CMainFrame, which is derived from
    CMDIFrameWnd and controls all MDI frame features.

res\Toolbar.bmp
    This bitmap file is used to create tiled images for the toolbar.
    The initial toolbar and status bar are constructed in the CMainFrame
    class. Edit this toolbar bitmap using the resource editor, and
    update the IDR_MAINFRAME TOOLBAR array in GuiTest3.rc to add
    toolbar buttons.
/////////////////////////////////////////////////////////////////////////////

For the child frame window:

ChildFrm.h, ChildFrm.cpp
    These files define and implement the CChildFrame class, which
    supports the child windows in an MDI application.

/////////////////////////////////////////////////////////////////////////////

The application wizard creates one document type and one view:

GuiTest3Doc.h, GuiTest3Doc.cpp - the document
    These files contain your CGuiTest3Doc class.  Edit these files to
    add your special document data and to implement file saving and loading
    (via CGuiTest3Doc::Serialize).
    The Document will have the following strings:
        File extension:      xml
        File type ID:        GuiTest3.Document
        Main frame caption:  GuiTest3
        Doc type name:       GuiTest3
        Filter name:         GuiTest3 Files (*.xml)
        File new short name: GuiTest3
        File type long name: GuiTest3.Document

GuiTest3View.h, GuiTest3View.cpp - the view of the document
    These files contain your CGuiTest3View class.
    CGuiTest3View objects are used to view CGuiTest3Doc objects.

res\GuiTest3Doc.ico
    This is an icon file, which is used as the icon for MDI child windows
    for the CGuiTest3Doc class.  This icon is included by the main
    resource file GuiTest3.rc.



/////////////////////////////////////////////////////////////////////////////

Help Support:

hlp\GuiTest3.hhp
    This file is a help project file. It contains the data needed to
    compile the help files into a .chm file.

hlp\GuiTest3.hhc
    This file lists the contents of the help project.

hlp\GuiTest3.hhk
    This file contains an index of the help topics.

hlp\afxcore.htm
    This file contains the standard help topics for standard MFC
    commands and screen objects. Add your own help topics to this file.

makehtmlhelp.bat
    This file is used by the build system to compile the help files.

hlp\Images\*.gif
    These are bitmap files required by the standard help file topics for
    Microsoft Foundation Class Library standard commands.


/////////////////////////////////////////////////////////////////////////////

Other Features:

ActiveX Controls
    The application includes support to use ActiveX controls.

/////////////////////////////////////////////////////////////////////////////

Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named GuiTest3.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

GuiTest3.manifest
	Application manifest files are used by Windows XP to describe an applications
	dependency on specific versions of Side-by-Side assemblies. The loader uses this
	information to load the appropriate assembly from the assembly cache or private
	from the application. The Application manifest  maybe included for redistribution
	as an external .manifest file that is installed in the same folder as the application
	executable or it may be included in the executable in the form of a resource.
/////////////////////////////////////////////////////////////////////////////

Other notes:

The application wizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

If your application uses MFC in a shared DLL, you will need
to redistribute the MFC DLLs. If your application is in a language
other than the operating system's locale, you will also have to
redistribute the corresponding localized resources MFC100XXX.DLL.
For more information on both of these topics, please see the section on
redistributing Visual C++ applications in MSDN documentation.

/////////////////////////////////////////////////////////////////////////////
