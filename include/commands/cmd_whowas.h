/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd: (C) 2002-2009 InspIRCd Development Team
 * See: http://wiki.inspircd.org/Credits
 *
 * This program is free but copyrighted software; see
 *      the file COPYING for details.
 *
 * ---------------------------------------------------
 */

#ifndef __CMD_WHOWAS_H__
#define __CMD_WHOWAS_H__


// include the common header files

#include "users.h"
#include "channels.h"

/* list of available internal commands */
enum Internals
{
	WHOWAS_ADD = 1,
	WHOWAS_STATS = 2,
	WHOWAS_PRUNE = 3,
	WHOWAS_MAINTAIN = 4
};

/* Forward ref for timer */
class WhoWasMaintainTimer;

/* Forward ref for typedefs */
class WhoWasGroup;

/** Timer that is used to maintain the whowas list, called once an hour
 */
extern WhoWasMaintainTimer* timer;

/** A group of users related by nickname
 */
typedef std::deque<WhoWasGroup*> whowas_set;

/** Sets of users in the whowas system
 */
typedef std::map<irc::string,whowas_set*> whowas_users;

/** Sets of time and users in whowas list
 */
typedef std::deque<std::pair<time_t,irc::string> > whowas_users_fifo;

/** Handle /WHOWAS. These command handlers can be reloaded by the core,
 * and handle basic RFC1459 commands. Commands within modules work
 * the same way, however, they can be fully unloaded, where these
 * may not.
 */
class CommandWhowas : public Command
{
  private:
	/** Whowas container, contains a map of vectors of users tracked by WHOWAS
	 */
	whowas_users whowas;

	/** Whowas container, contains a map of time_t to users tracked by WHOWAS
	 */
	whowas_users_fifo whowas_fifo;

	/* String holding stats so it can be collected
	 */
	std::string stats;

  public:
	CommandWhowas(InspIRCd* Instance);
	/** Handle command.
	 * @param parameters The parameters to the comamnd
	 * @param pcnt The number of parameters passed to teh command
	 * @param user The user issuing the command
	 * @return A value from CmdResult to indicate command success or failure.
	 */
	CmdResult Handle(const std::vector<std::string>& parameters, User *user);
	/** Handle an internal request from another command, the core, or a module
	 * @param Command ID
	 * @param Zero or more parameters, whos form is specified by the command ID.
	 * @return Return CMD_SUCCESS on success, or CMD_FAILURE on failure.
	 * If the command succeeds but should remain local to this server,
	 * return CMD_LOCALONLY.
	 */
	CmdResult HandleInternal(const unsigned int id, const std::deque<classbase*> &parameters);
	void AddToWhoWas(User* user);
	void GetStats(Extensible* ext);
	void PruneWhoWas(time_t t);
	void MaintainWhoWas(time_t t);
	virtual ~CommandWhowas();
};

/** Used to hold WHOWAS information
 */
class WhoWasGroup : public classbase
{
 public:
	/** Real host
	 */
	char* host;
	/** Displayed host
	 */
	char* dhost;
	/** Ident
	 */
	char* ident;
	/** Server name
	 */
	const char* server;
	/** Fullname (GECOS)
	 */
	char* gecos;
	/** Signon time
	 */
	time_t signon;

	/** Initialize this WhoQasFroup with a user
	 */
	WhoWasGroup(User* user);
	/** Destructor
	 */
	~WhoWasGroup();
};

class WhoWasMaintainTimer : public Timer
{
  private:
	InspIRCd* ServerInstance;
  public:
	WhoWasMaintainTimer(InspIRCd* Instance, long interval)
	: Timer(interval, Instance->Time(), true), ServerInstance(Instance)
	{
	}
	virtual void Tick(time_t TIME);
};

#endif
