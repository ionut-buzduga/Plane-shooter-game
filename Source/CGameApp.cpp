//-----------------------------------------------------------------------------
// File: CGameApp.cpp
//
// Desc: Game Application class, this is the central hub for all app processing
//
// Original design by Adam Hoult & Gary Simmons. Modified by Mihai Popescu.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CGameApp Specific Includes
//-----------------------------------------------------------------------------
#include "CGameApp.h"
#include <fstream>
#include <list>
#include <iterator>
#include <algorithm>
#include <ctime>
extern HINSTANCE g_hInst;

//-----------------------------------------------------------------------------
// CGameApp Member Functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : CGameApp () (Constructor)
// Desc : CGameApp Class Constructor
//-----------------------------------------------------------------------------
CGameApp::CGameApp()
{
	// Reset / Clear all required values
	m_hWnd			= NULL;
	m_hIcon			= NULL;
	m_hMenu			= NULL;
	m_pBBuffer		= NULL;
	m_pPlayer		= NULL;
	m_pPlayer2      = NULL;
	m_cCrate = NULL;
	m_cCrate1 = NULL;
	m_pEnemy = NULL;
	m_pHeart = NULL;
	m_LastFrameRate = 0;
}

//-----------------------------------------------------------------------------
// Name : ~CGameApp () (Destructor)
// Desc : CGameApp Class Destructor
//-----------------------------------------------------------------------------
CGameApp::~CGameApp()
{
	// Shut the engine down
	ShutDown();
}

//-----------------------------------------------------------------------------
// Name : InitInstance ()
// Desc : Initialises the entire Engine here.
//-----------------------------------------------------------------------------
bool CGameApp::InitInstance( LPCTSTR lpCmdLine, int iCmdShow )
{
	// Create the primary display device
	if (!CreateDisplay()) { ShutDown(); return false; }

	// Build Objects
	if (!BuildObjects()) 
	{ 
		MessageBox( 0, _T("Failed to initialize properly. Reinstalling the application may solve this problem.\nIf the problem persists, please contact technical support."), _T("Fatal Error"), MB_OK | MB_ICONSTOP);
		ShutDown(); 
		return false; 
	}

	// Set up all required game states
	SetupGameState();

	// Success!
	return true;
}

//-----------------------------------------------------------------------------
// Name : CreateDisplay ()
// Desc : Create the display windows, devices etc, ready for rendering.
//-----------------------------------------------------------------------------
bool CGameApp::CreateDisplay()
{
	LPTSTR			WindowTitle		= _T("GameFramework");
	LPCSTR			WindowClass		= _T("GameFramework_Class");
	USHORT			Width			= 800;
	USHORT			Height			= 600;
	RECT			rc;
	WNDCLASSEX		wcex;


	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= CGameApp::StaticWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= g_hInst;
	wcex.hIcon			= LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= WindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON));

	if(RegisterClassEx(&wcex)==0)
		return false;

	// Retrieve the final client size of the window
	::GetClientRect( m_hWnd, &rc );
	m_nViewX		= rc.left;
	m_nViewY		= rc.top;
	m_nViewWidth	= rc.right - rc.left;
	m_nViewHeight	= rc.bottom - rc.top;

	m_hWnd = CreateWindow(WindowClass, WindowTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, Width, Height, NULL, NULL, g_hInst, this);

	if (!m_hWnd)
		return false;

	// Show the window
	ShowWindow(m_hWnd, SW_SHOW);
	
	// Success!!
	return true;
}

//-----------------------------------------------------------------------------
// Name : BeginGame ()
// Desc : Signals the beginning of the physical post-initialisation stage.
//		From here on, the game engine has control over processing.
//-----------------------------------------------------------------------------
int CGameApp::BeginGame()
{
	MSG		msg;
	
	// Start main loop
	while(true) 
	{
		// Did we recieve a message, or are we idling ?
		if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) 
		{
			if (msg.message == WM_QUIT) break;
			TranslateMessage( &msg );
			DispatchMessage ( &msg );
		} 
		else 
		{
			// Advance Game Frame.
			FrameAdvance();

		} // End If messages waiting
	
	} // Until quit message is receieved

	return 0;
}

