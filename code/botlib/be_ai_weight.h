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

#define WT_BALANCE			1
#define MAX_WEIGHTS			128

/** Fuzzy seperator */
typedef struct fuzzyseperator_s
{
	int index;
	int value;
	int type;
	float weight;
	float minweight;
	float maxweight;
	struct fuzzyseperator_s *child;
	struct fuzzyseperator_s *next;
} fuzzyseperator_t;

/** Fuzzy weight */
typedef struct weight_s
{
	char *name;
	struct fuzzyseperator_s *firstseperator;
} weight_t;

/** Weight configuration */
typedef struct weightconfig_s
{
	int numweights;
	weight_t weights[MAX_WEIGHTS];
	char		filename[MAX_QPATH];
} weightconfig_t;

/** Reads a weight configuration */
weightconfig_t *ReadWeightConfig(char *filename);

/** Free a weight configuration */
void FreeWeightConfig(weightconfig_t *config);

/** Writes a weight configuration, returns true if successful */
qboolean WriteWeightConfig(char *filename, weightconfig_t *config);

/** Find the fuzzy weight with the given name */
int FindFuzzyWeight(weightconfig_t *wc, char *name);

/** Returns the fuzzy weight for the given inventory and weight */
float FuzzyWeight(int *inventory, weightconfig_t *wc, int weightnum);
float FuzzyWeightUndecided(int *inventory, weightconfig_t *wc, int weightnum);

/** Scales the weight with the given name */
void ScaleWeight(weightconfig_t *config, char *name, float scale);

/** Scale the balance range */
void ScaleBalanceRange(weightconfig_t *config, float scale);

/** Evolves the weight configuration */
void EvolveWeightConfig(weightconfig_t *config);

/** Interbreed the weight configurations and stores the interbreeded one in configout */
void InterbreedWeightConfigs(weightconfig_t *config1, weightconfig_t *config2, weightconfig_t *configout);

/** Frees cached weight configurations */
void BotShutdownWeights(void);
