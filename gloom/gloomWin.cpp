/*
 *	Boom3D BeOS port v1.0 (c) Sasha Chukov (sash@tspu.edu.ru), 2000
 */
#include <OS.h>
#include "gloomWin.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "boom.h"
#include "player.h"
#include "draw.h"
#include "geom.h"

#define	WIN_WIDTH	320
#define WIN_HEIGHT	240

uint8 *frame_buffer;
int32 DrawingThread(void *data);

gloomWin::gloomWin(char *name_level) : BDirectWindow(BRect(100,100,100+WIN_WIDTH,100+WIN_HEIGHT),
						"beGloom", B_TITLED_WINDOW,
						B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
	fConnected = false;
	fConnectionDisabled = false;
	locker = new BLocker();
	fClipList = NULL;
	fNumClipRects = 0;

	g_width = WIN_WIDTH;
	g_height = WIN_HEIGHT;
//	mv = new gloomView(BRect(0,0,255, 255), "Gloom");
//	AddChild(mv);

	player_keys = 0;
	frame_buffer = new uint8[g_width * g_height];
	setscreensize(g_width, g_height);
	if (!map_init(name_level)) { printf("xyu!\n"); exit(-1); }

	if (!SupportsWindowMode()) {
		SetFullScreen(true);
	}
	
	fDirty = true;
	fDrawThreadID = spawn_thread(DrawingThread, "drawing thread",
					B_NORMAL_PRIORITY, (void *) this);
	resume_thread(fDrawThreadID);
	
	Show();

//	unsigned char *frame_buffer = mv->FrameBuffer();
//	SetPulseRate(250000);
//	mv->MakeFocus();
//	Show();
}

gloomWin::~gloomWin()
{
	int32 result;
	
	fConnectionDisabled = true;
	Hide();
	Sync();
	wait_for_thread(fDrawThreadID, &result);
	free(fClipList);
	delete locker;
}