//-----------------------------------------------------------------------------
// Name : ShutDown ()
// Desc : Shuts down the game engine, and frees up all resources.
//-----------------------------------------------------------------------------
bool CGameApp::ShutDown()
{
	
	// Release any previously built objects
	ReleaseObjects ( );
	
	// Destroy menu, it may not be attached
	if ( m_hMenu ) DestroyMenu( m_hMenu );
	m_hMenu		 = NULL;

	// Destroy the render window
	SetMenu( m_hWnd, NULL );
	if ( m_hWnd ) DestroyWindow( m_hWnd );
	m_hWnd		  = NULL;
	
	// Shutdown Success
	return true;
}

//-----------------------------------------------------------------------------
// Name : StaticWndProc () (Static Callback)
// Desc : This is the main messge pump for ALL display devices, it captures
//		the appropriate messages, and routes them through to the application
//		class for which it was intended, therefore giving full class access.
// Note : It is VITALLY important that you should pass your 'this' pointer to
//		the lpParam parameter of the CreateWindow function if you wish to be
//		able to pass messages back to that app object.
//-----------------------------------------------------------------------------
LRESULT CALLBACK CGameApp::StaticWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	// If this is a create message, trap the 'this' pointer passed in and store it within the window.
	if ( Message == WM_CREATE ) SetWindowLong( hWnd, GWL_USERDATA, (LONG)((CREATESTRUCT FAR *)lParam)->lpCreateParams);

	// Obtain the correct destination for this message
	CGameApp *Destination = (CGameApp*)GetWindowLong( hWnd, GWL_USERDATA );
	
	// If the hWnd has a related class, pass it through
	if (Destination) return Destination->DisplayWndProc( hWnd, Message, wParam, lParam );
	
	// No destination found, defer to system...
	return DefWindowProc( hWnd, Message, wParam, lParam );
}

//-----------------------------------------------------------------------------
// Name : DisplayWndProc ()
// Desc : The display devices internal WndProc function. All messages being
//		passed to this function are relative to the window it owns.
//-----------------------------------------------------------------------------
LRESULT CGameApp::DisplayWndProc( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam )
{
	static UINT			fTimer;	

	// Determine message type
	switch (Message)
	{
		case WM_CREATE:
			break;
		
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		
		case WM_SIZE:
			if ( wParam == SIZE_MINIMIZED )
			{
				// App is inactive
				m_bActive = false;
			
			} // App has been minimized
			else
			{
				// App is active
				m_bActive = true;

				// Store new viewport sizes
				m_nViewWidth  = LOWORD( lParam );
				m_nViewHeight = HIWORD( lParam );
		
			
			} // End if !Minimized

			break;

		case WM_LBUTTONDOWN:
			// Capture the mouse
			SetCapture( m_hWnd );
			GetCursorPos( &m_OldCursorPos );
			break;

		case WM_LBUTTONUP:
			// Release the mouse
			ReleaseCapture( );
			break;
			
		case WM_KEYDOWN:
			switch(wParam)
			{
			case VK_ESCAPE:
				PostQuitMessage(0);
				break;
			case VK_RETURN:
				fTimer = SetTimer(m_hWnd, 1, 250, NULL);
				m_pPlayer->Explode();
				break;
			case VK_SPACE:
				
				m_pPlayer->Shoot();
				break;
			case VK_CONTROL:
				m_pPlayer2->Shoot();
				break;
			case 0x51:
				fTimer = SetTimer(m_hWnd, 2, 250, NULL);
				m_pPlayer2->Explode();
				break;
			case VK_TAB:
				
				break;
			}
			break;

		case WM_TIMER:
			switch(wParam)
			{
			case 1:
				if(!m_pPlayer->AdvanceExplosion())
					KillTimer(m_hWnd, 1);
			case 2:
				if (!m_pPlayer2->AdvanceExplosion())
					KillTimer(m_hWnd, 2);
			case 4:
				if (!m_cCrate->AdvanceExplosion())
					KillTimer(m_hWnd, 3);
			case 3:
				if (!m_cCrate1->AdvanceExplosion())
					KillTimer(m_hWnd, 4);
			case 5:
				if (!m_pEnemy->AdvanceExplosion())
					KillTimer(m_hWnd, 5);
			case 6:
				if (!m_pHeart->AdvanceExplosion())
					KillTimer(m_hWnd, 6);

			}
			break;

		case WM_COMMAND:
			break;

		default:
			return DefWindowProc(hWnd, Message, wParam, lParam);

	} // End Message Switch
	
	return 0;
}

