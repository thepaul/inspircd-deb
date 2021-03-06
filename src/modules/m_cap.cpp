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
#include "m_cap.h"

/* $ModDesc: Provides the CAP negotiation mechanism seen in ratbox-derived ircds */

/*
CAP LS
:alfred.staticbox.net CAP * LS :multi-prefix sasl
CAP REQ :multi-prefix
:alfred.staticbox.net CAP * ACK :multi-prefix
CAP CLEAR
:alfred.staticbox.net CAP * ACK :-multi-prefix
CAP REQ :multi-prefix
:alfred.staticbox.net CAP * ACK :multi-prefix
CAP LIST
:alfred.staticbox.net CAP * LIST :multi-prefix
CAP END
*/

/** Handle /CAP
 */
class CommandCAP : public Command
{
	Module* Creator;
 public:
	CommandCAP (InspIRCd* Instance, Module* mod) : Command(Instance,"CAP", 0, 1, true), Creator(mod)
	{
		this->source = "m_cap.so";
	}

	CmdResult Handle (const std::vector<std::string> &parameters, User *user)
	{
		irc::string subcommand = parameters[0].c_str();

		if (subcommand == "REQ")
		{
			CapData Data;

			Data.type = subcommand;
			Data.user = user;
			Data.creator = this->Creator;

			if (parameters.size() < 2)
				return CMD_FAILURE;

			// tokenize the input into a nice list of requested caps
			std::string param = parameters[1];
			std::string cap_;
			irc::spacesepstream cap_stream(param);

			while (cap_stream.GetToken(cap_))
			{
				Data.wanted.push_back(cap_);
			}

			user->Extend("CAP_REGHOLD");
			Event event((char*) &Data, (Module*)this->Creator, "cap_req");
			event.Send(this->ServerInstance);

			if (Data.ack.size() > 0)
			{
				std::string AckResult = irc::stringjoiner(" ", Data.ack, 0, Data.ack.size() - 1).GetJoined();
				user->WriteServ("CAP * ACK :%s", AckResult.c_str());
			}

			if (Data.wanted.size() > 0)
			{
				std::string NakResult = irc::stringjoiner(" ", Data.wanted, 0, Data.wanted.size() - 1).GetJoined();
				user->WriteServ("CAP * NAK :%s", NakResult.c_str());
			}
		}
		else if (subcommand == "END")
		{
			user->Shrink("CAP_REGHOLD");
		}
		else if ((subcommand == "LS") || (subcommand == "LIST"))
		{
			CapData Data;

			Data.type = subcommand;
			Data.user = user;
			Data.creator = this->Creator;

			user->Extend("CAP_REGHOLD");
			Event event((char*) &Data, (Module*)this->Creator, subcommand == "LS" ? "cap_ls" : "cap_list");
			event.Send(this->ServerInstance);

			std::string Result;
			if (Data.wanted.size() > 0)
				Result = irc::stringjoiner(" ", Data.wanted, 0, Data.wanted.size() - 1).GetJoined();
			else
				Result = "";

			user->WriteServ("CAP * %s :%s", subcommand.c_str(), Result.c_str());
		}
		else if (subcommand == "CLEAR")
		{
			CapData Data;

			Data.type = subcommand;
			Data.user = user;
			Data.creator = this->Creator;

			user->Extend("CAP_REGHOLD");
			Event event((char*) &Data, (Module*)this->Creator, "cap_clear");
			event.Send(this->ServerInstance);

			std::string Result = irc::stringjoiner(" ", Data.ack, 0, Data.ack.size() - 1).GetJoined();
			user->WriteServ("CAP * ACK :%s", Result.c_str());
		}
		else
		{
			user->WriteNumeric(ERR_INVALIDCAPSUBCOMMAND, "* %s :Invalid CAP subcommand", subcommand.c_str());
		}

		return CMD_FAILURE;
	}
};

class ModuleCAP : public Module
{
	CommandCAP* newcommand;
 public:
	ModuleCAP(InspIRCd* Me)
		: Module(Me)
	{
		// Create a new command
		newcommand = new CommandCAP(ServerInstance, this);
		ServerInstance->AddCommand(newcommand);

		Implementation eventlist[] = { I_OnCheckReady };
		ServerInstance->Modules->Attach(eventlist, this, 1);
	}

	virtual bool OnCheckReady(User* user)
	{
		/* Users in CAP state get held until CAP END */
		if (user->GetExt("CAP_REGHOLD"))
			return false;

		return true;
	}

	virtual ~ModuleCAP()
	{
	}

	virtual Version GetVersion()
	{
		return Version("$Id: m_cap.cpp 11223 2009-03-15 12:42:35Z psychon $", VF_VENDOR, API_VERSION);
	}
};

MODULE_INIT(ModuleCAP)

