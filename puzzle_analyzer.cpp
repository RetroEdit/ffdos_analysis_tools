/*
	Copyright 2020 RetroEdit

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include<algorithm>
#include<fstream>
#include<iostream>
#include<iterator>
#include<string>
#include<sstream>
#include<tuple>
#include<unordered_map>
#include<vector>

const std::string VERSION = "0.1.2";

unsigned long ff1_rand(unsigned long rng)
{
	return (rng * 0x6C078965 + 7) & 0xFFFFFFFF;
}

unsigned long indexRNG(unsigned long goal, unsigned long rng = 0)
{
	for (unsigned long i=1; i <= 0xFFFFFFFF; ++i)
	{
		rng = ff1_rand(rng);
		if (rng == goal)
		{
			return i;
		}
	}
	return 0;
}

const int BOARD_WIDTH = 4;
const int BOARD_HEIGHT = 4;
const int NUM_TILES = BOARD_WIDTH * BOARD_HEIGHT;
const int NUM_MOVES = 4096;

std::tuple<unsigned long, int*> scrambleBoard(unsigned long rng)
{
	int* tiles = new int[NUM_TILES];
	// Probably something like this exists;
	// std::range or something?
	for (int i = 0; i < NUM_TILES; ++i)
	{
		tiles[i] = i;
	}
	
	int x = BOARD_WIDTH - 1;
	int y = BOARD_HEIGHT - 1;
	int x0 = x;
	int y0 = y;
	bool move;
	long dir;
	int i = 0;
	while (i < NUM_MOVES)
	{
		move = false;
		rng = ff1_rand(rng);
		dir = rng % 160000;
		if (dir <= 40000)
		{
			if (y > 0)
			{
				y0 = y--;
				move = true;
			}
		}
		else if (dir <= 80000)
		{
			if (y < (BOARD_HEIGHT - 1))
			{
				y0 = y++;
				move = true;
			}
		}
		else if (dir <= 120000)
		{
			if (x > 0)
			{
				x0 = x--;
				move = true;
			}
		}
		else
		{
			if (x < (BOARD_HEIGHT - 1))
			{
				x0 = x++;
				move = true;
			}
		}
		if (move)
		{
			int temptile = tiles[y0 * 4 + x0];
			tiles[y0 * 4 + x0] = tiles[y * 4 + x];
			tiles[y * 4 + x] = temptile;
			x0 = x;
			y0 = y;
			++i;
		}
	}
	return std::make_tuple(rng, tiles);
}

void drawBoard(int* board, bool compact) {
	if (!compact)
	{
		std::cout << "\n";
	}
	for (int y = 0; y < BOARD_WIDTH; ++y)
	{
		for (int x = 0; x < BOARD_HEIGHT; ++x)
		{
			int tile = board[y * 4 + x];
			if (tile < NUM_TILES - 1)
			{
				printf("%2d ", tile + 1);
			}
			else
			{
				std::cout << "__ ";
			}
		}
		if (!compact)
		{
			std::cout << "\n";
		}
	}
}

// Maybe could convert to nested lists if worthwhile.
const std::vector<std::string> places1 {"Megalixir", "Megalixir"};
const std::vector<std::string> places2 {"Turbo Ether", "Elixir", "Remedy", "Hermes' Shoes", "Emergency Exit"};
const std::vector<std::string> places3 {"Spider's Silk", "White Fang", "Red Fang", "Blue Fang", "Red Curtain", "White Curtain", "Blue Curtain", "Vampire Fang", "Cockatrice Claw"};
const std::vector<std::string> participation {"Potion", "Antidote", "Gold Needle", "Ether", "Eye Drops", "Echo Grass", "Phoenix Down", "100 Gil"};

struct PrizeResult
{
	unsigned long rng;
	std::string place1, place2, place3, bonus, consolation;
	PrizeResult(unsigned long rng)
	{
		this->rng = rng;
		rng = ff1_rand(rng);
		this->place1 = places1[rng % places1.size()];
		this->place2 = places2[rng % places2.size()];
		this->place3 = places3[rng % places3.size()];
		this->consolation = participation[rng % participation.size()];

		rng = ff1_rand(rng);
		this->bonus = participation[rng % participation.size()];
	}
};

struct PrizeGoal
{
	std::string prizeName;
	int min, max;
	int num = 0;

	PrizeGoal(std::string prizeName, int min, int max)
	{
		this->prizeName = prizeName;
		this->min = min;
		this->max = max;
	}
	
	std::string getPrizeName()
	{
		return prizeName;
	}
};

struct PrizeGoals
{
	// The "participation" prizes are not currently checked,
	// because they aren't desirable for speedrunning.
	std::vector<PrizeGoal> goals1;
	std::vector<PrizeGoal> goals2;
	std::vector<PrizeGoal> goals3;
	int skipWait, skipsAllowed;
	int minVitalPrizes = 0;

	void addGoal(PrizeGoal goal)
	{
		std::string prizeName = goal.getPrizeName();
		minVitalPrizes += goal.min;
		// TODO: Would be more useful if they were all width-aligned.
		std::clog << prizeName << ", Min: " << goal.min << ", Max: " << goal.max << ", Place: ";
		if (std::find(places1.begin(), places1.end(), prizeName) != places1.end())
		{
			goals1.push_back(goal);
			std::clog << "1st\n";
		}
		else if (std::find(places2.begin(), places2.end(), prizeName) != places2.end())
		{
			goals2.push_back(goal);
			std::clog << "2nd\n";
		}
		else if (std::find(places3.begin(), places3.end(), prizeName) != places3.end())
		{
			goals3.push_back(goal);
			std::clog << "3rd\n";
		}
		else
		{
			std::cerr << "\"" << prizeName << "\" doesn't match a known prize name.\n";
		}
	}

	void resetGoals()
	{
		for (PrizeGoal goal : goals1)
		{
			goal.num = 0;
		}
		for (PrizeGoal goal : goals2)
		{
			goal.num = 0;
		}
		for (PrizeGoal goal : goals3)
		{
			goal.num = 0;
		}
	}
};

void drawBoards(unsigned long rng, int numPuzzles, PrizeGoals& prizeGoals, bool compact)
{
	int* board;

	for (int i = 0; i < numPuzzles; ++i)
	{
		std::cout << "\n\nPuzzle " << i << " (" << rng << "):\n";

		std::tie(rng, board) = scrambleBoard(rng);
		drawBoard(board, compact);
		delete[] board;

		PrizeResult result = PrizeResult(rng);
		// FIXME: Temporary; will probably have to be reworked.
		std::string place2 = result.place2;
		std::string place3 = result.place3;
		for (PrizeGoal goal : prizeGoals.goals2)
		{
			if (goal.prizeName == place2)
			{
				place2 = place2 + " (GOAL)";
				break;
			}
		}
		for (PrizeGoal goal : prizeGoals.goals3)
		{
			if (goal.prizeName == place3)
			{
				place3 = place3 + " (GOAL)";
				break;
			}
		}

		if (!compact)
		{
			std::cout << "\n2nd: " << place2;
			std::cout << "\n3rd: " << place3;
			std::cout << "\nBonus: " << result.bonus;
		}
		else
		{
			std::cout << "\n2: " << place2;
			std::cout << ", 3: " << place3;
			std::cout << ", +: " << result.bonus;
		}

		rng = ff1_rand(rng);
		rng = ff1_rand(rng);
	}
}

// Based on experimental results.
// Could make MAX_VCOUNT 240 for complete coverage.
const int MIN_VCOUNT = 0;
const int MAX_VCOUNT = 100;
const int MAX_STEPS = 1000;

// 30 (title)
// 23 * 2 - 1 = 45 ('B' presses)
// but perfect TAS gets 77?
// Wonder where those extra 2 come from.
// const int START_STEPS = 30 + 45;

// However, 0 is slightly slower, but less restrictive.
// (With game modification, certain RNG can be skipped).
const int START_STEPS = 0;

// std::tuple<unsigned long, int>
unsigned long findScrambleSeed(const int* goal)
{
	// Could make this a parameter.
	unsigned long rng = 0;
	int steps = 0;
	for (; steps < START_STEPS; ++steps)
	{
		rng = ff1_rand(rng);
	}

	for (; steps <= MAX_STEPS; ++steps)
	{
		for (int v = MIN_VCOUNT; v <= MAX_VCOUNT; ++v)
		{
			unsigned long curr_rng = (rng + v) & 0xFFFFFFFF;
			int* scramble = std::get<1>(scrambleBoard(curr_rng));
			bool matches = true;
			for (int i = 0; i < NUM_TILES; ++i)
			{
				if (scramble[i] != goal[i])
				{
					matches = false;
					break;
				}
			}
			delete[] scramble;
			if (matches)
			{
				printf("\nAfter %d advances, RNG: %10lu, VCOUNT: %d", steps, rng, v);
				return curr_rng;
			}
		}
		rng = ff1_rand(rng);
	}
	std::cerr << "\nScramble not found after " << MAX_STEPS << " steps.";
	return 0;
}

std::tuple<bool, unsigned long> parseScrambleGetSeed(std::string line)
{
	std::stringstream linereader;
	linereader << line;

	int goal[NUM_TILES] = {};

	// For validation, not directly functional
	bool found_tiles[NUM_TILES] = {};
	int duplicate_tiles[NUM_TILES] = {};
	std::vector<int> invalid_tiles;

	int tile;
	int tile_index = 0;
	for (; tile_index < NUM_TILES && linereader >> tile; ++tile_index)
	{
		// All validation
		if (tile < 0 || tile >= NUM_TILES)
		{
			invalid_tiles.push_back(tile);
			continue;
		}

		if (found_tiles[tile])
		{
			duplicate_tiles[tile] += 1;
		}
		else
		{
			found_tiles[tile] = true;
		}

		// The scrambler uses a different format that is inconvenient for human entry
		goal[tile_index] = tile == 0 ? NUM_TILES - 1 : tile - 1;
	}
	if (linereader >> tile)
	{
		std::cerr << "\nToo many tiles given; only " << NUM_TILES << " should be given.";
		return std::make_tuple(false, 0);
	}
	bool missing = false;
	for (int i = 0; i < NUM_TILES; ++i)
	{
		if (!found_tiles[i])
		{
			if (!missing)
			{
				std::cerr << "\nBad scramble provided:\nMissing tiles:";
				missing = true;
			}
			std::cerr << " " << i;
		}
	}
	if (missing)
	{
		bool duplicate = false;
		for (int i = 0; i < NUM_TILES; ++i)
		{
			if (duplicate_tiles[i] > 0)
			{
				if (!duplicate)
				{
					std::cerr << "\nDuplicate tiles:";
					duplicate = true;
				}
				std::cerr << " " << i;
			}
		}
		if (!invalid_tiles.empty())
		{
			std::cerr << "\nInvalid tiles:";
			for (int tile : invalid_tiles)
			{
				std::cerr << " " << tile;
			}
		}
		if (tile_index < NUM_TILES)
		{
			std::cerr << "\n" << NUM_TILES << " tiles must be given (" << tile_index << " were given).";
		}
		return std::make_tuple(false, 0);
	}
	// TODO: Could also add a parity check.

	// This system is imperfect, because of this component here.
	// I could make that method return a tuple too, in case the search fails.
	// RNG currently defaults to 0 on search failure.
	// Detectable mis-entry is probably more common than search failure.
	return std::make_tuple(true, findScrambleSeed(goal));
}

const std::string prizeFileName = "prizes.txt";

int main()
{
	std::string line;
	std::cout << "ffdos_analysis_tools version " << VERSION << "; by RetroEdit\n\n";
	
	// TODO: Probably should check if exists
	// Add error checking for comma parsing.
	// Maybe handle duplicate prize names, too?
	std::cout << "Loading prize goals from \"" << prizeFileName << "\".\n";
	std::ifstream prizes_file(prizeFileName);
	PrizeGoals prizeGoals;
	std::size_t sep1, sep2;
	while (getline(prizes_file, line))
	{
		sep1 = line.find(',');
		if (sep1 != std::string::npos)
		{
			sep2 = line.find(',', sep1 + 1);
			prizeGoals.addGoal(PrizeGoal(
				line.substr(0, sep1),
				std::stoi(line.substr(sep1 + 1, sep2 - sep1 - 1)),
				std::stoi(line.substr(sep2 + 1))
			));
		}
		else
		{
			sep1 = line.find(':');
			std::string label = line.substr(0, sep1);
			int value = std::stoi(line.substr(sep1 + 1));
			if (label == "SkipWait")
			{
				prizeGoals.skipWait = value;
			}
			else if (label == "SkipsAllowed")
			{
				prizeGoals.skipsAllowed = value;
			}
			else
			{
				std::cerr << "In prize goals file, label \"" << label << "\" not recognized.";
			}
		}
	}
	std::cout << "Prize goals loaded.\n\n";

	std::cout << "Each number corresponds to an action:\n";
	std::cout << "q - Quit\n";
	// std::cout << "a - Analyze a seed.\n";
	std::cout << "f - Find scramble seed\n";
	std::cout << "p - Predict puzzles/prizes\n";
	// std::cout << "r - Report on multiple scrambles\n";
	// std::cout << "i - Index an RNG value\n";
	char choice;
	while (true)
	{
		std::cout << "\nChoice action: ";
		getline(std::cin, line);
		choice = line.empty() ? '0' : line[0];

		if (choice == 'q')
		{
			break;
		}
		else if (choice == 'a')
		{
			// TODO: Finish.
			std::cerr << "\nSorry, this hasn't been implemented yet.";
		}
		else if (choice == 'f')
		{
			while (true)
			{
				std::cout << "\nEnter scramble: ";
				getline(std::cin, line);
				
				bool succeeded;
				unsigned long seed;
				std::tie(succeeded, seed) = parseScrambleGetSeed(line);
				if (succeeded)
				{
					std::cout << "\nScramble seed: " << seed << std::endl;
					break;
				}
				else
				{
					std::cerr << "\nInvalid scramble; try again.";
				}
			}
		}
		else if (choice == 'p')
		{
			unsigned long rng = 0;
			int numPuzzles = 0;
			try
			{
				std::cout << "\nPuzzle seed? ";
				getline(std::cin, line);
				rng = std::stoul(line, nullptr, 0);

				std::cout << "\nHow many puzzles forward would you like to look? ";
				getline(std::cin, line);
				numPuzzles = std::stoi(line, nullptr, 0);
			}
			catch(std::invalid_argument)
			{
				std::cerr << "\nBad inputs given";
				continue;
			}

			drawBoards(rng, numPuzzles, prizeGoals, true);
		}
		else if (choice == 'r')
		{
			std::cout << "\nEnter input file name: ";
			getline(std::cin, line);
			std::ifstream scrambles_file(line);
			while (getline(scrambles_file, line))
			{
				std::cout << "\n" << line;

				bool succeeded;
				unsigned long seed;
				std::tie(succeeded, seed) = parseScrambleGetSeed(line);
				if (succeeded)
				{
					std::cout << "\nScramble seed: " << seed << std::endl;
				}
				else
				{
					std::cerr << "\nSkipping line: " << line << "\n";
				}
			}
		}
		else if (choice == 'i')
		{
			unsigned long rng = 0;
			unsigned long goal = 0;
			try
			{
				std::cout << "\nStart seed: ";
				getline(std::cin, line);
				rng = std::stoul(line, nullptr, 0);

				std::cout << "\nGoal: ";
				getline(std::cin, line);
				goal = std::stoul(line, nullptr, 0);
			}
			catch(std::invalid_argument)
			{
				std::cerr << "\nBad inputs given";
				continue;
			}

			printf(
				"\nStarting from %8lX, goal %8lX (%lu) reached after %lu steps.\n",
				rng, goal, goal, indexRNG(goal, rng)
			);
		}
	}
}
