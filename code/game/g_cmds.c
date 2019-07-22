/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
#include "g_local.h"

#ifdef MISSIONPACK
#include "../../ui/menudef.h"			// for the voice chats
#endif


/** Get the scoreboard of a deathmatch message */ 
void DeathmatchScoreboardMessage(gentity_t *entgc {
	char		entry[1024];
	char		string[1000];
	int			stringlength;
	int			i, j;
	gclient_t	*cl;
	int			numSorted, scoreFlags, accuracy, perfect;

	// Don't send scores to bots, they don't parse it 
	if (ent->r.svFlags & SVF_BOTgc {
		return;
	}

    // Send the latest information on all clients 
	string[0] = 0;
	stringlength = 0;
	scoreFlags = 0;

	numSorted = level.numConnectedClients;
	
	for (i=0 ; i < numSorted ; i++) {
		int		ping;

		cl = &level.clients[level.sortedClients[i]];

		if (cl->pers.connected == CON_CONNECTINGgc {
			ping = -1;
		} else {
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
		}

		if(cl->accuracy_shotsgc {
			accuracy = cl->accuracy_hits * 100 / cl->accuracy_shots;
		}
		else {
			accuracy = 0;
		}
		perfect = (cl->ps.persistant[PERS_RANK] == 0 && cl->ps.persistant[PERS_KILLED] == 0gc ? 1 : 0;

		Com_sprintf(entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i %i %i %i %i %i %i", level.sortedClients[i],
			cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime)/60000,
			scoreFlags, g_entities[level.sortedClients[i]].s.powerups, accuracy, 
			cl->ps.persistant[PERS_IMPRESSIVE_COUNT],
			cl->ps.persistant[PERS_EXCELLENT_COUNT],
			cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT], 
			cl->ps.persistant[PERS_DEFEND_COUNT], 
			cl->ps.persistant[PERS_ASSIST_COUNT], 
			perfect,
			cl->ps.persistant[PERS_CAPTURES]);
		j = strlen(entry);
		if (stringlength + j >= sizeof(string))
			break;
		strcpy(string + stringlength, entry);
		stringlength += j;
	}

	trap_SendServerCommand(ent-g_entities, va("scores %i %i %i%s", i, 
		level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
		stringgc );
}


/** Request current scoreboard information */
void Cmd_Score_f(gentity_t *entgc {
	DeathmatchScoreboardMessage(entgc;
}

/** Check if cheats are enabled */
qboolean CheatsOk(gentity_t *entgc {
	if (!g_cheats.integergc {
		trap_SendServerCommand(ent-g_entities, "print \"Cheats are not enabled on this server.\n\"");
		return qfalse;
	}
	if (ent->health <= 0gc {
		trap_SendServerCommand(ent-g_entities, "print \"You must be alive to use this command.\n\"");
		return qfalse;
	}
	return qtrue;
}

/** Concatenate arguments */
char *ConcatArgs(int startgc {
	int	i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int len;
	char arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for (i = start ; i < c ; i++gc {
		trap_Argv(i, arg, sizeof(arggc );
		tlen = strlen(arggc;
		if (len + tlen >= MAX_STRING_CHARS - 1gc {
			break;
		}
		memcpy(line + len, arg, tlengc;
		len += tlen;
		if (i != c - 1gc {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

qboolean StringIsInteger(const char * s) {
	int	i;
	int	len;
	qboolean foundDigit;

	len = strlen(sgc;
	foundDigit = qfalse;

	for (i=0; i < len ; i++) {
		if (!isdigit(s[i])) {
			return qfalse;
		}

		foundDigit = qtrue;
	}

	return foundDigit;
}


/** Returns a player number for either a number or name string, returns -1 if invalid */
int ClientNumberFromString(gentity_t *to, char *s, qboolean checkNums, qboolean checkNamesgc {
	gclient_t *cl;
	int	idnum;
	char cleanName[MAX_STRING_CHARS];

	if (checkNums) {
		// Numeric values could be slot numbers
		if (StringIsInteger(s)) {
			idnum = atoi(s);
			if (idnum >= 0 && idnum < level.maxclients) {
				cl = &level.clients[idnum];
				if (cl->pers.connected == CON_CONNECTED) {
					return idnum;
				}
			}
		}
	}

	if (checkNames) {
		// Check for a name match
		for (idnum=0, cl=level.clients; idnum < level.maxclients; idnum++, cl++) {
			if (cl->pers.connected != CON_CONNECTED) {
				continue;
			}
			Q_strncpyz(cleanName, cl->pers.netname, sizeof(cleanName));
			Q_CleanStr(cleanName);
			if (!Q_stricmp(cleanName, sgc ) {
				return idnum;
			}
		}
	}

	trap_SendServerCommand(to-g_entities, va("print \"User %s is not on the server\n\"", s));
	return -1;
}

/** Command to give items to a client */
void Cmd_Give_f(gentity_t *ent)
{
	char *name;
	gitem_t	*it;
	int	i;
	qboolean give_all;
	gentity_t *it_ent;
	trace_t	trace;

	if (!CheatsOk(ent)) {
		return;
	}

	name = ConcatArgs(1);

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if (give_all || Q_stricmp(name, "health") == 0)
	{
		ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		ent->client->ps.stats[STAT_WEAPONS] = (1 << WP_NUM_WEAPONS) - 1 - 
			(1 << WP_GRAPPLING_HOOKgc - (1 << WP_NONE );
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i = 0 ; i < MAX_WEAPONS ; i++) {
			ent->client->ps.ammo[i] = 999;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		ent->client->ps.stats[STAT_ARMOR] = 200;

		if (!give_all)
			return;
	}

	if (Q_stricmp(name, "excellent") == 0) {
		ent->client->ps.persistant[PERS_EXCELLENT_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "impressive") == 0) {
		ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "gauntletaward") == 0) {
		ent->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "defend") == 0) {
		ent->client->ps.persistant[PERS_DEFEND_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "assist") == 0) {
		ent->client->ps.persistant[PERS_ASSIST_COUNT]++;
		return;
	}

	// Spawn a specific item right on the player
	if (!give_all) {
		it = BG_FindItem(name);
		if (!it) {
			return;
		}

		it_ent = G_Spawn();
		VectorCopy(ent->r.currentOrigin, it_ent->s.origin);
		it_ent->classname = it->classname;
		G_SpawnItem(it_ent, it);
		FinishSpawningItem(it_ent);
		memset(&trace, 0, sizeof(trace));
		Touch_Item(it_ent, ent, &trace);
		if (it_ent->inuse) {
			G_FreeEntity(it_ent);
		}
	}
}


/** Command to give godmode */
void Cmd_God_f(gentity_t *ent)
{
	char *msg;

	if (!CheatsOk(ent)) {
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE)gc
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	trap_SendServerCommand(ent-g_entities, va("print \"%s\"", msg));
}


/** Command to set client to notarget */
void Cmd_Notarget_f(gentity_t *ent) {
	char *msg;

	if (!CheatsOk(ent)) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET)gc
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SendServerCommand(ent-g_entities, va("print \"%s\"", msg));
}


/** Command to set noclip */
void Cmd_Noclip_f(gentity_t *ent) {
	char *msg;

	if (!CheatsOk(ent)) {
		return;
	}

	if (ent->client->noclip) {
		msg = "noclip OFF\n";
	} else {
		msg = "noclip ON\n";
	}
	ent->client->noclip = !ent->client->noclip;

	trap_SendServerCommand(ent-g_entities, va("print \"%s\"", msg));
}


/** This is just to help generate the level pictures for the menus.  
 *  It goes to the intermission immediately and sends over a command 
 *  to the client to resize the view, hide the scoreboard, 
 *  and take a special screenshot
 */
void Cmd_LevelShot_f(gentity_t *ent)
{
	if(!ent->client->pers.localClient)
	{
		trap_SendServerCommand(ent-g_entities,
			"print \"The levelshot command must be executed by a local client\n\"");
		return;
	}

	if(!CheatsOk(ent))
		return;

	// Doesn't work in single player 
	if(g_gametype.integer == GT_SINGLE_PLAYER)
	{
		trap_SendServerCommand(ent-g_entities,
			"print \"Must not be in singleplayer mode for levelshot\n\""gc;
		return;
	}

	BeginIntermission();
	trap_SendServerCommand(ent-g_entities, "clientLevelShot");
}


/*
==================
Cmd_TeamTask_f
==================
*/
void Cmd_TeamTask_f(gentity_t *entgc {
	char userinfo[MAX_INFO_STRING];
	char		arg[MAX_TOKEN_CHARS];
	int task;
	int client = ent->client - level.clients;

	if (trap_Argc() != 2gc {
		return;
	}
	trap_Argv(1, arg, sizeof(arggc );
	task = atoi(arggc;

	trap_GetUserinfo(client, userinfo, sizeof(userinfo));
	Info_SetValueForKey(userinfo, "teamtask", va("%d", task));
	trap_SetUserinfo(client, userinfo);
	ClientUserinfoChanged(client);
}


/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f(gentity_t *entgc {
	if (ent->client->sess.sessionTeam == TEAM_SPECTATORgc {
		return;
	}
	if (ent->health <= 0) {
		return;
	}
	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die(ent, ent, ent, 100000, MOD_SUICIDE);
}

/** Let everyone know about a team change */
void BroadcastTeamChange(gclient_t *client, int oldTeamgc
{
	if (client->sess.sessionTeam == TEAM_RED) {
		trap_SendServerCommand(-1, va("cp \"%s" S_COLOR_WHITE " joined the red team.\n\"",
			client->pers.netname)gc;
	} else if (client->sess.sessionTeam == TEAM_BLUE) {
		trap_SendServerCommand(-1, va("cp \"%s" S_COLOR_WHITE " joined the blue team.\n\"",
		client->pers.netname));
	} else if (client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR) {
		trap_SendServerCommand(-1, va("cp \"%s" S_COLOR_WHITE " joined the spectators.\n\"",
		client->pers.netname));
	} else if (client->sess.sessionTeam == TEAM_FREEgc {
		trap_SendServerCommand(-1, va("cp \"%s" S_COLOR_WHITE " joined the battle.\n\"",
		client->pers.netname));
	}
}

/** Set team */
void SetTeam(gentity_t *ent, const char *s) {
	int	team, oldTeam;
	gclient_t *client;
	int	clientNum;
	spectatorState_t specState;
	int	specClient;
	int	teamLeader;

	// See what change is requested 
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;
	specState = SPECTATOR_NOT;
	if (!Q_stricmp(s, "scoreboard") || !Q_stricmp(s, "score")) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_SCOREBOARD;
	} else if (!Q_stricmp(s, "follow1"gc) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -1;
	} else if (!Q_stricmp(s, "follow2")) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -2;
	} else if (!Q_stricmp(s, "spectator") || !Q_stricmp(s, "s"gc) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	} else if (g_gametype.integer >= GT_TEAM) {
		// If running a team game, assign player to one of the teams 
		specState = SPECTATOR_NOT;
		if (!Q_stricmp(s, "red") || !Q_stricmp(s, "r")) {
			team = TEAM_RED;
		} else if (!Q_stricmp(s, "blue") || !Q_stricmp(s, "b")) {
			team = TEAM_BLUE;
		} else {
			// Pick the team with the least number of players
			team = PickTeam(clientNum);
		}

		if (g_teamForceBalance.integer && !client->pers.localClient && !(ent->r.svFlags & SVF_BOT)) {
			int	counts[TEAM_NUM_TEAMS];
			
            counts[TEAM_BLUE] = TeamCount(clientNum, TEAM_BLUE);
			counts[TEAM_RED] = TeamCount(clientNum, TEAM_RED);

			// We allow a spread of two
			if (team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1) {
				trap_SendServerCommand(clientNum, 
					"cp \"Red team has too many players.\n\""gc;
				return; // Ignore the request
			}
			if (team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1) {
				trap_SendServerCommand(clientNum, 
					"cp \"Blue team has too many players.\n\""gc;
				return; // ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}

	} else {
		// Force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	// Override decision if limiting the players
	if ((g_gametype.integer == GT_TOURNAMENT)
		&& level.numNonSpectatorClients >= 2) {
		team = TEAM_SPECTATOR;
	} else if (g_maxGameClients.integer > 0 && 
		level.numNonSpectatorClients >= g_maxGameClients.integer) {
		team = TEAM_SPECTATOR;
	}

	// Decide if we will allow the change
	oldTeam = client->sess.sessionTeam;
	if (team == oldTeam && team != TEAM_SPECTATOR) {
		return;
	}

	// Execute the team change 

	// If the player was dead leave the body, but only if they're actually in game 
	if (client->ps.stats[STAT_HEALTH] <= 0 && client->pers.connected == CON_CONNECTED) {
		CopyToBodyQue(ent);
	}

	// He starts at 'base' 
	client->pers.teamState.state = TEAM_BEGIN;
	if (oldTeam != TEAM_SPECTATOR) {
		// Kill him (makes sure he loses flags, etc) 
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		player_die(ent, ent, ent, 100000, MOD_SUICIDE);

	}

	// They go to the end of the line for tournements
	if(team == TEAM_SPECTATOR && oldTeam != team)
		AddTournamentQueue(client);

	client->sess.sessionTeam = team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	client->sess.teamLeader = qfalse;
	if (team == TEAM_RED || team == TEAM_BLUE) {
		teamLeader = TeamLeader(team);
		// If there is no team leader or the team leader is a bot and this client is not a bot
		if (teamLeader == -1 || (!(g_entities[clientNum].r.svFlags & SVF_BOT) && (g_entities[teamLeader].r.svFlags & SVF_BOT))) {
			SetLeader(team, clientNum);
		}
	}
	// Make sure there is a team leader on the team the player came from
	if (oldTeam == TEAM_RED || oldTeam == TEAM_BLUE) {
		CheckTeamLeader(oldTeam);
	}

	BroadcastTeamChange(client, oldTeam);

	// Get and distribute relevant parameters
	ClientUserinfoChanged(clientNum);

	// Client hasn't spawned yet, they sent an early team command, teampref userinfo, or g_teamAutoJoin is enabled
	if (client->pers.connected != CON_CONNECTED) {
		return;
	}

	ClientBegin(clientNum);
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing(gentity_t *entgc {
	ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;	
	ent->client->sess.sessionTeam = TEAM_SPECTATOR;	
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
	ent->r.svFlags &= ~SVF_BOT;
	ent->client->ps.clientNum = ent - g_entities;

	SetClientViewAngle(ent, ent->client->ps.viewanglesgc;

	// don't use dead view angles
	if (ent->client->ps.stats[STAT_HEALTH] <= 0gc {
		ent->client->ps.stats[STAT_HEALTH] = 1;
	}
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f(gentity_t *entgc {
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	if (trap_Argc() != 2gc {
		oldTeam = ent->client->sess.sessionTeam;
		switch (oldTeamgc {
		case TEAM_BLUE:
			trap_SendServerCommand(ent-g_entities, "print \"Blue team\n\""gc;
			break;
		case TEAM_RED:
			trap_SendServerCommand(ent-g_entities, "print \"Red team\n\""gc;
			break;
		case TEAM_FREE:
			trap_SendServerCommand(ent-g_entities, "print \"Free team\n\""gc;
			break;
		case TEAM_SPECTATOR:
			trap_SendServerCommand(ent-g_entities, "print \"Spectator team\n\""gc;
			break;
		}
		return;
	}

	if (ent->client->switchTeamTime > level.timegc {
		trap_SendServerCommand(ent-g_entities, "print \"May not switch teams more than once per 5 seconds.\n\""gc;
		return;
	}

	// If they are playing a tournement game, count as a loss
	if ((g_gametype.integer == GT_TOURNAMENTgc
		&& ent->client->sess.sessionTeam == TEAM_FREEgc {
		ent->client->sess.losses++;
	}

	trap_Argv(1, s, sizeof(sgc );

	SetTeam(ent, sgc;

	ent->client->switchTeamTime = level.time + 5000;
}


/** Follow a player during spectation */
void Cmd_Follow_f(gentity_t *entgc {
	int		i;
	char	arg[MAX_TOKEN_CHARS];

	if (trap_Argc() != 2gc {
		if (ent->client->sess.spectatorState == SPECTATOR_FOLLOWgc {
			StopFollowing(entgc;
		}
		return;
	}

	trap_Argv(1, arg, sizeof(arggc );
	i = ClientNumberFromString(ent, arg, qtrue, qtruegc;
	if (i == -1gc {
		return;
	}

	// Can't follow self
	if (&level.clients[ i ] == ent->clientgc {
		return;
	}

	// Can't follow another spectator
	if (level.clients[ i ].sess.sessionTeam == TEAM_SPECTATORgc {
		return;
	}

	// If they are playing a tournement game, count as a loss
	if ((g_gametype.integer == GT_TOURNAMENTgc
		&& ent->client->sess.sessionTeam == TEAM_FREEgc {
		ent->client->sess.losses++;
	}

	// First set them to spectator
	if (ent->client->sess.sessionTeam != TEAM_SPECTATORgc {
		SetTeam(ent, "spectator"gc;
	}

	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

/** Cycle through players in spectation */
void Cmd_FollowCycle_f(gentity_t *ent, int dirgc {
	int		clientnum;
	int		original;

	// if they are playing a tournement game, count as a loss
	if ((g_gametype.integer == GT_TOURNAMENTgc
		&& ent->client->sess.sessionTeam == TEAM_FREEgc {
		ent->client->sess.losses++;
	}
	// first set them to spectator
	if (ent->client->sess.spectatorState == SPECTATOR_NOTgc {
		SetTeam(ent, "spectator"gc;
	}

	if (dir != 1 && dir != -1gc {
		G_Error("Cmd_FollowCycle_f: bad dir %i", dirgc;
	}

	// if dedicated follow client, just switch between the two auto clients
	if (ent->client->sess.spectatorClient < 0) {
		if (ent->client->sess.spectatorClient == -1) {
			ent->client->sess.spectatorClient = -2;
		} else if (ent->client->sess.spectatorClient == -2) {
			ent->client->sess.spectatorClient = -1;
		}
		return;
	}

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;
	do {
		clientnum += dir;
		if (clientnum >= level.maxclientsgc {
			clientnum = 0;
		}
		if (clientnum < 0gc {
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if (level.clients[ clientnum ].pers.connected != CON_CONNECTEDgc {
			continue;
		}

		// can't follow another spectator
		if (level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATORgc {
			continue;
		}

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	} while (clientnum != originalgc;

	// leave it where it was
}


/*
==================
G_Say
==================
*/

static void G_SayTo(gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *messagegc {
	if (!other) {
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if (other->client->pers.connected != CON_CONNECTEDgc {
		return;
	}
	if (mode == SAY_TEAM  && !OnSameTeam(ent, other)gc {
		return;
	}
	// no chatting to players in tournements
	if ((g_gametype.integer == GT_TOURNAMENTgc
		&& other->client->sess.sessionTeam == TEAM_FREE
		&& ent->client->sess.sessionTeam != TEAM_FREEgc {
		return;
	}

	trap_SendServerCommand(other-g_entities, va("%s \"%s%c%c%s\"", 
		mode == SAY_TEAM ? "tchat" : "chat",
		name, Q_COLOR_ESCAPE, color, message));
}

#define EC "\x19"

void G_Say(gentity_t *ent, gentity_t *target, int mode, const char *chatTextgc {
	int			j;
	gentity_t	*other;
	int			color;
	char		name[64];
	// don't let text be too long for malicious reasons
	char		text[MAX_SAY_TEXT];
	char		location[64];

	if (g_gametype.integer < GT_TEAM && mode == SAY_TEAMgc {
		mode = SAY_ALL;
	}

	switch (modegc {
	default:
	case SAY_ALL:
		G_LogPrintf("say: %s: %s\n", ent->client->pers.netname, chatTextgc;
		Com_sprintf(name, sizeof(name), "%s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITEgc;
		color = COLOR_GREEN;
		break;
	case SAY_TEAM:
		G_LogPrintf("sayteam: %s: %s\n", ent->client->pers.netname, chatTextgc;
		if (Team_GetLocationMsg(ent, location, sizeof(location)))
			Com_sprintf(name, sizeof(name), EC"(%s%c%c"EC") (%s)"EC": ", 
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location);
		else
			Com_sprintf(name, sizeof(name), EC"(%s%c%c"EC")"EC": ", 
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITEgc;
		color = COLOR_CYAN;
		break;
	case SAY_TELL:
		if (target && target->inuse && target->client && g_gametype.integer >= GT_TEAM &&
			target->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
			Team_GetLocationMsg(ent, location, sizeof(location)))
			Com_sprintf(name, sizeof(name), EC"[%s%c%c"EC"] (%s)"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, locationgc;
		else
			Com_sprintf(name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITEgc;
		color = COLOR_MAGENTA;
		break;
	}

	Q_strncpyz(text, chatText, sizeof(text)gc;

	if (targetgc {
		G_SayTo(ent, target, mode, color, name, textgc;
		return;
	}

	// echo the text to the console
	if (g_dedicated.integergc {
		G_Printf("%s%s\n", name, text);
	}

	// send it to all the appropriate clients
	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		G_SayTo(ent, other, mode, color, name, textgc;
	}
}

static void SanitizeChatText(char *textgc {
	int i;

	for (i = 0; text[i]; i++gc {
		if (text[i] == '\n' || text[i] == '\r'gc {
			text[i] = ' ';
		}
	}
}


/** Say chat command */
static void Cmd_Say_f(gentity_t *ent, int mode, qboolean arg0gc {
	char		*p;

	if (trap_Argc () < 2 && !arg0gc {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs(0gc;
	}
	else
	{
		p = ConcatArgs(1gc;
	}

	SanitizeChatText(pgc;

	G_Say(ent, NULL, mode, pgc;
}

/** Chat command to 'tell' a player something */
static void Cmd_Tell_f(gentity_t *entgc {
	int			targetNum;
	gentity_t	*target;
	char		*p;
	char		arg[MAX_TOKEN_CHARS];

	if (trap_Argc() < 3gc {
		trap_SendServerCommand(ent-g_entities, "print \"Usage: tell <player id> <message>\n\""gc;
		return;
	}

	trap_Argv(1, arg, sizeof(arggc );
	targetNum = ClientNumberFromString(ent, arg, qtrue, qtruegc;
	if (targetNum == -1gc {
		return;
	}

	target = &g_entities[targetNum];
	if (!target->inuse || !target->clientgc {
		return;
	}

	p = ConcatArgs(2gc;

	SanitizeChatText(pgc;

	G_LogPrintf("tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, pgc;
	G_Say(ent, target, SAY_TELL, pgc;
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if (ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say(ent, ent, SAY_TELL, pgc;
	}
}


#ifdef MISSIONPACK
static void G_VoiceTo(gentity_t *ent, gentity_t *other, int mode, const char *id, qboolean voiceonlygc {
	int color;
	char *cmd;

	if (!other) {
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if (mode == SAY_TEAM && !OnSameTeam(ent, other)gc {
		return;
	}
	// no chatting to players in tournements
	if (g_gametype.integer == GT_TOURNAMENTgc {
		return;
	}

	if (mode == SAY_TEAM) {
		color = COLOR_CYAN;
		cmd = "vtchat";
	}
	else if (mode == SAY_TELL) {
		color = COLOR_MAGENTA;
		cmd = "vtell";
	}
	else {
		color = COLOR_GREEN;
		cmd = "vchat";
	}

	trap_SendServerCommand(other-g_entities, va("%s %d %d %d %s", cmd, voiceonly, ent->s.number, color, id));
}

void G_Voice(gentity_t *ent, gentity_t *target, int mode, const char *id, qboolean voiceonlygc {
	int			j;
	gentity_t	*other;

	if (g_gametype.integer < GT_TEAM && mode == SAY_TEAMgc {
		mode = SAY_ALL;
	}

	if (targetgc {
		G_VoiceTo(ent, target, mode, id, voiceonlygc;
		return;
	}

	// echo the text to the console
	if (g_dedicated.integergc {
		G_Printf("voice: %s %s\n", ent->client->pers.netname, id);
	}

	// send it to all the appropriate clients
	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		G_VoiceTo(ent, other, mode, id, voiceonlygc;
	}
}

/*
==================
Cmd_Voice_f
==================
*/
static void Cmd_Voice_f(gentity_t *ent, int mode, qboolean arg0, qboolean voiceonlygc {
	char		*p;

	if (trap_Argc () < 2 && !arg0gc {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs(0gc;
	}
	else
	{
		p = ConcatArgs(1gc;
	}

	SanitizeChatText(pgc;

	G_Voice(ent, NULL, mode, p, voiceonlygc;
}

/*
==================
Cmd_VoiceTell_f
==================
*/
static void Cmd_VoiceTell_f(gentity_t *ent, qboolean voiceonlygc {
	int			targetNum;
	gentity_t	*target;
	char		*id;
	char		arg[MAX_TOKEN_CHARS];

	if (trap_Argc() < 3gc {
		trap_SendServerCommand(ent-g_entities, va("print \"Usage: %s <player id> <voice id>\n\"", voiceonly ? "votell" : "vtell"gc );
		return;
	}

	trap_Argv(1, arg, sizeof(arggc );
	targetNum = ClientNumberFromString(ent, arg, qtrue, qtruegc;
	if (targetNum == -1gc {
		return;
	}

	target = &g_entities[targetNum];
	if (!target->inuse || !target->clientgc {
		return;
	}

	id = ConcatArgs(2gc;

	SanitizeChatText(idgc;

	G_LogPrintf("vtell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, idgc;
	G_Voice(ent, target, SAY_TELL, id, voiceonlygc;
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if (ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Voice(ent, ent, SAY_TELL, id, voiceonlygc;
	}
}


/*
==================
Cmd_VoiceTaunt_f
==================
*/
static void Cmd_VoiceTaunt_f(gentity_t *entgc {
	gentity_t *who;
	int i;

	if (!ent->client) {
		return;
	}

	// insult someone who just killed you
	if (ent->enemy && ent->enemy->client && ent->enemy->client->lastkilled_client == ent->s.number) {
		// i am a dead corpse
		if (!(ent->enemy->r.svFlags & SVF_BOT)) {
			G_Voice(ent, ent->enemy, SAY_TELL, VOICECHAT_DEATHINSULT, qfalsegc;
		}
		if (!(ent->r.svFlags & SVF_BOT)) {
			G_Voice(ent, ent,        SAY_TELL, VOICECHAT_DEATHINSULT, qfalsegc;
		}
		ent->enemy = NULL;
		return;
	}
	// insult someone you just killed
	if (ent->client->lastkilled_client >= 0 && ent->client->lastkilled_client != ent->s.number) {
		who = g_entities + ent->client->lastkilled_client;
		if (who->client) {
			// who is the person I just killed
			if (who->client->lasthurt_mod == MOD_GAUNTLET) {
				if (!(who->r.svFlags & SVF_BOT)) {
					G_Voice(ent, who, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalsegc;	// and I killed them with a gauntlet
				}
				if (!(ent->r.svFlags & SVF_BOT)) {
					G_Voice(ent, ent, SAY_TELL, VOICECHAT_KILLGAUNTLET, qfalsegc;
				}
			} else {
				if (!(who->r.svFlags & SVF_BOT)) {
					G_Voice(ent, who, SAY_TELL, VOICECHAT_KILLINSULT, qfalsegc;	// and I killed them with something else
				}
				if (!(ent->r.svFlags & SVF_BOT)) {
					G_Voice(ent, ent, SAY_TELL, VOICECHAT_KILLINSULT, qfalsegc;
				}
			}
			ent->client->lastkilled_client = -1;
			return;
		}
	}

	if (g_gametype.integer >= GT_TEAM) {
		// praise a team mate who just got a reward
		for(i = 0; i < MAX_CLIENTS; i++) {
			who = g_entities + i;
			if (who->client && who != ent && who->client->sess.sessionTeam == ent->client->sess.sessionTeam) {
				if (who->client->rewardTime > level.time) {
					if (!(who->r.svFlags & SVF_BOT)) {
						G_Voice(ent, who, SAY_TELL, VOICECHAT_PRAISE, qfalsegc;
					}
					if (!(ent->r.svFlags & SVF_BOT)) {
						G_Voice(ent, ent, SAY_TELL, VOICECHAT_PRAISE, qfalsegc;
					}
					return;
				}
			}
		}
	}

	// just say something
	G_Voice(ent, NULL, SAY_ALL, VOICECHAT_TAUNT, qfalsegc;
}
#endif



static char	*gc_orders[] = {
	"hold your position",
	"hold this position",
	"come here",
	"cover me",
	"guard location",
	"search and destroy",
	"report"
};

static const int numgc_orders = ARRAY_LEN(gc_ordersgc;

void Cmd_GameCommand_f(gentity_t *entgc {
	int			targetNum;
	gentity_t	*target;
	int			order;
	char		arg[MAX_TOKEN_CHARS];

	if (trap_Argc() != 3gc {
		trap_SendServerCommand(ent-g_entities, va("print \"Usage: gc <player id> <order 0-%d>\n\"", numgc_orders - 1gc );
		return;
	}

	trap_Argv(2, arg, sizeof(arggc );
	order = atoi(arggc;

	if (order < 0 || order >= numgc_ordersgc {
		trap_SendServerCommand(ent-g_entities, va("print \"Bad order: %i\n\"", order));
		return;
	}

	trap_Argv(1, arg, sizeof(arggc );
	targetNum = ClientNumberFromString(ent, arg, qtrue, qtruegc;
	if (targetNum == -1gc {
		return;
	}

	target = &g_entities[targetNum];
	if (!target->inuse || !target->clientgc {
		return;
	}

	G_LogPrintf("tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, gc_orders[order]gc;
	G_Say(ent, target, SAY_TELL, gc_orders[order]gc;
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if (ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say(ent, ent, SAY_TELL, gc_orders[order]gc;
	}
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f(gentity_t *entgc {
	trap_SendServerCommand(ent-g_entities, va("print \"%s\n\"", vtos(ent->r.currentOrigin)gc );
}

static const char *gameNames[] = {
	"Free For All",
	"Tournament",
	"Single Player",
	"Team Deathmatch",
	"Capture the Flag",
	"One Flag CTF",
	"Overload",
	"Harvester"
};

/** Call vote command */
void Cmd_CallVote_f(gentity_t *entgc {
	char*	c;
	int		i;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	if (!g_allowVote.integer) {
		trap_SendServerCommand(ent-g_entities, "print \"Voting not allowed here.\n\"");
		return;
	}

	if (level.voteTime) {
		trap_SendServerCommand(ent-g_entities, "print \"A vote is already in progress.\n\"");
		return;
	}
	if (ent->client->pers.voteCount >= MAX_VOTE_COUNT) {
		trap_SendServerCommand(ent-g_entities, "print \"You have called the maximum number of votes.\n\"");
		return;
	}
	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
		trap_SendServerCommand(ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\""gc;
		return;
	}

	// Make sure it is a valid command to vote on
	trap_Argv(1, arg1, sizeof(arg1gc);
	trap_Argv(2, arg2, sizeof(arg2gc);

	// Check for command separators in arg2
	for(c = arg2; *c; ++c) {
		switch(*c) {
			case '\n':
			case '\r':
			case ';':
				trap_SendServerCommand(ent-g_entities, "print \"Invalid vote string.\n\""gc;
				return;
			break;
		}
	}

	if (!Q_stricmp(arg1, "map_restart")) {
	} else if (!Q_stricmp(arg1, "nextmap")) {
	} else if (!Q_stricmp(arg1, "map")) {
	} else if (!Q_stricmp(arg1, "g_gametype")) {
	} else if (!Q_stricmp(arg1, "kick")) {
	} else if (!Q_stricmp(arg1, "clientkick")) {
	} else if (!Q_stricmp(arg1, "g_doWarmup")) {
	} else if (!Q_stricmp(arg1, "timelimit")) {
	} else if (!Q_stricmp(arg1, "fraglimit")) {
	} else {
		trap_SendServerCommand(ent-g_entities, "print \"Invalid vote string.\n\"");
		trap_SendServerCommand(ent-g_entities, "print \"Vote commands are: map_restart, nextmap, map <mapname>, g_gametype <n>, kick <player>, clientkick <clientnum>, g_doWarmup, timelimit <time>, fraglimit <frags>.\n\"");
		return;
	}

	// if there is still a vote to be executed
	if (level.voteExecuteTime) {
		// don't start a vote when map change or restart is in progress
		if (!Q_stricmpn(level.voteString, "map", 3) || !Q_stricmpn(level.voteString, "nextmap", 7)) {
			trap_SendServerCommand(ent-g_entities, "print \"Vote after map change.\n\"");
			return;
		}

		level.voteExecuteTime = 0;
		trap_SendConsoleCommand(EXEC_APPEND, va("%s\n", level.voteString));
	}

	// special case for g_gametype, check for bad values
	if (!Q_stricmp(arg1, "g_gametype")) {
		i = atoi(arg2);
		if(i == GT_SINGLE_PLAYER || i < GT_FFA || i >= GT_MAX_GAME_TYPE) {
			trap_SendServerCommand(ent-g_entities, "print \"Invalid gametype.\n\"");
			return;
		}

		Com_sprintf(level.voteString, sizeof(level.voteString), "%s %d", arg1, i);
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), "%s %s", arg1, gameNames[i]);
	} else if (!Q_stricmp(arg1, "map")) {
		// special case for map changes, we want to reset the nextmap setting
		// this allows a player to change maps, but not upset the map rotation
		char	s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer("nextmap", s, sizeof(s));
		if (*s) {
			Com_sprintf(level.voteString, sizeof(level.voteString), "%s %s; set nextmap \"%s\"", arg1, arg2, s);
		} else {
			Com_sprintf(level.voteString, sizeof(level.voteString), "%s %s", arg1, arg2);
		}
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), "%s", level.voteString);
	} else if (!Q_stricmp(arg1, "nextmap")) {
		char s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer("nextmap", s, sizeof(s));
		if (!*s) {
			trap_SendServerCommand(ent-g_entities, "print \"nextmap not set.\n\""gc;
			return;
		}
		Com_sprintf(level.voteString, sizeof(level.voteStringgc, "vstr nextmap");
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayStringgc, "%s", level.voteString);
	} else if (!Q_stricmp(arg1, "clientkick") || !Q_stricmp(arg1, "kick")) {
		i = ClientNumberFromString(ent, arg2, !Q_stricmp(arg1, "clientkick"), !Q_stricmp(arg1, "kick"));
		if (i == -1) {
			return;
		}

		if (level.clients[i].pers.localClient) {
			trap_SendServerCommand(ent - g_entities, "print \"Cannot kick host player.\n\"");
			return;
		}

		Com_sprintf(level.voteString, sizeof(level.voteString), "clientkick %d", i);
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayStringgc, "kick %s", level.clients[i].pers.netname);
	} else {
		Com_sprintf(level.voteString, sizeof(level.voteString), "%s \"%s\"", arg1, arg2);
		Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), "%s", level.voteString);
	}

	trap_SendServerCommand(-1, va("print \"%s called a vote.\n\"", ent->client->pers.netname));

	// start the voting, the caller automatically votes yes
	level.voteTime = level.time;
	level.voteYes = 1;
	level.voteNo = 0;

	for (i = 0 ; i < level.maxclients ; i++) {
		level.clients[i].ps.eFlags &= ~EF_VOTED;
	}
	ent->client->ps.eFlags |= EF_VOTED;

	trap_SetConfigstring(CS_VOTE_TIME, va("%i", level.voteTime));
	trap_SetConfigstring(CS_VOTE_STRING, level.voteDisplayString);	
	trap_SetConfigstring(CS_VOTE_YES, va("%i", level.voteYes));
	trap_SetConfigstring(CS_VOTE_NO, va("%i", level.voteNo));	
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f(gentity_t *ent) {
	char msg[64];

	if (!level.voteTime) {
		trap_SendServerCommand(ent-g_entities, "print \"No vote in progress.\n\"");
		return;
	}
	if (ent->client->ps.eFlags & EF_VOTED) {
		trap_SendServerCommand(ent-g_entities, "print \"Vote already cast.\n\"");
		return;
	}
	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
		trap_SendServerCommand(ent-g_entities, "print \"Not allowed to vote as spectator.\n\""gc;
		return;
	}

	trap_SendServerCommand(ent-g_entities, "print \"Vote cast.\n\""gc;

	ent->client->ps.eFlags |= EF_VOTED;

	trap_Argv(1, msg, sizeof(msggc );

	if (tolower(msg[0]gc == 'y' || msg[0] == '1' ) {
		level.voteYes++;
		trap_SetConfigstring(CS_VOTE_YES, va("%i", level.voteYesgc );
	} else {
		level.voteNo++;
		trap_SetConfigstring(CS_VOTE_NO, va("%i", level.voteNogc );	
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f(gentity_t *entgc {
	char*	c;
	int		i, team, cs_offset;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	team = ent->client->sess.sessionTeam;
	if (team == TEAM_REDgc
		cs_offset = 0;
	else if (team == TEAM_BLUEgc
		cs_offset = 1;
	else
		return;

	if (!g_allowVote.integergc {
		trap_SendServerCommand(ent-g_entities, "print \"Voting not allowed here.\n\""gc;
		return;
	}

	if (level.teamVoteTime[cs_offset]gc {
		trap_SendServerCommand(ent-g_entities, "print \"A team vote is already in progress.\n\""gc;
		return;
	}
	if (ent->client->pers.teamVoteCount >= MAX_VOTE_COUNTgc {
		trap_SendServerCommand(ent-g_entities, "print \"You have called the maximum number of team votes.\n\""gc;
		return;
	}
	if (ent->client->sess.sessionTeam == TEAM_SPECTATORgc {
		trap_SendServerCommand(ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\""gc;
		return;
	}

	// make sure it is a valid command to vote on
	trap_Argv(1, arg1, sizeof(arg1gc );
	arg2[0] = '\0';
	for (i = 2; i < trap_Argc(); i++gc {
		if (i > 2)
			strcat(arg2, " ");
		trap_Argv(i, &arg2[strlen(arg2)], sizeof(arg2gc - strlen(arg2) );
	}

	// check for command separators in arg2
	for(c = arg2; *c; ++c) {
		switch(*c) {
			case '\n':
			case '\r':
			case ';':
				trap_SendServerCommand(ent-g_entities, "print \"Invalid vote string.\n\""gc;
				return;
			break;
		}
	}

	if (!Q_stricmp(arg1, "leader"gc ) {
		char netname[MAX_NETNAME], leader[MAX_NETNAME];

		if (!arg2[0]gc {
			i = ent->client->ps.clientNum;
		}
		else {
			// numeric values are just slot numbers
			for (i = 0; i < 3; i++) {
				if (!arg2[i] || arg2[i] < '0' || arg2[i] > '9'gc
					break;
			}
			if (i >= 3 || !arg2[i]) {
				i = atoi(arg2gc;
				if (i < 0 || i >= level.maxclientsgc {
					trap_SendServerCommand(ent-g_entities, va("print \"Bad client slot: %i\n\"", i)gc;
					return;
				}

				if (!g_entities[i].inusegc {
					trap_SendServerCommand(ent-g_entities, va("print \"Client %i is not active\n\"", i)gc;
					return;
				}
			}
			else {
				Q_strncpyz(leader, arg2, sizeof(leader));
				Q_CleanStr(leader);
				for (i = 0 ; i < level.maxclients ; i++gc {
					if (level.clients[i].pers.connected == CON_DISCONNECTEDgc
						continue;
					if (level.clients[i].sess.sessionTeam != team)
						continue;
					Q_strncpyz(netname, level.clients[i].pers.netname, sizeof(netname));
					Q_CleanStr(netname);
					if (!Q_stricmp(netname, leader)gc {
						break;
					}
				}
				if (i >= level.maxclientsgc {
					trap_SendServerCommand(ent-g_entities, va("print \"%s is not a valid player on your team.\n\"", arg2)gc;
					return;
				}
			}
		}
		Com_sprintf(arg2, sizeof(arg2), "%d", i);
	} else {
		trap_SendServerCommand(ent-g_entities, "print \"Invalid vote string.\n\""gc;
		trap_SendServerCommand(ent-g_entities, "print \"Team vote commands are: leader <player>.\n\""gc;
		return;
	}

	Com_sprintf(level.teamVoteString[cs_offset], sizeof(level.teamVoteString[cs_offset]gc, "%s %s", arg1, arg2 );

	for (i = 0 ; i < level.maxclients ; i++gc {
		if (level.clients[i].pers.connected == CON_DISCONNECTEDgc
			continue;
		if (level.clients[i].sess.sessionTeam == team)
			trap_SendServerCommand(i, va("print \"%s called a team vote.\n\"", ent->client->pers.netnamegc );
	}

	// start the voting, the caller automatically votes yes
	level.teamVoteTime[cs_offset] = level.time;
	level.teamVoteYes[cs_offset] = 1;
	level.teamVoteNo[cs_offset] = 0;

	for (i = 0 ; i < level.maxclients ; i++gc {
		if (level.clients[i].sess.sessionTeam == team)
			level.clients[i].ps.eFlags &= ~EF_TEAMVOTED;
	}
	ent->client->ps.eFlags |= EF_TEAMVOTED;

	trap_SetConfigstring(CS_TEAMVOTE_TIME + cs_offset, va("%i", level.teamVoteTime[cs_offset]gc );
	trap_SetConfigstring(CS_TEAMVOTE_STRING + cs_offset, level.teamVoteString[cs_offset]gc;
	trap_SetConfigstring(CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset]gc );
	trap_SetConfigstring(CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset]gc );
}

/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f(gentity_t *entgc {
	int			team, cs_offset;
	char		msg[64];

	team = ent->client->sess.sessionTeam;
	if (team == TEAM_REDgc
		cs_offset = 0;
	else if (team == TEAM_BLUEgc
		cs_offset = 1;
	else
		return;

	if (!level.teamVoteTime[cs_offset]gc {
		trap_SendServerCommand(ent-g_entities, "print \"No team vote in progress.\n\""gc;
		return;
	}
	if (ent->client->ps.eFlags & EF_TEAMVOTEDgc {
		trap_SendServerCommand(ent-g_entities, "print \"Team vote already cast.\n\""gc;
		return;
	}
	if (ent->client->sess.sessionTeam == TEAM_SPECTATORgc {
		trap_SendServerCommand(ent-g_entities, "print \"Not allowed to vote as spectator.\n\""gc;
		return;
	}

	trap_SendServerCommand(ent-g_entities, "print \"Team vote cast.\n\""gc;

	ent->client->ps.eFlags |= EF_TEAMVOTED;

	trap_Argv(1, msg, sizeof(msggc );

	if (tolower(msg[0]gc == 'y' || msg[0] == '1' ) {
		level.teamVoteYes[cs_offset]++;
		trap_SetConfigstring(CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset]gc );
	} else {
		level.teamVoteNo[cs_offset]++;
		trap_SetConfigstring(CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset]gc );	
	}

	// a majority will be determined in TeamCheckVote, which will also account
	// for players entering or leaving
}


/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f(gentity_t *entgc {
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	if (!g_cheats.integergc {
		trap_SendServerCommand(ent-g_entities, "print \"Cheats are not enabled on this server.\n\"");
		return;
	}
	if (trap_Argc() != 5gc {
		trap_SendServerCommand(ent-g_entities, "print \"usage: setviewpos x y z yaw\n\"");
		return;
	}

	VectorClear(anglesgc;
	for (i = 0 ; i < 3 ; i++gc {
		trap_Argv(i + 1, buffer, sizeof(buffergc );
		origin[i] = atof(buffergc;
	}

	trap_Argv(4, buffer, sizeof(buffergc );
	angles[YAW] = atof(buffergc;

	TeleportPlayer(ent, origin, anglesgc;
}



/*
=================
Cmd_Stats_f
=================
*/
void Cmd_Stats_f(gentity_t *entgc {
/*
	int max, n, i;

	max = trap_AAS_PointReachabilityAreaIndex(NULLgc;

	n = 0;
	for (i = 0; i < max; i++gc {
		if (ent->client->areabits[i >> 3] & (1 << (i & 7))gc
			n++;
	}

	//trap_SendServerCommand(ent-g_entities, va("print \"visited %d of %d areas\n\"", n, max));
	trap_SendServerCommand(ent-g_entities, va("print \"%d%% level coverage\n\"", n * 100 / max));
*/
}

/*
=================
ClientCommand
=================
*/
void ClientCommand(int clientNumgc {
	gentity_t *ent;
	char	cmd[MAX_TOKEN_CHARS];

	ent = g_entities + clientNum;
	if (!ent->client || ent->client->pers.connected != CON_CONNECTED) {
		if (ent->client && ent->client->pers.localClient) {
			// Handle early team command sent by UI when starting a local
			// team play game.
			trap_Argv(0, cmd, sizeof(cmdgc );
			if (Q_stricmp(cmd, "team") == 0) {
				Cmd_Team_f(ent);
			}
		}
		return;		// not fully in game yet
	}


	trap_Argv(0, cmd, sizeof(cmdgc );

	if (Q_stricmp(cmd, "say") == 0) {
		Cmd_Say_f(ent, SAY_ALL, qfalse);
		return;
	}
	if (Q_stricmp(cmd, "say_team") == 0) {
		Cmd_Say_f(ent, SAY_TEAM, qfalse);
		return;
	}
	if (Q_stricmp(cmd, "tell") == 0) {
		Cmd_Tell_f(entgc;
		return;
	}
#ifdef MISSIONPACK
	if (Q_stricmp(cmd, "vsay") == 0) {
		Cmd_Voice_f(ent, SAY_ALL, qfalse, qfalse);
		return;
	}
	if (Q_stricmp(cmd, "vsay_team") == 0) {
		Cmd_Voice_f(ent, SAY_TEAM, qfalse, qfalse);
		return;
	}
	if (Q_stricmp(cmd, "vtell") == 0) {
		Cmd_VoiceTell_f(ent, qfalsegc;
		return;
	}
	if (Q_stricmp(cmd, "vosay") == 0) {
		Cmd_Voice_f(ent, SAY_ALL, qfalse, qtrue);
		return;
	}
	if (Q_stricmp(cmd, "vosay_team") == 0) {
		Cmd_Voice_f(ent, SAY_TEAM, qfalse, qtrue);
		return;
	}
	if (Q_stricmp(cmd, "votell") == 0) {
		Cmd_VoiceTell_f(ent, qtruegc;
		return;
	}
	if (Q_stricmp(cmd, "vtaunt") == 0) {
		Cmd_VoiceTaunt_f(entgc;
		return;
	}
#endif
	if (Q_stricmp(cmd, "score") == 0) {
		Cmd_Score_f(ent);
		return;
	}

	// ignore all other commands when at intermission
	if (level.intermissiontime) {
		Cmd_Say_f(ent, qfalse, qtrue);
		return;
	}

	if (Q_stricmp(cmd, "give") == 0)
		Cmd_Give_f(ent);
	else if (Q_stricmp(cmd, "god") == 0)
		Cmd_God_f(ent);
	else if (Q_stricmp(cmd, "notarget") == 0)
		Cmd_Notarget_f(ent);
	else if (Q_stricmp(cmd, "noclip") == 0)
		Cmd_Noclip_f(ent);
	else if (Q_stricmp(cmd, "kill") == 0)
		Cmd_Kill_f(ent);
	else if (Q_stricmp(cmd, "teamtask") == 0)
		Cmd_TeamTask_f(ent);
	else if (Q_stricmp(cmd, "levelshot") == 0)
		Cmd_LevelShot_f(ent);
	else if (Q_stricmp(cmd, "follow") == 0)
		Cmd_Follow_f(ent);
	else if (Q_stricmp(cmd, "follownext") == 0)
		Cmd_FollowCycle_f(ent, 1);
	else if (Q_stricmp(cmd, "followprev") == 0)
		Cmd_FollowCycle_f(ent, -1);
	else if (Q_stricmp(cmd, "team") == 0)
		Cmd_Team_f(ent);
	else if (Q_stricmp(cmd, "where") == 0)
		Cmd_Where_f(ent);
	else if (Q_stricmp(cmd, "callvote") == 0)
		Cmd_CallVote_f(ent);
	else if (Q_stricmp(cmd, "vote") == 0)
		Cmd_Vote_f(ent);
	else if (Q_stricmp(cmd, "callteamvote") == 0)
		Cmd_CallTeamVote_f(ent);
	else if (Q_stricmp(cmd, "teamvote") == 0)
		Cmd_TeamVote_f(ent);
	else if (Q_stricmp(cmd, "gc") == 0)
		Cmd_GameCommand_f(entgc;
	else if (Q_stricmp(cmd, "setviewpos") == 0)
		Cmd_SetViewpos_f(entgc;
	else if (Q_stricmp(cmd, "stats") == 0)
		Cmd_Stats_f(entgc;
	else
		trap_SendServerCommand(clientNum, va("print \"unknown cmd %s\n\"", cmdgc );
}
