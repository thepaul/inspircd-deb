/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd: (C) 2002-2009 InspIRCd Development Team
 * See: http://wiki.inspircd.org/Credits
 *
 * This program is free but copyrighted software; see
 *          the file COPYING for details.
 *
 * ---------------------------------------------------
 */

#include "inspircd.h"
#include "xline.h"

/* $ModDesc: Provides the /shun command, which stops a user executing all commands except PING and PONG. */

class Shun : public XLine
{
public:
	std::string matchtext;

	Shun(InspIRCd* Instance, time_t s_time, long d, const char* src, const char* re, const char *shunmask) : XLine(Instance, s_time, d, src, re, "SHUN")
	{
		this->matchtext = shunmask;
	}

	~Shun()
	{
	}

	bool Matches(User *u)
	{
		// E: overrides shun
		if (u->exempt)
			return false;

		if (InspIRCd::Match(u->GetFullHost(), matchtext) || InspIRCd::Match(u->GetFullRealHost(), matchtext) || InspIRCd::Match(u->nick+"!"+u->ident+"@"+u->GetIPString(), matchtext))
			return true;

		return false;
	}

	bool Matches(const std::string &s)
	{
		if (matchtext == s)
			return true;
		return false;
	}

	void Apply(User *u)
	{
	}


	void DisplayExpiry()
	{
		ServerInstance->SNO->WriteToSnoMask('x',"Removing expired shun %s (set by %s %ld seconds ago)", this->matchtext.c_str(), this->source, (long int)(ServerInstance->Time() - this->set_time));
	}

	const char* Displayable()
	{
		return matchtext.c_str();
	}
};

/** An XLineFactory specialized to generate shun pointers
 */
class ShunFactory : public XLineFactory
{
 public:
	ShunFactory(InspIRCd* Instance) : XLineFactory(Instance, "SHUN") { }

	/** Generate a shun
 	*/
	XLine* Generate(time_t set_time, long duration, const char* source, const char* reason, const char* xline_specific_mask)
	{
		return new Shun(ServerInstance, set_time, duration, source, reason, xline_specific_mask);
	}
};

//typedef std::vector<Shun> shunlist;

class CommandShun : public Command
{
 public:
	CommandShun(InspIRCd* Me) : Command(Me, "SHUN", "o", 1, 3)
	{
		this->source = "m_shun.so";
		this->syntax = "<nick!user@hostmask> [<shun-duration>] :<reason>";
	}

	CmdResult Handle(const std::vector<std::string>& parameters, User *user)
	{
		/* syntax: SHUN nick!user@host time :reason goes here */
		/* 'time' is a human-readable timestring, like 2d3h2s. */

		std::string target = parameters[0];
		
		User *find = ServerInstance->FindNick(target.c_str());
		if (find)
			target = std::string("*!*@") + find->GetIPString();

		if (parameters.size() == 1)
		{
			if (ServerInstance->XLines->DelLine(target.c_str(), "SHUN", user))
			{
				ServerInstance->SNO->WriteToSnoMask('x',"%s Removed shun on %s.",user->nick.c_str(),target.c_str());
			}
			else
			{
				// XXX todo implement stats
				user->WriteServ("NOTICE %s :*** Shun %s not found in list, try /stats S.",user->nick.c_str(),target.c_str());
			}

			return CMD_SUCCESS;
		}
		else if (parameters.size() >= 2)
		{
			// Adding - XXX todo make this respect <insane> tag perhaps..
			long duration;
			std::string expr;
			if (parameters.size() > 2)
			{
				duration = ServerInstance->Duration(parameters[1]);
				expr = parameters[2];
			}
			else
			{
				duration = 0;
				expr = parameters[1];
			}
			Shun *r = NULL;

			try
			{
				r = new Shun(ServerInstance, ServerInstance->Time(), duration, user->nick.c_str(), expr.c_str(), target.c_str());
			}
			catch (...)
			{
				; // Do nothing. If we get here, the regex was fucked up, and they already got told it fucked up.
			}

			if (r)
			{
				if (ServerInstance->XLines->AddLine(r, user))
				{
					if (!duration)
					{
						ServerInstance->SNO->WriteToSnoMask('x',"%s added permanent shun for %s: %s",
							user->nick.c_str(), target.c_str(), expr.c_str());
					}
					else
					{
						time_t c_requires_crap = duration + ServerInstance->Time();
						ServerInstance->SNO->WriteToSnoMask('x', "%s added timed shun for %s, expires on %s: %s",
							user->nick.c_str(), target.c_str(), ServerInstance->TimeString(c_requires_crap).c_str(), expr.c_str());
					}

					ServerInstance->XLines->ApplyLines();
				}
				else
				{
					delete r;
					user->WriteServ("NOTICE %s :*** Shun for %s already exists", user->nick.c_str(), expr.c_str());
				}
			}
		}

		return CMD_FAILURE;
	}
};

