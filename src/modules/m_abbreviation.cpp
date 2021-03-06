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

/* $ModDesc: Provides the ability to abbreviate commands a-la BBC BASIC keywords. */

class ModuleAbbreviation : public Module
{

 public:

	ModuleAbbreviation(InspIRCd* Me)
		: Module(Me)
	{
		Me->Modules->Attach(I_OnPreCommand, this);
		/* Must do this first */
		Me->Modules->SetPriority(this, I_OnPreCommand, PRIORITY_FIRST);
	}

	virtual Version GetVersion()
	{
		return Version("$Id: m_abbreviation.cpp 11223 2009-03-15 12:42:35Z psychon $",VF_VENDOR,API_VERSION);
	}

	virtual int OnPreCommand(std::string &command, std::vector<std::string> &parameters, User *user, bool validated, const std::string &original_line)
	{
		/* Command is already validated, has a length of 0, or last character is not a . */
		if (validated || command.empty() || *command.rbegin() != '.')
			return 0;

		/* Whack the . off the end */
		command.erase(command.end() - 1);

		/* Look for any command that starts with the same characters, if it does, replace the command string with it */
		size_t clen = command.length();
		std::string foundcommand, matchlist;
		bool foundmatch = false;
		for (Commandtable::iterator n = ServerInstance->Parser->cmdlist.begin(); n != ServerInstance->Parser->cmdlist.end(); ++n)
		{
			if (n->first.length() < clen)
				continue;

			if (command == n->first.substr(0, clen))
			{
				if (matchlist.length() > 450)
				{
					user->WriteNumeric(420, "%s :Ambiguous abbreviation and too many possible matches.", user->nick.c_str());
					return true;
				}

				if (!foundmatch)
				{
					/* Found the command */
					foundcommand = n->first;
					foundmatch = true;
				}
				else
					matchlist.append(" ").append(n->first);
			}
		}

		/* Ambiguous command, list the matches */
		if (!matchlist.empty())
		{
			user->WriteNumeric(420, "%s :Ambiguous abbreviation, posssible matches: %s%s", user->nick.c_str(), foundcommand.c_str(), matchlist.c_str());
			return true;
		}

		if (foundcommand.empty())
		{
			/* No match, we have to put the . back again so that the invalid command numeric looks correct. */
			command += '.';
		}
		else
		{
			command = foundcommand;
		}

		return false;
	}
};

MODULE_INIT(ModuleAbbreviation)
