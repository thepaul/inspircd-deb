/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd: (C) 2002-2009 InspIRCd Development Team
 * See: http://wiki.inspircd.org/Credits
 *
 * This program is free but copyrighted software; see
 *            the file COPYING for details.
 *
 * ---------------------------------------------------
 */

#include "inspircd.h"

/* $ModDesc: Implements extban +b r: - realname (gecos) bans */

class ModuleGecosBan : public Module
{
 private:
 public:
	ModuleGecosBan(InspIRCd* Me) : Module(Me)
	{
		Implementation eventlist[] = { I_OnCheckBan, I_On005Numeric };
		ServerInstance->Modules->Attach(eventlist, this, 2);
	}

	virtual ~ModuleGecosBan()
	{
	}

	virtual Version GetVersion()
	{
		return Version("$Id: m_gecosban.cpp 11223 2009-03-15 12:42:35Z psychon $", VF_COMMON|VF_VENDOR, API_VERSION);
	}

	virtual int OnCheckBan(User *user, Channel *c)
	{
		return c->GetExtBanStatus(user->fullname, 'r');
	}

	virtual void On005Numeric(std::string &output)
	{
		ServerInstance->AddExtBanChar('r');
	}
};


MODULE_INIT(ModuleGecosBan)

