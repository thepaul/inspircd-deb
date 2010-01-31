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

/* $ModDesc: Provides a SATOPIC command */

#include "inspircd.h"

/** Handle /SATOPIC
 */
class CommandSATopic : public Command
{
 public:
	CommandSATopic (InspIRCd* Instance)
	: Command(Instance,"SATOPIC", "o", 2, 2, false, 0)
	{
		this->source = "m_satopic.so";
		syntax = "<target> <topic>";
	}

	CmdResult Handle (const std::vector<std::string>& parameters, User *user)
	{
		/*
		 * Handles a SATOPIC request. Notifies all +s users.
	 	 */
		Channel* target = ServerInstance->FindChan(parameters[0]);

		if(target)
		{
			std::string newTopic = parameters[1];

			// 3rd parameter overrides access checks
			target->SetTopic(user, newTopic, true);
			ServerInstance->SNO->WriteToSnoMask('a', user->nick + " used SATOPIC on " + target->name + ", new topic: " + newTopic);
			ServerInstance->PI->SendSNONotice("A", user->nick + " used SATOPIC on " + target->name + ", new topic: " + newTopic);

			return CMD_LOCALONLY;
		}
		else
		{
			user->WriteNumeric(401, "%s %s :No such nick/channel", user->nick.c_str(), parameters[0].c_str());
			return CMD_FAILURE;
		}
	}
};

class ModuleSATopic : public Module
{
	CommandSATopic*	mycommand;
 public:
	ModuleSATopic(InspIRCd* Me)
	: Module(Me)
	{
		mycommand = new CommandSATopic(ServerInstance);
		ServerInstance->AddCommand(mycommand);
	}

	virtual ~ModuleSATopic()
	{
	}

	virtual Version GetVersion()
	{
		return Version("$Id: m_satopic.cpp 11304 2009-04-16 15:51:05Z w00t $", VF_VENDOR, API_VERSION);
	}
};

MODULE_INIT(ModuleSATopic)
