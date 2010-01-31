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

/* $ModDesc: Implements extban +b j: - matching channel bans */

class ModuleBadChannelExtban : public Module
{
 private:
 public:
	ModuleBadChannelExtban(InspIRCd* Me) : Module(Me)
	{
		Implementation eventlist[] = { I_OnCheckBan, I_On005Numeric };
		ServerInstance->Modules->Attach(eventlist, this, 2);
	}

	virtual ~ModuleBadChannelExtban()
	{
	}

	virtual Version GetVersion()
	{
		return Version("$Id: m_channelban.cpp 11223 2009-03-15 12:42:35Z psychon $", VF_COMMON|VF_VENDOR,API_VERSION);
	}

	virtual int OnCheckBan(User *user, Channel *c)
	{
		int rv = 0;
		for (UCListIter i = user->chans.begin(); i != user->chans.end(); i++)
		{
			rv = banmatch_reduce(rv, c->GetExtBanStatus(i->first->name, 'j'));
		}

		return rv;
	}

	virtual void On005Numeric(std::string &output)
	{
		ServerInstance->AddExtBanChar('j');
	}
};


MODULE_INIT(ModuleBadChannelExtban)

