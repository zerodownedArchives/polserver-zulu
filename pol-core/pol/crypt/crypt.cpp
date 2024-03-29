#include <memory.h>
#include "crypt.h"
#include "logincrypt.h"
#include "blowfish.h"
#include "twofish.h"
#include "cryptbase.h"

// NOCRYPT

CCryptNoCrypt::CCryptNoCrypt()
{
}

CCryptNoCrypt::~CCryptNoCrypt()
{
}

int CCryptNoCrypt::Receive(void *buffer, int max_expected, SOCKET socket)
{
	return recv(socket, (char *) buffer, max_expected, 0);
}

void CCryptNoCrypt::Init(void *pvSeed, int type)
{
	// Nocrypt doesnt need to Initialize anything.
}

// BLOWFISH

CCryptBlowfish::CCryptBlowfish()
{
}

CCryptBlowfish::~CCryptBlowfish()
{
}

CCryptBlowfish::CCryptBlowfish(unsigned int masterKey1, unsigned int masterKey2)
{
	SetMasterKeys(masterKey1,masterKey2);
}

int CCryptBlowfish::Receive(void *buffer, int max_expected, SOCKET socket)
{
	int count = recv(socket, (char *) encrypted_data, max_expected, 0);
	if (count > 0)
	{
		passert( count <= max_expected );
		Decrypt( encrypted_data, buffer, count );
	}
	return count;
}

void CCryptBlowfish::SetMasterKeys(unsigned int masterKey1, unsigned int masterKey2)
{
	m_masterKey[0] = masterKey1 & 0xFFFFFFFF;
	m_masterKey[1] = masterKey2 & 0xFFFFFFFF;
}

void CCryptBlowfish::Init(void *pvSeed, int type)
{
	unsigned char *pSeed = (unsigned char *)pvSeed;
	
	lcrypt.Init( pSeed, m_masterKey[0], m_masterKey[1] );
	bfish.Init();

	m_type = type;
}

void CCryptBlowfish::Decrypt(void *pvIn, void *pvOut, int len)
{
	unsigned char *pIn = (unsigned char *)pvIn;
	unsigned char *pOut = (unsigned char *)pvOut;

	if(m_type == CCryptBase::typeAuto)
	{
		if(((*pIn ^ (unsigned char)lcrypt.lkey[0])) == CRYPT_AUTO_VALUE) m_type = CCryptBase::typeLogin;
		else m_type = CCryptBase::typeGame;
	}

	if(m_type == CCryptBase::typeLogin)
	{
		lcrypt.Decrypt( pIn, pOut, len );
	}
	else
	{
		bfish.Decrypt( pIn, pOut, len );
	}
}

// BLOWFISH OLD

CCryptBlowfishOld::CCryptBlowfishOld()
{
}

CCryptBlowfishOld::~CCryptBlowfishOld()
{
}

CCryptBlowfishOld::CCryptBlowfishOld(unsigned int masterKey1, unsigned int masterKey2)
{
	SetMasterKeys(masterKey1,masterKey2);
}

void CCryptBlowfishOld::Decrypt(void *pvIn, void *pvOut, int len)
{
	unsigned char *pIn = (unsigned char *)pvIn;
	unsigned char *pOut = (unsigned char *)pvOut;

	if(m_type == CCryptBase::typeAuto)
	{
		if(((*pIn ^ (unsigned char)lcrypt.lkey[0])) == CRYPT_AUTO_VALUE) m_type = CCryptBase::typeLogin;
		else m_type = CCryptBase::typeGame;
	}

	if(m_type == CCryptBase::typeLogin)
	{
		lcrypt.Decrypt_Old( pIn, pOut, len );
	}
	else
	{
		bfish.Decrypt( pIn, pOut, len );
	}
}

// BLOWFISH 1.25.36

CCrypt12536::CCrypt12536()
{
}

CCrypt12536::~CCrypt12536()
{
}

CCrypt12536::CCrypt12536(unsigned int masterKey1, unsigned int masterKey2)
{
	SetMasterKeys(masterKey1,masterKey2);
}

void CCrypt12536::Decrypt(void *pvIn, void *pvOut, int len)
{
	unsigned char *pIn = (unsigned char *)pvIn;
	unsigned char *pOut = (unsigned char *)pvOut;

	if(m_type == CCryptBase::typeAuto)
	{
		if(((*pIn ^ (unsigned char)lcrypt.lkey[0])) == CRYPT_AUTO_VALUE) m_type = CCryptBase::typeLogin;
		else m_type = CCryptBase::typeGame;
	}

	if(m_type == CCryptBase::typeLogin)
	{
		lcrypt.Decrypt_1_25_36( pIn, pOut, len );
	}
	else
	{
		bfish.Decrypt( pIn, pOut, len );
	}
}

