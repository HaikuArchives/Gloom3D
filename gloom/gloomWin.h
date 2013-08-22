/*
 *	Boom3D BeOS port v1.0 (c) Sasha Chukov (sash@tspu.edu.ru), 2000
 */
#ifndef _GLOOMWIN_H
#define _GLOOMWIN_H

#include <Application.h>
#include <DirectWindow.h>
#include <Locker.h>
#include <Message.h>

class gloomWin : public BDirectWindow {
 private:
 public:
	gloomWin(char *name_level);
	~gloomWin();
 	bool QuitRequested();
	virtual void DirectConnected( direct_buffer_info *info );
	virtual	void	MessageReceived(BMessage *message);

	uint32	g_width;
	uint32	g_height;

	uint8	*fBits;
	int32	fRowBytes;
	int32	fBpp;

	color_space		fFormat;
	clipping_rect	fBounds;
	
	uint32			fNumClipRects;
	clipping_rect	*fClipList;
	
	bool	fDirty;
	bool	fConnected;
	bool	fConnectionDisabled;
	BLocker	*locker;
	thread_id	fDrawThreadID;
};

#endif