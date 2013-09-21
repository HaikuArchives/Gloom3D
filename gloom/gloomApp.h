/*
 *	Boom3D BeOS port v1.0 (c) Sasha Chukov (sash@tspu.edu.ru), 2000
 */
#ifndef _GLOOMAPP_H
#define _GLOOMAPP_H

#include <Application.h>
#include <FilePanel.h>
#include <Roster.h>

class gloomApp : public BApplication
{
 private:
	BFilePanel*		openPanel;
 public:
	gloomApp( void );
	~gloomApp( void );
//	virtual void	RefsReceived(BMessage *message);
};

#endif