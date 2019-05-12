// UDP_Win_test.cpp : main project file.

#include "stdafx.h"
#include "Form1.h"

using namespace UDP_Win_test;

[STAThreadAttribute]
//int main(array<System::String ^> ^args)
int main(int argc, char* argv[])
{
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// Create the main window and run it
	Application::Run(gcnew Form1());
	return 0;
}
