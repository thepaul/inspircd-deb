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

/* $ModDesc: Provides support for /KNOCK and mode +K */

/** Handles the /KNOCK command
 */
class CommandKnock : public Command
{
 public:
	CommandKnock (InspIRCd* Instance) : Command(Instance,"KNOCK", 0, 2)
	{
		this->source = "m_knock.so";
		syntax = "<channel> <reason>";
		TRANSLATE3(TR_TEXT, TR_TEXT, TR_END);
	}

	CmdResult Handle (const std::vector<std::string> &parameters, User *user)
	{
		Channel* c = ServerInstance->FindChan(parameters[0]);
		std::string line;

		if (!c)
		{
			user->WriteNumeric(401, "%s %s :No such channel",user->nick.c_str(), parameters[0].c_str());
			return CMD_FAILURE;
		}

		if (c->HasUser(user))
		{
			user->WriteNumeric(480, "%s :Can't KNOCK on %s, you are already on that channel.", user->nick.c_str(), c->name.c_str());
			return CMD_FAILURE;
		}

		if (c->IsModeSet('K'))
		{
			user->WriteNumeric(480, "%s :Can't KNOCK on %s, +K is set.",user->nick.c_str(), c->name.c_str());
			return CMD_FAILURE;
		}

		if (!c->modes[CM_INVITEONLY])
		{
			user->WriteNumeric(480, "%s :Can't KNOCK on %s, channel is not invite only so knocking is pointless!",user->nick.c_str(), c->name.c_str());
			return CMD_FAILURE;
		}

		for (int i = 1; i < (int)parameters.size() - 1; i++)
		{
			line = line + parameters[i] + " ";
		}
		line = line + parameters[parameters.size()-1];

		c->WriteChannelWithServ((char*)ServerInstance->Config->ServerName,  "NOTICE %s :User %s is KNOCKing on %s (%s)", c->name.c_str(), user->nick.c_str(), c->name.c_str(), line.c_str());
		user->WriteServ("NOTICE %s :KNOCKing on %s", user->nick.c_str(), c->name.c_str());
		return CMD_SUCCESS;
	}
};

/** Handles channel mode +K
 */
class Knock : public SimpleChannelModeHandler
{
 public:
	Knock(InspIRCd* Instance) : SimpleChannelModeHandler(Instance, 'K') { }
};

class ModuleKnock : public Module
{
	CommandKnock* mycommand;
	Knock* kn;
 public:
	ModuleKnock(InspIRCd* Me) : Module(Me)
	{
		kn = new Knock(ServerInstance);

		if (!ServerInstance->Modes->AddMode(kn))
			throw ModuleException("Could not add new modes!");

		mycommand = new CommandKnock(ServerInstance);
		ServerInstance->AddCommand(mycommand);

	}


	virtual ~ModuleKnock()
	{
		ServerInstance->Modes->DelMode(kn);
		delete kn;
	}

	virtual Version GetVersion()
	{
		return Version("$Id: m_knock.cpp 11223 2009-03-15 12:42:35Z psychon $", VF_COMMON | VF_VENDOR, API_VERSION);
	}
};

MODULE_INIT(ModuleKnock)