class ModuleShun : public Module
{
	CommandShun* mycommand;
	ShunFactory *f;
	std::set<std::string> ShunEnabledCommands;
	bool NotifyOfShun;
	bool affectopers;

 public:
	ModuleShun(InspIRCd* Me) : Module(Me)
	{
		f = new ShunFactory(ServerInstance);
		ServerInstance->XLines->RegisterFactory(f);

		mycommand = new CommandShun(ServerInstance);
		ServerInstance->AddCommand(mycommand);

		Implementation eventlist[] = { I_OnStats, I_OnPreCommand, I_OnUserConnect, I_OnRehash };
		ServerInstance->Modules->Attach(eventlist, this, 4);
		OnRehash(NULL);
	}

	virtual ~ModuleShun()
	{
		ServerInstance->XLines->DelAll("SHUN");
		ServerInstance->XLines->UnregisterFactory(f);
	}

	virtual int OnStats(char symbol, User* user, string_list& out)
	{
		if (symbol != 'S')
			return 0;

		ServerInstance->XLines->InvokeStats("SHUN", 223, user, out);
		return 1;
	}

	virtual void OnRehash(User* user)
	{
		ConfigReader MyConf(ServerInstance);
		std::string cmds = MyConf.ReadValue("shun", "enabledcommands", 0);

		if (cmds.empty())
			cmds = "PING PONG QUIT";

		ShunEnabledCommands.clear();
		NotifyOfShun = true;
		affectopers = false;

		std::stringstream dcmds(cmds);
		std::string thiscmd;

		while (dcmds >> thiscmd)
		{
			ShunEnabledCommands.insert(thiscmd);
		}

		NotifyOfShun = MyConf.ReadFlag("shun", "notifyuser", "yes", 0);
		affectopers = MyConf.ReadFlag("shun", "affectopers", "no", 0);
	}

	virtual void OnUserConnect(User* user)
	{
		if (!IS_LOCAL(user))
			return;

		// Apply lines on user connect
		XLine *rl = ServerInstance->XLines->MatchesLine("SHUN", user);

		if (rl)
		{
			// Bang. :P
			rl->Apply(user);
		}
	}

	virtual int OnPreCommand(std::string &command, std::vector<std::string>& parameters, User* user, bool validated, const std::string &original_line)
	{
		if (validated)
			return 0;

		if (!ServerInstance->XLines->MatchesLine("SHUN", user))
		{
			/* Not shunned, don't touch. */
			return 0;
		}

		if (!affectopers && IS_OPER(user))
		{
			/* Don't do anything if the user is an operator and affectopers isn't set */
			return 0;
		}

		std::set<std::string>::iterator i = ShunEnabledCommands.find(command);

		if (i == ShunEnabledCommands.end())
		{
			if (NotifyOfShun)
				user->WriteServ("NOTICE %s :*** Command %s not processed, as you have been blocked from issuing commands (SHUN)", user->nick.c_str(), command.c_str());
			return 1;
		}

		if (command == "QUIT")
		{
			/* Allow QUIT but dont show any quit message */
			parameters.clear();
		}
		else if (command == "PART")
		{
			/* same for PART */
			parameters[1] = "";
		}

		/* if we're here, allow the command. */
		return 0;
	}

	virtual Version GetVersion()
	{
		return Version("$Id: m_shun.cpp 11388 2009-05-24 02:53:45Z danieldg $",VF_VENDOR|VF_COMMON,API_VERSION);
	}
};

MODULE_INIT(ModuleShun)