// BLOWFISH + TWOFISH

CCryptBlowfishTwofish::CCryptBlowfishTwofish()
{
}

CCryptBlowfishTwofish::~CCryptBlowfishTwofish()
{
}

CCryptBlowfishTwofish::CCryptBlowfishTwofish(unsigned int masterKey1, unsigned int masterKey2)
{
	SetMasterKeys(masterKey1,masterKey2);
}

int CCryptBlowfishTwofish::Receive(void *buffer, int max_expected, SOCKET socket)
{
	int count = recv(socket, (char *) encrypted_data, max_expected, 0);
	if (count > 0)
	{
		passert( count <= max_expected );
		Decrypt( encrypted_data, buffer, count );
	}
	return count;
}

void CCryptBlowfishTwofish::SetMasterKeys(unsigned int masterKey1, unsigned int masterKey2)
{
	m_masterKey[0] = masterKey1 & 0xFFFFFFFF;
	m_masterKey[1] = masterKey2 & 0xFFFFFFFF;
}

void CCryptBlowfishTwofish::Init(void *pvSeed, int type)
{
	unsigned char *pSeed = (unsigned char *)pvSeed;
	
	lcrypt.Init( pSeed, m_masterKey[0], m_masterKey[1] );
	tfish.Init( pSeed );
	bfish.Init();

	m_type = type;
}

void CCryptBlowfishTwofish::Decrypt(void *pvIn, void *pvOut, int len)
{
	unsigned char *pIn = (unsigned char *)pvIn;
	unsigned char *pOut = (unsigned char *)pvOut;

	if(m_type == CCryptBase::typeAuto)
	{
		if(((*pIn ^ (unsigned char)lcrypt.lkey[0])) == CRYPT_AUTO_VALUE) m_type = CCryptBase::typeLogin;
		else m_type = CCryptBase::typeGame;
	}

	if(m_type == CCryptBase::typeLogin)
	{
		lcrypt.Decrypt( pIn, pOut, len );
	}
	else
	{
		tfish.Decrypt( pIn, pOut, len );
		bfish.Decrypt( pOut, pOut, len );
	}
}

// TWOFISH

CCryptTwofish::CCryptTwofish()
{
}

CCryptTwofish::~CCryptTwofish()
{
}

CCryptTwofish::CCryptTwofish(unsigned int masterKey1, unsigned int masterKey2)
{
	SetMasterKeys(masterKey1,masterKey2);
}

int CCryptTwofish::Receive(void *buffer, int max_expected, SOCKET socket)
{
	int count = recv(socket, (char *) encrypted_data, max_expected, 0);
	if (count > 0)
	{
		passert( count <= max_expected );
		Decrypt( encrypted_data, buffer, count );
	}
	return count;
}

void CCryptTwofish::SetMasterKeys(unsigned int masterKey1, unsigned int masterKey2)
{
	m_masterKey[0] = masterKey1 & 0xFFFFFFFF;
	m_masterKey[1] = masterKey2 & 0xFFFFFFFF;
}

void CCryptTwofish::Init(void *pvSeed, int type)
{
	unsigned char *pSeed = (unsigned char *)pvSeed;
	
	lcrypt.Init( pSeed, m_masterKey[0], m_masterKey[1] );
	tfish.Init( pSeed );
	md5.Init( tfish.subData3, 256 );

	m_type = type;
}

void CCryptTwofish::Encrypt(void *pvIn, void *pvOut, int len)
{
	unsigned char *pIn = (unsigned char *)pvIn;
	unsigned char *pOut = (unsigned char *)pvOut;

	md5.Encrypt( pIn, pOut, len );
}

void CCryptTwofish::Decrypt(void *pvIn, void *pvOut, int len)
{
	unsigned char *pIn = (unsigned char *)pvIn;
	unsigned char *pOut = (unsigned char *)pvOut;

	if(m_type == CCryptBase::typeAuto)
	{
		if(((*pIn ^ (unsigned char)lcrypt.lkey[0])) == CRYPT_AUTO_VALUE) m_type = CCryptBase::typeLogin;
		else m_type = CCryptBase::typeGame;
	}

	if(m_type == CCryptBase::typeLogin)
	{
		lcrypt.Decrypt( pIn, pOut, len );
	}
	else
	{
		tfish.Decrypt( pIn, pOut, len );
	}
}