//-----------------------------------------------------------------------------
// Name : BuildObjects ()
// Desc : Build our demonstration meshes, and the objects that instance them
//-----------------------------------------------------------------------------
bool CGameApp::BuildObjects()
{
	
	m_pBBuffer = new BackBuffer(m_hWnd, m_nViewWidth, m_nViewHeight);
	m_pPlayer = new CPlayer(m_pBBuffer);
	
	m_pPlayer2 = new CPlayer2(m_pBBuffer);
	m_cCrate = new Crate(m_pBBuffer);
	m_cCrate1 = new Crate(m_pBBuffer);
	m_pEnemy = new Enemy(m_pBBuffer);
	m_pHeart = new Lives(m_pBBuffer);

	if(!m_imgBackground.LoadBitmapFromFile("data/background.bmp", GetDC(m_hWnd)))
		return false;

	// Success!
	return true;
	
}

//-----------------------------------------------------------------------------
// Name : SetupGameState ()
// Desc : Sets up all the initial states required by the game.
//-----------------------------------------------------------------------------
void CGameApp::SetupGameState()
{
	m_pPlayer->Position() = Vec2(400, 400);
	m_pPlayer2->Position() = Vec2(100, 400);
	m_pEnemy->Position() = Vec2(300, 200);
	m_cCrate->Position() = Vec2(200,100);
	m_cCrate1->Position() = Vec2(600, 50);
	m_pHeart->Position() = Vec2(400,200);
	
}

//-----------------------------------------------------------------------------
// Name : ReleaseObjects ()
// Desc : Releases our objects and their associated memory so that we can
//		rebuild them, if required, during our applications life-time.
//-----------------------------------------------------------------------------
void CGameApp::ReleaseObjects( )
{
	if(m_pPlayer != NULL)
	{
		delete m_pPlayer;
		m_pPlayer = NULL;
	}

	if(m_pBBuffer != NULL)
	{
		delete m_pBBuffer;
		m_pBBuffer = NULL;
	}

	if (m_pPlayer2 != NULL)
	{
		delete m_pPlayer2;
		m_pPlayer2 = NULL;
	}
	
	if (m_cCrate != NULL)
	{
		delete m_cCrate;
		m_cCrate = NULL;
	}

	if (m_cCrate1 != NULL)
	{
		delete m_cCrate1;
		m_cCrate1 = NULL;
	}

	if (m_pEnemy != NULL)
	{
		delete m_pEnemy;
		m_pEnemy = NULL;
	}

	if (m_pHeart != NULL)
	{
		delete m_pHeart;
		m_pHeart = NULL;
	}
	
}

