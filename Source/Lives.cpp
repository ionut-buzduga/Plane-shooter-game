//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//
// Desc: This file stores the player object class. This class performs tasks
//       such as player movement, some minor physics as well as rendering.
//
// Original design by Adam Hoult & Gary Simmons. Modified by Mihai Popescu.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CPlayer Specific Includes
//-----------------------------------------------------------------------------
#include "Lives.h"

//-----------------------------------------------------------------------------
// Name : CPlayer () (Constructor)
// Desc : CPlayer Class Constructor
//-----------------------------------------------------------------------------
Lives::Lives(const BackBuffer* pBackBuffer)
{
	//m_pSprite = new Sprite("data/planeimg.bmp", "data/planemask.bmp");
	m_pSprite = new Sprite("data/heart.bmp", RGB(0xff, 0x00, 0xff));
	m_pSprite->setBackBuffer(pBackBuffer);

	m_eSpeedState = SPEED_STOP;
	m_fTimer = 0;

	// Animation frame crop rectangle
	RECT r;
	r.left = 0;
	r.top = 0;
	r.right = 24;
	r.bottom = 22;

	m_pExplosionSprite = new AnimatedSprite("data/heartAnimation.bmp", "data/heartAnimationMask.bmp", r, 3);
	m_pExplosionSprite->setBackBuffer(pBackBuffer);
	m_bExplosion = false;
	m_iExplosionFrame = 0;


}

//-----------------------------------------------------------------------------
// Name : ~CPlayer () (Destructor)
// Desc : CPlayer Class Destructor
//-----------------------------------------------------------------------------
Lives::~Lives()
{
	delete m_pSprite;

	delete m_pExplosionSprite;
}

void Lives::Update(float dt)
{
	// Update sprite

	m_pSprite->update(dt);


	// Get velocity
	double v = m_pSprite->mVelocity.Magnitude();

	// NOTE: for each async sound played Windows creates a thread for you
	// but only one, so you cannot play multiple sounds at once.
	// This creation/destruction of threads also leads to bad performance
	// so this method is not recommanded to be used in complex projects.

	// update internal time counter used in sound handling (not to overlap sounds)
	m_fTimer += dt;

	// A FSM is used for sound manager 
	switch (m_eSpeedState)
	{
	case SPEED_STOP:
		if (v > 35.0f)
		{
			m_eSpeedState = SPEED_START;
			PlaySound("data/jet-start.wav", NULL, SND_FILENAME | SND_ASYNC);
			m_fTimer = 0;
		}
		break;
	case SPEED_START:
		if (v < 25.0f)
		{
			m_eSpeedState = SPEED_STOP;
			PlaySound("data/jet-stop.wav", NULL, SND_FILENAME | SND_ASYNC);
			m_fTimer = 0;
		}
		else
			if (m_fTimer > 1.f)
			{
				PlaySound("data/jet-cabin.wav", NULL, SND_FILENAME | SND_ASYNC);
				m_fTimer = 0;
			}
		break;
	}

	// NOTE: For sound you also can use MIDI but it's Win32 API it is a bit hard
	// see msdn reference: http://msdn.microsoft.com/en-us/library/ms711640.aspx
	// In this case you can use a C++ wrapper for it. See the following article:
	// http://www.codeproject.com/KB/audio-video/midiwrapper.aspx (with code also)
}

void Lives::Draw()
{


	if (!m_bExplosion)
		m_pSprite->draw();
	else
		m_pExplosionSprite->draw();
}

void Lives::Move(ULONG ulDirection)
{
	if (ulDirection & Lives::DIR_LEFT)
		m_pSprite->mVelocity.x -= 0.2;
	if (m_pSprite->mPosition.x < m_pSprite->width() / 2)
		m_pSprite->mVelocity.x = 0;

	if (ulDirection & Lives::DIR_RIGHT)
		m_pSprite->mVelocity.x += 0.2;


	if (m_pSprite->mPosition.x > 780 - m_pSprite->width() / 2)
	{
		m_pSprite->mVelocity.x = 0;
		if (m_pSprite->mVelocity.x == 0)
			m_pSprite->mPosition.x--;
	}


	if (ulDirection & Lives::DIR_FORWARD)
		m_pSprite->mVelocity.y -= 0.2;
	if (m_pSprite->mPosition.y < m_pSprite->height() / 2)
		m_pSprite->mVelocity.y = 0;

	if (ulDirection & Lives::DIR_BACKWARD)
		m_pSprite->mVelocity.y += 0.2;

	if (m_pSprite->mPosition.y > 560 - m_pSprite->height() / 2)
	{
		m_pSprite->mVelocity.y = 0;
		if (m_pSprite->mVelocity.y == 0)
			m_pSprite->mPosition.y--;
	}




}

double Lives::getWidth()
{
	return m_pSprite->width();
}

Vec2& Lives::Position()
{
	return m_pSprite->mPosition;
}

void Lives::GeterPositionX(double& x)
{
	x = m_pSprite->mPosition.x;
}

void Lives::GeterPositionY(double& y)
{
	y = m_pSprite->mPosition.y;
}

Vec2& Lives::Velocity()
{
	return m_pSprite->mVelocity;
}

void Lives::Explode()
{
	m_pExplosionSprite->mPosition = m_pSprite->mPosition;
	m_pExplosionSprite->SetFrame(0);
	PlaySound("data/explosion.wav", NULL, SND_FILENAME | SND_ASYNC);
	m_bExplosion = true;
}

bool Lives::AdvanceExplosion()
{
	if (m_bExplosion)
	{
		m_pExplosionSprite->SetFrame(m_iExplosionFrame++);
		if (m_iExplosionFrame == m_pExplosionSprite->GetFrameCount())
		{
			m_bExplosion = false;
			m_iExplosionFrame = 0;
			m_pSprite->mVelocity = Vec2(0, 0);
			m_eSpeedState = SPEED_STOP;
			return false;
		}
	}

	return true;
}

bool Lives::ifExploded()
{
	return m_bExplosion;
}
