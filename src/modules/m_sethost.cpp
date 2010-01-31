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

/* $ModDesc: Provides support for the SETHOST command */

/** Handle /SETHOST
 */
class CommandSethost : public Command
{
 private:
	char* hostmap;
 public:
	CommandSethost (InspIRCd* Instance, char* hmap) : Command(Instance,"SETHOST","o",1), hostmap(hmap)
	{
		this->source = "m_sethost.so";
		syntax = "<new-hostname>";
		TRANSLATE2(TR_TEXT, TR_END);
	}

	CmdResult Handle (const std::vector<std::string>& parameters, User *user)
	{
		size_t len = 0;
		for (std::string::const_iterator x = parameters[0].begin(); x != parameters[0].end(); x++, len++)
		{
			if (!hostmap[(const unsigned char)*x])
			{
				user->WriteServ("NOTICE "+std::string(user->nick)+" :*** SETHOST: Invalid characters in hostname");
				return CMD_FAILURE;
			}
		}
		if (len == 0)
		{
			user->WriteServ("NOTICE %s :*** SETHOST: Host must be specified", user->nick.c_str());
			return CMD_FAILURE;
		}
		if (len > 64)
		{
			user->WriteServ("NOTICE %s :*** SETHOST: Host too long",user->nick.c_str());
			return CMD_FAILURE;
		}

		if (user->ChangeDisplayedHost(parameters[0].c_str()))
		{
			ServerInstance->SNO->WriteGlobalSno('a', std::string(user->nick)+" used SETHOST to change their displayed host to "+user->dhost);
			return CMD_LOCALONLY;
		}

		return CMD_FAILURE;
	}
};


class ModuleSetHost : public Module
{
	CommandSethost* mycommand;
	char hostmap[256];
 public:
	ModuleSetHost(InspIRCd* Me)
		: Module(Me)
	{
		OnRehash(NULL);
		mycommand = new CommandSethost(ServerInstance, hostmap);
		ServerInstance->AddCommand(mycommand);
		Implementation eventlist[] = { I_OnRehash };
		ServerInstance->Modules->Attach(eventlist, this, 1);
	}


	void OnRehash(User* user)
	{
		ConfigReader Conf(ServerInstance);
		std::string hmap = Conf.ReadValue("hostname", "charmap", 0);

		if (hmap.empty())
			hmap = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.-_/0123456789";

		memset(hostmap, 0, sizeof(hostmap));
		for (std::string::iterator n = hmap.begin(); n != hmap.end(); n++)
			hostmap[(unsigned char)*n] = 1;
	}

	virtual ~ModuleSetHost()
	{
	}

	virtual Version GetVersion()
	{
		return Version("$Id: m_sethost.cpp 11410 2009-06-14 05:12:44Z danieldg $", VF_VENDOR, API_VERSION);
	}

};

MODULE_INIT(ModuleSetHost)