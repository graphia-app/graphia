=======================================
AXE 1.5 Community Edition
2011-2013 (c) GB Research, LLC
=======================================

=======================================
What's Included
=======================================

Installation package includes AXE library files, documentation, and license.

=======================================
Installing Windows XP/Vista/7/8 version
=======================================

Start the installer by launching setup.exe file and follow the instructions.

To uninstall the product, choose Uninstal from AXE program group.
Alternatively, click on the Start button, Control Panel, choose Add or 
Remove Programs, select the corresponding version of the software and click
Remove.

=======================================
Configuring compilers
=======================================

Current release of AXE requires C++11 compliant compiler. The library
was tested with Visual C++ 2010 compiler and gcc 4.7.2. 
In order to use AXE library in Visual C++ projects, a path to the library 
sources must be specified. In Visual C++ 2010 the library path can be specifies in 
Project/Properties/Configuration Properties/C/C++/Additional Include Directories.
C++ files which use AXE library should include axe.h header.
AXE is a header-only library, and thus it doesn't require linking.

=======================================
Known issues
=======================================

When using gcc version 4.6.x compiler, declaration auto rules in template functions causes 
conversion error, see:
http://stackoverflow.com/questions/6399363/can-auto-type-deduction-possibly-cause-conversion-error

To avoid this problem, an axe::r_rule<Iterator> type can be used instead of "auto".
This issue has been fixed in gcc-4.7.0

=======================================
Limitations
=======================================

See License Agreement for terms of use.

=======================================
Feedback
=======================================

Would you like to provide us with feedback, bug report, feature request, etc,
please use the form on "Contact Us" web page at 
http://www.gbresearch.com/contact_us.aspx
and specify AXE 1.5.3 Community Edition.
The above "Contact Us" form implements spam filter which may inadvertently 
filter out legitimate messages. If you believe your message was lost due to
filtering, please modify and re-post it.
