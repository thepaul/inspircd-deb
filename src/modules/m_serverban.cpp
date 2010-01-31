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

/* $ModDesc: Implements extban +b s: - server name bans */

class ModuleServerBan : public Module
{
 private:
 public:
	ModuleServerBan(InspIRCd* Me) : Module(Me)
	{
		Implementation eventlist[] = { I_OnCheckBan, I_On005Numeric };
		ServerInstance->Modules->Attach(eventlist, this, 2);
	}

	virtual ~ModuleServerBan()
	{
	}

	virtual Version GetVersion()
	{
		return Version("$Id: m_serverban.cpp 11223 2009-03-15 12:42:35Z psychon $",VF_COMMON|VF_VENDOR,API_VERSION);
	}

	virtual int OnCheckBan(User *user, Channel *c)
	{
		return c->GetExtBanStatus(user->server, 's');
	}

	virtual void On005Numeric(std::string &output)
	{
		ServerInstance->AddExtBanChar('s');
	}
};


MODULE_INIT(ModuleServerBan)