bool
gloomWin::QuitRequested()
{
//	mv->mLeft();
//	mv->DrawGLScene();
//	mv->ExitThreads();
//	mv->EnableDirectMode( false );
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

void 
gloomWin::DirectConnected( direct_buffer_info *info )
{
	if (!fConnected && fConnectionDisabled) {
		return;
	}
	locker->Lock();
	switch(info->buffer_state & B_DIRECT_MODE_MASK) {
		case B_DIRECT_START:
			fConnected = true;
		case B_DIRECT_MODIFY:
			fBits = (uint8 *) info->bits;
			fRowBytes = info->bytes_per_row;
			fFormat = info->pixel_format;
			fBounds = info->window_bounds;
			fDirty = true;
			fBpp = info->bits_per_pixel / 8;
			
			if (fClipList) {
				free(fClipList);
				fClipList = NULL;
			}
			fNumClipRects = info->clip_list_count;
			fClipList = (clipping_rect *)
				malloc(fNumClipRects*sizeof(clipping_rect));
			memcpy(fClipList, info->clip_list,
				fNumClipRects*sizeof(clipping_rect));
			break;
		case B_DIRECT_STOP:
		fConnected = false;
		break;
	}
	locker->Unlock();
}

/*
 * MMX optimized linear converter CMAP_8 to native RGB_15
 */
 /*
void lineCpy(uint8 *dst, uint8 *src, uint32 l)
{
	asm(
	" cld\n\t"
	" shr	$1, %%ecx\n\t"
	" movq	m_b, %%mm4\n\t"
	" movq	m_g, %%mm5\n\t"
	" movq	m_r, %%mm6\n\t"
	"1:\n\t"
	" subl	%%eax, %%eax\n\t"
	" lodsb\n\t"
	" movl	%%eax, %%edx\n\t"
	" addl	%%eax, %%eax\n\t"
	" addl	%%edx, %%eax\n\t"

	" movd	palette(%%eax), %%mm0\n\t"
	" movq	%%mm0, %%mm1\n\t"
	" pand	%%mm4, %%mm1\n\t"
	" psrld	$19, %%mm1\n\t"
	" movq	%%mm0, %%mm2\n\t"
	" pand	%%mm5, %%mm2\n\t"
	" psrld	$5, %%mm2\n\t"
	" pand	%%mm6, %%mm0\n\t"
	" pslld	$8, %%mm0\n\t"
	" por	%%mm2, %%mm1\n\t"
	" por	%%mm1, %%mm0\n\t"
	
	" subl	%%eax, %%eax\n\t"
	" lodsb\n\t"
	" movl	%%eax, %%edx\n\t"
	" addl	%%eax, %%eax\n\t"
	" addl	%%edx, %%eax\n\t"

	" movd	palette(%%eax), %%mm3\n\t"
	" movq	%%mm3, %%mm1\n\t"
	" pand	%%mm4, %%mm1\n\t"
	" psrld	$3, %%mm1\n\t"
	" movq	%%mm3, %%mm2\n\t"
	" pand	%%mm5, %%mm2\n\t"
	" pslld	$11, %%mm2\n\t"
	" pand	%%mm6, %%mm3\n\t"
	" pslld	$24, %%mm3\n\t"
	" por	%%mm2, %%mm1\n\t"
	" por	%%mm1, %%mm3\n\t"
	" por	%%mm0, %%mm3\n\t"
	" movd	%%mm3, (%%edi)\n\t"

	" add	$4, %%edi\n\t"
	" loop	1b\n\t"
	" emms\n\t"
	:
	: "D"(dst), "S"(src), "c"(l)
	: "memory", "%eax", "%edi", "%edx", "%esi", "%ecx"
	);
}
*/

/*
 * Color masks for rgb24->rgb15 converter
 */
int m_r[] = { 0x0000f8, 0 };
int m_g[] = { 0x00f800, 0 };
int m_b[] = { 0xf80000, 0 };

int32
DrawingThread(void *data)
{
	uint32 count_ticks = system_time();
	gloomWin *w;
	w = (gloomWin *) data;
	while (!w->fConnectionDisabled){

//		count_ticks += 10;
		bool as = player_idle((system_time() - count_ticks) / 4000);
		draw_init(frame_buffer, w->g_width, w->g_height);
		map_draw();

		w->locker->Lock();
		if (w->fConnected) {
			if (/*w->fFormat == B_CMAP8 && */w->fDirty) {
				uint32 y;
				uint32 width;
				uint32 height;
				uint32 adder;
				uint8 *p, *r;
				clipping_rect *clip;
				uint32 i = 0;
				
				adder = w->fRowBytes;
//				for(i = 0; i<w->fNumClipRects; i++){
					clip = &(w->fClipList[i]);
					width = w->g_width;//(clip->right - clip->left)+1;
					height = (clip->bottom - clip->top)+1;
					p = w->fBits + (clip->top*w->fRowBytes)+clip->left*w->fBpp;
					r = frame_buffer;
					y = 0;
					while (y < w->g_height) {
						memcpy(p, r, width);//frame_buffer, 256*256);
//						lineCpy(p, r, width);//frame_buffer, 256*256);
						y++; p += adder; r += width;
					}
//				}
			}
//			w->fDirty = false;
		}
		w->locker->Unlock();
		snooze(16000);
	}
	return B_OK;
}

void 
gloomWin::MessageReceived(BMessage *message)
{
	int8		key_code;

	switch(message->what) {
	// Switch between full-screen mode and windowed mode.
//	case 'full' :
//		SetFullScreen(!IsFullScreen());
//		break;
	case B_KEY_DOWN :
//		if (!IsFullScreen())
//			break;
		if (message->FindInt8("byte", &key_code) != B_OK)
			break;
		switch( key_code ){
			case B_ESCAPE:		PostMessage(B_QUIT_REQUESTED); break;
			case B_UP_ARROW:	player_keys |= kFORWARD; break;
			case B_DOWN_ARROW:	player_keys |= kBACKWARD; break;
			case B_RIGHT_ARROW: player_keys |= kRIGHT; break;
			case B_LEFT_ARROW:	player_keys |= kLEFT; break;
			case ',':
			case '<':			player_keys |= kMOVELEFT; break;
			case '.':
			case '>':			player_keys |= kMOVERIGHT; break;
			case ' ':
			case 'd':
			case 'D':			player_keys |= kJUMP;		break;
			case 'c':
			case 'C':			player_keys |= kCROUCH;		break;
			case 'a':
			case 'A':			player_keys |= kLOOKUP;		break;
			case 'z':
			case 'Z':			player_keys |= kLOOKDOWN;	break;
			case B_ENTER:		player_keys |= kCENTER;		break;
			case '[':
			case '{':			player_keys |= kROTLEFT;	break;
			case ']':
			case '}':			player_keys |= kROTRIGHT;	break;
			case B_TAB:			player_keys |= kRUN;		break;
			default: break;
		}

		break;
	case B_KEY_UP :
//		if (!IsFullScreen())
//			break;
		if (message->FindInt8("byte", &key_code) != B_OK)
			break;
		switch( key_code ){
//			case B_ESCAPE:		PostMessage(B_QUIT_REQUESTED); break;
			case B_UP_ARROW:	player_keys &= ~kFORWARD; break;
			case B_DOWN_ARROW:	player_keys &= ~kBACKWARD; break;
			case B_RIGHT_ARROW: player_keys &= ~kRIGHT; break;
			case B_LEFT_ARROW:	player_keys &= ~kLEFT; break;
			case ',':
			case '<':			player_keys &= ~kMOVELEFT; break;
			case '.':
			case '>':			player_keys &= ~kMOVERIGHT; break;
			case ' ':
			case 'd':
			case 'D':			player_keys &= ~kJUMP;		break;
			case 'c':
			case 'C':			player_keys &= ~kCROUCH;	break;
			case 'a':
			case 'A':			player_keys &= ~kLOOKUP;	break;
			case 'z':
			case 'Z':			player_keys &= ~kLOOKDOWN;	break;
			case B_ENTER:		player_keys &= ~kCENTER;	break;
			case '[':
			case '{':			player_keys &= ~kROTLEFT;	break;
			case ']':
			case '}':			player_keys &= ~kROTRIGHT;	break;
			case B_TAB:			player_keys &= ~kRUN;		break;
			default: break;
		}

		break;
	default :
		BDirectWindow::MessageReceived(message);
		break;
	}
}
