/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd: (C) 2002-2009 InspIRCd Development Team
 * See: http://wiki.inspircd.org/Credits
 *
 * This program is free but copyrighted software; see
 *	  the file COPYING for details.
 *
 * ---------------------------------------------------
 */

/* $ModDesc: Provides a spanning tree server link protocol */

#include "inspircd.h"
#include "commands/cmd_whois.h"
#include "commands/cmd_stats.h"
#include "socket.h"
#include "xline.h"
#include "transport.h"

#include "m_spanningtree/main.h"
#include "m_spanningtree/utils.h"
#include "m_spanningtree/treeserver.h"
#include "m_spanningtree/treesocket.h"

/* $ModDep: m_spanningtree/main.h m_spanningtree/utils.h m_spanningtree/treeserver.h m_spanningtree/treesocket.h */

int ModuleSpanningTree::HandleSquit(const std::vector<std::string>& parameters, User* user)
{
	TreeServer* s = Utils->FindServerMask(parameters[0]);
	if (s)
	{
		if (s == Utils->TreeRoot)
		{
			user->WriteServ("NOTICE %s :*** SQUIT: Foolish mortal, you cannot make a server SQUIT itself! (%s matches local server name)",user->nick.c_str(),parameters[0].c_str());
			return 1;
		}

		TreeSocket* sock = s->GetSocket();

		if (sock)
		{
			ServerInstance->SNO->WriteToSnoMask('l',"SQUIT: Server \002%s\002 removed from network by %s",parameters[0].c_str(),user->nick.c_str());
			sock->Squit(s,std::string("Server quit by ") + user->GetFullRealHost());
			ServerInstance->SE->DelFd(sock);
			sock->Close();
		}
		else
		{
			user->WriteServ("NOTICE %s :*** SQUIT may not be used to remove remote servers. Please use RSQUIT instead.",user->nick.c_str());
		}
	}
	else
	{
		 user->WriteServ("NOTICE %s :*** SQUIT: The server \002%s\002 does not exist on the network.",user->nick.c_str(),parameters[0].c_str());
	}
	return 1;
}

