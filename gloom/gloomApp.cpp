/*
 *	Boom3D BeOS port v1.0 (c) Sasha Chukov (sash@tspu.edu.ru), 2000
 */

#include <Entry.h>
#include <Path.h>
#include <string.h>
#include <stdio.h>
#include "gloomApp.h"
#include "gloomWin.h"

int main(int, char*)
{
	gloomApp *my_app = new gloomApp;
	my_app->Run();
	delete my_app;
	return 0;
}

gloomApp::gloomApp() : BApplication("application/x-vnd.beGloom")
{
	BEntry			entry;	
	BPath			path;
	app_info		info;

	char			buf[100];
	char*			appPath;
	char*			appLeaf;

	be_app->GetAppInfo(&info);
	entry.SetTo(&info.ref);
	entry.GetPath(&path);

	appPath = path.Path();
	appLeaf = path.Leaf();

	memset(buf, 0, 100);	
	memcpy(buf, appPath, (strlen(appPath)-strlen(appLeaf)) );

	strcpy(&buf[(strlen(appPath)-strlen(appLeaf))], "level.boo" );

//	printf("--%s\n", buf);
//	openPanel = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL,
//				B_FILE_NODE, false);
//	openPanel->Show();
//	BEntry entry("./level.boo");
//	BPath path1("./level.boo");
//	entry.GetPath(&path);
	gloomWin *wnd = new gloomWin(buf);
}

gloomApp::~gloomApp()
{
}

/*
void gloomApp::RefsReceived(BMessage *message)
{
	entry_ref 	ref;		// The entry_ref to open
	status_t 	err;		// The error code
	int32		ref_num;	// The index into the ref list

	ref_num = 0;
	if ((err = message->FindRef("refs", ref_num, &ref)) != B_OK) {
		return;
	}
	gloomWin *wnd = new gloomWin((char *)&ref);
}
 */