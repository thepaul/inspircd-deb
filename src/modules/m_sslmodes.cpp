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

/* $ModDesc: Provides support for unreal-style channel mode +z */

static char* dummy;

/** Handle channel mode +z
 */
class SSLMode : public ModeHandler
{
 public:
	SSLMode(InspIRCd* Instance) : ModeHandler(Instance, 'z', 0, 0, false, MODETYPE_CHANNEL, false) { }

	ModeAction OnModeChange(User* source, User* dest, Channel* channel, std::string &parameter, bool adding, bool)
	{
		if (adding)
		{
			if (!channel->IsModeSet('z'))
			{
				if (IS_LOCAL(source))
				{
					CUList* userlist = channel->GetUsers();
					for(CUList::iterator i = userlist->begin(); i != userlist->end(); i++)
					{
						if(!i->first->GetExt("ssl", dummy) && !ServerInstance->ULine(i->first->server))
						{
							source->WriteNumeric(ERR_ALLMUSTSSL, "%s %s :all members of the channel must be connected via SSL", source->nick.c_str(), channel->name.c_str());
							return MODEACTION_DENY;
						}
					}
				}
				channel->SetMode('z',true);
				return MODEACTION_ALLOW;
			}
			else
			{
				return MODEACTION_DENY;
			}
		}
		else
		{
			if (channel->IsModeSet('z'))
			{
				channel->SetMode('z',false);
				return MODEACTION_ALLOW;
			}

			return MODEACTION_DENY;
		}
	}
};

class ModuleSSLModes : public Module
{

	SSLMode* sslm;

 public:
	ModuleSSLModes(InspIRCd* Me)
		: Module(Me)
	{


		sslm = new SSLMode(ServerInstance);
		if (!ServerInstance->Modes->AddMode(sslm))
			throw ModuleException("Could not add new modes!");
		Implementation eventlist[] = { I_OnUserPreJoin };
		ServerInstance->Modules->Attach(eventlist, this, 1);
	}


	virtual int OnUserPreJoin(User* user, Channel* chan, const char* cname, std::string &privs, const std::string &keygiven)
	{
		if(chan && chan->IsModeSet('z'))
		{
			if(user->GetExt("ssl", dummy))
			{
				// Let them in
				return 0;
			}
			else
			{
				// Deny
				user->WriteServ( "489 %s %s :Cannot join channel; SSL users only (+z)", user->nick.c_str(), cname);
				return 1;
			}
		}

		return 0;
	}

	virtual ~ModuleSSLModes()
	{
		ServerInstance->Modes->DelMode(sslm);
		delete sslm;
	}

	virtual Version GetVersion()
	{
		return Version("$Id: m_sslmodes.cpp 11223 2009-03-15 12:42:35Z psychon $", VF_COMMON | VF_VENDOR, API_VERSION);
	}
};


MODULE_INIT(ModuleSSLModes)

