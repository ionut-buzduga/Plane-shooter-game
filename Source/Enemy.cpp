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
#include "Enemy.h"

//-----------------------------------------------------------------------------
// Name : CPlayer () (Constructor)
// Desc : CPlayer Class Constructor
//-----------------------------------------------------------------------------
Enemy::Enemy(const BackBuffer* pBackBuffer)
{
	//m_pSprite = new Sprite("data/planeimg.bmp", "data/planemask.bmp");
	m_pSprite = new Sprite("data/EnemyImgAndMask.bmp", RGB(0xff, 0x00, 0xff));
	m_pSprite->setBackBuffer(pBackBuffer);
	m_eSpeedState = SPEED_STOP;
	m_fTimer = 0;

	// Animation frame crop rectangle
	RECT r;
	r.left = 0;
	r.top = 0;
	r.right = 128;
	r.bottom = 128;

	m_pExplosionSprite = new AnimatedSprite("data/explosion.bmp", "data/explosionmask.bmp", r, 15);
	m_pExplosionSprite->setBackBuffer(pBackBuffer);
	m_bExplosion = false;
	m_iExplosionFrame = 0;

	bullet = new Sprite("data/bullet_img_and_mask_rotated.bmp", RGB(0xff, 0x00, 0xff));
	bullet->setBackBuffer(pBackBuffer);
	m_bullet = false;

	bullet2 = new Sprite("data/bullet_img_and_mask_rotated.bmp", RGB(0xff, 0x00, 0xff));
	bullet2->setBackBuffer(pBackBuffer);
}

//-----------------------------------------------------------------------------
// Name : ~CPlayer () (Destructor)
// Desc : CPlayer Class Destructor
//-----------------------------------------------------------------------------
Enemy::~Enemy()
{
	delete m_pSprite;
	delete bullet;
	delete bullet2;
	delete m_pExplosionSprite;
}

void Enemy::Update(float dt)
{
	// Update sprite
	bullet->update(dt);
	bullet2->update(dt);
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

void Enemy::Draw()
{
	if (m_bullet && !m_bExplosion) {
		bullet->draw();
		bullet2->draw();
	}

	if (!m_bExplosion)
		m_pSprite->draw();
	else
		m_pExplosionSprite->draw();
}

void Enemy::Move(ULONG ulDirection)
{
	if (ulDirection & Enemy::DIR_LEFT)
		m_pSprite->mVelocity.x -= 0.1;
	if (m_pSprite->mPosition.x < m_pSprite->width() / 2)
		m_pSprite->mVelocity.x = 0;

	if (ulDirection & Enemy::DIR_RIGHT)
		m_pSprite->mVelocity.x += 0.1;


	if (m_pSprite->mPosition.x > 780 - m_pSprite->width() / 2)
	{
		m_pSprite->mVelocity.x = 0;
		if (m_pSprite->mVelocity.x == 0)
			m_pSprite->mPosition.x--;
	}


	if (ulDirection & Enemy::DIR_FORWARD)
		m_pSprite->mVelocity.y -= 0.1;
	if (m_pSprite->mPosition.y < m_pSprite->height() / 2)
		m_pSprite->mVelocity.y = 0;

	if (ulDirection & Enemy::DIR_BACKWARD)
		m_pSprite->mVelocity.y += 0.1;

	if (m_pSprite->mPosition.y > 260 - m_pSprite->height() / 2)
	{
		m_pSprite->mVelocity.y = 0;
		if (m_pSprite->mVelocity.y == 0)
			m_pSprite->mPosition.y--;
	}


}

double Enemy::getWidth()
{
	return m_pSprite->width();
}

double Enemy::getWidthBullet()
{
	return bullet->width();
}

Vec2& Enemy::Position()
{
	return m_pSprite->mPosition;
}


Vec2& Enemy::PositionBullet()
{
	return bullet->mPosition;
}

void Enemy::GeterPositionX(double& x)
{
	x = m_pSprite->mPosition.x;
}

void Enemy::GeterPositionY(double& y)
{
	y = m_pSprite->mPosition.y;
}





Vec2& Enemy::Velocity()
{
	return m_pSprite->mVelocity;
}

void Enemy::Explode()
{
	m_pExplosionSprite->mPosition = m_pSprite->mPosition;
	m_pExplosionSprite->SetFrame(0);
	PlaySound("data/explosion.wav", NULL, SND_FILENAME | SND_ASYNC);
	m_bExplosion = true;
}

bool Enemy::AdvanceExplosion()
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

bool Enemy::ifExploded()
{
	return m_bExplosion;
}

void Enemy::Shoot() {
	RECT desktop;
	GetWindowRect(GetDesktopWindow(), &desktop);
	if (bullet->mPosition.y > desktop.bottom) {
		m_bullet = true;
		bullet->mPosition.x = m_pSprite->mPosition.x - 15;
		bullet->mPosition.y = m_pSprite->mPosition.y + 35;

		bullet2->mPosition.x = m_pSprite->mPosition.x + 10;
		bullet2->mPosition.y = m_pSprite->mPosition.y + 35;

	}
	bullet->mVelocity.y = 1000;
	bullet2->mVelocity.y = 1000;

}


void Enemy::MoveLeft(const BackBuffer* pBackBuffer)
{
	double a, b;
	GeterPositionX(a);
	GeterPositionY(b);

	m_pSprite = new Sprite("data/PlaneImgAndMaskLeft.bmp", RGB(0xff, 0x00, 0xff));

	m_pSprite->setBackBuffer(pBackBuffer);
	m_pSprite->mPosition = Vec2(a, b);

}

void Enemy::MoveRight(const BackBuffer* pBackBuffer)
{
	double a, b;
	GeterPositionX(a);
	GeterPositionY(b);

	m_pSprite = new Sprite("data/PlaneImgAndMaskRight.bmp", RGB(0xff, 0x00, 0xff));

	m_pSprite->setBackBuffer(pBackBuffer);
	m_pSprite->mPosition = Vec2(a, b);

}

void Enemy::MoveUp(const BackBuffer* pBackBuffer)
{
	double a, b;
	GeterPositionX(a);
	GeterPositionY(b);

	m_pSprite = new Sprite("data/PlaneImgAndMaskUp.bmp", RGB(0xff, 0x00, 0xff));

	m_pSprite->setBackBuffer(pBackBuffer);
	m_pSprite->mPosition = Vec2(a, b);

}

void Enemy::MoveDown(const BackBuffer* pBackBuffer)
{
	double a, b;
	GeterPositionX(a);
	GeterPositionY(b);

	m_pSprite = new Sprite("data/PlaneImgAndMaskDown.bmp", RGB(0xff, 0x00, 0xff));

	m_pSprite->setBackBuffer(pBackBuffer);
	m_pSprite->mPosition = Vec2(a, b);

}