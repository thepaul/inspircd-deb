/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd: (C) 2002-2009 InspIRCd Development Team
 * See: http://wiki.inspircd.org/Credits
 *
 * This program is free but copyrighted software; see
 *	    the file COPYING for details.
 *
 * ---------------------------------------------------
 */

/* $Core */

#include <signal.h>
#include "exitcodes.h"
#include "inspircd.h"


void InspIRCd::SignalHandler(int signal)
{
	switch (signal)
	{
		case SIGHUP:
			Rehash("due to SIGHUP");
			break;
		case SIGTERM:
			Exit(signal);
			break;
	}
}

void InspIRCd::Exit(int status)
{
#ifdef WINDOWS
	if (WindowsIPC)
		delete WindowsIPC;
	SetServiceStopped(status);
#endif
	if (this)
	{
		this->SendError("Exiting with status " + ConvToStr(status) + " (" + std::string(ExitCodes[status]) + ")");
		this->Cleanup();
	}
	exit (status);
}

void RehashHandler::Call(const std::string &reason)
{
	Server->SNO->WriteToSnoMask('a', "Rehashing config file %s %s",ServerConfig::CleanFilename(Server->ConfigFileName), reason.c_str());
	Server->RehashUsersAndChans();
	FOREACH_MOD_I(Server, I_OnGarbageCollect, OnGarbageCollect());
	if (!Server->ConfigThread)
	{
		Server->ConfigThread = new ConfigReaderThread(Server, "");
		Server->Threads->Start(Server->ConfigThread);
	}
}

void InspIRCd::RehashServer()
{
	this->Rehash("");
}

std::string InspIRCd::GetVersionString()
{
	char versiondata[MAXBUF];
	if (*Config->CustomVersion)
	{
		snprintf(versiondata,MAXBUF,"InspIRCd-1.2 %s :%s",Config->ServerName,Config->CustomVersion);
	}
	else
	{
		snprintf(versiondata,MAXBUF,"InspIRCd-1.2 %s :%s (%s) [FLAGS=%s,%s,%s]",Config->ServerName,SYSTEM,VERSION,REVISION,SE->GetName().c_str(),Config->sid);
	}
	return versiondata;
}

void InspIRCd::BuildISupport()
{
	// the neatest way to construct the initial 005 numeric, considering the number of configure constants to go in it...
	std::stringstream v;
	v << "WALLCHOPS WALLVOICES MODES=" << Config->Limits.MaxModes - 1 << " CHANTYPES=# PREFIX=" << this->Modes->BuildPrefixes() << " MAP MAXCHANNELS=" << Config->MaxChans << " MAXBANS=60 VBANLIST NICKLEN=" << Config->Limits.NickMax - 1;
	v << " CASEMAPPING=rfc1459 STATUSMSG=@" << (this->Config->AllowHalfop ? "%" : "") << "+ CHARSET=ascii TOPICLEN=" << Config->Limits.MaxTopic - 1 << " KICKLEN=" << Config->Limits.MaxKick - 1 << " MAXTARGETS=" << Config->MaxTargets - 1;
	v << " AWAYLEN=" << Config->Limits.MaxAway - 1 << " CHANMODES=" << this->Modes->GiveModeList(MASK_CHANNEL) << " FNC NETWORK=" << Config->Network << " MAXPARA=32 ELIST=MU";
	Config->data005 = v.str();
	FOREACH_MOD_I(this,I_On005Numeric,On005Numeric(Config->data005));
	Config->Update005();
}

std::string InspIRCd::GetRevision()
{
	return REVISION;
}

void InspIRCd::AddServerName(const std::string &servername)
{
	servernamelist::iterator itr = servernames.begin();
	for(; itr != servernames.end(); ++itr)
		if(**itr == servername)
			return;

	std::string * ns = new std::string(servername);
	servernames.push_back(ns);
}

const char* InspIRCd::FindServerNamePtr(const std::string &servername)
{
	servernamelist::iterator itr = servernames.begin();
	for(; itr != servernames.end(); ++itr)
		if(**itr == servername)
			return (*itr)->c_str();

	servernames.push_back(new std::string(servername));
	itr = --servernames.end();
	return (*itr)->c_str();
}

bool InspIRCd::FindServerName(const std::string &servername)
{
	servernamelist::iterator itr = servernames.begin();
	for(; itr != servernames.end(); ++itr)
		if(**itr == servername)
			return true;
	return false;
}

void InspIRCd::IncrementUID(int pos)
{
	/*
	 * Okay. The rules for generating a UID go like this...
	 * -- > ABCDEFGHIJKLMNOPQRSTUVWXYZ --> 012345679 --> WRAP
	 * That is, we start at A. When we reach Z, we go to 0. At 9, we go to
	 * A again, in an iterative fashion.. so..
	 * AAA9 -> AABA, and so on. -- w00t
	 */
	if (pos == 3)
	{
		// At pos 3, if we hit '9', we've run out of available UIDs, and need to reset to AAA..AAA.
		if (current_uid[pos] == '9')
		{
			for (int i = 3; i < UUID_LENGTH; i++)
			{
				current_uid[i] = 'A';
				pos  = UUID_LENGTH - 1;
			}
		}
		else
		{
			// Buf if we haven't, just keep incrementing merrily.
			current_uid[pos]++;
		}
	}
	else
	{
		// If we hit Z, wrap around to 0.
		if (current_uid[pos] == 'Z')
		{
			current_uid[pos] = '0';
		}
		else if (current_uid[pos] == '9')
		{
			/*
			 * Or, if we hit 9, wrap around to pos = 'A' and (pos - 1)++,
			 * e.g. A9 -> BA -> BB ..
			 */
			current_uid[pos] = 'A';
			this->IncrementUID(pos - 1);
		}
		else
		{
			// Anything else, nobody gives a shit. Just increment.
			current_uid[pos]++;
		}
	}
}

/*
 * Retrieve the next valid UUID that is free for this server.
 */
std::string InspIRCd::GetUID()
{
	static int curindex = -1;

	/*
	 * If -1, we're setting up. Copy SID into the first three digits, 9's to the rest, null term at the end
	 * Why 9? Well, we increment before we find, otherwise we have an unnecessary copy, and I want UID to start at AAA..AA
	 * and not AA..AB. So by initialising to 99999, we force it to rollover to AAAAA on the first IncrementUID call.
	 * Kind of silly, but I like how it looks.
	 *		-- w
	 */
	if (curindex == -1)
	{
		current_uid[0] = Config->sid[0];
		current_uid[1] = Config->sid[1];
		current_uid[2] = Config->sid[2];

		for (int i = 3; i < (UUID_LENGTH - 1); i++)
			current_uid[i] = '9';

		curindex = UUID_LENGTH - 2; // look at the end of the string now kthx, ignore null

		// Null terminator. Important.
		current_uid[UUID_LENGTH - 1] = '\0';
	}

	while (1)
	{
		// Add one to the last UID
		this->IncrementUID(curindex);

		if (this->FindUUID(current_uid))
		{
			/*
			 * It's in use. We need to try the loop again.
			 */
			continue;
		}

		return current_uid;
	}

	/* not reached. */
	return "";
}