//-----------------------------------------------------------------------------
// Name : FrameAdvance () (Private)
// Desc : Called to signal that we are now rendering the next frame.
//-----------------------------------------------------------------------------
void CGameApp::FrameAdvance()
{
	static TCHAR FrameRate[ 50 ];
	static TCHAR TitleBuffer[ 255 ];

	// Advance the timer
	m_Timer.Tick( );

	// Skip if app is inactive
	if ( !m_bActive ) return;
	
	// Get / Display the framerate
	if ( m_LastFrameRate != m_Timer.GetFrameRate() )
	{
		
		m_LastFrameRate = m_Timer.GetFrameRate( FrameRate, 50 );
		sprintf_s(TitleBuffer, _T("Game : %s  Lives: %d  Score:%d"), FrameRate,lives,score );
		SetWindowText( m_hWnd, TitleBuffer );

	} // End if Frame Rate Altered

	// Poll & Process input devices
	ProcessInput();

	// Animate the game objects
	AnimateObjects();

	// Drawing the game objects
	DrawObjects();

	checkCollision();

	if (checkCollisionC() == true)
	{
		lives--;

	}
	if (checkCollisionCB() == true)
	{
		score++;
	}

	if (checkCollisionPBE() == true)
	{
		score++;
	}
	
	if (checkCollisionEBP() == true)
	{
		lives--;

	}


	if (checkCollisionL() == true)
	{
		lives++;

	}

	
	
	
}

//-----------------------------------------------------------------------------
// Name : ProcessInput () (Private)
// Desc : Simply polls the input devices and performs basic input operations
//-----------------------------------------------------------------------------
void CGameApp::ProcessInput( )
{
	
	static UCHAR pKeyBuffer[ 256 ];
	ULONG		Direction = 0;
	ULONG		Direction2 = 0;
	ULONG       Direction3 = 0;
	ULONG       Direction4 = 0;
	ULONG       Direction5 = 0;
	ULONG       Direction6 = 0;
	POINT		CursorPos;
	float		X = 0.0f, Y = 0.0f;

	// Retrieve keyboard state
	if ( !GetKeyboardState( pKeyBuffer ) ) return;

	// Check the relevant keys
	if ( pKeyBuffer[ VK_UP	] & 0xF0 ) Direction |= CPlayer::DIR_FORWARD;
	if ( pKeyBuffer[ VK_DOWN  ] & 0xF0 ) Direction |= CPlayer::DIR_BACKWARD;
	if ( pKeyBuffer[ VK_LEFT  ] & 0xF0 ) Direction |= CPlayer::DIR_LEFT;
	if ( pKeyBuffer[ VK_RIGHT ] & 0xF0 ) Direction |= CPlayer::DIR_RIGHT;

	if (pKeyBuffer[0x57] & 0xF0) {
		Direction2 |= CPlayer2::DIR_FORWARD;
		
	}
	if (pKeyBuffer[0x53] & 0xF0) {
		Direction2 |= CPlayer2::DIR_BACKWARD;
		
	}
	if (pKeyBuffer[0x41] & 0xF0) {
		Direction2 |= CPlayer2::DIR_LEFT;
		
	}
	if (pKeyBuffer[0x44] & 0xF0) {
		Direction2 |= CPlayer2::DIR_RIGHT;
		
	}
	
	
	
	srand((unsigned)time(0));
	printf("Your dice has been rolled! You got: \n ");
	int result = 1 + (rand() % 4);
	
	switch (result) {
	case 1:
		Direction3 |= Enemy::DIR_FORWARD;
		Direction4 |= Crate::DIR_LEFT;
		Direction5 |= Crate::DIR_BACKWARD;
		Direction6 |= Lives::DIR_FORWARD;
		break;
	case 2:
		Direction3 |= Enemy::DIR_BACKWARD;
		Direction4 |= Crate::DIR_FORWARD;
		Direction5 |= Crate::DIR_RIGHT;
		Direction6 |= Lives::DIR_RIGHT;
		m_pEnemy->Shoot();
		break;
	case 3:
		Direction3 |= Enemy::DIR_LEFT;
		Direction4 |= Crate::DIR_RIGHT;
		Direction5 |= Crate::DIR_LEFT;
		Direction6 |= Lives::DIR_BACKWARD;
		
		break;
	case 4:
		Direction3 |= Enemy::DIR_RIGHT;
		Direction4 |= Crate::DIR_BACKWARD;
		Direction5 |= Crate::DIR_FORWARD;
		Direction6 |= Lives::DIR_LEFT;
		m_pEnemy->Shoot();
		break;
	}

	// Move the player
	m_pPlayer->Move(Direction);
	m_pPlayer2->Move(Direction2);
	m_pEnemy->Move(Direction3);
	
	m_cCrate->Move(Direction4);
	m_cCrate1->Move(Direction5);
	m_pHeart->Move(Direction6);

	if (pKeyBuffer[0x4D] & 0xF0)
	{
		std::ofstream output;
		output.open("data/save.txt");
		double x1_axis, y1_axis, x2_axis, y2_axis;
		m_pPlayer->GeterPositionX(x1_axis);
		m_pPlayer->GeterPositionY(y1_axis);
		m_pPlayer2->GeterPositionX(x2_axis);
		m_pPlayer2->GeterPositionY(y2_axis);
 		output << x1_axis << " " << y1_axis<<" ";
		output << x2_axis << " " << y2_axis;
		output.close();
	}

	if (pKeyBuffer[0x4C] & 0xF0)
	{
		std::ifstream input;
		input.open("data/save.txt");
		double x1_axis, y1_axis, x2_axis, y2_axis;
		input >> x1_axis >> y1_axis>> x2_axis>> y2_axis;
		m_pPlayer->Position() = Vec2(x1_axis, y1_axis);
		m_pPlayer2->Position() = Vec2(x2_axis, y2_axis);
		input.close();
	}


	if (pKeyBuffer[VK_NUMPAD4] & 0xF0)
		m_pPlayer->MoveLeft(m_pBBuffer);

	if (pKeyBuffer[VK_NUMPAD2] & 0xF0)
		m_pPlayer->MoveDown(m_pBBuffer);

	if (pKeyBuffer[VK_NUMPAD6] & 0xF0)
		m_pPlayer->MoveRight(m_pBBuffer);

	if (pKeyBuffer[VK_NUMPAD8] & 0xF0)
		m_pPlayer->MoveUp(m_pBBuffer);


	// Now process the mouse (if the button is pressed)
	if ( GetCapture() == m_hWnd )
	{
		// Hide the mouse pointer
		SetCursor( NULL );

		// Retrieve the cursor position
		GetCursorPos( &CursorPos );

		// Reset our cursor position so we can keep going forever :)
		SetCursorPos( m_OldCursorPos.x, m_OldCursorPos.y );

	} // End if Captured
}

//-----------------------------------------------------------------------------
// Name : AnimateObjects () (Private)
// Desc : Animates the objects we currently have loaded.
//-----------------------------------------------------------------------------
void CGameApp::AnimateObjects()
{
	m_pPlayer->Update(m_Timer.GetTimeElapsed());
	m_pPlayer2->Update(m_Timer.GetTimeElapsed());
	m_cCrate->Update(m_Timer.GetTimeElapsed());
	m_cCrate1->Update(m_Timer.GetTimeElapsed());
	m_pEnemy->Update(m_Timer.GetTimeElapsed());
	m_pHeart->Update(m_Timer.GetTimeElapsed());

}

//-----------------------------------------------------------------------------
// Name : DrawObjects () (Private)
// Desc : Draws the game objects
//-----------------------------------------------------------------------------
void CGameApp::DrawObjects()
{
	m_pBBuffer->reset();

	//m_imgBackground.Paint(m_pBBuffer->getDC(), 0, 0);


	m_imgBackground.Paint(m_pBBuffer->getDC(), 0, y-600);
	y = y + 2;
	    if (y == 600)
			y =-120;
		
	

	m_pPlayer->Draw();
	m_pPlayer2->Draw();
	m_cCrate->Draw();
	m_cCrate1->Draw();
	m_pEnemy->Draw();
	m_pHeart->Draw();

	m_pBBuffer->present();

	
}


void CGameApp::checkCollision()
{
	static UINT fTimer;
	double distance = m_pPlayer->Position().Distance(m_pPlayer2->Position());
	if (distance <= (m_pPlayer->getWidth() + m_pPlayer2->getWidth()) / 2 && !m_pPlayer->ifExploded() && !m_pPlayer2->ifExploded())
	{
		fTimer = SetTimer(m_hWnd, 1, 250, NULL);
		m_pPlayer->Explode();
		fTimer = SetTimer(m_hWnd, 2, 250, NULL);
		m_pPlayer2->Explode();

	}

}




bool CGameApp::checkCollisionC()
{
	static UINT fTimer;
	double distance = m_pPlayer->Position().Distance(m_cCrate->Position());
	if (distance <= (m_pPlayer->getWidth() + m_cCrate->getWidth()) / 2 && !m_cCrate->ifExploded())
	{
		fTimer = SetTimer(m_hWnd, 1, 250, NULL);
		m_cCrate->Explode();
		fTimer = SetTimer(m_hWnd, 2, 250, NULL);
		m_pPlayer->Explode();
		return true;
	}


	distance = m_pPlayer->Position().Distance(m_cCrate1->Position());
	if (distance <= (m_pPlayer->getWidth() + m_cCrate1->getWidth()) / 2 && !m_cCrate1->ifExploded())
	{
		fTimer = SetTimer(m_hWnd, 1, 250, NULL);
		m_cCrate1->Explode();
		fTimer = SetTimer(m_hWnd, 2, 250, NULL);
		m_pPlayer->Explode();
		return true;
		
	}
	return false;
}

bool CGameApp::checkCollisionCB()
{
	static UINT fTimer;
	double distance = m_pPlayer->PositionBullet().Distance(m_cCrate->Position());
	if (distance <= (m_pPlayer->getWidthBullet() + m_cCrate->getWidth()) / 2 && !m_cCrate->ifExploded())
	{
		fTimer = SetTimer(m_hWnd, 1, 250, NULL);
		m_cCrate->Explode();
		return true;
	}


	distance = m_pPlayer->PositionBullet().Distance(m_cCrate1->Position());
	if (distance <= (m_pPlayer->getWidthBullet() + m_cCrate1->getWidth()) / 2  && !m_cCrate1->ifExploded())
	{
		fTimer = SetTimer(m_hWnd, 2, 250, NULL);
		m_cCrate1->Explode();
		
		return true;
	}

    

	return false;
}



bool CGameApp::checkCollisionPBE()
{
	static UINT fTimer;
	double distance = m_pPlayer->PositionBullet().Distance(m_pEnemy->Position());
	if (distance <= (m_pPlayer->getWidthBullet() + m_pEnemy->getWidth()) / 2 && !m_pEnemy->ifExploded())
	{
		fTimer = SetTimer(m_hWnd, 1, 250, NULL);
		m_pEnemy->Explode();
		return true;
	}


	return false;
}

bool CGameApp::checkCollisionEBP()
{
	static UINT fTimer;
	double distance = m_pEnemy->Position().Distance(m_pPlayer->Position());
	if (distance <= (m_pEnemy->getWidth() + m_pPlayer->getWidth()) / 2 && !m_pPlayer->ifExploded())
	{
		fTimer = SetTimer(m_hWnd, 1, 250, NULL);
		m_pPlayer->Explode();
		return true;
	}


	 distance = m_pEnemy->PositionBullet().Distance(m_pPlayer->Position());
	if (distance <= (m_pEnemy->getWidthBullet() + m_pPlayer->getWidth()) / 2 && !m_pPlayer->ifExploded())
	{
		fTimer = SetTimer(m_hWnd, 1 , 250, NULL);
		m_pPlayer->Explode();
		return true;
	}

	return false;
}


bool CGameApp::checkCollisionL()
{
	static UINT fTimer;
	double distance = m_pPlayer->Position().Distance(m_pHeart->Position());
	if (distance <= (m_pPlayer->getWidth() + m_pHeart->getWidth()) / 2 && !m_pHeart->ifExploded())
	{
		fTimer = SetTimer(m_hWnd, 1, 250, NULL);
		m_pHeart->Explode();
		
		return true;
	}


	return false;
}